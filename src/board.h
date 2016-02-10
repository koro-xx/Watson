#ifndef __freesherlock__board__
#define __freesherlock__board__

#include <stdio.h>
#include "game.h"
#include "TiledBlock.h"
#include "macros.h"
#include <allegro5/allegro_font.h>

typedef struct Board {
    int n;
    int h;
    int xsize, ysize;
    int max_xsize, max_ysize;
    TiledBlock panel;
    TiledBlock vclue;
    TiledBlock hclue;
    TiledBlock info_panel;
    TiledBlock time_panel;
    TiledBlock s; // settings
    TiledBlock all;
    TiledBlock **clue_tiledblock; // pointer to the tiled block where clue is
    TiledBlock *dragging;
    TiledBlock *highlight;
    TiledBlock *rule_out;
    int panel_tile_size;
    int clue_unit_size;
    int clue_unit_space;
    int hclue_n; // number of hclues
    int vclue_n; // number of vclue
    int dragging_ox, dragging_oy; // initial position of TiledBlock being dragged
    int dragging_cx, dragging_cy; // relative poisition of grabbing point
    int type_of_tiles; // 0 = use ttf font, 1 = use file bitmaps, 2 = use classic tiles from grid
    float time_start;
    int restart;
    int show_help;
    int show_settings;
    ALLEGRO_COLOR bg_color;
    ALLEGRO_BITMAP *panel_tile_bmp[8][8];
    ALLEGRO_BITMAP *guess_bmp[8][8];
    ALLEGRO_BITMAP *clue_unit_bmp[8][8];
    ALLEGRO_BITMAP *symbol_bmp[8];
    ALLEGRO_BITMAP **clue_bmp;
    ALLEGRO_BITMAP *button_bmp[3];
    ALLEGRO_BITMAP *time_bmp;
    ALLEGRO_BITMAP *info_text_bmp;
    ALLEGRO_BITMAP *s_bmp[14]; // settings block bitmaps
    ALLEGRO_FONT *text_font;
} Board;


typedef enum BLOCK_TYPE{
    TB_OTHER = 0, // don't change this
    TB_PANEL,
    TB_PANEL_COLUMN,
    TB_PANEL_BLOCK,
    TB_PANEL_TILE,
    TB_HCLUEBOX,
    TB_HCLUE_TILE,
    TB_VCLUEBOX,
    TB_VCLUE_TILE,
    TB_INFO_PANEL,
    TB_TIME_PANEL,
    TB_BUTTON_SETTINGS,
    TB_BUTTON_HELP,
    TB_BUTTON_CLUE,
    TB_TIMER,
    TB_ALL,
    TB_SETTINGS_OK,
    TB_SETTINGS_CANCEL,
    TB_SETTINGS_SOUND,
    TB_SETTINGS_ROWS,
    TB_SETTINGS_COLUMNS,
    TB_SETTINGS_ADVANCED,
    TB_SETTINGS_ABOUT,
    TB_SETTINGS
}BLOCK_TYPE;

//Prototypes
int create_board(Game *g, Board *b, int mode); // mode = 0: update, 1: create
void destroy_board(Board *b);
void destroy_board_clue_blocks(Board *b);
    
#endif /* defined(__freesherlock__board__) */
