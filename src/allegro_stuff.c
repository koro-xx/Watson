#include "allegro_stuff.h"
#include "sound.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_memfile.h>
#include "main.h"

ALLEGRO_FONT *default_font = NULL;
MemFile text_font_mem={0};
MemFile tile_font_mem={0};
const char *TILE_FONT_FILE = "fonts/tiles.ttf";
const char *TEXT_FONT_FILE = "fonts/text_font.ttf";

struct Buffer_USTR{
    ALLEGRO_USTR *ustr;
    struct Buffer_USTR *next;
};

struct Buffer_USTR *buffer_ustr = NULL;

char DEFAULT_FONT_FILE[]="fonts/fixed_font.tga";

int init_fonts(void){
    
    default_font = al_load_font(DEFAULT_FONT_FILE, 16, 0);
    
    text_font_mem = create_memfile(TEXT_FONT_FILE);
    tile_font_mem = create_memfile(TILE_FONT_FILE);
    if(!text_font_mem.mem || !tile_font_mem.mem || !default_font){
        fprintf(stderr, "Error loading fonts.\n");
        return -1;
    }
    
    return 0;
}

ALLEGRO_FONT *load_font_mem(MemFile font_mem, const char *filename, int size){
    // filename is only to detect extension
    ALLEGRO_FILE *fp = NULL;
    ALLEGRO_FONT *font;
    
    fp = al_open_memfile(font_mem.mem, font_mem.size, "r");
    if(!fp) return NULL;
    
    font = al_load_ttf_font_f(fp, filename, size, 0);
    return font;
}

int init_allegro(void){
    ALLEGRO_PATH *path;
    int no_input = 1;
    
	if(!al_init()) {
        errlog("failed to initalize allegro!\n");
        return -1;
    }
    
    path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    al_destroy_path(path);
    
    if(!al_install_keyboard()) {
        errlog("failed to initialize the keyboard!\n");
        //return -1;
    }
    
    if(!al_install_mouse()) {
        errlog("failed to initialize the mouse.\n");
    } else no_input = 0;
    
    init_sound(); // I don't care if there was an error here.

    if(!al_install_touch_input()) {
        errlog("failed to initialize touch input.\n");
    } else no_input = 0;
    
    if(no_input){
        errlog("no input found. Exitting.");
        return -1;
    }

    init_sound(); // I don't care if there was an error here.
    
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
	
    if (!al_init_primitives_addon()) {
		errlog("Failed to initialize primitives addon.\n");
        return -1;
	}
    
    if(init_fonts()) return -1;
    
	return 0;
};


MemFile create_memfile(const char* filename){
    MemFile ret = {0};

    ALLEGRO_FILE *fp = al_fopen(filename, "r");

    if(!fp) return ret;
    ret.size = al_fsize(fp);
    ret.mem = malloc(ret.size);
    if(!ret.mem || (al_fread(fp, ret.mem, ret.size) != ret.size))
        ret.mem=NULL;
    al_fclose(fp);
    return ret;
}



// adapter = 0 for first desktop
void get_desktop_resolution(int adapter, int *w, int *h)
{
    ALLEGRO_MONITOR_INFO info;
    al_get_monitor_info(adapter, &info);
    
    *w = info.x2 - info.x1;
    *h = info.y2 - info.y1;
};

// get best fullscreen resolution
void get_highest_resolution(int *w, int *h){
    ALLEGRO_DISPLAY_MODE disp_data;
    int i;

    *w = 0;
    *h = 0;
    for(i=0; i<al_get_num_display_modes(); i++){
        al_get_display_mode(i, &disp_data);
        if(*w < disp_data.width)
            *w = disp_data.width;
    }

    if((*w == disp_data.width) && (*h < disp_data.height)){
        *h = disp_data.height;
    }
}

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
void wait_for_input(ALLEGRO_EVENT_QUEUE *queue){
    ALLEGRO_EVENT ev;
    int done = 0;
    int own_queue = queue ? 0 : 1;

    if(own_queue)
    {
        queue = al_create_event_queue();

        if(al_is_keyboard_installed())
            al_register_event_source(queue, al_get_keyboard_event_source());
        if(al_is_mouse_installed())
            al_register_event_source(queue, al_get_mouse_event_source());
        if(al_is_touch_input_installed())
            al_register_event_source(queue, al_get_touch_input_event_source());
    }
    
    while(!done)
    {
        while(!al_peek_next_event(queue, &ev))
            al_rest(0.001);
        
        switch(ev.type)
        {
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
            case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
                if(!own_queue) done = 1;
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            case ALLEGRO_EVENT_KEY_CHAR:
            case ALLEGRO_EVENT_TOUCH_BEGIN:
                done = 1;
            default:
                al_drop_next_event(queue);
        }
    }
    
    if(own_queue) al_destroy_event_queue(queue);
}

ALLEGRO_USTR *new_ustr(const char *str){
    struct Buffer_USTR *buf = malloc(sizeof(*buf));
    buf->ustr = al_ustr_new(str);
    buf->next = buffer_ustr;
    buffer_ustr = buf;
    return buf->ustr;
}

void free_ustr(void){
    while(buffer_ustr){
        al_ustr_free(buffer_ustr->ustr);
        buffer_ustr = buffer_ustr->next;
    }
}

ALLEGRO_BITMAP *screenshot(){
    int store = al_get_new_bitmap_format();
    ALLEGRO_BITMAP *ret;
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_888);
    ret = al_clone_bitmap(al_get_target_bitmap());
    al_set_new_bitmap_format(store);
    return ret;
}

ALLEGRO_BITMAP *screenshot_part(int x, int y, int w, int h){
    int store = al_get_new_bitmap_format();
    ALLEGRO_BITMAP *ret;
    ALLEGRO_BITMAP *currbuf = al_get_target_bitmap();
    
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_888);
    ret = al_create_bitmap(w,h);
    al_set_target_bitmap(ret);
    al_draw_bitmap_region(currbuf, x, y, w, h, 0, 0, 0);
    al_set_target_bitmap(currbuf);
    al_set_new_bitmap_format(store);
    return ret;
}

ALLEGRO_BITMAP *scaled_clone_bitmap(ALLEGRO_BITMAP *source, int w, int h){
    ALLEGRO_BITMAP *currbuf = al_get_target_bitmap();
    ALLEGRO_BITMAP *ret = al_create_bitmap(w,h);
    al_set_target_bitmap(ret);
    al_clear_to_color(NULL_COLOR);
    al_draw_scaled_bitmap(source, 0, 0, al_get_bitmap_width(source), al_get_bitmap_height(source), 0, 0, w, h, 0);
    al_set_target_bitmap(currbuf);
    return ret;
}
