// Based in example.c
// from the WidgetZ GUI library by Pavel Sountsov


#include "gui.h"
#include <stdio.h>
#include <math.h>
#include "widgetz/widgetz.h"
#include "allegro_stuff.h"
#include "main.h"
#include "text.h"

#define GUI_XFACTOR 0.5
#define GUI_YFACTOR 0.5
#define GUI_BG_COLOR al_map_rgb(120, 120, 120) //al_map_rgb(108, 122, 137);//(0.1, 0.6, 0.8, 1)
#define GUI_TEXT_COLOR al_map_rgb(255, 255, 255)

const char ABOUT_TEXT[] = "Watson v" PRE_VERSION " - " PRE_DATE ", by Koro.\n"
"\n"
"Watson is an open source clone of \"Sherlock\", an old game by Evertt Kaser which is itself based on the a classic puzzle known as \"Zebra puzzle\" or \"Einstein's riddle\".\n"
"\n"
"Watson is programmed in plain C with the Allegro 5 library. Big thanks to the friendly folks from Freenode #allegro for all the tips and advice.\n"
"\n"
"The tile set is rendered from TTF fonts obtained from fontlibrary.org. Pressing the game will switch to custom tiles, which sould be stored in <appdir>/icons into 8 separate folders numbered 0 to 7, each with 8 square tiles in .png format. Of course, these won't look as nice as the fonts due to the anti-aliasing. The tiles provided here were downloaded from www.icons8.com.\n"
"\n"
"The \"advanced\" mode generates (much) harder puzzles, to the point of being almost impossible, so it needs to be tuned.";


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


enum {
    BUTTON_ROWS_4 = 1,
    BUTTON_ROWS_5,
    BUTTON_ROWS_6,
    BUTTON_ROWS_7,
    BUTTON_ROWS_8,
    BUTTON_COLS_4,
    BUTTON_COLS_5,
    BUTTON_COLS_6,
    BUTTON_COLS_7,
    BUTTON_COLS_8,
    BUTTON_OK,
    BUTTON_CANCEL,
    BUTTON_SOUND,
    BUTTON_ADVANCED,
    BUTTON_RESTART,
    BUTTON_ABOUT,
    BUTTON_EXIT,
    BUTTON_TILES,
    GROUP_COLS,
    GROUP_ROWS,
    BUTTON_SAVE,
    BUTTON_LOAD
    
};

// even if wgt->own = 1, the original function duplicates the string
// this avoids a memory leak
void wz_set_text_own(WZ_WIDGET* wgt, ALLEGRO_USTR* text){
    wz_set_text(wgt, text);
    al_ustr_free(text);
}


int confirm_restart(Board *b, Settings *set, ALLEGRO_EVENT_QUEUE *queue)
{
    if(yes_no_gui(al_ustr_newf("Start new %dx%d%s game?", set->n, set->h, set->advanced ? " advanced" : ""), b->all.x+b->xsize/2, b->all.y+b->ysize/2, max(b->xsize*0.5*GUI_XFACTOR, 300), queue))
        return 1;
    else
        return 0;
}

int confirm_exit(Board *b, ALLEGRO_EVENT_QUEUE *queue)
{
    if(yes_no_gui(al_ustr_newf("Exit game?"), b->all.x+b->xsize/2, b->all.y+b->ysize/2, max(b->xsize*0.5*GUI_XFACTOR, 300), queue))
        return 1;
    else
        return 0;
}

void draw_center_textbox_wait(const char *text, float width_factor, Board *b, ALLEGRO_EVENT_QUEUE *queue){
    ALLEGRO_BITMAP *keep = screenshot();
    draw_multiline_wz_box(text, b->all.x+b->xsize/2, b->all.y + b->ysize/2, width_factor*b->xsize);
    al_wait_for_vsync();
    al_flip_display();
    wait_for_input(queue);
    al_draw_bitmap(keep,0,0,0);
    al_destroy_bitmap(keep);
}


void show_help(Board *b, ALLEGRO_EVENT_QUEUE *queue){
    draw_center_textbox_wait(HELP_TEXT, min(0.96,max(0.5, 800.0/b->xsize)), b, queue);
}

void show_about(Board *b, ALLEGRO_EVENT_QUEUE *queue){
    draw_center_textbox_wait(ABOUT_TEXT, min(0.96,max(0.5, 800.0/b->xsize)), b, queue);
}

int show_settings(Settings *set, Board *b, ALLEGRO_EVENT_QUEUE *queue)
{
    // Initialize Allegro 5 and the font routines
    int refresh_rate;
    float size = 1.5;
    int font_size = 18;
    int gui_w = 500;
    double fixed_dt;
    double old_time;
    double game_time;
    double start_time;
    WZ_WIDGET* gui;
    WZ_SKIN_THEME skin_theme;
    WZ_WIDGET* wgt;
    int done = 0;
    int cancel = 0;
    ALLEGRO_FONT *font;
    ALLEGRO_EVENT event;
    WZ_WIDGET * button_rows[5];
    WZ_WIDGET * button_cols[5];
    WZ_WIDGET *button_advanced;
    WZ_WIDGET *button_mute;
    Settings nset = *set;

#ifdef ALLEGRO_ANDROID
    al_android_set_apk_file_interface();
#endif
    

    //xxx todo: change this factor depending on dpi???
    size = min(max(1,GUI_XFACTOR*(float)b->xsize/gui_w), max(1,GUI_YFACTOR*(float)b->ysize/470.0));
    
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size * size);
    
    refresh_rate = 60;
    fixed_dt = 1.0f / refresh_rate;
    old_time = al_current_time();
    game_time = al_current_time();

    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    skin_theme.theme.font = font;
    skin_theme.theme.color1 = GUI_BG_COLOR;
    skin_theme.theme.color2 = GUI_TEXT_COLOR;
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);

    // main gui
    gui = wz_create_widget(0, b->all.x + (b->xsize-size*gui_w)/2, b->all.y + (b->ysize-size*470)/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);

    // about button
    wz_create_fill_layout(gui, 0, 0, gui_w * size, 50 * size, 10 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 200 * size, 40 * size, al_ustr_new("About Watson"), 1, BUTTON_ABOUT);
    
    // number of rows multitoggle
    wz_create_fill_layout(gui, 0, 50 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_LEFT, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Rows:"), 1, -1);
    button_rows[0] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("4"), 1, GROUP_ROWS, 4);
    button_rows[1] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("5"), 1, GROUP_ROWS, 5);
    button_rows[2] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("6"), 1, GROUP_ROWS, 6);
    button_rows[3] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("7"), 1,  GROUP_ROWS, 7);
    button_rows[4] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("8"), 1,  GROUP_ROWS, 8);

    // number of columns multitoggle
    wz_create_fill_layout(gui, 0, 120 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_LEFT, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Columns:"), 1, -1);
    button_cols[0] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("4"), 1, GROUP_COLS, 4);
    button_cols[1] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("5"), 1, GROUP_COLS, 5);
    button_cols[2] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("6"), 1, GROUP_COLS, 6);
    button_cols[3] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("7"), 1, GROUP_COLS, 7);
    button_cols[4] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("8"), 1, GROUP_COLS, 8);
    
    // start with the current row/col setting
    ((WZ_BUTTON*)button_cols[set->h-4])->down=1;
    ((WZ_BUTTON*)button_rows[set->n-4])->down=1;
    
    // Sound + Advanced buttons
    wz_create_fill_layout(gui, 0, 190 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Sound:"), 1, -1);
    button_mute = (WZ_WIDGET*) wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, set->sound_mute ? al_ustr_new("off") : al_ustr_new("on"), 1, -1, BUTTON_SOUND);
    ((WZ_BUTTON*) button_mute)->down = !set->sound_mute;
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Advanced:"), 1, -1);
    button_advanced = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, set->advanced ? al_ustr_new("on") : al_ustr_new("off"), 1, -1, BUTTON_ADVANCED);
    ((WZ_BUTTON*) button_advanced)->down = set->advanced;

    
    // Save + Load buttons
    wz_create_fill_layout(gui, 0, 260 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
//    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Sound:"), 1, -1);
    wz_create_toggle_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Save game"), 1, -1, BUTTON_SAVE);
//    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Advanced:"), 1, -1);
    wgt = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Load game"), 1, -1, BUTTON_LOAD);
    ((WZ_BUTTON*) wgt)->down = !set->saved;

    // restart/exit/switch tiles buttons
    wz_create_fill_layout(gui, 0, 330 * size, gui_w * size, 70 * size, 30 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("New game"), 1, BUTTON_RESTART);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Exit game"), 1, BUTTON_EXIT);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Switch tiles"), 1, BUTTON_TILES);

    
    // ok/cancel buttons
    wz_create_fill_layout(gui, 0, 400 * size, gui_w * size, 70 * size, 30 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET*) wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    // escape key cancels and exits
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    

    wz_register_sources(gui, queue);
    
    while(!done)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_current_time() - old_time;
        old_time = al_current_time();
        
        if(old_time - game_time > dt)    //eliminate excess overflow
        {
            game_time += fixed_dt * floor((old_time - game_time) / fixed_dt);
        }
        
        start_time = al_current_time();
        
        while(old_time - game_time >= 0)
        {
            game_time += fixed_dt;
            /*
             Update gui
             */
            wz_update(gui, fixed_dt);
            
            while(!done && al_peek_next_event(queue, &event))
            {
                if((event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) || (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)){ // any other values that require returning?
                    done=1;
                    break;
                }
                al_drop_next_event(queue);
                
                /*
                 Give the gui the event, in case it wants it
                 */
                wz_send_event(gui, &event);
                
            
                switch(event.type)
                {
                    case WZ_BUTTON_PRESSED:
                    {
                        if( ((WZ_TOGGLE *) event.user.data2)->group == GROUP_ROWS)
                            nset.n = (int) event.user.data1;
                        else if( ((WZ_TOGGLE *) event.user.data2)->group == GROUP_COLS)
                            nset.h = (int) event.user.data1;
                        
                        switch((int)event.user.data1)
                        {
                            case BUTTON_CANCEL:
                                done = 1;
                                cancel = 1;
                                break;
                                
                            case BUTTON_EXIT:
                                emit_event(EVENT_EXIT);
                                if(confirm_exit(b, queue))
                                {
                                    emit_event(EVENT_EXIT);
                                    done = 1;
                                }
                                break;
                                
                            case BUTTON_OK:
                                if( (nset.n != set->n) || (nset.h != set->h) || (nset.advanced != set->advanced) )
                                {
                                    if(!confirm_restart(b, &nset, queue))
                                        break;
                                    else
                                        emit_event(EVENT_RESTART);
                                }
                                
                                cancel=0;
                                done=1;
                                break;
                                
                            case BUTTON_RESTART:
                                if(!confirm_restart(b, &nset, queue))
                                    break;
                                emit_event(EVENT_RESTART);
                                nset.restart=1;
                                done=1;
                                cancel=0;
                                break;
                                
                            case BUTTON_TILES:
                                emit_event(EVENT_SWITCH_TILES);
                                done=1;
                                break;
                                
                            case BUTTON_ABOUT:
                                show_about(b, queue);
                                break;
                                
                            case BUTTON_SOUND:
                                SWITCH(nset.sound_mute);
                                if(nset.sound_mute){
                                    wz_set_text_own((WZ_WIDGET*) event.user.data2, al_ustr_new("off"));
                                } else {
                                    wz_set_text_own((WZ_WIDGET*) event.user.data2, al_ustr_new("on"));
                                }
                                break;
                                
                            case BUTTON_ADVANCED:
                                SWITCH(nset.advanced);
                                if(!nset.advanced)
                                    wz_set_text_own((WZ_WIDGET *) event.user.data2, al_ustr_new("off"));
                                else
                                    wz_set_text_own((WZ_WIDGET *) event.user.data2, al_ustr_new("on"));
                                break;
                                
                            case BUTTON_SAVE:
                                if(set->saved)
                                    if(!yes_no_gui(al_ustr_new("Save game? This will overwrite\na previous save."), b->all.x+b->xsize/2, b->all.y+b->ysize/2, b->xsize*0.33, queue))
                                        break;
                                emit_event(EVENT_SAVE);
                                cancel=1;
                                done=1;
                                break;
                                
                            case BUTTON_LOAD:
                                if( !set->saved || !yes_no_gui(al_ustr_new("Discard current game?"), b->all.x+b->xsize/2, b->all.y+b->ysize/2, b->xsize*0.33, queue) )
                                {
                                    {
                                        ((WZ_BUTTON*) event.user.data2)->down = 1;
                                        break;
                                    }
                                }
                                emit_event(EVENT_LOAD);
                                cancel=1;
                                done=1;
                                break;

                        }
                        break;
                    }
                }
            }
            
            if(al_current_time() - start_time > fixed_dt) //break if we start taking too long
                break;
        }
        
        if(!done){
            wz_draw(gui);
            al_wait_for_vsync();
            al_flip_display();
        }
    }
    
    if(!cancel) *set = nset;

    al_destroy_bitmap(skin_theme.box_bitmap);
    al_destroy_bitmap(skin_theme.button_up_bitmap);
    al_destroy_bitmap(skin_theme.button_down_bitmap);
    wz_destroy_skin_theme(&skin_theme);
    wz_destroy(gui);
    return 0;
}


int yes_no_gui(ALLEGRO_USTR *text, int center_x, int center_y, int min_width, ALLEGRO_EVENT_QUEUE *queue)
{
    // Initialize Allegro 5 and the font routines
    int refresh_rate;
    float size = 2.0;
    int font_size = 25;
    double fixed_dt;
    double old_time;
    double game_time;
    double start_time;
    WZ_WIDGET* gui;
    WZ_SKIN_THEME skin_theme;
    WZ_WIDGET* wgt;
    bool done = false;
    ALLEGRO_FONT *font;
    int ret=0;
    int gui_w = 400;
    ALLEGRO_EVENT event;
    
    size = (float)min_width/gui_w;

    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size * size);
    gui_w = max(gui_w, al_get_ustr_width(font, text)/size);
    
#if (ALLEGRO_SUB_VERSION > 0)
    if(al_install_touch_input())
        al_register_event_source(queue, al_get_touch_input_event_source());
#endif
    refresh_rate = 60;
    fixed_dt = 1.0f / refresh_rate;
    old_time = al_current_time();
    game_time = al_current_time();

    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    skin_theme.theme.font = font;
    skin_theme.theme.color1 = GUI_BG_COLOR;
    skin_theme.theme.color2 = GUI_TEXT_COLOR;
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);
    
    gui = wz_create_widget(0, center_x-size*gui_w/2, center_y-size*200/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, gui_w * size, 200 * size, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wz_create_textbox(gui, 0, 0, gui_w * size, 100 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, text, 1, -1);
    wgt = (WZ_WIDGET *) wz_create_fill_layout(gui, 0, 100 * size, gui_w * size, 100 * size, 30*size, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wgt->flags |= WZ_STATE_LAYOUT | WZ_STATE_HIDDEN;
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);

    wz_register_sources(gui, queue);
    
    while(!done)
    {
        double dt = al_current_time() - old_time;
        al_rest(fixed_dt - dt); //rest at least fixed_dt
        dt = al_current_time() - old_time;
        old_time = al_current_time();
        
        if(old_time - game_time > dt)    //eliminate excess overflow
        {
            game_time += fixed_dt * floor((old_time - game_time) / fixed_dt);
        }
        
        start_time = al_current_time();
        
        while(old_time - game_time >= 0)
        {
            game_time += fixed_dt;
            wz_update(gui, fixed_dt);
            
            while(!done && al_peek_next_event(queue, &event))
            {
                if((event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) || (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)){ // any other values that require returning?
                    done=1;
                    break;
                }
                al_drop_next_event(queue);
                /*
                 Give the gui the event, in case it wants it
                 */
                wz_send_event(gui, &event);
                
                switch(event.type)
                {
                    case WZ_BUTTON_PRESSED:
                    {
                        switch((int)event.user.data1)
                        {
                            case BUTTON_CANCEL:
                            {
                                done = true;
                                ret = 0;
                                break;
                            }
                                
                            case BUTTON_OK:
                                done = true;
                                ret = 1;
                                break;
                        }
                    }
                }
            }
            
            if(al_current_time() - start_time > fixed_dt) //break if we start taking too long
                break;
        }
        
        //al_clear_to_color(al_map_rgba_f(0.5, 0.5, 0.7, 1));
        /*
         Draw the gui
         */
        if(!done){
            wz_draw(gui);
            al_wait_for_vsync();
            al_flip_display();
        }
    }
    
    al_destroy_bitmap(skin_theme.box_bitmap);
    al_destroy_bitmap(skin_theme.button_up_bitmap);
    al_destroy_bitmap(skin_theme.button_down_bitmap);
    wz_destroy_skin_theme(&skin_theme);
    wz_destroy(gui);
    return ret;
}


void draw_multiline_wz_box(const char *text, int cx, int cy, int width)
{
    // Initialize Allegro 5 and the font routines
    int font_size = 30;
    WZ_WIDGET *gui, *wgt;
    WZ_SKIN_THEME skin_theme;
    ALLEGRO_FONT *font;
    int text_h;
    
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size);
    
    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    skin_theme.theme.font = font;
    skin_theme.theme.color1 = GUI_BG_COLOR;
    skin_theme.theme.color2 = GUI_TEXT_COLOR;
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);
    
    text_h = 1 + get_multiline_text_lines(font, width - 40, text);
    text_h *= al_get_font_line_height(font);
    text_h += 40;
    
    gui = wz_create_widget(0, cx - width/2, cy-text_h/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, width, text_h, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    wgt->flags &= !WZ_STYLE_FOCUSED;
    wz_update(gui, 1);
    
    wz_draw(gui);
    draw_multiline_text_bf(font, WHITE_COLOR, cx-width/2 + 20, cy-text_h/2+20, width-40, al_get_font_line_height(font), ALLEGRO_ALIGN_LEFT, text);
    
    al_destroy_bitmap(skin_theme.box_bitmap);
    wz_destroy_skin_theme(&skin_theme);
    wz_destroy(gui);
}

