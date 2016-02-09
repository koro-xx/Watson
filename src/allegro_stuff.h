#ifndef __freesherlock__allegro_stuff__
#define __freesherlock__allegro_stuff__

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "macros.h"

#define NULL_COLOR (al_map_rgba_f(0,0,0,0))
#define WHITE_COLOR (al_map_rgba_f(1,1,1,1))
#define WINE_COLOR (al_map_rgba_f(0.4, 0.1, 0.1, 1))
#define GREY_COLOR (al_map_rgba_f(0.5, 0.5, 0.5, 1))
#define WINDOW_BG_COLOR (al_map_rgba_f(0.3,0.3,0.3,1))
#define WINDOW_BD_COLOR (al_map_rgba_f(1,1,1,1))
#define DARK_GREY_COLOR (al_map_rgba_f(.2,.2,.2,1))
#define BLACK_COLOR (al_map_rgb(0,0,0))

ALLEGRO_FONT *default_font;

// Prototypes

// Init mouse, kbd, addons, set path to resources path (or cwd), etc
int init_allegro(void);

// adapter = 0 for first desktop
void get_desktop_resolution(int adapter, int *w, int *h);
void wait_for_keypress(void);
// wait for keypress or mouse click:
void wait_for_input(void);

// variables
extern ALLEGRO_FONT *default_font;

#endif /* defined(__freesherlock__allegro_stuff__) */
