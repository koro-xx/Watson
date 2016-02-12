#ifndef __watson__text__
#define __watson__text__
#include "allegro_stuff.h"

// works like al_draw_multiline_text but it allows the tag %b in the format string
// and a list of pointers to bitmaps which are placed (scaled to the font size) at each %b
// can take a maximum of 32 bitmaps
void draw_multiline_text_bf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, ...);

// like draw_multiline_text_bf but takes a va_list instead of a variable list of parameters
void draw_multiline_text_vbf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, va_list ap);

int get_multiline_text_lines(const ALLEGRO_FONT *font, float max_width, const char *text);

#endif /* defined(__watson__text__) */
