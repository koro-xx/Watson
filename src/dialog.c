#include "dialog.h"
#include "allegro_stuff.h"

/* xxx todo: improve dialog options */

int yes_no_dialog(const char *text){
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_EVENT ev;
    int noexit;
    int yes_x, yes_y, no_x, no_y;
    int dw = al_get_bitmap_width(al_get_target_bitmap());
    int dh = al_get_bitmap_height(al_get_target_bitmap());
    int ret=0;
    int bh = 120, bw = 400;
    
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());

//    al_clear_to_color(NULL_COLOR);
    al_draw_filled_rectangle((dw - bw)/2, (dh-bh)/2, (dw+bw)/2, (dh+bh)/2, WINDOW_BG_COLOR);
    al_draw_rectangle((dw - bw)/2, (dh-bh)/2, (dw+bw)/2, (dh+bh)/2, WINDOW_BD_COLOR,3);
    
    al_draw_multiline_text(default_font, al_map_rgb_f(1,1,1), dw/2, (dh-bh)/2+20, 360, 18, ALLEGRO_ALIGN_CENTER, text);
    
    yes_x = (dw-bw)/2 + 64;
    yes_y = (dh+bh)/2 - 44;
    no_x = (dw+bw)/2 - 128;
    no_y = (dh+bh)/2 - 44;
    
    al_draw_rectangle(yes_x, yes_y, yes_x+64, yes_y+24, al_map_rgb_f(1,1,1), 1);
    al_draw_rectangle(no_x, no_y, no_x+64, no_y+24, al_map_rgb_f(1,1,1), 1);
    al_draw_text(default_font, al_map_rgb_f(1,1,1), yes_x+32, yes_y+4, ALLEGRO_ALIGN_CENTER, "OK");
    al_draw_text(default_font, al_map_rgb_f(1,1,1), no_x+32, no_y+4, ALLEGRO_ALIGN_CENTER, "Cancel");
    
    al_flip_display();
    
    noexit=1;
    while(noexit){
        al_wait_for_event(queue, &ev);
        if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
            if( (ev.mouse.x > yes_x) && (ev.mouse.x < yes_x+64) && (ev.mouse.y > yes_y) && (ev.mouse.y < yes_y+24) ){
                ret = 1;
                noexit = 0;
            } else if( (ev.mouse.x > no_x) && (ev.mouse.x < no_x+64) && (ev.mouse.y > no_y) && (ev.mouse.y < no_y+24) ){
                ret = 0; noexit = 0;
            }
        }
        if(ev.type == ALLEGRO_EVENT_KEY_CHAR)
            noexit = 0;
    }
    al_destroy_event_queue(queue);
    return ret;
}



//xxx todo:
//void text_box(const char *text){
//    
//}