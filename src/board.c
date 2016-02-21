#include "board.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include "allegro_stuff.h"
#include "bitmaps.h"
#include "main.h"

// spacing, in fraction of corresponding display dimensions
float INFO_PANEL_PORTION = 0.1;
float VCLUEBOX_PORTION = .25;
float HCLUEBOX_PORTION = .25;
int VCLUEBOX_MARGIN = 4; // pixels for now
int HCLUEBOX_MARGIN = 4;
int PANEL_MARGIN = 4;
int PANEL_COLUMN_SPACE = 4;
int PANEL_BLOCK_MARGIN = 4;
int PANEL_TILE_SPACE = 0;
int CLUE_UNIT_SPACE = 0;
int CLUE_TILE_MARGIN = 3;
int VCLUE_COLUMN_SPACE = 0;
int HCLUE_ROW_SPACE = 0;
int INFO_PANEL_MARGIN = 4;
float H_FLEX_FACTOR = 0.03;

// general tile settings
#define BG_COLOR NULL_COLOR
#define PANEL_BD_COLOR NULL_COLOR
#define PANEL_BG_COLOR DARK_GREY_COLOR
#define PANEL_COLUMN_BG_COLOR DARK_GREY_COLOR
#define PANEL_COLUMN_BD_COLOR GREY_COLOR
#define CLUE_TILE_BG_COLOR NULL_COLOR //al_map_rgb_f(.2, .2, .2)
#define CLUE_TILE_BD_COLOR NULL_COLOR
#define PANEL_TILE_BD_COLOR NULL_COLOR
#define CLUE_PANEL_BG_COLOR DARK_GREY_COLOR
#define CLUE_PANEL_BD_COLOR GREY_COLOR
#define INFO_PANEL_BG_COLOR DARK_GREY_COLOR
#define INFO_PANEL_BD_COLOR GREY_COLOR
#define TIME_PANEL_BG_COLOR DARK_GREY_COLOR
#define TIME_PANEL_BD_COLOR GREY_COLOR

#define TIME_PANEL_BUTTONS 4

int PANEL_TILE_COLUMNS[9] = {0, 1, 2, 2, 2, 3, 3, 4, 4};
int PANEL_TILE_ROWS[9] = {0, 1, 1, 2, 2, 2, 2, 2, 2};

void destroy_board(Board *b){ // note that b->vclue.b and v->hclue.b are destroyed elsewhere
    int i,j;
    
    for(i=0; i<b->n; i++){
        for(j=0; j<b->h; j++){
            nfree(b->panel.b[i]->b[j]->b);
        }
        nfree(b->panel.b[i]->b);
    }
    nfree(b->panel.b);
    
    for(i=0; i<b->time_panel.sb;i++)
        nfree(b->time_panel.b[i]);
    
    destroy_board_clue_blocks(b);
    destroy_all_bitmaps(b);
    nfree(b->all.b);
    nfree(b->clue_bmp);
    nfree(b->clue_tiledblock);
}

void destroy_board_clue_blocks(Board *b){
    int i;
    
    if(b->hclue.b){
        for(i=0; i<b->hclue.sb; i++){
            nfree(b->hclue.b[i]);
        }
        nfree(b->hclue.b);
    }
    
    if(b->vclue.b){
        for(i=0; i<b->vclue.sb; i++){
            nfree(b->vclue.b[i]);
        }
        nfree(b->vclue.b);
    }
}

//xxx todo: better board generation. Fix tile size first, then compute everything?
// mode: 1 = create, 0 = update, 2 = create fullscreen
int create_board(Game *g, Board *b, int mode){
    int i,j,k;
    int column_w, column_h, block_w, block_h, block_space;
    int hclue_tile_w, hclue_tile_h, vclue_tile_w, vclue_tile_h;
    int panel_tile_w, panel_tile_h;
    int cus;
    
    b->clue_unit_space = CLUE_UNIT_SPACE;
    
    if(mode){
        b->clue_bmp = malloc(g->clue_n*sizeof(*b->clue_bmp));
        b->clue_tiledblock = malloc(g->clue_n*sizeof(*b->clue_tiledblock));
    }
    
    if(b->max_ysize*INFO_PANEL_PORTION -2*INFO_PANEL_MARGIN < 32) // guarantee height of 32 pixels in info panel
        b->max_ysize = (32+2*INFO_PANEL_MARGIN)/INFO_PANEL_PORTION;
    
    b->xsize = b->max_xsize;
    b->ysize = b->max_ysize*(1.0-INFO_PANEL_PORTION);
    
    b->bg_color = BG_COLOR;
    b->dragging = NULL;
    b->highlight = NULL;
    b->rule_out = NULL;
 
    // panel dimensions
    b->panel.x = PANEL_MARGIN;
    b->panel.y = PANEL_MARGIN;
    b->panel.w = (b->xsize - 2*PANEL_MARGIN - b->xsize*HCLUEBOX_PORTION - 2*HCLUEBOX_MARGIN);
    b->panel.h = (b->ysize - 2*PANEL_MARGIN - b->ysize*VCLUEBOX_PORTION - 2*VCLUEBOX_MARGIN);
    b->panel.margin = PANEL_MARGIN;
    b->panel.bd_color = PANEL_BD_COLOR;
    b->panel.bg_color = PANEL_BG_COLOR;
    b->panel.bd = 1;
    b->panel.sb = b->n; // n subpanels
    b->panel.parent = &b->all;
    b->panel.type = TB_PANEL;
    b->panel.index = 0;
    b->panel.hidden = 0;
    b->panel.bmp = NULL;
    
    // for nons-square panel tiles:
    panel_tile_w = ((b->panel.w - (b->n-1)*PANEL_COLUMN_SPACE)/b->n-2*PANEL_BLOCK_MARGIN-(PANEL_TILE_COLUMNS[b->n]-1)*PANEL_TILE_SPACE)/(PANEL_TILE_COLUMNS[b->n]);
    panel_tile_h = (b->panel.h - b->h*((PANEL_TILE_ROWS[b->n]-1)*PANEL_TILE_SPACE + 2*PANEL_BLOCK_MARGIN))/(b->h*PANEL_TILE_ROWS[b->n]); //ok go
    
    // make square tiles
    b->panel_tile_size = min(panel_tile_w, panel_tile_h);
    panel_tile_w = b->panel_tile_size; // to use below
    panel_tile_h = b->panel_tile_size; // to use below
    
    block_w = PANEL_TILE_COLUMNS[b->n]*panel_tile_w + (PANEL_TILE_COLUMNS[b->n]-1)*PANEL_TILE_SPACE;
    block_h = PANEL_TILE_ROWS[b->n]*panel_tile_h + (PANEL_TILE_ROWS[b->n]-1)*PANEL_TILE_SPACE;
    column_w = (block_w + 2*PANEL_BLOCK_MARGIN);
    // block_space = (b->panel.h - b->h*(block_h + 2*PANEL_BLOCK_MARGIN))/(b->h-1);
    block_space = 0; // don't separate blocks too much
    column_h = b->h*(block_h + 2*PANEL_BLOCK_MARGIN);
    
    // adjust panel dimensions to account for integer arithmetic
    b->panel.w = b->n*column_w + (b->n-1)*PANEL_COLUMN_SPACE;
    b->panel.h = column_h;
    
    // panel columns
    if(mode) b->panel.b = malloc(b->n*sizeof(struct TiledBlock *));
    for(i=0; i<b->n; i++){
        if(mode){
            b->panel.b[i] = malloc(sizeof(struct TiledBlock));
            b->panel.b[i]->bg_color = PANEL_COLUMN_BG_COLOR;
            b->panel.b[i]->bd_color = PANEL_COLUMN_BD_COLOR;
            b->panel.b[i]->parent = &b->panel;
            b->panel.b[i]->bd = 1; // draw boundary
            b->panel.b[i]->sb = b->h;
            b->panel.b[i]->b = malloc(b->h*sizeof(struct TiledBlock *));
            b->panel.b[i]->type = TB_PANEL_COLUMN;
            b->panel.b[i]->index = i;
            b->panel.b[i]->hidden=0;
            b->panel.b[i]->bmp=NULL; // no background image
        }
        
        b->panel.b[i]->w = column_w;
        b->panel.b[i]->h = b->panel.h;
        b->panel.b[i]->margin=0;
        b->panel.b[i]->x = i*(b->panel.b[i]->w + PANEL_COLUMN_SPACE); // position relative to parent
        b->panel.b[i]->y = 0;
        
        // panel blocks
        for(j=0; j<b->h ; j++){
            if(mode) {
                b->panel.b[i]->b[j] = malloc(sizeof(struct TiledBlock));
                b->panel.b[i]->b[j]->bd_color = NULL_COLOR;
                b->panel.b[i]->b[j]->bg_color = NULL_COLOR;
                b->panel.b[i]->b[j]->bd = 0;
                b->panel.b[i]->b[j]->sb = b->n;
                b->panel.b[i]->b[j]->b = malloc(b->n*sizeof(struct TiledBlock *));
                b->panel.b[i]->b[j]->parent = b->panel.b[i];
                b->panel.b[i]->b[j]->type = TB_PANEL_BLOCK;
                b->panel.b[i]->b[j]->index = j;
                b->panel.b[i]->b[j]->hidden=0;
                b->panel.b[i]->b[j]->bmp = NULL;
            }
            b->panel.b[i]->b[j]->margin = PANEL_BLOCK_MARGIN;
            b->panel.b[i]->b[j]->w = block_w;
            b->panel.b[i]->b[j]->h = PANEL_TILE_ROWS[b->n]*panel_tile_h + (PANEL_TILE_ROWS[b->n]-1)*PANEL_TILE_SPACE;
            b->panel.b[i]->b[j]->x = PANEL_BLOCK_MARGIN;
            b->panel.b[i]->b[j]->y = j*(b->panel.b[i]->b[j]->h+block_space + 2*PANEL_BLOCK_MARGIN) + PANEL_BLOCK_MARGIN;
            
            // panel tiles
            for(k=0; k<b->n; k++){
                if(mode){
                    b->panel.b[i]->b[j]->b[k] = malloc(sizeof(struct TiledBlock));
                    b->panel.b[i]->b[j]->b[k]->bd_color = PANEL_TILE_BD_COLOR;
                    b->panel.b[i]->b[j]->b[k]->bg_color = NULL_COLOR;
                    b->panel.b[i]->b[j]->b[k]->bd = 1;
                    b->panel.b[i]->b[j]->b[k]->sb = 0;
                    b->panel.b[i]->b[j]->b[k]->b = NULL;
                    b->panel.b[i]->b[j]->b[k]->parent = b->panel.b[i]->b[j];
                    b->panel.b[i]->b[j]->b[k]->type = TB_PANEL_TILE;
                    b->panel.b[i]->b[j]->b[k]->index = k;
                    b->panel.b[i]->b[j]->b[k]->hidden=0;
                }
                b->panel.b[i]->b[j]->b[k]->margin = 0;
                b->panel.b[i]->b[j]->b[k]->w = panel_tile_w;
                b->panel.b[i]->b[j]->b[k]->h = panel_tile_h;
                b->panel.b[i]->b[j]->b[k]->x = (k % PANEL_TILE_COLUMNS[b->n])*panel_tile_w;
                b->panel.b[i]->b[j]->b[k]->y = (k / PANEL_TILE_COLUMNS[b->n])*panel_tile_h;
                b->panel.b[i]->b[j]->b[k]->bmp = &(b->panel_tile_bmp[j][k]);
            }
        }
    }
    
    // VCluebox dimensions
    b->vclue.margin = VCLUEBOX_MARGIN;
    b->vclue.y = b->panel.y + b->panel.h + b->panel.margin + b->vclue.margin;
    b->vclue.x = b->vclue.margin;
    b->vclue.bg_color = CLUE_PANEL_BG_COLOR;
    b->vclue.bd_color = CLUE_PANEL_BD_COLOR;
    b->vclue.bd = 1;
    b->vclue.bmp = NULL;
    b->vclue.w = b->panel.w;
    b->vclue.h = b->ysize - b->panel.h - 2*b->panel.margin - 2*b->vclue.margin;
    b->vclue.parent = &b->all;
    b->vclue.b = NULL; // later change
    b->vclue.sb = 0; // later change
    b->vclue.hidden=0;
    
    // HCluebox dimension
    b->hclue.margin = HCLUEBOX_MARGIN;
    b->hclue.x = b->panel.x + b->panel.w + b->panel.margin + b->hclue.margin;
    b->hclue.y = b->panel.y;
    b->hclue.w = b->xsize - b->panel.w - 2*b->panel.margin - 2*b->hclue.margin;
    b->hclue.h = b->ysize - 2*HCLUEBOX_MARGIN;
    b->hclue.bg_color = CLUE_PANEL_BG_COLOR;
    b->hclue.bd_color = CLUE_PANEL_BD_COLOR;
    b->hclue.bd = 1;
    b->hclue.parent = &b->all;
    b->hclue.sb = 0; // later change
    b->hclue.b = NULL; // later change
    b->hclue.hidden=0;
    
    // count number of vclues and hclues
    if(mode){
        b->vclue_n=0; b->hclue_n=0;
        for(i=0;i<g->clue_n;i++){
            if(is_vclue(g->clue[i].rel))
                b->vclue_n++;
            else
                b->hclue_n++;
        }
    }
    //xxx todo: allow multiple columns/rows of hclues/vclues
    
    cus = min(b->hclue.w - 2*CLUE_TILE_MARGIN - 2*CLUE_UNIT_SPACE, b->vclue.h - 2*CLUE_TILE_MARGIN - 2*CLUE_UNIT_SPACE)/3;
    cus = min(cus, (b->panel.h + 2*CLUE_UNIT_SPACE + 2*CLUE_TILE_MARGIN + VCLUEBOX_MARGIN +b->panel.margin-2*CLUE_TILE_MARGIN*b->hclue_n)/(max(b->hclue_n-3, 1)));
    b->clue_unit_size = min(cus, (b->panel.w/b->vclue_n - 2*CLUE_TILE_MARGIN));
    
//    b->clue_unit_size = min((b->hclue.h - 2*b->hclue_n*CLUE_TILE_MARGIN)/b->hclue_n, (b->vclue.w - 2*b->vclue_n*CLUE_TILE_MARGIN)/b->vclue_n);
//    b->clue_unit_size = min(cus, b->clue_unit_size);
    vclue_tile_h = b->clue_unit_size*3 + 2*CLUE_UNIT_SPACE;
    vclue_tile_w = b->clue_unit_size;
    hclue_tile_h = b->clue_unit_size;
    hclue_tile_w = b->clue_unit_size*3 + 2*CLUE_UNIT_SPACE;
    
    // update hclue and vclue to fit tight
    b->hclue.w = hclue_tile_w + 2*CLUE_TILE_MARGIN;
    b->hclue.h = b->panel.h + b->panel.margin + b->vclue.margin + vclue_tile_h + 2*CLUE_TILE_MARGIN;
    b->vclue.h = vclue_tile_h + 2*CLUE_TILE_MARGIN;
    b->vclue.w = b->panel.w;
    b->vclue.sb = b->vclue.w/(b->clue_unit_size + 2*CLUE_TILE_MARGIN);
    b->hclue.sb = b->hclue.h/(b->clue_unit_size + 2*CLUE_TILE_MARGIN);

    //fit tight again
    b->hclue.h = b->hclue.sb*(hclue_tile_h+2*CLUE_TILE_MARGIN);
    b->vclue.w = b->vclue.sb*(vclue_tile_w+2*CLUE_TILE_MARGIN);

    //create Vclue tiles
    b->vclue.b = malloc(b->vclue.sb*sizeof(struct TiledBlock *));
    for(i=0; i<b->vclue.sb; i++){
        b->vclue.b[i] = malloc(sizeof(struct TiledBlock));
        b->vclue.b[i]->w = vclue_tile_w;
        b->vclue.b[i]->h = vclue_tile_h;
        b->vclue.b[i]->margin = CLUE_TILE_MARGIN;
        b->vclue.b[i]->x = b->vclue.b[i]->margin + i*(b->vclue.b[i]->w + 2*b->vclue.b[i]->margin);
        b->vclue.b[i]->y = b->vclue.b[i]->margin;
        b->vclue.b[i]->bd_color = CLUE_TILE_BD_COLOR;
        b->vclue.b[i]->bg_color = CLUE_TILE_BG_COLOR;
        b->vclue.b[i]->bd = 1;
        b->vclue.b[i]->sb = 0;
        b->vclue.b[i]->b = NULL;
        b->vclue.b[i]->parent = &(b->vclue);
        b->vclue.b[i]->type = TB_VCLUE_TILE;
        b->vclue.b[i]->index = -1;
        b->vclue.b[i]->hidden = 0;
        b->vclue.b[i]->bmp = NULL;
    }

    //create Hclue tiles
    b->hclue.b = malloc(b->hclue.sb*sizeof(struct TiledBlock *));
    for(i=0; i<b->hclue.sb; i++){
        b->hclue.b[i] = malloc(sizeof(struct TiledBlock));
        b->hclue.b[i]->w = hclue_tile_w;
        b->hclue.b[i]->h = hclue_tile_h;
        b->hclue.b[i]->margin = CLUE_TILE_MARGIN;
        b->hclue.b[i]->x = b->hclue.b[i]->margin;
        b->hclue.b[i]->y = b->hclue.b[i]->margin + i*(b->hclue.b[i]->h + 2*b->hclue.b[i]->margin);;
        b->hclue.b[i]->bd_color = CLUE_TILE_BD_COLOR;
        b->hclue.b[i]->bg_color = CLUE_TILE_BG_COLOR;
        b->hclue.b[i]->bd = 1;
        b->hclue.b[i]->sb = 0;
        b->hclue.b[i]->b = NULL;
        b->hclue.b[i]->parent = &(b->hclue);
        b->hclue.b[i]->type = TB_HCLUE_TILE;
        b->hclue.b[i]->index=-1;
        b->hclue.b[i]->hidden = 0;
        b->hclue.b[i]->bmp = NULL;
    }
    
    //load Hclues
    j=0; k=0;
    for(i=0; i<g->clue_n; i++){
        if(is_vclue(g->clue[i].rel)) {
            b->vclue.b[j]->index = i;
            b->vclue.b[j]->bmp = &(b->clue_bmp[i]);
            b->clue_tiledblock[i] = b->vclue.b[j];
            j++;
        } else{
            b->hclue.b[k]->index = i;
            b->hclue.b[k]->bmp = &(b->clue_bmp[i]);
            b->clue_tiledblock[i] = b->hclue.b[k];
            k++;
        }
    }
    
    //resize board to fit tight
    //    fit_board(b);
	b->xsize = b->hclue.x + b->hclue.w + b->hclue.margin;
	b->ysize = max(b->hclue.h + 2 * b->hclue.margin, b->vclue.y + b->vclue.h + b->vclue.margin);

	for (i = 1; i < b->n; i++)
		b->panel.b[i]->x += i*min((b->max_xsize*H_FLEX_FACTOR), (b->max_xsize - b->xsize) / b->n);
    b->hclue.x += b->n * min((b->max_xsize*H_FLEX_FACTOR), (b->max_xsize - b->xsize) / b->n);

	b->info_panel.h = (float)b->max_ysize*INFO_PANEL_PORTION - 2 * INFO_PANEL_MARGIN;
	b->vclue.y += min((b->max_ysize - b->ysize - b->info_panel.h - 2*b->info_panel.margin)/2, 30);
	b->info_panel.y = b->vclue.y + b->vclue.h + b->vclue.margin + INFO_PANEL_MARGIN + min((b->max_ysize - b->ysize - b->info_panel.h - 2 * INFO_PANEL_MARGIN) / 2, 30);
	b->hclue.y = (b->info_panel.y - b->hclue.h) / 2;


// center vclues

	b->panel.w = b->panel.b[b->n - 1]->x + b->panel.b[b->n - 1]->w - b->panel.b[0]->x;
	b->vclue.x = (b->panel.w - b->vclue.w) / 2;
	//	b->panel.x = (b->hclue.x - b->panel.w) / 2;

	// info panel
	b->info_panel.x = b->panel.x; //  INFO_PANEL_MARGIN;
	b->info_panel.w = b->hclue.x - b->hclue.margin - INFO_PANEL_MARGIN - b->panel.x; // b->vclue.w; // assumes margins are equal
    b->info_panel.margin = INFO_PANEL_MARGIN;
    b->info_panel.bd_color = INFO_PANEL_BD_COLOR;
    b->info_panel.bg_color = INFO_PANEL_BG_COLOR;
    b->info_panel.bd = 1;
    b->info_panel.sb=0;
    b->info_panel.b=NULL;
    b->info_panel.parent = &b->all;
    b->info_panel.type = TB_INFO_PANEL;
    b->info_panel.index = 0;
    b->info_panel.hidden = 0;
    b->info_panel.bmp = NULL;
    
    // time panel
    b->time_panel.y = b->info_panel.y;
    b->time_panel.x = b->hclue.x;
    b->time_panel.w = b->hclue.w;
    b->time_panel.h = b->info_panel.h;
    b->time_panel.margin = b->info_panel.margin;
    b->time_panel.bg_color = TIME_PANEL_BG_COLOR;
    b->time_panel.bd_color = TIME_PANEL_BD_COLOR;
    b->time_panel.bd = 1;
    b->time_panel.sb = 1+TIME_PANEL_BUTTONS;
    b->time_panel.parent = &b->all;
    b->time_panel.type = TB_TIME_PANEL;
    b->time_panel.index = 0;
    b->time_panel.hidden = 0;
    b->time_panel.bmp = NULL;

    if(mode){ // if board is being created
        b->time_panel.b = malloc(6*sizeof(struct TiledBlock *));
        for(i=0;i<5;i++)
            b->time_panel.b[i] = malloc(sizeof(struct TiledBlock));
    }
    
    // timer
    b->time_panel.b[0]->x = 2;
    b->time_panel.b[0]->y = 4;
    b->time_panel.b[0]->h = 16;
    b->time_panel.b[0]->w = b->time_panel.w-4;
    b->time_panel.b[0]->margin = 0;
    b->time_panel.b[0]->bd_color = NULL_COLOR;
    b->time_panel.b[0]->bg_color = TIME_PANEL_BG_COLOR;
    b->time_panel.b[0]->bd = 0;
    b->time_panel.b[0]->sb = 0;
    b->time_panel.b[0]->b = NULL;
    b->time_panel.b[0]->parent = &b->time_panel;
    b->time_panel.b[0]->type = TB_TIMER;
    b->time_panel.b[0]->index = 0;
    b->time_panel.b[0]->hidden=0;
    b->time_panel.b[0]->bmp = &b->time_bmp;
    
    for(i=0; i<TIME_PANEL_BUTTONS; i++){ // buttons
        b->time_panel.b[i+1]->h = min((b->time_panel.h-24), b->time_panel.w/5);
        b->time_panel.b[i+1]->w = b->time_panel.b[i+1]->h;
        b->time_panel.b[i+1]->y = ((b->time_panel.h + (b->time_panel.b[0]->y+b->time_panel.b[0]->h))-b->time_panel.b[i+1]->h)/2;
        b->time_panel.b[i+1]->x = ((i+1)*b->time_panel.w  +(i-TIME_PANEL_BUTTONS)*b->time_panel.b[i+1]->w)/(TIME_PANEL_BUTTONS+1);
        b->time_panel.b[i+1]->margin = 0;
        b->time_panel.b[i+1]->bg_color = NULL_COLOR;
        b->time_panel.b[i+1]->bd_color = WHITE_COLOR;
        b->time_panel.b[i+1]->bd = 0;
        b->time_panel.b[i+1]->sb=0;
        b->time_panel.b[i+1]->b = NULL;
        b->time_panel.b[i+1]->parent = &b->time_panel;
        b->time_panel.b[i+1]->bmp = &b->button_bmp_scaled[i];
        b->time_panel.b[i+1]->index =0;
        b->time_panel.b[i+1]->hidden=0;
    }
    
    b->time_panel.b[1]->type = TB_BUTTON_CLUE;
    b->time_panel.b[2]->type = TB_BUTTON_HELP;
    b->time_panel.b[3]->type = TB_BUTTON_SETTINGS;
    b->time_panel.b[4]->type = TB_BUTTON_UNDO;
    
    // collect TiledBlocks into an array for convenience
    // and create settings block
    if(mode){ // only if board is being created
        b->all.x = 0;
        b->all.y = 0;
        b->all.margin = 0;
        b->all.bd_color = NULL_COLOR;
        b->all.bg_color = b->bg_color;
        b->all.bd = 0;
        b->all.parent = NULL;
        b->all.sb=5;
        b->all.b = malloc(b->all.sb*sizeof(struct TiledBlock*));
        b->all.type = TB_ALL;
        b->all.index = 0;
        b->all.hidden = 0;
        b->all.bmp = NULL;
        
        b->all.b[0] = &b->panel;
        b->all.b[1] = &b->hclue;
        b->all.b[2] = &b->vclue;
        b->all.b[3] = &b->time_panel;
        b->all.b[4] = &b->info_panel;
    }
    
    // final size adjustment
	b->xsize = b->hclue.x + b->hclue.margin + b->hclue.w;
	b->ysize = b->info_panel.y + b->info_panel.h + b->info_panel.margin;
	b->all.w = b->xsize;
	b->all.h = b->ysize;

    if((mode != 1) || MOBILE ){ // only for update or fullscreen or mobile
        b->all.x = (b->max_xsize - b->xsize)/2;
        b->all.y = (b->max_ysize - b->ysize)/2;
    }
    
    if(mode)
        if(init_bitmaps(b)) return -1;
    
    if(update_bitmaps(g, b)) return -1;

    create_font_symbols(b);

    return 0;
}

