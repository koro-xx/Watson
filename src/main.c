#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

//#define _DEBUG

/*

 Watson, a logic game.
 by Koro (1/2016)

 Todo:
 - Add better help for beginners
 - Mouse click: delay for drag is too long (what's the right way to handle drag/click?)
 - g->tile[i][j] should have been a bitfield, but it probably doesn't matter now...
 - add additional clue type
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <string.h>
#include "macros.h"
#include "game.h"
#include "sound.h"
#include "TiledBlock.h"
#include "board.h"
#include "allegro_stuff.h"
#include "game.h"
#include "bitmaps.h"
#include "dialog.h"
#include "text.h"
#include "settings.h"
#include "main.h"

#define FPS 30

// defaults:
Settings set = {
    6, // n
    6, // h
    0, // advanced
    0, // sound_mute
    0, // type_of_tiles
    0, // fat_fingers
    0  // restart
};

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
ALLEGRO_EVENT_SOURCE user_event_src;

ALLEGRO_EVENT_QUEUE *event_queue = NULL;

float RESIZE_DELAY = 0.04;
float BLINK_DELAY = .3; float BLINK_TIME = .5;
float CLICK_DELAY = 0.2;
int swap_mouse_buttons=0;

GAME_STATE game_state;
int desktop_xsize, desktop_ysize;
int fullscreen;

const char HELP_TEXT[]="Watson is a puzzle similar to the classic \"Zebra puzzle\" or \"Einstein's Riddle\". The goal is to figure out which item goes where on the board.\n"
    "The main panel has a number of columns, each dividided into blocks of same-type items. Each item in a block is secretly assigned to a different column, without repetition. Some of these assignments may be partially revealed at the beginning. Use the clues provided (right and bottom panels) to deduce which item goes where. \nTO GET STARTED: If you don't know how to play, click on the lightbulb to get a hint (or press C).\n"
    "\n"
    "Left-click on a clue tile to see an explanation of the clue. Right-click to hide the clue (in case you don't need it anymore). Click and drag the clues to rearrange.\n"
    "Right click on an item in the main panel to rule it out (right-click again to bring it back). Left-click on the same item to assign it to its column (and right-click on an assigned item to unassign).\n"
    "\n"
    "Press R to restart or ESC to quit. You can resize the window or press F to go fullscreen.\n"
    "Choose \"advanced\" in the settings for a more difficult game (really hard - not recommended).\n"
    "\n"
    "DEBUG KEYS: S: show solution. T: switch font tiles / bitmaps\n"
    "Note: advanced game generation may take a while for large boards.";

const char ABOUT_TEXT_[] = "Watson v" PRE_VERSION " - " PRE_DATE ", by Koro.\n"
    "\n"
    "Watson is an open source clone of \"Sherlock\", an old game by Evertt Kaser which is itself based on the a classic puzzle known as \"Zebra puzzle\" or \"Einstein's riddle\".\n"
    "\n"
    "Watson is programmed in plain C with the Allegro 5 library. Big thanks to the friendly folks from Freenode #allegro for all the tips and advice.\n"
    "\n"
    "The tile set is rendered from TTF fonts obtained from fontlibrary.org. Pressing the game will switch to custom tiles, which sould be stored in <appdir>/icons into 8 separate folders numbered 0 to 7, each with 8 square tiles in .png format. Of course, these won't look as nice as the fonts due to the anti-aliasing. The tiles provided here were downloaded from www.icons8.com.\n"
    "\n"
    "The \"advanced\" mode generates (much) harder puzzles, to the point of being almost impossible, so it needs to be tuned.";


struct Panel_State{
    int tile[8][8][8];
    struct Panel_State *parent;
};

struct Panel_State *undo = NULL;


const char *CLUE_TEXT[NUMBER_OF_RELATIONS] = {
    [NEXT_TO] = "The two items are in adjacent columns, on either side.",
    [NOT_NEXT_TO] = "The two items are not in adjacent columns",
    [ONE_SIDE] = "The first item's column is to the left of the second item's.",
    [CONSECUTIVE] = "The middle item's column is between the columns of the other two, in any order",
    [NOT_MIDDLE] = "The first and last items have one column between them, and the middle item is not in that column.",
    [TOGETHER_2] = "The two items are on the same column.",
    [TOGETHER_3] = "The three items are on the same column.",
    [NOT_TOGETHER] = "The two items are not on the same column.",
    [TOGETHER_NOT_MIDDLE] = "The first and last item are on the same column, and the middle item is not there.",
    [TOGETHER_FIRST_WITH_ONLY_ONE] = "Add description later.",
    [REVEAL] = "This message should not be showing up."
};

// Prototypes
void draw_stuff(Board *b);
void handle_mouse_click(Game *g, Board *b, int mx, int my, int mclick);
void update_board(Game *g, Board *b);
void mouse_grab(Board *b, int mx, int my);
void mouse_drop(Board *b, int mx, int my);
TiledBlock* settings_block(void);
void update_settings_block(Game *g, Board *b);
TiledBlock *get_TiledBlock_at(Board *b, int x, int y);
void show_hint(Game *g, Board *b);
void update_guessed(Game *g);
void execute_undo(Game *g);
void save_state(Game *g);
void destroy_undo();

// debug: show solution
void switch_solve_puzzle(Game *g, Board *b){
    SWAP(g->guess, g->puzzle);
    update_board(g, b);
}


void emit_event(int event_type){
    static ALLEGRO_EVENT user_event = {0};
    
    user_event.type = event_type;
    al_emit_user_event(&user_event_src, &user_event, NULL);
}

void show_help(Board *b){
    al_pause_event_queue(event_queue, 1);
    draw_center_text_box(b->text_font, WHITE_COLOR, WINDOW_BG_COLOR, WINDOW_BD_COLOR, 0.5, HELP_TEXT);
    al_flip_display();
    wait_for_input();
    al_pause_event_queue(event_queue, 0);
    b->show_help=0;
}

void show_about(Board *b){
    al_pause_event_queue(event_queue, 1);
    draw_center_text_box(b->text_font, WHITE_COLOR, WINDOW_BG_COLOR, WINDOW_BD_COLOR, 0.5, ABOUT_TEXT_);
    al_flip_display();
    wait_for_input();
    al_pause_event_queue(event_queue, 0);
    b->show_about=0;
}

void draw_stuff(Board *b){
    static float time = -1;
    float t;
    int x, y;
    
    al_clear_to_color(b->bg_color);
    if(b->show_settings){
        if(b->show_about){
            show_about(b);
            return;
        }
        draw_TiledBlock(&b->s, 0,0);
		al_flip_display();
        return;
    }

    draw_TiledBlock(&b->all,0,0);
    
    if(b->rule_out){
        t=al_get_time();
        if (t-time >= BLINK_TIME + BLINK_DELAY){
            time = t;
        }
        if (t-time < BLINK_TIME){
            if(b->rule_out) highlight_TiledBlock(b->rule_out);
            if(b->highlight) highlight_TiledBlock(b->highlight);
        }
    } else {
        if(b->highlight) highlight_TiledBlock(b->highlight);
    }
    
    
    if(b->show_help){
        show_help(b);
        return;
    } else if(b->dragging){ // redraw tile to get it on top
        get_TiledBlock_offset(b->dragging->parent, &x, &y);
        draw_TiledBlock(b->dragging, x,y); //b->dragging->parent->x, b->dragging->parent->y);
    }

	al_flip_display();
};

void animate_win(Board *b) {
	int i, j, k=0;

    al_pause_event_queue(event_queue,1);
	for (j = 0; j < b->h; j++) {
		for (i = 0; i < b->n; i++) {
			k++;
			convert_grayscale(b->guess_bmp[i][j]);
			draw_stuff(b);
			if (!set.sound_mute) { play_sound(SOUND_STONE); }
			al_rest(0.5*(1 - sqrt((float) k / (b->h*b->n))));
		}
	}
    al_pause_event_queue(event_queue,0);
}

void draw_generating_puzzle(Game *g, Board *b) {
    int h = al_get_bitmap_height(al_get_target_bitmap());
    int w = al_get_bitmap_width(al_get_target_bitmap());
//    al_clear_to_color(b->bg_color);

    al_draw_filled_rectangle((w-450)/2, (h-100)/2, (w+450)/2, (h+100)/2, WINDOW_BG_COLOR);
    al_draw_rectangle((w-450)/2, (h-100)/2, (w+450)/2, (h+100)/2, WINDOW_BD_COLOR,3);
    al_draw_textf(default_font, WHITE_COLOR, w/2, h/2 - 8, ALLEGRO_ALIGN_CENTER, "Generating %d x %d%s puzzle, please wait...", g->n, g->h, g->advanced ? " advanced" : "");
    al_flip_display();
}

int switch_tiles(Game *g, Board *b, ALLEGRO_DISPLAY *display){
    // cycle through tyle types (font, bitmap, classic)
    al_set_target_backbuffer(display);
    destroy_all_bitmaps(b);
    b->type_of_tiles = (b->type_of_tiles + 1) %3;
    if(init_bitmaps(b)){ // cycle through all three options
        b->type_of_tiles = (b->type_of_tiles + 1) %3;
        if(init_bitmaps(b))
            b->type_of_tiles = (b->type_of_tiles + 1) %3;
        if(init_bitmaps(b))
            return -1; // error
    }
    b->max_xsize = al_get_display_width(display);
    b->max_ysize = al_get_display_height(display);
    create_board(g, b, 0);
    al_set_target_backbuffer(display);
    update_board(g, b);
    al_convert_bitmaps();
    return 0;
}

void win_or_lose(Game *g, Board *b){
    if(check_solution(g)){
        game_state = GAME_OVER;
        show_info_text(b, "Elementary, watson!");
        animate_win(b);
        if (!set.sound_mute) play_sound(SOUND_WIN);
    } else {
        show_info_text(b, "Something is wrong. Try again, or press R to start a new puzzle.");
        if (!set.sound_mute) play_sound(SOUND_WRONG);
    }
}

void destroy_everything(Board *b){
    destroy_board(b);
    destroy_sound();
    destroy_undo();
}

int toggle_fullscreen(Game *g, Board *b, ALLEGRO_DISPLAY **display){
    ALLEGRO_DISPLAY *newdisp;
    float display_factor;

    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
    
    if(!fullscreen){
        al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
        display_factor = 1;
    } else {
        al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
        display_factor = 0.9;
    }
  

    newdisp = al_create_display(desktop_xsize*display_factor, desktop_ysize*display_factor);
    if(!newdisp){
        fprintf(stderr, "Error switching fullscreen mode.\n");
        return 0;
    }
    
    SWITCH(fullscreen);
    destroy_board(b);

    al_destroy_display(*display);

    *display = newdisp;
    al_set_target_backbuffer(*display);
    b->max_xsize = desktop_xsize*display_factor;
    b->max_ysize = desktop_ysize*display_factor;
 
    create_board(g, b, fullscreen ? 2 : 1);
    al_set_target_backbuffer(*display);

    if(!fullscreen){
        al_resize_display(*display, b->xsize, b->ysize);
        al_set_window_position(*display, (desktop_xsize-b->xsize)/2, (desktop_ysize-b->ysize)/2);
        al_acknowledge_resize(*display);
        al_set_target_backbuffer(*display);
    }
    
    update_board(g, b);
    al_convert_bitmaps();
    return 1;
}

int main(int argc, char **argv){
    Game g = {0};
    Board b = {0};
    ALLEGRO_EVENT ev;
    ALLEGRO_TIMER *timer = NULL, *timer_second = NULL;
    ALLEGRO_DISPLAY *display = NULL;
    double time_foo, dt, last_draw, resize_time, mouse_button_time;
    int noexit, mouse_click,redraw, mouse_move,keypress, second_tick, resizing, mouse_drag, resize_update, mouse_button_down, mouse_button_up, request_exit;
    int mouse_x, mouse_y, mouse_cx, mouse_cy;
	float max_display_factor;
    TiledBlock *tb = NULL;
    char str[500];
    
    // seed random number generator. comment out for debug
#ifndef _DEBUG
        srand((unsigned int) time(NULL));
#endif
    
    if (init_allegro()) return -1;

#ifndef _WIN32
     // use anti-aliasing if available (seems to cause problems in windows)
     al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
     al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
#endif
    
    fullscreen = 0;
    
    // use vsync if available
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);

	if (fullscreen) {
        al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
        display = al_create_display(desktop_xsize, desktop_ysize);
	} else {
		al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
        display = al_create_display(800,600);
	}
    
    if(!display) {
        fprintf(stderr, "Failed to create display!\n");
        return -1;
    }
    
    if(init_fonts()) return -1;
    
	al_set_target_backbuffer(display);
    al_set_window_title(display, "Watson");
    al_clear_to_color(NULL_COLOR);
	
    al_init_user_event_source(&user_event_src);

    draw_title();
    al_flip_display();
    wait_for_input();
    
RESTART:
    b.type_of_tiles = set.type_of_tiles; // use font tiles by default
    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
  
	if (!fullscreen) {
        max_display_factor = 0.9;
    } else
        max_display_factor = 1;

    if(set.restart){
        al_set_target_backbuffer(display);
        destroy_board(&b);
        destroy_undo();
        set.restart=0;
        al_set_target_backbuffer(display);
    }
    
    b.max_xsize = desktop_xsize*max_display_factor;
    b.max_ysize = desktop_ysize*max_display_factor; // change this later to something adequate

    g.n = b.n = set.n;
    g.h = b.h = set.h;
    g.advanced = set.advanced; // use "what if" depth 1?
    b.type_of_tiles = set.type_of_tiles;
    
    draw_generating_puzzle(&g, &b);
    create_game_with_clues(&g);

    if(create_board(&g, &b, 1)){
        fprintf(stderr, "Failed to create game board.\n");
        return -1;
    }
    
    if (!fullscreen) {
        al_set_target_backbuffer(display);
        al_resize_display(display, b.xsize, b.ysize);
        al_set_window_position(display, (desktop_xsize-b.xsize)/2, (desktop_ysize-b.ysize)/2);
        al_acknowledge_resize(display);
        al_set_target_backbuffer(display);
    }
    	
	al_convert_bitmaps(); // turn bitmaps to memory bitmaps after resize (bug in allegro doesn't autoconvert)

    
    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        return -1;
    }

	if (!(timer = al_create_timer(1.0 / FPS))) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	if (!(timer_second = al_create_timer(1.0))) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_timer_event_source(timer_second));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue , &user_event_src);

    update_board(&g,&b);
    al_set_target_backbuffer(display);
    al_start_timer(timer);
    al_start_timer(timer_second);

//  initialize flags
    al_set_timer_count(timer, 0);
    al_set_timer_count(timer_second, 0);

    mouse_button_up=0;
    redraw=1; mouse_click=0;
    noexit=1; mouse_move=0;
    set.restart=0; keypress=0;
    last_draw=0; resizing=0;
    mouse_drag = 0; mouse_button_down = 0;
    resize_update=0; resize_time = 0;
    second_tick=1; mouse_button_time=0;
    request_exit = 0;
    mouse_cx = mouse_cy = 0;
    mouse_x = mouse_y = 0;
    game_state = GAME_PLAYING;
    b.time_start=al_get_time();
    b.show_help = 0;
    b.show_about = 0;
    b.show_settings = 0;

    show_info_text_b(&b, "Click on clue for info. Click %b for help, %b for settings, or %b for a hint at any time. Press R to start a new game.", b.button_bmp[0], b.button_bmp[2], b.button_bmp[1]);

    while(noexit)
    {
        al_wait_for_event(event_queue, &ev);
 //       do{ // empty out the event queue
           switch(ev.type){
               case EVENT_REDRAW:
                   redraw=1;
                   break;
                case ALLEGRO_EVENT_TIMER:
                    if (ev.timer.source==timer_second) second_tick=1;
                    else if (b.rule_out) redraw=1;
                    break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    mouse_button_down = ev.mouse.button;
                    mouse_button_time = al_get_time();
                    mouse_cx = ev.mouse.x;
                    mouse_cy = ev.mouse.y;
                    tb = get_TiledBlock_at(&b, mouse_cx, mouse_cy);
                    break;
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                    if(mouse_drag==2){
                        mouse_drag=3;
                    } else { //xxx todo: check tile on button_down, compare on button_up, click if same
                        if(tb == get_TiledBlock_at(&b, mouse_x, mouse_y))
                            mouse_click = mouse_button_down;
                    }
                    mouse_button_down=0;
                    break;
                case ALLEGRO_EVENT_MOUSE_AXES:
                    mouse_x = ev.mouse.x;
                    mouse_y = ev.mouse.y;
                    mouse_move=1;
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    request_exit=1;
                    break;
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    switch(ev.keyboard.keycode){
                        case ALLEGRO_KEY_ESCAPE:
                            if(b.show_settings){
                                b.show_settings=0;
                                redraw=1;
                            }
                            else
                                request_exit=1;
                            break;
                        case ALLEGRO_KEY_R:
                            set.restart=1;
                            break;
                        case ALLEGRO_KEY_S: // debug: show solution
                            switch_solve_puzzle(&g, &b);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_T:
                            if(switch_tiles(&g, &b, display)){
                                fprintf(stderr, "Error switching tiles.\n");
                                return -1;
                            }
                            al_flush_event_queue(event_queue);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_SPACE:
                            mouse_click=2;
                            mouse_cx = mouse_x;
                            mouse_cy = mouse_y;
                            break;
                        case ALLEGRO_KEY_H:
                            SWITCH(b.show_help);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_C:
                            show_hint(&g, &b);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_F:
                            if(toggle_fullscreen(&g, &b, &display)){
                                al_register_event_source(event_queue, al_get_display_event_source(display));
                            }
                            al_flush_event_queue(event_queue);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_U:
                            execute_undo(&g);
                            update_board(&g, &b);
                            al_flush_event_queue(event_queue);
                            redraw=1;
                            break;
                    }
                    break;
                case ALLEGRO_EVENT_KEY_DOWN:
                case ALLEGRO_EVENT_KEY_UP:
                case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
                case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
                    break;
                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    if (fullscreen) break;
                    al_acknowledge_resize(display);
                    resizing=1; resize_time=al_get_time();
                    break;
            }
//        } while(al_get_next_event(event_queue, &ev));

        if(resizing){
            if(al_get_time()-resize_time > RESIZE_DELAY){
                resizing =0; resize_update=1;
            }
        }
    
        if(resize_update){
            resize_update=0;
			al_acknowledge_resize(display);
            destroy_board_bitmaps(&b);
			b.max_xsize = al_get_display_width(display);
			b.max_ysize = al_get_display_height(display);
            create_board(&g, &b, 0);
            al_set_target_backbuffer(display);
            update_board(&g, &b);
			al_convert_bitmaps(); // turn bitmaps to video bitmaps
            redraw=1;
        }
        
        if(resizing) // skip redraw and other stuff
            continue;
        
        if(mouse_button_down){
            if(!mouse_drag){
                if ((al_get_time() - mouse_button_time > CLICK_DELAY) && (tb) && ((tb->type == TB_HCLUE_TILE) || (tb->type == TB_VCLUE_TILE))){
                    if(mouse_button_down == 1){ //xxx todo: change to mouse_drag = mouse_button_down
                                                // change how dragging is tracked (mouse_drag_state)
                        mouse_drag=1;
                    }
                    mouse_button_down=0;
                }
            }
        }
        
        if(mouse_click){
            handle_mouse_click(&g, &b, mouse_cx, mouse_cy, mouse_click);
            mouse_click=0;
            redraw=1;
            
            if((game_state == GAME_PLAYING) && (g.guessed == g.h*g.n)){
                win_or_lose(&g, &b); // check if player has won
                al_flush_event_queue(event_queue);
                redraw=1;
            }
        }
        
        if(game_state == GAME_PLAYING){
            if(!b.show_help && !b.show_settings){
                if(mouse_move && (mouse_drag == 2)){
                    mouse_move=0;
                    b.dragging->x = mouse_x + b.dragging_cx;
                    b.dragging->y = mouse_y + b.dragging_cy;
                    redraw=1;
                }
                
                if(mouse_drag == 1){ // 1 for drag, 3 for drop, 2 for dragging
                    mouse_grab(&b, mouse_x, mouse_y);
                    if(b.dragging) mouse_drag=2; else mouse_drag = 0;
                    redraw=1;
                } else if (mouse_drag == 3){
                    mouse_drop(&b, mouse_x, mouse_y);
                    mouse_drag=0;
                    redraw=1;
                }
            }
        }
        
        if(keypress){
            keypress=0;
            if(b.show_help){
                b.show_help = 1; // b.show_help>1 ? 0 : 2;
                redraw=1;
            }
        }
        
        if(second_tick && (game_state == GAME_PLAYING)){ 
// there is a bug in allegro with display bitmaps after window resize
// it happens on windows wiht D3D. This is why we use OPENGL.
            update_timer(&b);
			second_tick = 0;
            redraw=1;
        }
            
        if(redraw) {
            redraw = 0;
            time_foo = al_get_time();
            dt = time_foo-last_draw;
            al_set_target_backbuffer(display);
            draw_stuff(&b);
            last_draw=time_foo;
        }
        
        if(request_exit){
            request_exit = 0;
            al_pause_event_queue(event_queue,1);
            if(yes_no_dialog("Exit game?"))
                noexit=0;
            al_pause_event_queue(event_queue,0);
            redraw=1;
        }
        
        if(b.restart){
            if(game_state == GAME_PLAYING){
                al_pause_event_queue(event_queue,0);
                snprintf(str, 500, "Start new %dx%d%s game?", set.n, set.h, set.advanced ? " advanced" : "");
                if(!yes_no_dialog(str)){
                    b.restart = 0;
                }
                al_pause_event_queue(event_queue,1);
                redraw=1;
            }
            
            if(b.restart){
                draw_stuff(&b);
                goto RESTART;
            }
        }
    } // end of game loop
    
    destroy_everything(&b);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
    al_destroy_timer(timer_second);
    return(0);
}

void update_guessed(Game *g){
    int i,j,k, count, val;
    
    g->guessed=0;
    
    for(i=0; i<g->n; i++){
        for(j=0; j<g->h; j++){
            count=0;
            val = -1;
            for(k=0; k<g->n; k++){
                if(g->tile[i][j][k]){
                    count++;
                    val = k;
                    if(count>1) break;
                }
            }
            if(count == 1){
                g->guess[i][j] = val;
                g->guessed++;
            } else
                g->guess[i][j] = -1;
        }
    }
}

void update_board(Game *g, Board *b){
    int i,j,k;
    
    for(i=0; i<g->n; i++){
        for(j=0; j<g->h; j++){
            if(g->guess[i][j]>=0){
                b->panel.b[i]->b[j]->sb = 0;
                b->panel.b[i]->b[j]->bmp = &(b->guess_bmp[j][g->guess[i][j]]);
            } else {
                b->panel.b[i]->b[j]->sb = b->n;
                b->panel.b[i]->b[j]->bmp = NULL;
                for(k=0; k<g->n; k++){
                    if(g->tile[i][j][k]){
                        b->panel.b[i]->b[j]->b[k]->hidden = 0;
                    } else {
                        b->panel.b[i]->b[j]->b[k]->hidden = 1;
                    }
                }
            }
        }
    }
}

void swap_clues(Board *b, TiledBlock *c1, TiledBlock *c2){
    SWAP(c1->bmp, c2->bmp);
    
    if(c1->index>=0)
        b->clue_tiledblock[c1->index] = c2;
    if(c2->index>=0)
        b->clue_tiledblock[c2->index] = c1;
    SWAP(c1->index, c2->index);
    SWAP(c1->hidden, c2->hidden);
};

TiledBlock *get_TiledBlock_at(Board *b, int x, int y){
    if(b->show_settings){
        return get_TiledBlock(&b->s, x, y);
    } else if(b->show_help)
        return &b->all;
    else
        return get_TiledBlock(&b->all, x, y);
}

void mouse_grab(Board *b, int mx, int my){
    b->dragging = get_TiledBlock_at(b, mx, my);
    if(b->dragging && b->dragging->bmp){
        if( (b->dragging->type == TB_HCLUE_TILE) || (b->dragging->type == TB_VCLUE_TILE) ){
            b->dragging_ox = b->dragging->x;
            b->dragging_oy = b->dragging->y;
            b->dragging_cx = b->dragging->x  - mx+5;
            b->dragging_cy = b->dragging->y - my+5;
            b->dragging->x = mx + b->dragging_cx;
            b->dragging->y = my + b->dragging_cy;
            if(!set.sound_mute) play_sound(SOUND_UNHIDE_TILE);
            return;
        }
    }
    
    b->dragging = NULL;
    return;
};



void mouse_drop(Board *b, int mx, int my){
    TiledBlock *t;
    
    if(!b->dragging) return;
    b->dragging->x = b->dragging_ox;
    b->dragging->y = b->dragging_oy;
 
    t = get_TiledBlock_at(b, mx, my);
    if(t->type == b->dragging->type){
        swap_clues(b, b->dragging, t);
        if(b->highlight == b->dragging) b->highlight = t;
        else if(b->highlight == t) b->highlight = b->dragging;
        if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    }
    b->dragging = NULL;
    return;
};

void check_settings(Game *g, Board *b){
    int i, n = b->n, h= b->h;

    for(i=0; i<5; i++){
        if(b->s.b[0]->b[i]->hidden==0){h=i+4;}
        if(b->s.b[1]->b[i]->hidden==0){n=i+4;}
    }
    if((b->n != n) || (b->h != h)){
        set.n=n;
        set.h=h;
        b->restart=1;
    }
    
    if(b->s.b[2]->b[0]->hidden==0)
        set.sound_mute=set.sound_mute = 0;
    else
        set.sound_mute=set.sound_mute = 1;
    
    if(b->s.b[6]->b[0]->hidden == g->advanced){ // has changed
        set.advanced = b->s.b[6]->b[0]->hidden ? 0 : 1;
        b->restart = 1;
    }
}



void destroy_undo(){
    struct Panel_State *foo;
    
    while(undo){
        foo = undo->parent;
        free(undo);
        undo = foo;
    }
}

void execute_undo(Game *g){
    struct Panel_State *undo_old;
    
    if(!undo) return;
    memcpy(&g->tile, &undo->tile, sizeof(g->tile));
    undo_old = undo->parent;
    free(undo);
    undo = undo_old;
    if(!set.sound_mute)play_sound(SOUND_UNHIDE_TILE);
    update_guessed(g);
}

void save_state(Game *g){
    struct Panel_State *foo;
    
    foo = malloc(sizeof(*foo));
    foo->parent = undo;
    undo = foo;
    memcpy(&undo->tile, &g->tile, sizeof(undo->tile));
}

void explain_clue(Board *b, Clue *clue)
{
    ALLEGRO_BITMAP *b0, *b1, *b2;
    b0 = b->clue_unit_bmp[clue->j[0]][clue->k[0]];
    b1 = b->clue_unit_bmp[clue->j[1]][clue->k[1]];
    b2 = b->clue_unit_bmp[clue->j[2]][clue->k[2]];
    
    switch(clue->rel){
        case CONSECUTIVE:
            show_info_text_b(b, "The column of %b is between %b and %b, but they could be on either side.", b1, b0, b2);
            break;
        case NEXT_TO:
            show_info_text_b(b, "The columns of %b and %b are next to each other, but they could be on either side.", b0, b1);
            break;
        case NOT_NEXT_TO:
            show_info_text_b(b, "The column of %b is NOT next to the column of %b.", b0, b1);
            break;
        case NOT_MIDDLE:
            show_info_text_b(b, "There is exactly one column between %b and %b, and %b is NOT in that column.", b0, b2, b1);
            break;
        case ONE_SIDE:
            show_info_text_b(b, "The column of %b is strictly to the left of %b.", b0, b1);
            break;
        case TOGETHER_2:
            show_info_text_b(b, "%b and %b are on the same column.", b0, b1);
            break;
        case TOGETHER_3:
            show_info_text_b(b, "%b, %b and %b are on the same column.", b0, b1, b2);
            break;
        case NOT_TOGETHER:
            show_info_text_b(b, "%b and %b are NOT on the same column.", b0, b1);
            break;
        case TOGETHER_NOT_MIDDLE:
            show_info_text_b(b, "%b and %b are on the same column, and %b is NOT in that column.", b0, b2, b1);
            break;
        default:
            break;
    }
}

void show_hint(Game *g, Board *b){
    ALLEGRO_BITMAP *b0, *b1, *b2, *b3;
    char hint[1000] = "";
    int i;
    
    if(!check_panel_correctness(g)){
        show_info_text(b, "Something is wrong. An item was ruled out incorrectly.");
        return;
    }
    
    i=get_hint(g);
    if(!i){
        show_info_text(b, "No hint available.");
        return;
    }
    
    b->highlight = b->clue_tiledblock[i & 255];
    b->rule_out = b->panel.b[(i>>15) & 7]->b[(i>>12) & 7]->b[(i>>9) & 7];
    strcat(hint, CLUE_TEXT[g->clue[i & 255].rel]);
    strcat(hint, " So we can rule out the blinking tile.");
    show_info_text(b, hint);
    
    b0 = b->clue_unit_bmp[g->clue[i & 255].j[0]][g->clue[i & 255].k[0]];
    b1 = b->clue_unit_bmp[g->clue[i & 255].j[1]][g->clue[i & 255].k[1]];
    b2 = b->clue_unit_bmp[g->clue[i & 255].j[2]][g->clue[i & 255].k[2]];
    b3 = b->clue_unit_bmp[(i>>12) & 7][(i>>9) & 7];
    
    switch(g->clue[i & 255].rel){
        case CONSECUTIVE:
            show_info_text_b(b, "The column of %b is between %b and %b, so we can rule out %b from here.", b1, b0, b2, b3);
            break;
        case NEXT_TO:
            show_info_text_b(b, "The columns of %b and %b are next to each other, so we can rule out %b from here.", b0, b1, b3);
            break;
        case NOT_NEXT_TO:
            show_info_text_b(b, "The column of %b is NOT next to the column of %b, so we can rule out %b from here.", b0, b1, b3);
            break;
        case NOT_MIDDLE:
            show_info_text_b(b, "There is exactly one column between %b and %b, and %b is NOT in that column, so we can rule out %b from here.", b0, b2, b1, b3);
            break;
        case ONE_SIDE:
            show_info_text_b(b, "The column of %b is strictly to the left of %b, so we can rule out %b from here.", b0, b1, b3);
            break;
        case TOGETHER_2:
            show_info_text_b(b, "%b and %b are on the same column, so we can rule out %b from here.", b0, b1,b3);
            break;
        case TOGETHER_3:
            show_info_text_b(b, "%b, %b and %b are on the same column, so we can rule out %b from here.", b0, b1, b2, b3);
            break;
        case NOT_TOGETHER:
            show_info_text_b(b, "%b and %b are NOT on the same column, so we can rule out %b from here.", b0, b1, b3);
            break;
        case TOGETHER_NOT_MIDDLE:
            show_info_text_b(b, "%b and %b are on the same column, and %b is NOT in that column, so we can rule out %b from here.", b0, b2, b1, b3);
            break;
        default:
            break;
    }
}

void handle_mouse_click(Game *g, Board *b, int mx, int my, int mclick){
    int i,j,k;
    TiledBlock *t;
    
    if(swap_mouse_buttons){
        if(mclick==1) mclick=2;
        else if(mclick==2) mclick=1;
    }
    
    clear_info_panel(b); // remove text if there was any
    
    if(b->highlight){
        b->highlight=NULL;
    }
    if(b->rule_out){
        b->rule_out=NULL;
    }
    
    t = get_TiledBlock_at(b, mx, my);
    if (!t) return;
    
    if(b->show_settings){ // handle settings panel
	   if(t->type == TB_SETTINGS_OK){
            b->show_settings=0;
            check_settings(g, b);
        } else if(t->type == TB_SETTINGS_CANCEL){
            b->show_settings=0;
        } else if(t->type == TB_SETTINGS_ABOUT){
            b->show_about =1;
        } else if(t->parent) {
            switch(t->parent->type){
                case TB_SETTINGS_ROWS:
                    for(i=0; i<5; i++){
                        b->s.b[0]->b[i]->hidden=1;
                    }
                    b->s.b[0]->b[t->index-4]->hidden=0;
                    break;
                case TB_SETTINGS_COLUMNS:
                    for(i=0; i<5; i++){
                        b->s.b[1]->b[i]->hidden=1;
                    }
                    b->s.b[1]->b[t->index-4]->hidden=0;
                    break;
                case TB_SETTINGS_SOUND:
                    SWITCH(b->s.b[2]->b[0]->hidden);
                    break;
                case TB_SETTINGS_ADVANCED:
                    SWITCH(b->s.b[6]->b[0]->hidden);
                    break;
            }
        }
        return;
    }
    
    if(b->show_help){
        b->show_help=0;
        return;
    }
    
    if(game_state == GAME_OVER){
        show_info_text(b, "Press R to start a new puzzle.");
    }
    
    switch(t->type){ // which board component was clicked
        case TB_PANEL_TILE:
            if(game_state != GAME_PLAYING) break;
            k = t->index; j=t->parent->index; i = t->parent->parent->index;
            if(mclick==2){
                save_state(g);
                if(g->tile[i][j][k]){ // hide tile
                    hide_tile_and_check(g, i, j, k);
                    if (!set.sound_mute) play_sound(SOUND_HIDE_TILE);
                } else { // tile was hidden, unhide
                    if(!is_guessed(g, j, k)){
                        g->tile[i][j][k]=1;
                        if (!set.sound_mute) play_sound(SOUND_UNHIDE_TILE);
                    }
                }
            } else if (mclick==1){
                if(g->tile[i][j][k]){
                    save_state(g);
                    guess_tile(g, i, j, k);
                    if (!set.sound_mute) play_sound(SOUND_GUESS_TILE);
                }
            }
            update_board(g, b);
            break;

        case TB_PANEL_BLOCK:
            if(game_state != GAME_PLAYING) break;
            if ((mclick==2) && (g->guess[t->parent->index][t->index]>=0)){
                // we found guessed block - unguess it
                save_state(g);
                unguess_tile(g, t->parent->index, t->index);
                if (!set.sound_mute) play_sound(SOUND_UNHIDE_TILE);
            }
            update_board(g, b);
            break;

        case TB_HCLUE_TILE:
        case TB_VCLUE_TILE:
            if(game_state != GAME_PLAYING) break;
            // check that this is a real clue
            if(t->bmp && (t->index >= 0)){
                if(mclick==2){ // toggle hide-show clue
                    SWITCH(t->hidden);
                    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
                } else if (mclick==1){ // explain clue in info panel
                    explain_clue(b, &g->clue[t->index]);
                    b->highlight = t; // highlight clue
                }
            }
            break;
            
        case TB_BUTTON_CLUE: // time panel
            if(game_state != GAME_PLAYING) break;
            show_hint(g,b);
            break;
        case TB_BUTTON_SETTINGS:
            update_settings_block(g, b);
            show_settings(&set, b, event_queue);
            b->show_settings=1;
            break;
    
        case TB_BUTTON_HELP:
            b->show_help=1;
            break;

        case TB_BUTTON_UNDO:
            execute_undo(g);
            update_board(g,b);
            break;
            
        case TB_BUTTON_TILES:
            switch_tiles(g,b,al_get_current_display());
            update_board(g,b);
            break;
      
        default:
            break;
    }
}


void update_settings_block(Game *g, Board *b){
    int i;
    
    for(i=0; i<5; i++){
        b->s.b[0]->b[i]->hidden=1;
        b->s.b[1]->b[i]->hidden=1;
    }
    b->s.b[0]->b[b->h-4]->hidden = 0;
    b->s.b[1]->b[b->n-4]->hidden = 0;
    if(set.sound_mute)
        b->s.b[2]->b[0]->hidden = 1;
    else
        b->s.b[2]->b[0]->hidden = 0;
    
    if(g->advanced)
        b->s.b[6]->b[0]->hidden = 0;
    else
        b->s.b[6]->b[0]->hidden = 1;
}
