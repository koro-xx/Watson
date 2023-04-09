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

#define al_rewind(x) al_fseek(x,0,ALLEGRO_SEEK_SET)

extern ALLEGRO_FONT *default_font;


typedef struct MemFile{
    void *mem;
    int64_t size;
} MemFile;


// Prototypes

// Init mouse, kbd, addons, set path to resources path (or cwd), etc
int init_allegro(void);

// adapter = 0 for first desktop
void get_desktop_resolution(int adapter, int *w, int *h);

// get best fullscreen resolution
void get_highest_resolution(int *w, int *h);

void wait_for_keypress(void);

// wait for any input
// if queue = NULL will use own queue, with installed input devices
// otherwise uses provided queue with registered input devices
void wait_for_input(ALLEGRO_EVENT_QUEUE *queue);
MemFile create_memfile(const char* filename);
int init_fonts(void);
ALLEGRO_FONT *load_font_mem(MemFile font_mem, const char *filename, int size);

ALLEGRO_USTR *new_ustr(const char *str);
void free_ustr(void);

// clones the target bitmap (usually the display backbuffer)
ALLEGRO_BITMAP *screenshot();
ALLEGRO_BITMAP *screenshot_part(int x, int y, int w, int h);

ALLEGRO_BITMAP *scaled_clone_bitmap(ALLEGRO_BITMAP *source, int w, int h);


    
// variables
extern ALLEGRO_FONT *default_font;
extern MemFile text_font_mem;
extern MemFile tile_font_mem;
extern const char *TILE_FONT_FILE;
extern const char *TEXT_FONT_FILE;




#endif /* defined(__freesherlock__allegro_stuff__) */
