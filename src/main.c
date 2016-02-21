#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

// INTEGRATION WITH ANDROID
//#define _DEBUG

/*

 Watson, a logic game.
 by Koro (1/2016)

 Todo
 - add highscore table / input
 - finish tutorial
 - fix time update: should use "game time" to account for saved games (fixed?)
 - change wait_for_input to return key pressed or event type
 - add additional clue type
 
 
 NOTE: in android, to draw text to a bitmap without issues we need the bitmap to be created detached from the screen.
 so we set a null target bitmap before creating them and then we use al_convert_bitmaps
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
#include "gui.h"
#include "main.h"

#define FPS 60.0

// defaults:
Settings set = {
    6, // n
    6, // h
    0, // advanced
    0, // sound_mute
    0, // type_of_tiles
    0, // fat_fingers
    0,  // restart
    0 // saved
};

// this is mainly for testing, not actually used. use emit_event(EVENT_TYPE) to emit user events.
#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
ALLEGRO_EVENT_SOURCE user_event_src;

ALLEGRO_EVENT_QUEUE *event_queue = NULL;

float fixed_dt = 1.0/FPS;
float RESIZE_DELAY = 0.04;
float BLINK_DELAY = .3; float BLINK_TIME = .5;
int swap_mouse_buttons=0;

GAME_STATE game_state;
int desktop_xsize, desktop_ysize;
int fullscreen;

struct Panel_State{
    int tile[8][8][8];
    struct Panel_State *parent;
};

struct Panel_State *undo = NULL;

// Prototypes
void draw_stuff(Board *b);
void handle_mouse_click(Game *g, Board *b, TiledBlock *t, int x, int y, int mclick);
void update_board(Game *g, Board *b);
void mouse_grab(Board *b, int mx, int my);
void mouse_drop(Board *b, int mx, int my);
TiledBlock* settings_block(void);
TiledBlock *get_TiledBlock_at(Board *b, int x, int y);
void show_hint(Game *g, Board *b);
void update_guessed(Game *g);
void execute_undo(Game *g);
void save_state(Game *g);
void destroy_undo();
void switch_solve_puzzle(Game *g, Board *b);
int save_game_f(Game *g, Board *b);
int load_game_f(Game *g, Board *b);
void  explain_clue(Board *b, Clue *clue);



//void blink_TB(TiledBlock *t){
//    ALLEGRO_BITMAP *screen = screenshot();
//    int x,y;
//    
//    get_TiledBlock_offset(t, &x, &y);
//    al_draw_filled_rectangle(x,y, x+t->w, y+t->h, al_premul_rgba(255,255,255,150));
//    al_flip_display();
//    al_rest(0.1);
//    al_draw_bitmap(screen,0,0,0);
//    al_flip_display();
//    al_rest(0.2);
//    al_draw_bitmap(screen,0,0,0);
//    al_draw_filled_rectangle(x,y,x+ t->w, y+t->h, al_premul_rgba(255,255,255,150));
//    al_flip_display();
//    al_rest(0.1);
//    al_draw_bitmap(screen,0,0,0);
//    al_flip_display();
//    al_destroy_bitmap(screen);
//    
//}


//ALLEGRO_TRANSFORM T;
//al_identity_transform(&T);
//al_scale_transform(&T, 5, 5);
//al_use_transform(&T);
//al_draw_filled_rectangle(0,0,80, 20, BLACK_COLOR);
//al_draw_textf(default_font, WHITE_COLOR, 0,0, ALLEGRO_ALIGN_LEFT, "%d, %d | %d, %d", nset.n, nset.h, set->n, set->h);
//al_identity_transform(&T);
//al_use_transform(&T);

void halt(ALLEGRO_EVENT_QUEUE *queue){
    ALLEGRO_DISPLAY *disp =al_get_current_display();
    al_acknowledge_drawing_halt(disp);
    deblog("ACKNOWLEDGED HALT");
    ALLEGRO_EVENT ev;
    
    do{
        al_wait_for_event(queue, &ev);
    }while(ev.type != ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING);
 
    al_acknowledge_drawing_resume(disp);
    deblog("ACKNOWLEDGED RESUME");
    al_rest(0.01);
    al_flush_event_queue(queue);
}

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


void draw_stuff(Board *b){
    int x, y;
    int i;
    
    al_clear_to_color(BLACK_COLOR); // (b->bg_color);
    draw_TiledBlock(&b->all,0,0);
    
    if(b->rule_out){
        if(b->blink){
            highlight_TiledBlock(b->rule_out);
            highlight_TiledBlock(b->highlight);
        }
    } else {
        if(b->highlight) highlight_TiledBlock(b->highlight);
    }
    
    if(b->dragging){ // redraw tile to get it on top
        get_TiledBlock_offset(b->dragging->parent, &x, &y);
        draw_TiledBlock(b->dragging, x,y); //b->dragging->parent->x, b->dragging->parent->y);
    }
    
    if(b->zoom){
        al_draw_filled_rectangle(0,0,b->max_xsize, b->max_ysize, al_premul_rgba(0,0,0,150));
        al_use_transform(&b->zoom_transform);
        // draw dark background in case of transparent elements
        al_draw_filled_rectangle(b->zoom->x,b->zoom->y, b->zoom->x + b->zoom->w, b->zoom->y + b->zoom->h, b->zoom->parent->bg_color);
        draw_TiledBlock(b->zoom,0,0);
        al_use_transform(&b->identity_transform);
    }

    for(i=0; i<b->gui_n; i++){
        wz_draw(b->gui[i]);
    }
    
};

void animate_win(Board *b) {
	int k, ii, jj, kk=0;
    int x,y;
    ALLEGRO_BITMAP *currbuf = al_get_target_bitmap();
    ALLEGRO_BITMAP *bmp = al_clone_bitmap(currbuf);
    ALLEGRO_EVENT ev;
    int a[64];
    
    al_set_target_bitmap(bmp);
    al_clear_to_color(BLACK_COLOR);
    draw_stuff(b);
    al_set_target_bitmap(currbuf);
    
    for(k=0;k<b->n*b->h; k++){
        a[k] = k;
    }
    
    shuffle(a, b->n*b->h);
    
    for(k=0;k<b->n*b->h; k++){
        al_clear_to_color(BLACK_COLOR);
        al_draw_bitmap(bmp,0,0,0);
        for(kk=0;kk<=k; kk++){
            ii = a[kk] / b->h;
            jj = a[kk] % b->h;
            get_TiledBlock_offset(b->panel.b[ii]->b[jj], &x, &y);
            al_draw_filled_rectangle(x,y, x+b->panel.b[ii]->b[jj]->w, y+b->panel.b[ii]->b[jj]->h, al_premul_rgba(0,0,0,200));
        }
        al_flip_display();
        if (!set.sound_mute) { play_sound(SOUND_STONE); }
        while(al_get_next_event(event_queue, &ev)){
            if(ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING){
                halt(event_queue);
                al_acknowledge_resize(al_get_current_display());
                return;
                // should do something else here
            }
        }
        al_rest(max(0.15, 0.6*(1 - sqrt((float) k / (b->h*b->n)))));
    }
}

void draw_generating_puzzle(Game *g, Board *b) {
    char msg[1000];
    int w = al_get_display_width(al_get_current_display());
    int h = al_get_display_height(al_get_current_display());
    if(!g->advanced)
        snprintf(msg, 999, "Generating %d x %d puzzle, please wait...", g->n, g->h);
    else
        snprintf(msg, 999, "Generating %d x %d advanced puzzle, please wait (this could take a while)...", g->n, g->h);
    al_clear_to_color(BLACK_COLOR);
    draw_multiline_wz_box(msg, w/2, h/2, 0.4*w, 0.2*w);
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
        if(init_bitmaps(b)){
            fprintf(stderr, "error switching tiles.\n");
            exit(-1);
        }
    }
    b->max_xsize = al_get_display_width(display);
    b->max_ysize = al_get_display_height(display);
    create_board(g, b, 0);
    al_set_target_backbuffer(display);
    update_board(g, b);
    al_convert_bitmaps();
    al_clear_to_color(BLACK_COLOR);
    al_flip_display();
    return 0;
}

void win_or_lose(Game *g, Board *b){
    if(check_solution(g)){
        game_state = GAME_OVER;
        show_info_text(b, al_ustr_new("Elementary, watson!"));
        animate_win(b);
        if (!set.sound_mute) play_sound(SOUND_WIN);
    } else {
        show_info_text(b, al_ustr_new("Something is wrong. Try again, go to settings to start a new puzzle."));
        if (!set.sound_mute) play_sound(SOUND_WRONG);
        execute_undo(g);
        update_board(g, b);
        emit_event(EVENT_REDRAW);
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
    
    init_fonts(); // do this after creating display OK
    
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
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_DISPLAY *display = NULL;
    double resize_time, old_time;
    double blink_time = 0, play_time = 0;
    int noexit, mouse_click,redraw, mouse_move,keypress, resizing, resize_update, mouse_button_down, restart;
    float max_display_factor;
    TiledBlock  *tb_down = NULL, *tb_up = NULL;
    double mouse_up_time = 0, mouse_down_time = 0;
    int wait_for_double_click = 0, hold_click_check = 0;
    float DELTA_DOUBLE_CLICK = 0.2;
    float DELTA_SHORT_CLICK = 0.1;
    float DELTA_HOLD_CLICK = 0.3;
    int mbdown_x, mbdown_y;
    
    // seed random number generator. comment out for debug
    srand((unsigned int) time(NULL));
   
    deblog("Watson has started");
    if (init_allegro()) return -1;
    deblog("Allegro initialized");

#ifndef _WIN32
     // use anti-aliasing if available (seems to cause problems in windows)
     al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
     al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
#endif
    
    fullscreen = 0;
    
    // use vsync if available
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
    
    if(!MOBILE){
        if (fullscreen) {
            al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
            display = al_create_display(desktop_xsize, desktop_ysize);
        } else {
            al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
            display = al_create_display(800,600);
        }
    } else {
        al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);

        al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
        display = al_create_display(desktop_xsize, desktop_ysize);
        set.fat_fingers = 1;
    }
    
    
    if(!display) {
        fprintf(stderr, "Failed to create display!\n");
        return -1;
    }
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);

    al_set_window_title(display, "Watson");
    al_init_user_event_source(&user_event_src);

    if(!load_game_f(&g,&b)) set.saved=1;
    
    draw_title();
    al_flip_display();
    wait_for_input(NULL);
    restart=0;
    
RESTART:
    b.type_of_tiles = set.type_of_tiles; // use font tiles by default
    get_desktop_resolution(0, &desktop_xsize, &desktop_ysize);
    
    if(restart == 1) g.time = 0; // new game, otherwise it's a load game
    
	if (!fullscreen && !MOBILE) {
        max_display_factor = 0.9;
    } else
        max_display_factor = 1;

    if(restart){
        al_set_target_backbuffer(display);
        destroy_board(&b);
        destroy_undo();
        al_set_target_backbuffer(display);
    }
    
    b.max_xsize = desktop_xsize*max_display_factor;
    b.max_ysize = desktop_ysize*max_display_factor; // change this later to something adequate

    g.advanced = set.advanced; // use "what if" depth 1?
    b.type_of_tiles = set.type_of_tiles;
    
    if(restart != 2){ // 2 is for loaded game
        g.n = b.n = set.n;
        g.h = b.h = set.h;
        draw_generating_puzzle(&g, &b);
        create_game_with_clues(&g);
    } else {
        set.n = g.n;
        set.h = g.h;
    }
    
    restart = 0;
    
    if(create_board(&g, &b, 1)){
        fprintf(stderr, "Failed to create game board.\n");
        return -1;
    }
    
    if(!MOBILE && !fullscreen) {
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

    //this timer is here just in case

	if (!(timer = al_create_timer(1.0 / FPS))) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_register_event_source(event_queue, al_get_display_event_source(display));
    if(al_is_keyboard_installed())
        al_register_event_source(event_queue, al_get_keyboard_event_source());
    if(al_is_mouse_installed())
        al_register_event_source(event_queue, al_get_mouse_event_source());
    if(al_is_touch_input_installed())
        al_register_event_source(event_queue, al_get_touch_input_event_source());

    al_register_event_source(event_queue , &user_event_src);

    update_board(&g,&b);
    al_set_target_backbuffer(display);
//    al_start_timer(timer);

//  initialize flags
    redraw=1; mouse_click=0;
    noexit=1; mouse_move=0;
    restart=0; keypress=0;
    resizing=0;
    mouse_button_down = 0;
    resize_update=0; resize_time = 0;
    game_state = GAME_PLAYING;
    b.time_start=al_get_time();
    blink_time = 0;
    b.blink = 0;
    mbdown_x = 0;
    mbdown_y = 0;
    tb_down = tb_up = NULL;
    
    show_info_text(&b, al_ustr_newf("Click on clue for info. Click %s for help, %s for settings, or %s for a hint at any time.", symbol_char[b.h][0], symbol_char[b.h][2], symbol_char[b.h][1]));
    
    al_set_target_backbuffer(display);
    al_clear_to_color(BLACK_COLOR);
    al_flip_display();
    al_flush_event_queue(event_queue);
    play_time = old_time = al_get_time();

    while(noexit)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_get_time() - old_time;
        if(game_state == GAME_PLAYING) g.time += dt;
        old_time = al_get_time();
       // al_wait_for_event(event_queue, &ev);
        while(al_get_next_event(event_queue, &ev)){ // empty out the event queue
            switch(ev.type){
                case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
                    deblog("RECEIVED HALT");
                    al_stop_timer(timer);
                    halt(event_queue);
                    resize_update=1;
                    al_start_timer(timer);
                    break;
                case EVENT_RESTART:
                    restart=1;
                    goto RESTART;
                    
                case EVENT_EXIT:
                    noexit=0;
                    break;
                    
                case EVENT_SAVE:
                    if(!save_game_f(&g, &b)){
                        show_info_text(&b, al_ustr_new("Game saved"));
                        set.saved = 1;
                    } else {
                        show_info_text(&b, al_ustr_new("Error: game could not be saved."));
                    }
                    break;
                case EVENT_LOAD:
                    if(load_game_f(&g, &b)){
                        show_info_text(&b, al_ustr_new("Error game could not be loaded."));
                    } else {
                        restart = 2;
                        goto RESTART;
                    }
                    break;
                case EVENT_SETTINGS:
                    show_settings(&set, &b, event_queue);
                    emit_event(EVENT_REDRAW);
                    break;
                case ALLEGRO_EVENT_TIMER:
                    //redraw=1;
                    break;
                    
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                    if(mouse_button_down) break;
                    mouse_down_time = ev.any.timestamp;
                    mbdown_x = ev.mouse.x; mbdown_y = ev.mouse.y;
                    tb_down = get_TiledBlock_at(&b, ev.mouse.x, ev.mouse.y);
                    
                    if(b.zoom && !tb_down){
                        b.zoom = NULL;
                        redraw = 1;
                        break;
                    }
                    
                    if(wait_for_double_click){
                        wait_for_double_click = 0;
                        if( (tb_up == tb_down) && (mouse_down_time - mouse_up_time < DELTA_DOUBLE_CLICK) ){
                            handle_mouse_click(&g, &b, tb_down, ev.mouse.x, ev.mouse.y, 3); // double click
                            break;
                        }
                    }
                    mouse_button_down = ev.mouse.button;
                    break;
                    
                case ALLEGRO_EVENT_TOUCH_END:
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
                    ev.mouse.button = 1;
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                    if(wait_for_double_click) wait_for_double_click = 0;
                    
                    
                    if(b.dragging){
                        mouse_drop(&b, ev.mouse.x, ev.mouse.y);
                        mouse_button_down = 0;
                    }
                    
                    if(hold_click_check == 2){
                        hold_click_check = 0;
                        mouse_button_down = 0;
                        break;
                    }
                    hold_click_check = 0;
                   
                    if(!mouse_button_down) break;
                    
                    mouse_up_time = ev.any.timestamp;
                    tb_up = get_TiledBlock_at(&b, ev.mouse.x, ev.mouse.y);
                    if((tb_up) && (tb_up == tb_down)){
                        if( ((tb_up->type == TB_HCLUE_TILE) || (tb_up->type == TB_VCLUE_TILE)) && (mouse_button_down == 1) ){
                            if(mouse_up_time - mouse_down_time < DELTA_SHORT_CLICK) {
                                wait_for_double_click = 1;
                            }
                        }
                        if(!wait_for_double_click)
                            handle_mouse_click(&g, &b, tb_up, ev.mouse.x, ev.mouse.y, mouse_button_down);

                    }
                    mouse_button_down = 0;
                    break;
                    
                case ALLEGRO_EVENT_TOUCH_MOVE:
                    if(!ev.touch.primary) break;
                    ev.mouse.x = ev.touch.x;
                    ev.mouse.y = ev.touch.y;
//                    ev.mouse.dx = ev.touch.dx;
//                    ev.mouse.dy = ev.touch.dy;
                case ALLEGRO_EVENT_MOUSE_AXES:
                    if(b.dragging){
                        b.dragging->x = ev.mouse.x + b.dragging_cx;
                        b.dragging->y = ev.mouse.y + b.dragging_cy;
                    }
                    mouse_move=1;
                    // don't grab if movement was small
                    if((abs(ev.mouse.x - mbdown_x) < 10) && ((ev.mouse.y - mbdown_y) < 10)) break;
                    
                    if(mouse_button_down && !hold_click_check)
                        if(tb_down && ((tb_down->type == TB_HCLUE_TILE) || (tb_down->type == TB_VCLUE_TILE)) )
                            {
                                handle_mouse_click(&g, &b, tb_down, mbdown_x, mbdown_y, 4);
                                hold_click_check = 1;
                            }
                    break;
                    
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    emit_event(EVENT_EXIT);
                    break;
                    
                case ALLEGRO_EVENT_KEY_CHAR:
                    keypress=1;
                    switch(ev.keyboard.keycode){
                        case ALLEGRO_KEY_ESCAPE:
                            if(confirm_exit(&b, event_queue))
                                noexit=0;
                            else
                                redraw=1;
                            break;
                        case ALLEGRO_KEY_BACK: // android: back key
                            show_settings(&set, &b, event_queue);
                            emit_event(EVENT_REDRAW);
                            break;
                        case ALLEGRO_KEY_R:
                            if(confirm_restart(&b, &set, event_queue)){
                                restart=1;
                                goto RESTART;
                            }
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_S: // debug: show solution
                            switch_solve_puzzle(&g, &b);
                            redraw=1;
                            break;
                        case ALLEGRO_KEY_T:
                            emit_event(EVENT_SWITCH_TILES);
                            break;
                        case ALLEGRO_KEY_H:
                            show_help(&b, event_queue);
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
                        case ALLEGRO_KEY_ENTER:
                            break;
                    }
                    break;
                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    if(b.dragging) mouse_drop(&b, -1,-1);
                    if (fullscreen) break;
                    al_acknowledge_resize(display);
                    resizing=1; resize_time=al_get_time();
                    break;
                case EVENT_REDRAW:
                    redraw=1;
                    break;
                case EVENT_SWITCH_TILES:
                    switch_tiles(&g, &b, display);
                    al_flush_event_queue(event_queue);
                    redraw=1;
                    break;
            }
        }// while(al_get_next_event(event_queue, &ev));
	
	if(resizing){
            if(al_get_time()-resize_time > RESIZE_DELAY){
                resizing =0; resize_update=1;
            }
        }
    
        if(resize_update){
            resize_update=0;
			al_acknowledge_resize(display);
            if(MOBILE){
                destroy_all_bitmaps(&b);
                init_bitmaps(&b);
            } else
                destroy_board_bitmaps(&b);
			b.max_xsize = al_get_display_width(display);
			b.max_ysize = al_get_display_height(display);
            create_board(&g, &b, 0);
            al_set_target_backbuffer(display);
            update_board(&g, &b);
			al_convert_bitmaps(); // turn bitmaps to video bitmaps
            redraw=1;
        // android workaround, try removing:
            al_clear_to_color(BLACK_COLOR);
            al_flip_display(); 
        }
        
        if(resizing) // skip redraw and other stuff
            continue;
        
        
        if( wait_for_double_click && (al_get_time() - mouse_up_time > DELTA_DOUBLE_CLICK) ){
                wait_for_double_click = 0;
                ALLEGRO_MOUSE_STATE mouse;
                al_get_mouse_state(&mouse);
            tb_down=get_TiledBlock_at(&b, mouse.x, mouse.y);
                handle_mouse_click(&g, &b, tb_down, mouse.x, mouse.y, 1); // single click
        }

        if(mouse_button_down && !hold_click_check && !b.dragging ){
            if (al_get_time() - mouse_down_time > DELTA_HOLD_CLICK){
                hold_click_check = 1;
                if(tb_down){
                    ALLEGRO_MOUSE_STATE mouse;
                    al_get_mouse_state(&mouse);
                    tb_up = get_TiledBlock_at(&b, mouse.x, mouse.y);
                    if(tb_up == tb_down){
                        handle_mouse_click(&g, &b, tb_up, mouse.x, mouse.y, 4); // hold click
                        hold_click_check = 2;
                    }
                }
            }
        }

        if(mouse_move){
            mouse_move = 0;
            if(b.dragging)
                redraw = 1;
        }
    
        if(keypress){
            keypress=0;
        }

        if( old_time - play_time > 1 ){ // runs every second
            if ( game_state == GAME_PLAYING ){
                play_time = al_get_time();
                update_timer((int) g.time, &b); // this draws on a timer bitmap
                
                if(g.guessed == g.h*g.n){
                    win_or_lose(&g, &b); // check if player has won
                    al_flush_event_queue(event_queue);
                }
                emit_event(EVENT_REDRAW);
            }
        }
        
        if(b.rule_out && (al_get_time() - blink_time > BLINK_DELAY) ){
            SWITCH(b.blink);
            blink_time = al_get_time();
            redraw=1;
        }
        
        if(game_state == GAME_OVER){
            win_gui(&g, &b, event_queue);
        }
        
        if(redraw) {
            redraw = 0;
            al_set_target_backbuffer(display);
            draw_stuff(&b);
            al_flip_display();
        }

    }// end of game loop
    
    destroy_everything(&b);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
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
    float xx =x, yy=y;
    TiledBlock *t;
    
    if(b->zoom){
        al_transform_coordinates(&b->zoom_transform_inv, &xx, &yy);
        t = get_TiledBlock(b->zoom, xx, yy);
        if(t && (t->parent == b->zoom)) return t;
        else return NULL; // else return get_TiledBlock(&b->all, x, y);
    }
    
    return get_TiledBlock(&b->all, x, y);
}

void mouse_grab(Board *b, int mx, int my){
    emit_event(EVENT_REDRAW);
    b->dragging = get_TiledBlock_at(b, mx, my);
    if(b->dragging && b->dragging->bmp){
        if( (b->dragging->type == TB_HCLUE_TILE) || (b->dragging->type == TB_VCLUE_TILE) ){
            b->dragging_ox = b->dragging->x;
            b->dragging_oy = b->dragging->y;
            b->dragging_cx = b->dragging->x  - mx+5;
            b->dragging_cy = b->dragging->y - my+5;
            b->dragging->x = mx + b->dragging_cx;
            b->dragging->y = my + b->dragging_cy;
           // if(!set.sound_mute) play_sound(SOUND_UNHIDE_TILE);
            return;
        }
    }
    
    b->dragging = NULL;
        return;
};



void mouse_drop(Board *b, int mx, int my){
    TiledBlock *t;

    emit_event(EVENT_REDRAW);
    if(!b->dragging) return;
    b->dragging->x = b->dragging_ox;
    b->dragging->y = b->dragging_oy;
 
    t = get_TiledBlock_at(b, mx, my);
    if(t && (t->type == b->dragging->type)){
        swap_clues(b, b->dragging, t);
        if(b->highlight == b->dragging) b->highlight = t;
        else if(b->highlight == t) b->highlight = b->dragging;
        if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    }
    b->dragging = NULL;
    clear_info_panel(b);
    return;
};

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
    char *b0, *b1, *b2;
    b0 = symbol_char[clue->j[0]][clue->k[0]];
    b1 = symbol_char[clue->j[1]][clue->k[1]];
    b2 = symbol_char[clue->j[2]][clue->k[2]];
 
    switch(clue->rel){
        case CONSECUTIVE:
            show_info_text(b, al_ustr_newf("The column of %s is between %s and %s, but they could be on either side.", b1, b0, b2));
            break;
        case NEXT_TO:
            show_info_text(b, al_ustr_newf("The columns of %s and %s are next to each other, but they could be on either side.", b0, b1));
            break;
        case NOT_NEXT_TO:
            show_info_text(b, al_ustr_newf("The column of %s is NOT next to the column of %s.", b0, b1));
            break;
        case NOT_MIDDLE:
            show_info_text(b, al_ustr_newf("There is exactly one column between %s and %s, and %s is NOT in that column.", b0, b2, b1));
            break;
        case ONE_SIDE:
            show_info_text(b, al_ustr_newf("The column of %s is strictly to the left of %s.", b0, b1));
            break;
        case TOGETHER_2:
            show_info_text(b, al_ustr_newf("%s and %s are on the same column.", b0, b1));
            break;
        case TOGETHER_3:
            show_info_text(b, al_ustr_newf("%s, %s and %s are on the same column.", b0, b1, b2));
            break;
        case NOT_TOGETHER:
            show_info_text(b, al_ustr_newf("%s and %s are NOT on the same column.", b0, b1));
            break;
        case TOGETHER_NOT_MIDDLE:
            show_info_text(b, al_ustr_newf("%s and %s are on the same column, and %s is NOT in that column.", b0, b2, b1));
            break;
        default:
            break;
    }
}

void show_hint(Game *g, Board *b){
    char *b0, *b1, *b2, *b3;
    int i;
    
    if(!check_panel_correctness(g)){
        show_info_text(b, al_ustr_new("Something is wrong. An item was ruled out incorrectly."));
        return;
    }
    
    i=get_hint(g);
    if(!i){
        show_info_text(b, al_ustr_new("No hint available."));
        return;
    }
    
    b->highlight = b->clue_tiledblock[i & 255];
    b->rule_out = b->panel.b[(i>>15) & 7]->b[(i>>12) & 7]->b[(i>>9) & 7];

    b0 = symbol_char[g->clue[i & 255].j[0]][g->clue[i & 255].k[0]];
    b1 = symbol_char[g->clue[i & 255].j[1]][g->clue[i & 255].k[1]];
    b2 = symbol_char[g->clue[i & 255].j[2]][g->clue[i & 255].k[2]];
    b3 = symbol_char[(i>>12) & 7][(i>>9) & 7];
    
    switch(g->clue[i & 255].rel){
        case CONSECUTIVE:
            show_info_text(b, al_ustr_newf("The column of %s is between %s and %s, so we can rule out %s from here.", b1, b0, b2, b3));
            break;
        case NEXT_TO:
            show_info_text(b, al_ustr_newf("The columns of %s and %s are next to each other, so we can rule out %s from here.", b0, b1, b3));
            break;
        case NOT_NEXT_TO:
            show_info_text(b, al_ustr_newf("The column of %s is NOT next to the column of %s, so we can rule out %s from here.", b0, b1, b3));
            break;
        case NOT_MIDDLE:
            show_info_text(b, al_ustr_newf("There is exactly one column between %s and %s, and %s is NOT in that column, so we can rule out %s from here.", b0, b2, b1, b3));
            break;
        case ONE_SIDE:
            show_info_text(b, al_ustr_newf("The column of %s is strictly to the left of %s, so we can rule out %s from here.", b0, b1, b3));
            break;
        case TOGETHER_2:
            show_info_text(b, al_ustr_newf("%s and %s are on the same column, so we can rule out %s from here.", b0, b1,b3));
            break;
        case TOGETHER_3:
            show_info_text(b, al_ustr_newf("%s, %s and %s are on the same column, so we can rule out %s from here.", b0, b1, b2, b3));
            break;
        case NOT_TOGETHER:
            show_info_text(b, al_ustr_newf("%s and %s are NOT on the same column, so we can rule out %s from here.", b0, b1, b3));
            break;
        case TOGETHER_NOT_MIDDLE:
            show_info_text(b, al_ustr_newf("%s and %s are on the same column, and %s is NOT in that column, so we can rule out %s from here.", b0, b2, b1, b3));
            break;
        default:
            break;
    }
}

void zoom_TB(Board *b, TiledBlock *t){
    float c = 2.5;
    int x,y, dw = al_get_display_width(al_get_current_display()), dh = al_get_display_height(al_get_current_display());
    int tr_x, tr_y;
    
    if(!t) return;
    get_TiledBlock_offset(t, &x, &y);

    tr_x = -(c-1)*(x+t->w/2);
    if(c*x+tr_x < 0)
        tr_x = -c*x;
    else if(c*(x+t->w) + tr_x > dw)
        tr_x = dw - c*(x+t->w);
    
    tr_y = -(c-1)*(y+t->h/2);
    if(c*y + tr_y < 0)
        tr_y = -c*y;
    else if(c*(y+t->h) + tr_y > dh)
        tr_y = dh - c*(y+t->h);
        
    al_identity_transform(&b->identity_transform);
    al_identity_transform(&b->zoom_transform);
    al_build_transform(&b->zoom_transform, tr_x, tr_y, c, c, 0);
    if(t->parent)
        get_TiledBlock_offset(t->parent, &x, &y);
    al_translate_transform(&b->zoom_transform, c*x, c*y);
    b->zoom_transform_inv  = b->zoom_transform;
    al_invert_transform(&b->zoom_transform_inv);
    b->zoom = t;
    if(!set.sound_mute) play_sound(SOUND_CLICK);
}

void handle_mouse_click(Game *g, Board *b, TiledBlock *t, int mx, int my, int mclick){
    int i,j,k;
    
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

    emit_event(EVENT_REDRAW);

    if (!t) return;
    
    if(game_state == GAME_OVER){
        show_info_text(b, al_ustr_new("Press R to start a new puzzle."));
    }
    
    if(set.fat_fingers){
        if(!b->zoom || (t->parent != b->zoom)){
            if( ((t->parent) && (t->parent->type == TB_TIME_PANEL)) || (t->type == TB_TIME_PANEL) ) {
                zoom_TB(b, &b->time_panel);
                return;
            } else if(t->type == TB_PANEL_TILE){
                zoom_TB(b, b->zoom = t->parent);
                return;
            }
        }
    }

    if(b->zoom)
        b->zoom = NULL;

    switch(t->type){ // which board component was clicked
        case TB_PANEL_TILE:
            if(game_state != GAME_PLAYING) break;
            k = t->index; j=t->parent->index; i = t->parent->parent->index;
            if(mclick==1){
                save_state(g);
                if(g->tile[i][j][k]){ // hide tile
                    hide_tile_and_check(g, i, j, k);
                    if (!set.sound_mute) play_sound(SOUND_HIDE_TILE);
                }
            } else if ((mclick==2) || (mclick == 4)){ // hold or right click
                if(g->tile[i][j][k]){
                    save_state(g);
                    guess_tile(g, i, j, k);
                    if (!set.sound_mute) play_sound(SOUND_GUESS_TILE);
                } else { // tile was hidden, unhide
                    if(!is_guessed(g, j, k)){
                        g->tile[i][j][k]=1;
                        if (!set.sound_mute) play_sound(SOUND_UNHIDE_TILE);
                    }
                }
            }
            update_board(g, b);
            break;

        case TB_PANEL_BLOCK:
            if(game_state != GAME_PLAYING) break;
            if ( ((mclick == 2) || (mclick == 4)) && (g->guess[t->parent->index][t->index]>=0)){
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
                if((mclick==2) || (mclick==3)){ // toggle hide-show clue on double or right click
                    SWITCH(t->hidden);
                    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
                } else if (mclick==1){ // explain clue in info panel
//                    {
//                    //xxx DEBUG text
//                    char str[1000];
//                        snprintf(str, 999, "Clue number %d", t->index);
//                    show_info_text_b(b,str);
//                    }
                    if(!t->hidden){
                        if(!set.sound_mute) play_sound(SOUND_CLICK);
                        explain_clue(b, &g->clue[t->index]);
                        b->highlight = t; // highlight clue
                    }
                } else if (mclick == 4) { // hold-click
                    mouse_grab(b, mx, my);
//                    if(MOBILE){
//                        b->highlight = t;
//                        show_info_text(b, "Tap somewhere in the clue box to move this clue");
//                        
//                    }
                }
            }
            break;
            
        case TB_BUTTON_CLUE: // time panel
            if(game_state != GAME_PLAYING) break;
            if(!set.sound_mute) play_sound(SOUND_CLICK);
            show_hint(g,b);
            break;
        case TB_BUTTON_SETTINGS:
            if(!set.sound_mute) play_sound(SOUND_CLICK);
            show_settings(&set, b, event_queue);
            emit_event(EVENT_REDRAW);
            break;
    
        case TB_BUTTON_HELP:
            if(!set.sound_mute) play_sound(SOUND_CLICK);
            show_help(b, event_queue);
            emit_event(EVENT_REDRAW);
            break;

        case TB_BUTTON_UNDO:
            execute_undo(g);
            update_board(g,b);
            break;
            
        case TB_BUTTON_TILES:
            emit_event(EVENT_SWITCH_TILES);
            break;
      
        default:
            break;
    }
}

// work in progress
int save_game_f(Game *g, Board *b){
    ALLEGRO_PATH *path;
    ALLEGRO_FILE *fp;
    
#ifdef ALLEGRO_ANDROID
    al_set_standard_file_interface();
#endif
    
    path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);
    if(!al_make_directory(al_path_cstr(path, '/'))){
        fprintf(stderr, "could not open or create path %s.\n", al_path_cstr(path, '/'));
        return -1;
    }
    
    al_set_path_filename(path, "Watson.sav");
    fp = al_fopen(al_path_cstr(path, '/'), "wb");
    if(!fp){
        fprintf(stderr, "Couldn't open %s for writing.\n", (char *) al_path_cstr(path, '/'));
        al_destroy_path(path);
        return -1;
    }
    al_fwrite(fp, &g->n, sizeof(g->n));
    al_fwrite(fp, &g->h, sizeof(g->h));
    al_fwrite(fp, &g->puzzle, sizeof(g->puzzle));
    al_fwrite(fp, &g->clue_n, sizeof(g->clue_n));
    al_fwrite(fp, &g->clue, sizeof(g->clue));
    al_fwrite(fp, &g->tile, sizeof(g->tile));
    al_fwrite(fp, &g->time, sizeof(g->time));
    al_fclose(fp);

    deblog("Saved game at %s.", al_path_cstr(path, '/'));
    al_destroy_path(path);
    return 0;
}


int load_game_f(Game *g, Board *b){
    ALLEGRO_PATH *path;
    ALLEGRO_FILE *fp;
    ALLEGRO_FS_ENTRY *entry;
    //	int i,j,k;
    
#ifdef ALLEGRO_ANDROID
    al_set_standard_file_interface();
#endif
    path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);
    al_set_path_filename(path, "Watson.sav");
    fp = al_fopen(al_path_cstr(path, '/'), "rb");
    if(!fp){
        errlog("Couldn't open %s for reading.", (char *) al_path_cstr(path, '/'));
        al_destroy_path(path);
        return -1;
    }
    
    if(al_fread(fp, &g->n, sizeof(g->n)) != sizeof(g->n)){
        al_destroy_path(path);
        errlog("Error reading %s.", (char *) al_path_cstr(path, '/'));
        return -1;
    }
    
    al_fread(fp, &g->h, sizeof(g->h));
    al_fread(fp, &g->puzzle, sizeof(g->puzzle));
    al_fread(fp, &g->clue_n, sizeof(g->clue_n));
    al_fread(fp, &g->clue, sizeof(g->clue));
    al_fread(fp, &g->tile, sizeof(g->tile));
    al_fread(fp, &g->time, sizeof(g->time));
    al_fclose(fp);
    
    b->n=g->n; b->h=g->h;
    
    update_guessed(g);
    /* delete saved game after reading
    entry = al_create_fs_entry(al_path_cstr(path, '/'));
    al_remove_fs_entry(entry);
    */
    al_destroy_path(path);

    return 0;
}

