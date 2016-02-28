#ifndef freesherlock__game__
#define freesherlock__game__

#include "TiledBlock.h"
#include "macros.h"

#define MAX_CLUES 100

// Structures
typedef enum GAME_STATE {
    GAME_NULL = 0,
    GAME_INTRO = 1,
    GAME_PLAYING,
    GAME_WRONG,
    GAME_OVER = 3,
    GAME_SETTINGS,
    NUMBER_OF_STATES
} GAME_STATE;

typedef enum RELATION {
    // Horizontal
    NEXT_TO,
    NOT_NEXT_TO,
    ONE_SIDE,
    CONSECUTIVE,
    NOT_MIDDLE,
    // Vertical
    TOGETHER_2,
    TOGETHER_3,
    NOT_TOGETHER,
    TOGETHER_NOT_MIDDLE,
    TOGETHER_FIRST_WITH_ONLY_ONE,
    // Positional
    REVEAL,
    NUMBER_OF_RELATIONS
} RELATION;

typedef struct Clue{
    // the three tiles from the clue are j[m], k[m] for m=0,1,2
    // if clue uses only 1 or 2 tiles, use the first and repeat them arbitrarily
    // i coordinate points to the column in the solution where the item appears (not shown to user)
    int i[3], j[3], k[3];
    RELATION rel; // how they relate
    int index;
    int hidden;
} Clue;

typedef struct Game {
    int guess[8][8]; // guessed value for [col][block]
    int puzzle[8][8]; // [col][clock] = [tile]
    int tile[8][8][8]; // [col][block][tile]
    Clue clue[MAX_CLUES];
    int clue_n;
    int n; // number of columns
    int h; // column height
    double time;
    int guessed;
    int tile_col[8][8]; // column where puzzle tile [row][tile] is located (in solution);
    int where[8][8];
    int advanced;
} Game;

typedef struct Pair {
    int i;
    int j;
} Pair;

// Prototypes

int rand_int(int n);
void init_game(Game *g); // clean board and guesses xxx todo: add clues?
void create_game_with_clues(Game *g);
void create_puzzle(Game *g);
int get_hint(Game *g);
int check_solution (Game *g);
int check_panel_consistency(Game *g);
int check_panel_correctness(Game *g);
void shuffle(int p[], int n);
void guess_tile(Game *g, int i, int j, int k);
void hide_tile_and_check(Game *g, int i, int j, int k);
void unguess_tile(Game *g,  int i, int j);
int is_guessed(Game *g, int j, int k); // is the value k on row j guessed?
void get_clue(Game *g, int i, int j, int rel, Clue *clue);
int is_vclue(RELATION rel); // is this relation a vertical clue?

// debug
int is_clue_valid(Game *g, Clue *clue);



// globals
extern int REL_PERCENT[NUMBER_OF_RELATIONS];
#endif
