#include "dialog.h"
#include "allegro_stuff.h"
#include "text.h"

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

// width_factor is the preferred width (portion of the screen width), but if text won't fit
// it will increase width until it fits
void draw_center_text_box(ALLEGRO_FONT *font, ALLEGRO_COLOR text_color, ALLEGRO_COLOR bg_color, ALLEGRO_COLOR bd_color, float width_factor, const char *text){
    int dw = al_get_bitmap_width(al_get_target_bitmap());
    int dh = al_get_bitmap_height(al_get_target_bitmap());
    float factor=width_factor;
    int w,h;
    
    if(!font) return;
    
    do{
        w=dw*factor;
        h = get_multiline_text_lines(font,  w-40, text) * al_get_font_line_height(font) + 30;
        factor += 0.05;
    }while( (h > dh*0.9) && (factor < 1) );
    
    al_draw_filled_rectangle((dw-w)/2, (dh-h)/2, (dw+w)/2, (dh+h)/2, bg_color);
    al_draw_rectangle((dw-w)/2, (dh-h)/2, (dw+w)/2, (dh+h)/2, bd_color, 3);
    al_draw_multiline_text(font, text_color, (dw-w+40)/2, (dh-h+30)/2, w-30, al_get_font_line_height(font), ALLEGRO_ALIGN_LEFT, text);
}