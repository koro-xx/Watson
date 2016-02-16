#pragma once
#include "widgetz.h"

#ifdef __cplusplus
extern "C" {
#endif

int wz_ask_parent_for_focus(WZ_WIDGET* wgt);//asks parent to focus the widget
void wz_ask_parent_to_focus_next(WZ_WIDGET* wgt);//asks parent to focus the next widget after this one
void wz_ask_parent_to_focus_prev(WZ_WIDGET* wgt);//asks parent to focus the previous widget before this one

void wz_craft_event(ALLEGRO_EVENT* event, int type, WZ_WIDGET* source, intptr_t data);
WZ_WIDGET* wz_get_widget_dir(WZ_WIDGET* wgt, int dir);

ALLEGRO_COLOR wz_blend_colors(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac);
int wz_get_text_pos(ALLEGRO_FONT* font, ALLEGRO_USTR* text, float x);

void wz_def_draw_box(struct WZ_THEME* theme, float x, float y, float w, float h, int style);
void wz_def_draw_button(WZ_THEME* theme, float x, float y, float w, float h, ALLEGRO_USTR* text, int style);

int wz_box_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_widget_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_toggle_button_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_button_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_fill_layout_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_scroll_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_textbox_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);
int wz_editbox_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event);

void wz_init_widget(WZ_WIDGET* wgt, WZ_WIDGET* parent, float x, float y, float w, float h, int id);
void wz_init_grid_layout(WZ_FILL_LAYOUT* box, WZ_WIDGET* parent, float x, float y, float w, float h, float hspace, float vspace, int halign, int valign, int id);
void wz_init_box(WZ_WIDGET* wgt, WZ_WIDGET* parent, float x, float y, float w, float h, int id);
void wz_init_textbox(WZ_TEXTBOX* box, WZ_WIDGET* parent, float x, float y, float w, float h, int halign, int valign, ALLEGRO_USTR* text, int own, int id);
void wz_init_button(WZ_BUTTON* but, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int id);
void wz_init_scroll(WZ_SCROLL* scl, WZ_WIDGET* parent, float x, float y, float w, float h, int max_pos, int slider_size, int id);
void wz_init_toggle_button(WZ_TOGGLE* tog, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int group, int id);
void wz_init_editbox(WZ_EDITBOX* box, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int id);
void wz_init_layout_stop(WZ_WIDGET* box, WZ_WIDGET* parent, int id);
void wz_init_image_button(WZ_IMAGE_BUTTON* but, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_BITMAP* normal, ALLEGRO_BITMAP* down, ALLEGRO_BITMAP* focused, ALLEGRO_BITMAP* disabled, int id);

#ifdef __cplusplus
}
#endif
