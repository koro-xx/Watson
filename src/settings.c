#include "settings.h"
#include <stdio.h>
#include <math.h>
#include "widgetz/widgetz.h"
#include "allegro_stuff.h"

int show_settings(Board *b, ALLEGRO_EVENT_QUEUE *queue)
{
    // Initialize Allegro 5 and the font routines
    int refresh_rate;
    float size = 2.0;
    int font_size = 18;
    double fixed_dt;
    double old_time;
    double game_time;
    double start_time;
    WZ_WIDGET* gui;
    WZ_DEF_THEME theme;
    WZ_SKIN_THEME skin_theme;
    WZ_WIDGET* wgt;
    bool done = false;
    bool fancy_theme = true;
    ALLEGRO_FONT *font;
    
    font = load_font_mem(text_font_mem, TEXT_FONT_FILE, font_size * size);

#if (ALLEGRO_SUB_VERSION > 0)
    if(al_install_touch_input())
        al_register_event_source(queue, al_get_touch_input_event_source());
#endif
    refresh_rate = 60;
    fixed_dt = 1.0f / refresh_rate;
    old_time = al_current_time();
    game_time = al_current_time();
    /*
     Define custom theme
     wz_def_theme is a global vtable defined by the header
     */
    memset(&theme, 0, sizeof(theme));
    memcpy(&theme, &wz_def_theme, sizeof(theme));
    theme.font = font;
    theme.color1 = al_map_rgba_f(0, 0.6, 0, 1);
    theme.color2 = al_map_rgba_f(1, 1, 0, 1);
    
    /*
     Define a custom skin theme
     wz_skin_theme is a global vtable defined by the header
     */
    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    skin_theme.theme.font = font;
    skin_theme.theme.color1 = al_map_rgba_f(0, 0.6, 0, 1);
    skin_theme.theme.color2 = al_map_rgba_f(1, 1, 0, 1);
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    skin_theme.editbox_bitmap = al_load_bitmap("data/editbox.png");
    skin_theme.scroll_track_bitmap = al_load_bitmap("data/scroll_track.png");
    skin_theme.slider_bitmap = al_load_bitmap("data/slider.png");
    
    wz_init_skin_theme(&skin_theme);
    
    /*
     Define root gui element
     */
    gui = wz_create_widget(0, 0, 0, -1);
    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    /*
     Define all other gui elements, fill_layout is an automatic layout container
     */
    wz_create_fill_layout(gui, 50 * size, 50 * size, 300 * size, 450 * size, 50 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, 200 * size, 50 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new("Welcome to WidgetZ!"), 1, -1);
    wz_create_toggle_button(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Toggle 1"), 1, 1, 5);
    wz_create_toggle_button(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Toggle 2"), 1, 1, 6);
    wz_create_toggle_button(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Toggle 3"), 1, 1, 7);
    wz_create_button(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Switch Themes"), 1, 666);
    wgt = (WZ_WIDGET*)wz_create_button(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Quit"), 1, 1);
    wz_set_shortcut(wgt, ALLEGRO_KEY_Q, ALLEGRO_KEYMOD_CTRL);
    wz_create_fill_layout(gui, 350 * size, 50 * size, 300 * size, 450 * size, 50 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, 200 * size, 50 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new("Scroll Bars:"), 1, -1);
    wz_create_scroll(gui, 0, 0, 200 * size, 20 * size, 20, 50 * size, 9);
    wz_create_scroll(gui, 0, 0, 20 * size, 200 * size, 20, 50 * size, 9);
    wz_create_scroll(gui, 0, 0, 20 * size, 200 * size, 20, 50 * size, 9);
    wz_create_fill_layout(gui, 650 * size, 50 * size, 300 * size, 450 * size, 20 * size, 20 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, 200 * size, 50 * size, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new("Edit Box:"), 1, -1);
    wgt = (WZ_WIDGET*)wz_create_editbox(gui, 0, 0, 200 * size, 50 * size, al_ustr_new("Type here..."), 1, -1);
    wz_create_textbox(gui, 0, 0, 200 * size, 50 * size, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, al_ustr_new("A textbox with a lot of text"
                                                                                                 " in it. Also supports new lines:\n\nNew paragraph.\n"
                                                                                                 "Also supports unicode:\n\n"
                                                                                                 "Привет"), 1, -1);
    /*
     Register the gui with the event queue
     */
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
            
            while(al_get_next_event(queue, &event))
            {
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
                            case 1:
                            {
                                done = true;
                                break;
                            }
                            case 666:
                            {
                                fancy_theme = !fancy_theme;
                                if(fancy_theme)
                                    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
                                else
                                    wz_set_theme(gui, (WZ_THEME*)&theme);
                            }
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
        wz_draw(gui);
        al_wait_for_vsync();
        al_flip_display();
    }
    
    al_destroy_bitmap(skin_theme.box_bitmap);
    al_destroy_bitmap(skin_theme.button_up_bitmap);
    al_destroy_bitmap(skin_theme.button_down_bitmap);
    al_destroy_bitmap(skin_theme.scroll_track_bitmap);
    al_destroy_bitmap(skin_theme.slider_bitmap);
    al_destroy_bitmap(skin_theme.editbox_bitmap);
    wz_destroy_skin_theme(&skin_theme);
    
    /*
     Destroy the gui
     */
    wz_destroy(gui);
    /*
     Deinit Allegro 5
     */
    return 0;
}

