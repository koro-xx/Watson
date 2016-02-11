#include<allegro5/allegro_font.h>

#ifndef __watson__dialog__
#define __watson__dialog__

int yes_no_dialog(const char *text);
void  draw_text_bf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, int tx, int ty, int tw, int th, int flags, const char *format, ...);

#endif /* defined(__watson__dialog__) */
