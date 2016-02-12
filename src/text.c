#include "text.h"

#define MAX_BF_BITMAPS 32
#define BF_CODEPOINT_START 0x0860

// works like al_draw_multiline_text but it allows the tag %b in the format string
// and a list of pointers to bitmaps which are placed (scaled to the font size) at each %b
// can take a maximum of 32 bitmaps
void draw_multiline_text_bf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    draw_multiline_text_vbf(font, color, tx, ty, tw, th, flags, format, ap);
    va_end(ap);
}

// like draw_multiline_text_bf but takes a va_list instead of a variable list of parameters
// the dirty trick is to use a list of unused unicode code points and create a font
// that maps them to our bitmpaps. Then use the font as a fallback font for the current one
// and replace each %b by the correpsonding unicode code.
void draw_multiline_text_vbf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, va_list ap){
    ALLEGRO_BITMAP *tmp[MAX_BF_BITMAPS], *bmp = NULL, *currbuf = al_get_target_bitmap();
    ALLEGRO_USTR *ustr = al_ustr_new("");
    ALLEGRO_FONT *newfont=NULL;
    char *fmt_cpy = malloc(strlen(format)+1), *pos;
    const char *btag = format;
    int i, bn = 0;
    int texth = al_get_font_line_height(font);
    int bitmap_w, x, bw, bh;
    int range[2];
    
    strcpy(fmt_cpy, format);
    
    pos = fmt_cpy;
    while( (btag = strstr(btag, "%b")) ){ // todo: should skip %%b
        fmt_cpy[btag - format] = '\0';
        al_ustr_append_cstr(ustr, pos);
        pos = fmt_cpy+(btag-format+2);
        al_ustr_append_chr(ustr, BF_CODEPOINT_START + bn);
        tmp[bn] = va_arg(ap, ALLEGRO_BITMAP *);
        bn++;
        btag+=2;
    }
    al_ustr_append_cstr(ustr, pos);
    
    if(bn>0){
        bitmap_w = 1;
        for(i=0; i<bn; i++){
            bitmap_w += 1 + al_get_bitmap_width(tmp[i])*(float) texth / al_get_bitmap_height(tmp[i]);
        }
        
        bmp = al_create_bitmap(bitmap_w,texth+2);
        al_set_target_bitmap(bmp);
        al_clear_to_color(NULL_COLOR);
        x=1;
        for(i=0; i<bn; i++){
            bw = al_get_bitmap_width(tmp[i]);
            bh = al_get_bitmap_height(tmp[i]);
            // the rectangle is to guarantee the right height for al_grab_font
            al_draw_rectangle(x+1.5, 1.5,  floor(x+1+bw*(float) texth/bh)-0.5, 1+texth-0.5, al_map_rgba(0,0,0,1),1);
            al_draw_scaled_bitmap(tmp[i], 0, 0, bw, bh, x, 1, bw*(float) texth/bh, texth, 0);
            x+= 2 + bw*(float) texth/bh;
        }
        
        range[0] = BF_CODEPOINT_START;
        range[1] = BF_CODEPOINT_START + bn-1;
        newfont = al_grab_font_from_bitmap(bmp, 1, range);
        al_set_fallback_font(font, newfont);
    }
    
    al_set_target_bitmap(currbuf);
    al_draw_multiline_ustr(font, color, tx, ty, tw, th, flags, ustr);
    
    if(bn>0){
        al_set_fallback_font(font, NULL);
        al_destroy_bitmap(bmp);
        al_destroy_font(newfont);
    }
    
    al_ustr_free(ustr);
    free(fmt_cpy);
}




