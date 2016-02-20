#ifndef watson_tests_main_h
#define watson_tests_main_h

#include "board.h"

#define PRE_VERSION "0.79.3"
#define PRE_DATE "2016-02-12"

#ifdef ALLEGRO_ANDROID
    #include <allegro5/allegro_android.h>
    #include <android/log.h>

    #define MOBILE 1
    #define deblog(x,...) __android_log_print(ANDROID_LOG_INFO,"koro: ","%s:"x, __FILE__, ##__VA_ARGS__)
    #define errlog(x,...) __android_log_print(ANDROID_LOG_INFO,"koro: ","%s:"x, __FILE__, ##__VA_ARGS__)
#else
    #define MOBILE 0
    #define deblog(x, ...) fprintf(stderr, "koro:%s:%u: "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #define errlog(x, ...) fprintf(stderr, "koro ERROR:%s:%u: "x"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif


#define DEFAULT_FONT_FILE "fonts/fixed_font.tga"

#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
#define EVENT_SWITCH_TILES (BASE_USER_EVENT_TYPE + 2)
#define EVENT_RESTART (BASE_USER_EVENT_TYPE + 3)
#define EVENT_EXIT (BASE_USER_EVENT_TYPE + 4)
#define EVENT_LOAD (BASE_USER_EVENT_TYPE + 5)
#define EVENT_SAVE (BASE_USER_EVENT_TYPE + 6)

typedef struct Settings{
    int n;
    int h;
    int advanced;
    int sound_mute;
    int type_of_tiles;
    int fat_fingers; // todo: implement zoom of tiledblocks for small screens
    int restart;
    int saved; // is there a saved game?
} Settings;

void emit_event(int event_type);
void draw_stuff(Board *b);

#endif
