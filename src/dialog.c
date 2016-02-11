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

#define MAX_BF_BITMAPS 32
#define BF_CODEPOINT_START 0xA78D

void draw_text_bf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, ...){
    va_list ap;
    ALLEGRO_BITMAP *tmp[MAX_BF_BITMAPS], *bmp, *currbuf = al_get_target_bitmap();
    ALLEGRO_USTR *ustr = al_ustr_new("");
    ALLEGRO_FONT *newfont;
    char *fmt_cpy = malloc(sizeof(format)), *pos;
    const char *btag = format;
    int i, bn = 0;
    int texth = al_get_font_line_height(font);
    int bitmap_w, x, bw, bh;
    int range[2];
    
    strcpy(fmt_cpy, format);

    pos = fmt_cpy;
    va_start(ap, format);
    while( (btag = strstr(btag, "%b")) ){
        pos[btag - format] = '\0';
        al_ustr_append_cstr(ustr, pos);
        al_ustr_append_chr(ustr, BF_CODEPOINT_START + bn);
        pos += btag-format + 2;
        tmp[bn] = va_arg(ap, ALLEGRO_BITMAP *);
        bn++;
        btag+=2;
    }
    al_ustr_append_cstr(ustr, pos);

    
    bitmap_w = 1;
    for(i=0; i<bn; i++){
        bitmap_w += 1 + al_get_bitmap_width(tmp[i])*(float) texth / al_get_bitmap_height(tmp[i]);
    }
    
    bmp = al_create_bitmap(texth+2, bitmap_w);
    al_set_target_bitmap(bmp);
    al_clear_to_color(NULL_COLOR);
    x=0;
    for(i=0; i<bn; i++){
        x+=1;
        bw = al_get_bitmap_width(tmp[i]);
        bh = al_get_bitmap_height(tmp[i]);
        al_draw_scaled_bitmap(tmp[i], 0, 0, bw, bh, x+1, 1, bw*(float) texth/bh, texth, 0);
    }
    
    range[0] = BF_CODEPOINT_START;
    range[1] = BF_CODEPOINT_START + bn-1;
    newfont = al_grab_font_from_bitmap(bmp, 1, range);
    al_set_fallback_font(font, newfont);
    al_draw_multiline_ustr(font, color, tx, ty, tw, th, flags, ustr);
    al_draw_multiline_text(font, WHITE_COLOR, 0, 0, 500, 50, ALLEGRO_ALIGN_LEFT, "test");
    
    al_set_fallback_font(font, NULL);
    al_destroy_bitmap(bmp);
    al_destroy_font(newfont);
    al_ustr_free(ustr);
    free(fmt_cpy);
    al_flip_display();
    al_rest(5);
    al_set_target_bitmap(currbuf);
}

/* This helper function helps splitting an ustr in several delimited parts.
 * It returns an ustr that refers to the next part of the string that
 * is delimited by the delimiters in delimiter.
 * Returns NULL at the end of the string.
 * Pos is updated to byte index of character after the delimiter or
 * to the end of the string.
 */
static const ALLEGRO_USTR *ustr_split_next(const ALLEGRO_USTR *ustr,
                                           ALLEGRO_USTR_INFO *info, int *pos, const char *delimiter)
{
    const ALLEGRO_USTR *result;
    int end, size;
    
    size = al_ustr_size(ustr);
    if (*pos >= size) {
        return NULL;
    }
    
    end = al_ustr_find_set_cstr(ustr, *pos, delimiter);
    if (end == -1)
        end = size;
    
    result = al_ref_ustr(info, ustr, *pos, end);
    /* Set pos to character AFTER delimiter */
    al_ustr_next(ustr, &end);
    (*pos) = end;
    return result;
}

//xxx todo:
//void text_box(const char *text){
//    
//}