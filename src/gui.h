#ifndef __watson_tests__gui__
#define __watson_tests__gui__

#include "board.h"
#include "main.h"

// Prototypes

//void draw_center_textbox_wait(const char *text, float width_factor, Board *b, ALLEGRO_EVENT_QUEUE *queue);

    
//int show_settings(Settings *set, Board *b, ALLEGRO_EVENT_QUEUE *queue);
//void draw_multiline_wz_box(const char *text, int cx, int cy, int width, int max_height);
//int yes_no_gui(Board *b, ALLEGRO_USTR *text, int center_x, int center_y, int min_width, ALLEGRO_EVENT_QUEUE *queue);

void draw_guis(void);
int handle_gui_event(ALLEGRO_EVENT *event);
void gui_send_event(ALLEGRO_EVENT *event);
void init_theme(void);
void init_guis(int x, int y, int w, int h);
void update_guis(int x, int y, int w, int h);
void confirm_restart(void);
void confirm_exit(void);
void confirm_save(void);
void confirm_load(void);
void show_settings(void);
void show_help(void);
void show_about(void);

void update_base_gui(float dt);

void win_gui(Game *g, Board *b, ALLEGRO_EVENT_QUEUE *queue);
void params_gui(Board *b, ALLEGRO_EVENT_QUEUE *queue);
// wrapper for WidgetZ gui to use cstr instead of ustr

#endif /* defined(__watson_tests__gui__) */
