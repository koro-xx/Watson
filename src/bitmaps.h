//
//  bitmaps.h
//  watson
//
//  Created by koro on 2/4/16.
//  Copyright (c) 2016 koro. All rights reserved.
//

#ifndef __watson__bitmaps__
#define __watson__bitmaps__

#include <stdio.h>
#include "game.h"
#include "macros.h"
#include "board.h"
#include "TiledBlock.h"

void destroy_board_bitmaps(Board *b);
void destroy_all_bitmaps(Board *b);
int init_bitmaps(Board *b);
int init_bitmaps_classic(Board *b);
int update_bitmaps(Game *g, Board *b);
int update_font_bitmaps(Game *g, Board *b);
void fit_board (Board *b);
ALLEGRO_BITMAP *create_title_bmp(void);
//void explain_clue(Game *g, Board *b, int i);
void destroy_settings_bitmaps(Board *b);
void create_settings_bitmaps(Board *b);
void update_timer(int t, Board *b);
void show_info_text(Board *b, ALLEGRO_USTR * msg); //msg will be freed
void show_info_text_b(Board *b, const char*msg, ...);
void clear_info_panel(Board *b);
void draw_title(void);
void convert_grayscale(ALLEGRO_BITMAP *bmp);
void create_font_symbols(Board *b);


// globals
extern char symbol_char[9][8][6];


// debug
ALLEGRO_BITMAP *get_clue_bitmap(Board *b, Clue *clue);



#endif /* defined(__watson__bitmaps__) */
