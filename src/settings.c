// Based in example.c
// from the WidgetZ GUI library by Pavel Sountsov

// * deal with window resize
// * pre-initialize ustr[blah] with enum, and destroy at end

#include "settings.h"
#include <stdio.h>
#include <math.h>
#include "widgetz/widgetz.h"
#include "allegro_stuff.h"
#include "main.h"
#include "text.h"

const char ABOUT_TEXT[] = "Watson v" PRE_VERSION " - " PRE_DATE ", by Koro.\n"
"\n"
"Watson is an open source clone of \"Sherlock\", an old game by Evertt Kaser which is itself based on the a classic puzzle known as \"Zebra puzzle\" or \"Einstein's riddle\".\n"
"\n"
"Watson is programmed in plain C with the Allegro 5 library. Big thanks to the friendly folks from Freenode #allegro for all the tips and advice.\n"
"\n"
"The tile set is rendered from TTF fonts obtained from fontlibrary.org. Pressing the game will switch to custom tiles, which sould be stored in <appdir>/icons into 8 separate folders numbered 0 to 7, each with 8 square tiles in .png format. Of course, these won't look as nice as the fonts due to the anti-aliasing. The tiles provided here were downloaded from www.icons8.com.\n"
"\n"
"The \"advanced\" mode generates (much) harder puzzles, to the point of being almost impossible, so it needs to be tuned.";


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
    GROUP_ROWS
};


void multiline_text_box(const char *text, int cx, int cy, int width, ALLEGRO_EVENT_QUEUE *queue);

int yes_no_gui(ALLEGRO_USTR *text, int center_x, int center_y, int min_width, ALLEGRO_EVENT_QUEUE *queue);

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
    
    
    //xxx todo: change this factor depending on dpi???
    size = min(0.7*(float)b->xsize/gui_w, 0.8*(float)b->ysize/400.0);
    
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size * size);
    
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
    skin_theme.theme.color1 = al_map_rgb(120, 120, 70);//al_map_rgb(153, 127, 102);//(0.1, 0.6, 0.8, 1);
    skin_theme.theme.color2 = al_map_rgba_f(1, 1, 1, 1);
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);
// main gui
    gui = wz_create_widget(0, b->all.x + (b->xsize-size*gui_w)/2, b->all.y + (b->ysize-size*400)/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);

    wz_create_fill_layout(gui, 0, 0, gui_w * size, 50 * size, 10 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 200 * size, 40 * size, al_ustr_new("About Watson"), 1, BUTTON_ABOUT);

    wz_create_fill_layout(gui, 0, 50 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_LEFT, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Rows:"), 1, -1);
    button_rows[0] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("4"), 1, GROUP_ROWS, 4);
    button_rows[1] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("5"), 1, GROUP_ROWS, 5);
    button_rows[2] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("6"), 1, GROUP_ROWS, 6);
    button_rows[3] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("7"), 1,  GROUP_ROWS, 7);
    button_rows[4] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("8"), 1,  GROUP_ROWS, 8);

    wz_create_fill_layout(gui, 0, 120 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_LEFT, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Columns:"), 1, -1);
    button_cols[0] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("4"), 1, GROUP_COLS, 4);
    button_cols[1] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, al_ustr_new("5"), 1, GROUP_COLS, 5);
    button_cols[2] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("6"), 1, GROUP_COLS, 6);
    button_cols[3] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("7"), 1, GROUP_COLS, 7);
    button_cols[4] = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50* size, 50 * size, al_ustr_new("8"), 1, GROUP_COLS, 8);
    
    ((WZ_BUTTON*)button_cols[set->h-4])->down=1;
    ((WZ_BUTTON*)button_rows[set->n-4])->down=1;
    
    wz_create_fill_layout(gui, 0, 190 * size, gui_w * size, 70 * size, 10 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Sound:"), 1, -1);
    button_mute = (WZ_WIDGET*) wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, set->sound_mute ? al_ustr_new("off") : al_ustr_new("on"), 1, -1, BUTTON_SOUND);
    ((WZ_BUTTON*) button_mute)->down = !set->sound_mute;
    
    wz_create_textbox(gui, 0, 0, 100 * size, 50 * size, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Advanced:"), 1, -1);
    button_advanced = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, 50 * size, 50 * size, set->advanced ? al_ustr_new("on") : al_ustr_new("off"), 1, -1, BUTTON_ADVANCED);
    ((WZ_BUTTON*) button_advanced)->down = set->advanced;
    
    wz_create_fill_layout(gui, 0, 260 * size, gui_w * size, 70 * size, 30 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("New game"), 1, BUTTON_RESTART);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Exit game"), 1, BUTTON_EXIT);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Switch tiles"), 1, BUTTON_TILES);
    
    wz_create_fill_layout(gui, 0, 330 * size, gui_w * size, 70 * size, 30 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET*) wz_create_button(gui, 0, 0, 120 * size, 50 * size, al_ustr_new("Cancel"), 1, BUTTON_CANCEL);
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
                                
                            case BUTTON_ABOUT:
                                al_pause_event_queue(queue,1);
                                multiline_text_box(ABOUT_TEXT, b->all.x+b->xsize/2, b->all.y + b->ysize/2, 0.6*b->xsize, queue);
                                al_pause_event_queue(queue,0);
                                break;
                            case BUTTON_TILES:
                                emit_event(EVENT_SWITCH_TILES);
                                done=1;
                                break;
                            case BUTTON_RESTART:
                                if(!yes_no_gui(al_ustr_newf("Start new %dx%d%s game?", nset.n, nset.h, nset.advanced ? " advanced" : ""), b->all.x+b->xsize/2, b->all.y+b->ysize/2, b->xsize*0.3, queue))
                                    break;
                                emit_event(EVENT_RESTART);
                                nset.restart=1;
                                done=1;
                                cancel=0;
                                break;
                            case BUTTON_ADVANCED:
                                SWITCH(nset.advanced);
                                if(!nset.advanced)
                                    wz_set_text((WZ_WIDGET *) event.user.data2, al_ustr_new("off"));
                                else
                                    wz_set_text((WZ_WIDGET *) event.user.data2, al_ustr_new("on"));
                                break;
                                
                            case BUTTON_SOUND:
                                SWITCH(nset.sound_mute);
                                if(nset.sound_mute){
                                    wz_set_text((WZ_WIDGET*) event.user.data2, al_ustr_new("off"));
                                } else {
                                    wz_set_text((WZ_WIDGET*) event.user.data2, al_ustr_new("on"));
                                }
                                break;
                            case BUTTON_OK:
                                if( (nset.n != set->n) || (nset.h != set->h) || (nset.advanced != set->advanced) )
                                    if(!yes_no_gui(al_ustr_newf("Start new %dx%d%s game?", nset.n, nset.h, nset.advanced ? " advanced" : ""), b->all.x+b->xsize/2, b->all.y+b->ysize/2, b->xsize*0.3, queue))
                                        break;
                                cancel=0;
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
    
    if(!cancel) *set = nset;
//    
//    if(!cancel){
//        // update data from gui
//        nset->advanced = ((WZ_BUTTON *)button_advanced)->down;
//        nset->sound_mute = !((WZ_BUTTON *)button_mute)->down;
//        for(i=0; i<5;i++)
//        {
//            if(((WZ_BUTTON *)button_rows[i])->down)
//            {
//                nset->n=i+4;
//                break;
//            }
//        }
//        
//        for(i=0; i<5;i++){
//            if(((WZ_BUTTON *)button_cols[i])->down)
//            {
//                nset->h=i+4;
//                break;
//            }
//        }
//    }
//    
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
    skin_theme.theme.color1 = al_map_rgb(120, 120, 70);//(0.1, 0.6, 0.8, 1);
    skin_theme.theme.color2 = al_map_rgba_f(1, 1, 1, 1);
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);
    
    gui = wz_create_widget(0, center_x-size*gui_w/2, center_y-size*200/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    
    wz_create_box(gui, 0, 0, gui_w * size, 200 * size, -1);
    wz_create_textbox(gui, 0, 0, gui_w * size, 100 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, text, 1, -1);
    wgt = (WZ_WIDGET *) wz_create_fill_layout(gui, 0, 100 * size, gui_w * size, 100 * size, 30*size, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    wgt->flags = WZ_STATE_LAYOUT | WZ_STATE_HIDDEN;
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
            ALLEGRO_EVENT event;
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
    free(text);
    wz_destroy(gui);
    return ret;
}

void multiline_text_box(const char *text, int cx, int cy, int width, ALLEGRO_EVENT_QUEUE *queue)
{
    // Initialize Allegro 5 and the font routines
    int font_size = 30;
    WZ_WIDGET* gui;
    WZ_SKIN_THEME skin_theme;
    ALLEGRO_FONT *font;
    int text_h;
    
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size);
    
    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    skin_theme.theme.font = font;
    skin_theme.theme.color1 = al_map_rgb(70, 70, 50);//(0.1, 0.6, 0.8, 1);
    skin_theme.theme.color2 = al_map_rgba_f(1, 1, 1, 1);
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    
    wz_init_skin_theme(&skin_theme);
    
    text_h = 1 + get_multiline_text_lines(font, width - 40, text);
    text_h *= al_get_font_line_height(font);
    text_h += 40;
    
    gui = wz_create_widget(0, cx - width/2, cy-text_h/2, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    
    wz_create_box(gui, 0, 0, width, text_h, -1);

    wz_update(gui, 0.01);
    
    wz_draw(gui);
    draw_multiline_text_bf(font, WHITE_COLOR, cx-width/2 + 20, cy-text_h/2+20, width-40, al_get_font_line_height(font), ALLEGRO_ALIGN_LEFT, text);
    al_wait_for_vsync();
    al_flip_display();
    wait_for_input(); // fix this to take existing queue if not null
    
    al_destroy_bitmap(skin_theme.box_bitmap);
    wz_destroy_skin_theme(&skin_theme);
    wz_destroy(gui);
}
