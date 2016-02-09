#include "allegro_stuff.h"
#include "sound.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

ALLEGRO_FONT *default_font = NULL;
char DEFAULT_FONT_FILE[]="fonts/fixed_font.tga";

int init_allegro(void){
    ALLEGRO_PATH *path;

	if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }
    
    path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    al_destroy_path(path);
    
    if(!al_install_keyboard()) {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }
    
    if(!al_install_mouse()) {
        fprintf(stderr, "failed to initialize the mouse!\n");
        return -1;
    }
    
    init_sound(); // I don't care if there was an error here.
    
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
	if (!al_init_primitives_addon()) {
		fprintf(stderr, "Failed to initialize primitives addon.\n");
	}

    if(!default_font){
        if( !(default_font = al_load_font(DEFAULT_FONT_FILE, 16, 0)) ){
            fprintf(stderr, "Error loading font %s\n", DEFAULT_FONT_FILE);
            return -1;
        }
    }

	return 0;
};

// adapter = 0 for first desktop
void get_desktop_resolution(int adapter, int *w, int *h)
{
    ALLEGRO_MONITOR_INFO info;
    al_get_monitor_info(adapter, &info);
    
    *w = info.x2 - info.x1;
    *h = info.y2 - info.y1;
};

// quick helpful thingy
void wait_for_keypress()
{
    ALLEGRO_EVENT_QUEUE *queue = NULL;
    queue = al_create_event_queue();
    al_register_event_source(queue,al_get_keyboard_event_source());
    al_wait_for_event(queue,NULL);
    al_destroy_event_queue(queue);
}

//wait for keypress or mouse click
void wait_for_input(){
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_EVENT ev;
    
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    
    do{
        al_wait_for_event(queue, &ev);
    } while( (ev.type != ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) && (ev.type != ALLEGRO_EVENT_KEY_CHAR) );
    al_destroy_event_queue(queue);
}