#include "bitmaps.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include "allegro_stuff.h"
#include "dialog.h"
#include "text.h"
#include <allegro5/allegro_memfile.h>

typedef enum SYMBOL{
    SYM_FORBIDDEN,
    SYM_SWAPPABLE,
    SYM_ONE_SIDE,
    NUMBER_OF_SYMBOLS
} SYMBOL;

ALLEGRO_COLOR INFO_TEXT_COLOR = {1,1,1,1};
ALLEGRO_COLOR TILE_GENERAL_BD_COLOR = {0,0,0,1};


// temporary bmp to store icons
ALLEGRO_BITMAP *basic_bmp[8][8];
ALLEGRO_BITMAP *symbol_bmp[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

int TILE_SHADOWS = 1;
int GLYPH_SHADOWS = 0;

const float SHADOW_ALPHA = 0.3;
//const char *TEXT_FONT_FILE = "fonts/aileron-regular.otf";



// prototypes

int make_clue_bitmaps(Game *g, Board *b);

char *CLUE_BG_COLOR[8]={
    "808080",
    "5522F0",
    "008080",
    "840543",
    "BB6000",
    "DAB520",
    "FFB6C1",
    "9ACD32"
};

//    "800080",
char *CLUE_FG_COLOR[8]={
    "FFFFFF",
    "FFFFFF",
    "FFFFFF",
    "FFFFFF",
    "FFFFFF",
    "000000",
    "000000",
    "000000",
};

char *CLUE_BG_COLOR_BMP[8]={
    "999999",
    "999999",
    "999999",
    "999999",
    "999999",
    "999999",
    "999999",
    "999999",
};


//char *CLUE_BG_COLOR_BMP[8]={
//    "777777",
//    "555555",
//    "777777",
//    "555555",
//    "777777",
//    "555555",
//    "777777",
//    "555555"
//};


char *CLUE_CODE[8][8] = {
    {"A", "B", "C", "D", "E", "F", "G", "H"},
    {"Q", "R", "S", "T", "U", "V", "W", "X"},
    {"I", "J", "K", "L", "M", "N", "O", "P"},
    {"a", "b", "c", "d", "e", "f", "g", "h"},
    {"1", "2", "3", "4", "5", "6", "7", "8"},
    {"!", "\"", "#", "$", "\%", "&", "\'", "("},
    {"i", "j", "k", "l", "m", "n", "o", "p"},
    {"q", "r", "s", "t", "u", "v", "w", "x"}
};


void destroy_all_bitmaps(Board *b){
    int i,j;
    
    if(b->type_of_tiles != 0){
        for(i=0;i<b->h;i++){
            for(j=0;j<b->n;j++){
                ndestroy_bitmap(basic_bmp[i][j]);
            }
        }
    }
    
    for(i=0; i<NUMBER_OF_SYMBOLS; i++){
        ndestroy_bitmap(symbol_bmp[i]);
    }
    
    for(i=0; i<4; i++){
        ndestroy_bitmap(b->button_bmp[i]);
        ndestroy_bitmap(b->button_bmp_scaled[i]);
    }

    destroy_board_bitmaps(b);
}

void update_timer(Board *b) {
	int t;
	ALLEGRO_BITMAP *bmp = al_get_target_bitmap();
	t=al_get_time()-b->time_start;
	al_set_target_bitmap(*b->time_panel.b[0]->bmp);
	al_clear_to_color(b->time_panel.b[0]->bg_color);
	al_draw_textf(default_font, WHITE_COLOR,  b->time_panel.b[0]->w/2, 0, ALLEGRO_ALIGN_CENTER, "%02d:%02d:%02d", t/3600, t/60, t%60);
	al_set_target_bitmap(bmp);
}

void destroy_board_bitmaps(Board *b){
    int i,j;
    
    
    for(i=0;i<b->h;i++){
        for(j=0;j<b->n;j++){
            ndestroy_bitmap(b->panel_tile_bmp[i][j]);
            ndestroy_bitmap(b->clue_unit_bmp[i][j]);
            ndestroy_bitmap(b->guess_bmp[i][j]);
        }
    }
    
    for(i=0; i<b->hclue_n+b->vclue_n; i++){
        ndestroy_bitmap(b->clue_bmp[i]);
    }
    
    for(i=0; i<NUMBER_OF_SYMBOLS; i++){
        ndestroy_bitmap(b->symbol_bmp[i]);
    }
    
    destroy_board_clue_blocks(b);
    
    ndestroy_bitmap(b->time_bmp);
    ndestroy_bitmap(b->info_text_bmp);
    al_destroy_font(b->text_font);
}


int init_bitmaps(Board *b){
    // will load bitmaps from folders named 0, 1,..., 7
    // inside the folder "icons", each containing 8 square bitmaps
    ALLEGRO_FS_ENTRY *dir = NULL;
    ALLEGRO_FS_ENTRY *file;
    ALLEGRO_FILE *fp;
    int j, k=0;
    char pathname[1000];
    ALLEGRO_PATH *path;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();
    
#ifdef ALLEGRO_ANDROID
    al_android_set_apk_file_interface();
#endif

    // create buttons
    // xxx todo: improve these
    
    b->info_text_bmp = NULL;
    b->info_panel.bmp = NULL;
    
    // if this fails, buttons will be created anyway at update_bitmaps
    b->button_bmp[0] = al_load_bitmap("buttons/question.png");
    b->button_bmp[1] = al_load_bitmap("buttons/light-bulb.png");
    b->button_bmp[2] = al_load_bitmap("buttons/gear.png");
    b->button_bmp[3] = al_load_bitmap("buttons/undo.png");
    
    if(b->type_of_tiles == 2)
        return init_bitmaps_classic(b);
    
    if(b->type_of_tiles == 1){ // use bitmaps
        path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
        al_path_cstr(path, '/');
        
        
        for(j=0; j<b->h; j++){
            snprintf(pathname, 999, "%s/icons/%d", al_path_cstr(path, '/'), j);
            if(al_filename_exists(pathname)){
                dir = al_create_fs_entry(pathname);
                if(al_open_directory(dir)){
                    k=0;
                    while((file = al_read_directory(dir)) ){
                        fp = al_open_fs_entry(file, "rb");
                        basic_bmp[j][k] = al_load_bitmap_f(fp, ".png");
                        al_fclose(fp);
						if (!basic_bmp[j][k]) continue;
                        k++;
                    }
                }
                if(k<b->n){
                    fprintf(stderr, "Error loading 8 bitmaps in folder %d\n", j);
                    return -1;
                }
            } else {
                if(k<b->n){
                    fprintf(stderr, "Bitmap folder \"%s\" not found\n", pathname);
                    return -1;
                }
            }
            al_destroy_fs_entry(dir);
        }
        al_destroy_path(path);
    }
    
    // create symbols (alternatively we could load these from files!))
    symbol_bmp[SYM_FORBIDDEN] = al_create_bitmap(256, 256);
    symbol_bmp[SYM_SWAPPABLE] = al_create_bitmap(3*256 + 2*b->clue_unit_space, 256);
    symbol_bmp[SYM_ONE_SIDE] = al_create_bitmap(256, 256);
    
    if( (!symbol_bmp[SYM_FORBIDDEN]) || (!symbol_bmp[SYM_SWAPPABLE]) || (!symbol_bmp[SYM_ONE_SIDE]) ){
        fprintf(stderr, "Error creating bitmap.\n");
        return -1;
    }
    al_set_target_bitmap(symbol_bmp[SYM_FORBIDDEN]);
    al_clear_to_color(NULL_COLOR);
    al_draw_line(1, 1, 254, 254, al_map_rgba_f(1,0,0,0.5),4);
    al_draw_line(1, 254, 254, 1, al_map_rgba_f(1,0,0,0.5),4);
    
    al_set_target_bitmap(symbol_bmp[SYM_SWAPPABLE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_line(256*0.7,256*0.9, 256*(3-0.7),256*0.9, al_map_rgba_f(1,0,0,0.5), 2);
    al_draw_filled_triangle(256*0.5,256*0.9, 256*0.7,256, 256*0.7, 256*0.8, al_map_rgba_f(1,0,0,0.35));
    
    
    al_set_target_bitmap(symbol_bmp[SYM_ONE_SIDE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_filled_circle(256/2, 256/2, 0.03*256, WHITE_COLOR);
    al_draw_filled_circle(256/2 - 0.2*256, 256/2, 0.03*256, WHITE_COLOR);
    al_draw_filled_circle(256/2 + 0.2*256, 256/2, 0.03*256, WHITE_COLOR);
    
    al_set_target_bitmap(dispbuf);
    return 0;
}


void draw_shadow(int w, int h, int t){
    float a=(float)t*0.5;
    al_draw_line(a, a, a, (h-a), al_premul_rgba_f(1,1,1,SHADOW_ALPHA),t);
    al_draw_line(a, a, (w-a), a, al_premul_rgba_f(1,1,1,SHADOW_ALPHA),t);
    al_draw_line(a , h-a, w-a,h-a, al_map_rgba_f(0,0,0,SHADOW_ALPHA),t);
    al_draw_line(w-a, a, w-a,h-a, al_map_rgba_f(0,0,0,SHADOW_ALPHA),t);
}

ALLEGRO_COLOR invert_color(ALLEGRO_COLOR c){
    return (ALLEGRO_COLOR){1-c.r, 1-c.g, 1-c.b, c.a};
}


int update_font_bitmaps(Game *g, Board *b){
    int i, j, s;
    float FONT_FACTOR=1;
    ALLEGRO_FONT *tile_font1, *tile_font2, *tile_font3;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();
    int bbx, bby, bbw, bbh;
    
    s=min(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h);
    tile_font1 = load_font_mem(tile_font_mem, TILE_FONT_FILE, -s*FONT_FACTOR);
    tile_font2 = load_font_mem(tile_font_mem, TILE_FONT_FILE, -b->panel_tile_size*FONT_FACTOR);
    tile_font3 = load_font_mem(tile_font_mem, TILE_FONT_FILE, -b->clue_unit_size*FONT_FACTOR);

    if(!tile_font1 || !tile_font2 || !tile_font3) {
        fprintf(stderr, "Error loading tile font file %s.\n", TILE_FONT_FILE);
        return -1;
    }
    
    for(i=0;i<b->h;i++){
        for(j=0;j<b->n;j++){
            b->guess_bmp[i][j] = al_create_bitmap(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h);
            b->panel_tile_bmp[i][j] = al_create_bitmap(b->panel_tile_size,b->panel_tile_size);
            b->clue_unit_bmp[i][j] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
            if(!b->guess_bmp[i][j] || !b->panel_tile_bmp[i][j] || !b->clue_unit_bmp[i][j]){
                fprintf(stderr, "Error creating bitmap.\n");
                return -1;
            }
            
            // guessed bitmaps
            al_set_target_bitmap(b->guess_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR[i]));
            al_get_glyph_dimensions(tile_font1, CLUE_CODE[i][j][0], &bbx, &bby, &bbw, &bbh);
            if(GLYPH_SHADOWS){
                al_draw_glyph(tile_font1, DARK_GREY_COLOR, (b->panel.b[0]->b[0]->w-bbw)/2 -bbx +1,(b->panel.b[0]->b[0]->h-bbh)/2-bby+1, CLUE_CODE[i][j][0]);
            }
            al_draw_glyph(tile_font1, al_color_html(CLUE_FG_COLOR[i]), (b->panel.b[0]->b[0]->w-bbw)/2 -bbx,(b->panel.b[0]->b[0]->h-bbh)/2-bby, CLUE_CODE[i][j][0]);
            // this draws a border for all tiles, independent of the "bd" setting in b
            if(TILE_SHADOWS)
                draw_shadow(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h,2);
            else
                al_draw_rectangle(.5,.5,b->panel.b[0]->b[0]->w-.5, b->panel.b[0]->b[0]->h-.5, TILE_GENERAL_BD_COLOR,1);
            
            // panel bitmaps
    
            al_set_target_bitmap(b->panel_tile_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR[i]));
            al_get_glyph_dimensions(tile_font2, CLUE_CODE[i][j][0], &bbx, &bby, &bbw, &bbh);
            if(GLYPH_SHADOWS){
                al_draw_glyph(tile_font2, DARK_GREY_COLOR,(b->panel_tile_size -bbw)/2-bbx+1, (b->panel_tile_size - bbh)/2-bby+1, CLUE_CODE[i][j][0]);
            }
            al_draw_glyph(tile_font2, al_color_html(CLUE_FG_COLOR[i]),(b->panel_tile_size -bbw)/2-bbx, (b->panel_tile_size - bbh)/2-bby, CLUE_CODE[i][j][0]);
            if(TILE_SHADOWS)
                draw_shadow(b->panel_tile_size, b->panel_tile_size,2);
            else
                al_draw_rectangle(.5,.5,b->panel_tile_size-.5,b->panel_tile_size-.5, TILE_GENERAL_BD_COLOR,1);
            
            // clue unit tile bitmaps
            al_set_target_bitmap(b->clue_unit_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR[i]));
            al_get_glyph_dimensions(tile_font3, CLUE_CODE[i][j][0], &bbx, &bby, &bbw, &bbh);
            if(GLYPH_SHADOWS){
                al_draw_glyph(tile_font3, DARK_GREY_COLOR, (b->clue_unit_size-bbw)/2 -bbx +1,(b->clue_unit_size-bbh)/2-bby+1, CLUE_CODE[i][j][0]);
            }
            al_draw_glyph(tile_font3, al_color_html(CLUE_FG_COLOR[i]), (b->clue_unit_size-bbw)/2 -bbx,(b->clue_unit_size-bbh)/2-bby, CLUE_CODE[i][j][0]);
            if(TILE_SHADOWS)
                draw_shadow(b->clue_unit_size,b->clue_unit_size,2);
            else
                al_draw_rectangle(.5,.5,b->clue_unit_size-.5, b->clue_unit_size-.5, TILE_GENERAL_BD_COLOR,1);
        }
    }
    
    // create symbols
    b->symbol_bmp[SYM_FORBIDDEN] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
    b->symbol_bmp[SYM_SWAPPABLE] = al_create_bitmap(3*b->clue_unit_size + 2*b->clue_unit_space, b->clue_unit_size);
    b->symbol_bmp[SYM_ONE_SIDE] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
    
    if( (!b->symbol_bmp[SYM_FORBIDDEN]) || (!b->symbol_bmp[SYM_SWAPPABLE]) || (!b->symbol_bmp[SYM_ONE_SIDE]) ){
        fprintf(stderr, "Error creating bitmap.\n");
        return -1;
    }
    
    al_set_target_bitmap(b->symbol_bmp[SYM_FORBIDDEN]);
    al_clear_to_color(NULL_COLOR);
    al_draw_line(0, 0, b->clue_unit_size, b->clue_unit_size,al_map_rgba_f(1,0,0,0.5),4);
    al_draw_line(0, b->clue_unit_size, b->clue_unit_size,0, al_map_rgba_f(1,0,0,0.5),4);
    
    al_set_target_bitmap(b->symbol_bmp[SYM_SWAPPABLE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_line(b->clue_unit_size*0.7,b->clue_unit_size*0.9, b->clue_unit_size*(3-0.7),b->clue_unit_size*0.9, al_map_rgba_f(1,0,0,0.5), 2);
    al_draw_filled_triangle(b->clue_unit_size*0.5,b->clue_unit_size*0.9, b->clue_unit_size*0.7,b->clue_unit_size*0.97, b->clue_unit_size*0.7, b->clue_unit_size*0.83, al_map_rgba_f(1,0,0,0.5));
    al_draw_filled_triangle(b->clue_unit_size*2.5,b->clue_unit_size*0.9, b->clue_unit_size*2.3,b->clue_unit_size*0.97, b->clue_unit_size*2.3, b->clue_unit_size*0.83, al_map_rgba_f(1,0,0,0.5));
    
    al_set_target_bitmap(b->symbol_bmp[SYM_ONE_SIDE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_textf(tile_font3, WHITE_COLOR, b->clue_unit_size/2,b->clue_unit_size/2-al_get_font_line_height(tile_font3)/2, ALLEGRO_ALIGN_CENTER, "%s", "...");
    al_draw_line(b->clue_unit_size*0.2,b->clue_unit_size*0.5, b->clue_unit_size*0.7,b->clue_unit_size*0.5, al_map_rgba_f(1,0,0,1), 2);
    al_draw_filled_triangle(b->clue_unit_size*0.6,b->clue_unit_size*0.45, b->clue_unit_size*0.6,b->clue_unit_size*0.55, b->clue_unit_size*0.8, b->clue_unit_size*0.5, al_map_rgba_f(1,0,0,1));
    
    al_destroy_font(tile_font1);
    al_destroy_font(tile_font2);
    al_destroy_font(tile_font3);
    
    al_set_target_backbuffer(al_get_current_display());
    // create clue tile bmps
    al_set_target_bitmap(dispbuf);
    return make_clue_bitmaps(g, b);
    
}

int make_clue_bitmaps(Game *g, Board *b){
    int i;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();

    for(i=0;i<g->clue_n; i++){
        b->clue_bmp[i] = al_create_bitmap(b->clue_tiledblock[i]->w, b->clue_tiledblock[i]->h);
        if(!b->clue_bmp[i]){
            fprintf(stderr, "Error creating clue bitmap.\n");
            return -1;
        }
        al_set_target_bitmap(b->clue_bmp[i]);
        al_clear_to_color(b->clue_tiledblock[i]->bg_color);
        switch(g->clue[i].rel){
            case NEXT_TO:
            case CONSECUTIVE:
            case NOT_NEXT_TO:
            case NOT_MIDDLE:
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[0]][g->clue[i].k[0]], 0, 0, 0);
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[1]][g->clue[i].k[1]], b->clue_unit_size + b->clue_unit_space, 0, 0);
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[2]][g->clue[i].k[2]], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
                if(g->clue[i].rel == NOT_NEXT_TO){
                    al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, 0, 0);
                    al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
                } else if(g->clue[i].rel == NOT_MIDDLE){
                    al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], b->clue_unit_size + b->clue_unit_space, 0, 0);
                }
                
                if((g->clue[i].rel == NOT_MIDDLE) || (g->clue[i].rel == CONSECUTIVE))
                    al_draw_bitmap(b->symbol_bmp[SYM_SWAPPABLE], 0,0,0);
                
                break;
            case ONE_SIDE:
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[0]][g->clue[i].k[0]], 0, 0, 0);
                al_draw_bitmap(b->symbol_bmp[SYM_ONE_SIDE], b->clue_unit_size + b->clue_unit_space, 0, 0);
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[1]][g->clue[i].k[1]], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
                break;
            case TOGETHER_3:
            case TOGETHER_NOT_MIDDLE:
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[2]][g->clue[i].k[2]], 0, 2*(b->clue_unit_size + b->clue_unit_space), 0);
            case TOGETHER_2:
            case NOT_TOGETHER:
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[0]][g->clue[i].k[0]], 0, 0, 0);
                al_draw_bitmap(b->clue_unit_bmp[g->clue[i].j[1]][g->clue[i].k[1]], 0, b->clue_unit_size + b->clue_unit_space, 0);
                if((g->clue[i].rel==NOT_TOGETHER) || (g->clue[i].rel==TOGETHER_NOT_MIDDLE))
                    al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, b->clue_unit_size + b->clue_unit_space, 0);
                break;
                
            case TOGETHER_FIRST_WITH_ONLY_ONE: // temporary debug bmp, shouldn't show up
                al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, b->clue_unit_size + b->clue_unit_space, 0);
                break;
            default:
                break;
        }        
    }
  
    al_set_target_bitmap(dispbuf);
    return 0;
}


void show_info_text(Board *b, const char* msg){
    ALLEGRO_FONT *font;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();
    
    font = b->text_font ? b->text_font : default_font;
    ndestroy_bitmap(b->info_text_bmp);
    b->info_text_bmp = al_create_bitmap(b->info_panel.w, b->info_panel.h);
    al_set_target_bitmap(b->info_text_bmp);
    al_clear_to_color(b->info_panel.bg_color);
    draw_multiline_text_bf(font, INFO_TEXT_COLOR, 10, 3, b->info_panel.w-30, al_get_font_line_height(b->text_font), ALLEGRO_ALIGN_LEFT, msg);
    
    b->info_panel.bmp = &b->info_text_bmp; // make it show in the info_panel
    al_set_target_bitmap(dispbuf);
}

void clear_info_panel(Board *b){
    b->info_panel.bmp = NULL;
}

void show_info_text_b(Board *b, const char* msg, ...){
    ALLEGRO_FONT *font;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();
    va_list ap;
    
    font = b->text_font ? b->text_font : default_font;
    ndestroy_bitmap(b->info_text_bmp);
    b->info_text_bmp = al_create_bitmap(b->info_panel.w, b->info_panel.h);
    al_set_target_bitmap(b->info_text_bmp);
    al_clear_to_color(b->info_panel.bg_color);
    va_start(ap, msg);
    draw_multiline_text_vbf(font, INFO_TEXT_COLOR, 10, 3, b->info_panel.w-30, al_get_font_line_height(font), ALLEGRO_ALIGN_LEFT, msg, ap);
    va_end(ap);
    b->info_panel.bmp = &b->info_text_bmp; // make it show in the info_panel
    al_set_target_bitmap(dispbuf);
}




int update_bitmaps(Game *g, Board *b){
    int i, j, s;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();

    
    // reload text fonts
    if(!(b->text_font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -min(b->info_panel.h/2.2, (float)b->info_panel.w/30)))){
        fprintf(stderr, "Error loading font %s.\n", TEXT_FONT_FILE);
    }
    
    // update buttons and timer bmps
    b->time_bmp = al_create_bitmap(b->time_panel.b[0]->w, b->time_panel.b[0]->h);
	al_set_target_bitmap(b->time_bmp);
	al_clear_to_color(b->time_panel.b[0]->bg_color);
    
    for(i=0; i<4; i++){
        b->button_bmp_scaled[i] = scaled_clone_bitmap(b->button_bmp[i], b->time_panel.b[i+1]->w, b->time_panel.b[i+1]->h);
    }
    
	al_set_target_bitmap(dispbuf);

	if (b->type_of_tiles == 0) { // use font bitmaps
		return update_font_bitmaps(g, b);
	}

    // else update normal bitmaps:
    
    s=min(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h);
    for(i=0;i<b->h;i++){
        for(j=0;j<b->n;j++){
            b->guess_bmp[i][j] = al_create_bitmap(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h);
            b->panel_tile_bmp[i][j] = al_create_bitmap(b->panel_tile_size,b->panel_tile_size);
            b->clue_unit_bmp[i][j] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
            if(!b->guess_bmp[i][j] || !b->panel_tile_bmp[i][j] || !b->clue_unit_bmp[i][j]){
                fprintf(stderr, "Error creating bitmap.\n");
                return -1;
            }
            
            // guessed bitmaps
            al_set_target_bitmap(b->guess_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR_BMP[i]));
            if(TILE_SHADOWS)
                draw_shadow(b->panel.b[0]->b[0]->w, b->panel.b[0]->b[0]->h,2);
            else
                al_draw_rectangle(.5,.5,b->panel.b[0]->b[0]->w-.5, b->panel.b[0]->b[0]->h-.5, TILE_GENERAL_BD_COLOR,1);
            
            al_draw_scaled_bitmap(basic_bmp[i][j], 0, 0, al_get_bitmap_width(basic_bmp[i][j]), al_get_bitmap_height(basic_bmp[i][j]), (b->panel.b[0]->b[0]->w-s)/2, (b->panel.b[0]->b[0]->h-s)/2, s, s, 0);
            
            // panel bitmaps
            al_set_target_bitmap(b->panel_tile_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR_BMP[i]));
            if(TILE_SHADOWS)
                draw_shadow(b->panel_tile_size, b->panel_tile_size,1);
            else
                al_draw_rectangle(.5,.5,b->panel_tile_size-.5,b->panel_tile_size-.5, TILE_GENERAL_BD_COLOR,1);
            
            
            
            al_draw_scaled_bitmap(basic_bmp[i][j], 0, 0, al_get_bitmap_width(basic_bmp[i][j]), al_get_bitmap_height(basic_bmp[i][j]), 0, 0, b->panel_tile_size, b->panel_tile_size,0);
            
            // clue unit tile bitmaps
            al_set_target_bitmap(b->clue_unit_bmp[i][j]);
            al_clear_to_color(al_color_html(CLUE_BG_COLOR_BMP[i]));
            if(TILE_SHADOWS)
                draw_shadow(b->clue_unit_size,b->clue_unit_size,2);
            else
                al_draw_rectangle(.5,.5,b->clue_unit_size-.5, b->clue_unit_size-.5, TILE_GENERAL_BD_COLOR,1);

            al_draw_scaled_bitmap(basic_bmp[i][j], 0, 0, al_get_bitmap_width(basic_bmp[i][j]), al_get_bitmap_height(basic_bmp[i][j]), 0, 0, b->clue_unit_size, b->clue_unit_size,0);
        }
    }
    
    // create symbols (alternatively we could load these as bitmaps earlier))
    //xxx todo: i have already loaded symbols in symbol_bmp. We can skip this and change the creation of tiles to use scaled bitmaps
    b->symbol_bmp[SYM_FORBIDDEN] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
    b->symbol_bmp[SYM_SWAPPABLE] = al_create_bitmap(3*b->clue_unit_size + 2*b->clue_unit_space, b->clue_unit_size);
    b->symbol_bmp[SYM_ONE_SIDE] = al_create_bitmap(b->clue_unit_size, b->clue_unit_size);
    
    if( (!b->symbol_bmp[SYM_FORBIDDEN]) || (!b->symbol_bmp[SYM_SWAPPABLE]) || (!b->symbol_bmp[SYM_ONE_SIDE]) ){
        fprintf(stderr, "Error creating bitmap.\n");
        return -1;
    }
    al_set_target_bitmap(b->symbol_bmp[SYM_FORBIDDEN]);
    al_clear_to_color(NULL_COLOR);
    if(b->type_of_tiles == 2){ // clasic tiles
        al_draw_scaled_bitmap(symbol_bmp[SYM_FORBIDDEN], 0, 0, 80, 80, 0,0,b->clue_unit_size, b->clue_unit_size, 0);
    } else {
        al_draw_line(0, 0, b->clue_unit_size, b->clue_unit_size,al_map_rgba_f(1,0,0,0.5),4);
        al_draw_line(0, b->clue_unit_size, b->clue_unit_size,0, al_map_rgba_f(1,0,0,0.5), 4);
    }
    
    al_set_target_bitmap(b->symbol_bmp[SYM_SWAPPABLE]);
    al_clear_to_color(NULL_COLOR);
    if(b->type_of_tiles == 2){ // clasic tiles
        al_draw_scaled_bitmap(symbol_bmp[SYM_SWAPPABLE], 0, 0, 3*80, 80, 0,0,3*b->clue_unit_size, b->clue_unit_size, 0);
    } else {
        al_draw_line(b->clue_unit_size*0.7,b->clue_unit_size*0.9, b->clue_unit_size*(3-0.7),b->clue_unit_size*0.9, al_map_rgba_f(1,0,0,0.5), 2);
        al_draw_filled_triangle(b->clue_unit_size*0.5,b->clue_unit_size*0.9, b->clue_unit_size*0.7,b->clue_unit_size, b->clue_unit_size*0.7, b->clue_unit_size*0.8, al_map_rgba_f(1,0,0,0.35));
        al_draw_filled_triangle(b->clue_unit_size*2.5,b->clue_unit_size*0.9, b->clue_unit_size*2.3,b->clue_unit_size, b->clue_unit_size*2.3, b->clue_unit_size*0.8, al_map_rgba_f(1,0,0,0.35));
    }
    
    al_set_target_bitmap(b->symbol_bmp[SYM_ONE_SIDE]);
    al_clear_to_color(NULL_COLOR);
    if(b->type_of_tiles == 2){ // clasic tiles
        al_draw_scaled_bitmap(symbol_bmp[SYM_ONE_SIDE], 0, 0, 80, 80, 0,0,b->clue_unit_size, b->clue_unit_size, 0);
    } else {
        
        al_draw_filled_circle(b->clue_unit_size/2, b->clue_unit_size/2, 0.03*b->clue_unit_size, WHITE_COLOR);
        al_draw_filled_circle(b->clue_unit_size/2 - 0.2*b->clue_unit_size, b->clue_unit_size/2, 0.03*b->clue_unit_size, WHITE_COLOR);
        al_draw_filled_circle(b->clue_unit_size/2 + 0.2*b->clue_unit_size, b->clue_unit_size/2, 0.03*b->clue_unit_size, WHITE_COLOR);
    }
    
    al_set_target_bitmap(dispbuf);
    // create clue tile bmps
    return make_clue_bitmaps(g, b);
}

int init_bitmaps_classic(Board *b){
    ALLEGRO_BITMAP *test_bmp;
    int i,j;
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();

    // tile_file.bmp should be a bmp with 80x80 tiles, 10 rows of 8 tiles
    // the first row of tiles is ignored. Rows 2 to 9 are the game tiles
    // the last row should contain the extra symbols
    // b->clue_unit_space must be 0
    
    if( !(test_bmp = al_load_bitmap("tile_file.bmp")) ){
        fprintf(stderr, "Error loading tile_file.bmp.\n");
        return -1;
    }
    ALLEGRO_COLOR trans = al_get_pixel (test_bmp, 2*80, 9*80 + 40);
    al_convert_mask_to_alpha(test_bmp, trans);
    
    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            // create basic bitmaps from big file
            basic_bmp[i][j] = al_create_bitmap(80, 80);
            al_set_target_bitmap(basic_bmp[i][j]);
            al_clear_to_color(NULL_COLOR);
            al_draw_bitmap_region(test_bmp, j*80, (i+1)*80, 80, 80, 0,0,0);
        }
    }
    
    // create symbols
    symbol_bmp[SYM_FORBIDDEN] = al_create_bitmap(80,80);
    al_set_target_bitmap(symbol_bmp[SYM_FORBIDDEN]);
    al_clear_to_color(NULL_COLOR);
    al_draw_bitmap_region(test_bmp, 80, 9*80, 80, 80, 0, 0, 0);
    
    symbol_bmp[SYM_SWAPPABLE] = al_create_bitmap(3*80, 80);
    al_set_target_bitmap(symbol_bmp[SYM_SWAPPABLE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_bitmap_region(test_bmp, 2*80, 9*80, 3*80, 80, 0, 0,0);
    
    symbol_bmp[SYM_ONE_SIDE] = al_create_bitmap(80,80);
    al_set_target_bitmap(symbol_bmp[SYM_ONE_SIDE]);
    al_clear_to_color(NULL_COLOR);
    al_draw_bitmap_region(test_bmp, 5*80, 9*80, 80, 80, 0, 0, 0);
    
    al_set_target_bitmap(dispbuf);
    return 0;
};

void draw_title(void) {
	ALLEGRO_FONT *font = NULL;
	int w;
	int dw = (al_get_display_width(al_get_current_display())-500)/2;
	int dh = (al_get_display_height(al_get_current_display()) - 250) / 2;

	if (!(font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -64))) {
		fprintf(stderr, "Error loading font %s.\n", TEXT_FONT_FILE);
		return;
	}

	al_clear_to_color(BLACK_COLOR);
	for (w = 0; w<150; w++) {
		al_draw_filled_rectangle(w+dw, w / 2 + dh, 500 - w+dw, 250 - w / 2+dh, al_map_rgba_f(0.03, 0.01, 0.01, 0.04));
	}
    al_draw_rectangle(dw, dh, dw+500, dh+250, al_map_rgba_f(.5, .5, 0, 1), 2);
	w = al_get_text_width(font, "WATSON");
	al_draw_textf(font, al_color_html("#576220"), (500 - w) / 2+dw, (250 - 64) / 2+dh, ALLEGRO_ALIGN_LEFT, "WATSON");
	al_draw_textf(font, al_color_html("#F0C010"), (500 - w) / 2 - 3+dw, (250 - 64) / 2 - 3+dh, ALLEGRO_ALIGN_LEFT, "WATSON");
	al_destroy_font(font);

}



// debug
ALLEGRO_BITMAP *get_clue_bitmap(Board *b, Clue *clue){
    int w,h;
    int size = b->clue_unit_size;
    if(is_vclue(clue->rel)){ w = size; h = 3*size; } else {w = 3*size; h = size; }
    ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();

    ALLEGRO_BITMAP *clue_bmp = al_create_bitmap(w, h);
    al_set_target_bitmap(clue_bmp);
    al_clear_to_color(NULL_COLOR);
    
    switch(clue->rel){
        case NEXT_TO:
        case CONSECUTIVE:
        case NOT_NEXT_TO:
        case NOT_MIDDLE:
            al_draw_bitmap(b->clue_unit_bmp[clue->j[0]][clue->k[0]], 0, 0, 0);
            al_draw_bitmap(b->clue_unit_bmp[clue->j[1]][clue->k[1]], b->clue_unit_size + b->clue_unit_space, 0, 0);
            al_draw_bitmap(b->clue_unit_bmp[clue->j[2]][clue->k[2]], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
            if(clue->rel == NOT_NEXT_TO){
                al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, 0, 0);
                al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
            } else if(clue->rel == NOT_MIDDLE){
                al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], b->clue_unit_size + b->clue_unit_space, 0, 0);
            }
            
            if((clue->rel == NOT_MIDDLE) || (clue->rel == CONSECUTIVE))
                al_draw_bitmap(b->symbol_bmp[SYM_SWAPPABLE], 0,0,0);
            
            break;
        case ONE_SIDE:
            al_draw_bitmap(b->clue_unit_bmp[clue->j[0]][clue->k[0]], 0, 0, 0);
            al_draw_bitmap(b->symbol_bmp[SYM_ONE_SIDE], b->clue_unit_size + b->clue_unit_space, 0, 0);
            al_draw_bitmap(b->clue_unit_bmp[clue->j[1]][clue->k[1]], 2*(b->clue_unit_size + b->clue_unit_space), 0, 0);
            break;
        case TOGETHER_3:
        case TOGETHER_NOT_MIDDLE:
            al_draw_bitmap(b->clue_unit_bmp[clue->j[2]][clue->k[2]], 0, 2*(b->clue_unit_size + b->clue_unit_space), 0);
        case TOGETHER_2:
        case NOT_TOGETHER:
            al_draw_bitmap(b->clue_unit_bmp[clue->j[0]][clue->k[0]], 0, 0, 0);
            al_draw_bitmap(b->clue_unit_bmp[clue->j[1]][clue->k[1]], 0, b->clue_unit_size + b->clue_unit_space, 0);
            if((clue->rel==NOT_TOGETHER) || (clue->rel==TOGETHER_NOT_MIDDLE))
                al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, b->clue_unit_size + b->clue_unit_space, 0);
            break;
            
        case TOGETHER_FIRST_WITH_ONLY_ONE: // temporary debug bmp, shouldn't show up
            al_draw_bitmap(b->symbol_bmp[SYM_FORBIDDEN], 0, b->clue_unit_size + b->clue_unit_space, 0);
            break;
        default:
            break;
    }
    
    al_set_target_bitmap(dispbuf);
    return clue_bmp;
}


void convert_grayscale(ALLEGRO_BITMAP *bmp) {
	ALLEGRO_BITMAP *dispbuf = al_get_target_bitmap();
	int x, y, w, h, lum;
	unsigned char r, g, b;
	w = al_get_bitmap_width(bmp);
	h = al_get_bitmap_height(bmp);

	al_set_target_bitmap(bmp);
	al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_READWRITE);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			al_unmap_rgb(al_get_pixel(bmp, x, y), &r, &g, &b);
			lum = (0.299*r + 0.587*g + 0.114*b)/2; // dividing by two makes it darker, default should be not dividing
			al_put_pixel(x, y, al_map_rgb(lum, lum, lum)); // RGB to grayscale
		}
	}
	al_unlock_bitmap(bmp);
	al_set_target_bitmap(dispbuf);
}


