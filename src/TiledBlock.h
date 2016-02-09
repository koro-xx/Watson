#ifndef __TiledBlock__
#define __TiledBlock__

#include <stdio.h>
#include <allegro5/allegro5.h>
#include "macros.h"

typedef struct TiledBlock {
    int x,y,w,h;
    int margin;
    ALLEGRO_COLOR bd_color;
    ALLEGRO_COLOR bg_color; // set to something if no background bitmap
    int bd;
    int sb; // number of subblocks
    struct TiledBlock **b;
    struct TiledBlock *parent;
    int type; // a descriptor
    int index; // a number identifying it among same-type blocks
    int hidden;
    ALLEGRO_BITMAP **bmp; // set to NULL for filled background
} TiledBlock;

// find the tile at x,y. Returns an array of integers starting at p[0] representing the
// nested sequence of subblocks that leads to it. p should be an int array of size
// at least the max depth of the subblock sequence
int get_TiledBlock_tile(TiledBlock *t, int x, int y, int *p);
void draw_TiledBlock(TiledBlock *t, int x, int y);
void highlight_TiledBlock(TiledBlock *t);
void get_TiledBlock_offset(TiledBlock *t, int *x, int *y);
TiledBlock* get_TiledBlock(TiledBlock *t, int x, int y);
// returns pointer to new (malloc'd) tiled block initialized to 0 / NULL
TiledBlock *new_TiledBlock(void);
#endif /* defined(__TiledBlock__) */
