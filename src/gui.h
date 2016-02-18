#ifndef __watson_tests__gui__
#define __watson_tests__gui__

#include "board.h"

// Prototypes
void show_help(Board *b, ALLEGRO_EVENT_QUEUE *queue);
void show_about(Board *b, ALLEGRO_EVENT_QUEUE *queue);
void draw_center_textbox_wait(const char *text, float width_factor, Board *b, ALLEGRO_EVENT_QUEUE *queue);

    
int show_settings(Settings *set, Board *b, ALLEGRO_EVENT_QUEUE *queue);
void draw_multiline_wz_box(const char *text, int cx, int cy, int width);
int yes_no_gui(ALLEGRO_USTR *text, int center_x, int center_y, int min_width, ALLEGRO_EVENT_QUEUE *queue);
int confirm_restart(Board *b, Settings *set, ALLEGRO_EVENT_QUEUE *queue);
int confirm_exit(Board *b, ALLEGRO_EVENT_QUEUE *queue);

// wrapper for WidgetZ gui to use cstr instead of ustr


#endif /* defined(__watson_tests__gui__) */
