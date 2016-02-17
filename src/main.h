#ifndef watson_tests_main_h
#define watson_tests_main_h

#define PRE_VERSION "0.79.3"
#define PRE_DATE "2016-02-12"

#define BASE_USER_EVENT_TYPE ALLEGRO_GET_EVENT_TYPE('c','c','c','c')
#define EVENT_REDRAW (BASE_USER_EVENT_TYPE + 1)
#define EVENT_SWITCH_TILES (BASE_USER_EVENT_TYPE + 2)
#define EVENT_RESTART (BASE_USER_EVENT_TYPE + 3)
#define EVENT_EXIT (BASE_USER_EVENT_TYPE + 4)

typedef struct Settings{
    int n;
    int h;
    int advanced;
    int sound_mute;
    int type_of_tiles;
    int fat_fingers; // todo: implement zoom of tiledblocks for small screens
    int restart;
} Settings;

void emit_event(int event_type);


#endif
