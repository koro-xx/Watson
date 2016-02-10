#include "game.h"
//xxx todo: add TOGETHER_FIRST_WITH_ONLY_ONE logic
//xxx todo: improve composite clue checking (or what-ifs up to a given level)
//xxx todo: check for difficulty counting the number of positive clue checks (even for repeated clues)
//xxx todo: add composite board checking (check if there are only 2 of each item remaining on 2 columns only and clean accordingly)
//xxx todo: add bound on reveal clues
//xxx todo: tune probabilities for each clue type
//xxx todo: pass to binary and bitwise operations

char *clue_description[NUMBER_OF_RELATIONS] = {
    // Horizontal
    [NEXT_TO] = "Neighbors",
    [NOT_NEXT_TO] ="Middle not next to other",
    [ONE_SIDE] = "Second to the right of first",
    [CONSECUTIVE] = "Middle has other two one on each side",
    [NOT_MIDDLE] = "First and third two away, middle not in between",
    // Vertical
    [TOGETHER_2] ="First and second on same column",
    [TOGETHER_3] ="All three on same column",
    [NOT_TOGETHER] ="Second not on same column as first",
    [TOGETHER_NOT_MIDDLE] = "First and third on same column, second not",
    [TOGETHER_FIRST_WITH_ONLY_ONE] = "Unused",
    // Positional
    [REVEAL] = "First on given column"
};

int REL_PERCENT[NUMBER_OF_RELATIONS] = {
    [NEXT_TO] = 20,
    [NOT_NEXT_TO] = 5,
    [ONE_SIDE] = 2,
    [CONSECUTIVE] = 3,
    [NOT_MIDDLE] = 5,
    [TOGETHER_2] = 15,
    [TOGETHER_3] = 5,
    [NOT_TOGETHER] = 5,
    [TOGETHER_NOT_MIDDLE] = 5,
    [TOGETHER_FIRST_WITH_ONLY_ONE]=0,
    [REVEAL] = 1
};

int REL_PERCENT_MAX;

// Prototypes
void get_clue(Game *g, int i, int j, int rel, Clue *clue);
int filter_clues(Game *g);

void create_puzzle(Game *g){
    int i, j;
    int p[8];
    
    g->guessed=0;
    for(i=0; i<8;i++){
        p[i]=i;
    }
    
    for(i=0; i<g->h; i++){
        shuffle(p, g->n);
        for(j=0;j<g->n; j++){
            g->puzzle[j][i] = p[j];
            g->where[i][p[j]] = j;
        }
    }
};

int rand_int(int n) { // make static
    int limit = RAND_MAX - RAND_MAX % n;
    int rnd;
    
    do {
        rnd = rand();
    } while (rnd >= limit);
    return rnd % n;
};

static int rand_sign(void){
    return (rand()%2 == 0) ? -1 : 1;
};
    
void shuffle(int p[], int n) {
    int i, j, tmp;
    
    for (i = n - 1; i > 0; i--) {
        j = rand_int(i + 1);
        tmp = p[j];
        p[j] = p[i];
        p[i] = tmp;
    }
};

void remove_clue(Game *g, int i){
// swap clue[i] with last clue, reduce clue_n
    g->clue_n--;
    g->clue[i] = g->clue[g->clue_n];
};


int check_this_clue(Game *g, Clue *clue){
    int i,m, ret=0, hide_first;
    int j0, k0, j1, k1, j2, k2;
    
    j0 = clue->j[0]; k0 = clue->k[0];
    j1 = clue->j[1]; k1 = clue->k[1];
    j2 = clue->j[2]; k2 = clue->k[2];
    
    ret=0;
    hide_first=0;
    
    switch(clue->rel){
        case REVEAL:
            if(g->guess[clue->i[0]][j0]<0){
                guess_tile(g, clue->i[0],j0,k0);
                ret=1| k0<<1 | j0<<4 | clue->i[0]<<7 | 1<<10;
            }
            break;
        case ONE_SIDE:
            for(i=0;i<g->n;i++){
                if(g->tile[i][j1][k1]){
                    hide_tile_and_check(g, i, j1, k1);
                    ret=1 | k1<<1 | j1<<4 | i<<7;
                }
                if(g->tile[i][j0][k0]) break;
            }
            for(i=g->n-1;i>=0;i--){
                if(g->tile[i][j0][k0]){
                    hide_tile_and_check(g, i, j0, k0);
                    ret=1 | k0<<1 | j0<<4 | i<< 7;
                }
                if(g->tile[i][j1][k1]) break;
            }
            break;
            
        case TOGETHER_2:
            for(i=0;i<g->n;i++){
                if(!g->tile[i][j0][k0] || !g->tile[i][j1][k1]){
                    if(g->tile[i][j0][k0]){
                        hide_tile_and_check(g, i, j0, k0);
                        ret=1 | k0<<1 | j0<<4 | i<<7;
                    }
                    if(g->tile[i][j1][k1]){
                        hide_tile_and_check(g, i, j1, k1);
                        ret=1 | k1<<1 | j1<<4 | i<<7;;
                    }
                }
            }
            break;
        case TOGETHER_3:
            for(i=0;i<g->n;i++){
                if(!g->tile[i][j0][k0] || !g->tile[i][j1][k1] || !g->tile[i][j2][k2]){// if one exists but one doesn't
                    if(g->tile[i][j0][k0]){
                        hide_tile_and_check(g, i, j0, k0); ret=1| k0<<1 | j0<<4 | i<<7;
                    }
                    if(g->tile[i][j1][k1]){
                        hide_tile_and_check(g, i, j1, k1); ret=1| k1<<1 | j1<<4 | i<<7;;
                    }
                    if(g->tile[i][j2][k2]){
                        hide_tile_and_check(g, i, j2, k2); ret=1| k2<<1 | j2<<4 | i<<7;;
                    }
                        
                }
            }
            break;
            
        case TOGETHER_NOT_MIDDLE:
            for(i=0;i<g->n;i++){
                if( (g->guess[i][j0] == k0) || (g->guess[i][j2] == k2)){
                    if(g->tile[i][j1][k1]){
                        hide_tile_and_check(g, i, j1, k1);
                        ret=1| k1<<1 | j1<<4 | i<<7;
                    }
                }
                if( (!g->tile[i][j0][k0]) || (g->guess[i][j1] == k1) || !g->tile[i][j2][k2] ){
                    if(g->tile[i][j0][k0]){
                        hide_tile_and_check(g, i, j0, k0);
                        ret=1 | k0<<1 | j0<<4 | i<<7;
                    }
                    if(g->tile[i][j2][k2]){
                        hide_tile_and_check(g, i, j2, k2);
                        ret=1| k2<<1 | j2<<4 | i<<7;;
                    }
                }
            }
            break;
            
        case NOT_TOGETHER:
            for(i=0;i<g->n;i++){
                if((g->guess[i][j0]==k0) && g->tile[i][j1][k1]){
                    hide_tile_and_check(g, i, j1, k1);
                    ret =1| k1<<1 | j1<<4 | i<<7;
                }
                if((g->guess[i][j1]==k1) && g->tile[i][j0][k0]){
                    hide_tile_and_check(g, i, j0, k0);
                    ret=1 | k0<<1 | j0<<4 | i<<7;;
                }
            }
            break;
            
        case NEXT_TO:
            if(!g->tile[1][j0][k0] && g->tile[0][j1][k1]){
                hide_tile_and_check(g, 0, j1, k1);
                ret=1 | k1<<1 | j1<<4 | 0 << 7;
            }
            if(!g->tile[1][j1][k1] && g->tile[0][j0][k0]){
                hide_tile_and_check(g, 0, j0, k0);
                ret=1| k0<<1 | j0<<4 | 0<<7;
            }
            if(!g->tile[g->n-2][j0][k0] && g->tile[g->n-1][j1][k1]){
                hide_tile_and_check(g, g->n-1, j1, k1);
                ret=1| k1<<1 | j1<<4 | (g->n-1)<<7;
            }
            if(!g->tile[g->n-2][j1][k1] && g->tile[g->n-1][j0][k0]){
                hide_tile_and_check(g, g->n-1, j0, k0);
                ret=1| k0<<1 | j0<<4 | (g->n-1)<<7;;
            }

            for(i=1;i<g->n-1;i++){
                if(!g->tile[i-1][j0][k0] && !g->tile[i+1][j0][k0]){
                    if(g->tile[i][j1][k1]){
                        hide_tile_and_check(g, i, j1, k1);
                        ret=1| k1<<1 | j1<<4 | i<<7;;
                    }
                }
                if(!g->tile[i-1][j1][k1] && !g->tile[i+1][j1][k1]){
                    if(g->tile[i][j0][k0]){
                        hide_tile_and_check(g, i, j0, k0);
                        ret= 1 | k0<<1 | j0<<4 | i<<7;;
                    }
                }
            }
            break;
            
        case NOT_NEXT_TO:
            for(i=0;i<g->n;i++){
                if(i<g->n-1){
                    if((g->guess[i][j0] == k0) && g->tile[i+1][j1][k1]){
                        hide_tile_and_check(g, i+1, j1, k1); ret=1| k1<<1 | j1<<4 | (i+1)<<7;;
                    }
                    
                    if((g->guess[i][j1] == k1) && g->tile[i+1][j0][k0]){
                        hide_tile_and_check(g, i+1, j0, k0); ret=1| k0<<1 | j0<<4 | (i+1)<<7;;
                    }
                }
                if(i>0){
                    if((g->guess[i][j0] == k0) && g->tile[i-1][j1][k1]){
                        hide_tile_and_check(g, i-1, j1, k1); ret=1| k1<<1 | 10<<4 | (i-1)<<7;;
                    }
                    
                    if((g->guess[i][j1] == k1) && g->tile[i-1][j0][k0]){
                        hide_tile_and_check(g, i-1, j0, k0); ret=1| k0<<1 | j0<<4 | (i-1)<<7;;
                    }
                }
            }
            break;
            
        case CONSECUTIVE:
            for(i=0;i<g->n;i++){
                for(m=0; m<2; m++){
                    if(g->tile[i][j0][k0]){
                        if((i<g->n-2) && (i<2)){
                            if((!g->tile[i+1][j1][k1]) || (!g->tile[i+2][j2][k2]) ){
                                hide_first=1;
                            }
                        }
                        if((i>=2) && (i>=g->n-2)){
                            if((!g->tile[i-2][j2][k2]) || (!g->tile[i-1][j1][k1]) ){
                                hide_first=1;
                            }
                        }
                        
                        if((i>=2) && (i<g->n-2)){
                            if( ((!g->tile[i+1][j1][k1]) || (!g->tile[i+2][j2][k2])) && ((!g->tile[i-2][j2][k2]) || (!g->tile[i-1][j1][k1])) ) {
                            hide_first =1;
                            }
                        }
                        if(hide_first){
                            hide_first = 0;
                            hide_tile_and_check(g, i, j0, k0);
                            ret=1| k0<<1 | j0<<4 | i<<7;
                        }
                    }
                    SWAP(j0, j2); SWAP(k0, k2);
                }
                
                if(g->tile[i][j1][k1]){
                    if((i==0)||(i==g->n-1)){
                         hide_first=1;
                    } else{
                        if(((!g->tile[i-1][j0][k0]) && !(g->tile[i+1][j0][k0])) || ((!g->tile[i-1][j2][k2]) && !(g->tile[i+1][j2][k2]))){ // error here! incorrect check!
                            hide_first=1;
                        }
                    }
                    if(hide_first){
                        hide_first =0;
                        hide_tile_and_check(g, i, j1, k1);
                        ret=1| k1<<1 | j1<<4 | i<<7;;
                    }
                }
            }
            break;
            
        case NOT_MIDDLE:
            for(i=0;i<g->n;i++){ // apply mask
                for(m=0; m<2; m++){
                    if(g->tile[i][j0][k0]){
                        if((i<g->n-2) && (i<2)){
                            if( (g->guess[i+1][j1]==k1) || (!g->tile[i+2][j2][k2]) ){
                                hide_first=1;
                            }
                        }
                        if((i>=2) && (i>=g->n-2)){
                            if( (g->guess[i-1][j1]==k1) || (!g->tile[i-2][j2][k2]) ){
                                hide_first=1;
                            }
                        }
                        
                        if((i>=2) && (i<g->n-2)){
                            if( ((g->guess[i+1][j1]==k1) || (!g->tile[i+2][j2][k2]))  && ((!g->tile[i-2][j2][k2]) || (g->guess[i-1][j1]==k1)) ) {
                                hide_first =1;
                            }
                        }
                        if(hide_first){
                            hide_first = 0;
                            ret=1| k0<<1 | j0<<4 | i<<7;;
                            hide_tile_and_check(g, i, j0, k0);
                        }
                    }
                    SWAP(j0, j2); SWAP(k0, k2);
                }
                if( (i>=1) && (i<=g->n-2) ){
                    if( ((g->guess[i-1][j0]==k0) && (g->guess[i+1][j2]==k2)) || ((g->guess[i-1][j2]==k2) && (g->guess[i+1][j0]==k0))){
                        if(g->tile[i][j1][k1]) {
                            hide_tile_and_check(g, i, j1, k1);
                            ret=1| k1<<1 | j1<<4 | i<<7;
                        }
                    }
                }
            }
            break;
        case TOGETHER_FIRST_WITH_ONLY_ONE:
            break;
        default:
            break;
    }
    return ret;
}


int check_solution(Game *g){
    int i,j;
    for(i=0; i<g->n; i++)
        for(j=0; j<g->h; j++)
            if(!g->tile[i][j][g->puzzle[i][j]]) return 0;

    return 1;
}

// save a backup copy of game data if type==0, restore the data if type==1
void switch_game(Game *g, int type){
    static int tile[8][8][8];
    static int guess[8][8];
    static int guessed;
    
    if(type == 0){
        memcpy(&tile, &g->tile, sizeof(tile));
        memcpy(&guess, &g->guess, sizeof(guess));
        guessed = g->guessed;
    }
    SWAP(g->tile, tile);
    SWAP(g->guess, guess);
    SWAP(g->guessed, guessed);
}

// returns: clue number | 1<<8 | k<<9 | j<<12 | k<<15
// where i,j,k is a tile that can be ruled out with this clue
int get_hint(Game *g){ // still not working properly
    int i, ret=0, tro=0;
    
    switch_game(g, 0); // store game
    for(i=0; i<g->clue_n; i++){
        if( (tro = check_this_clue(g, &g->clue[i])) ){
            ret=i; break;
        }
    }
    switch_game(g,1); // restore game
    return ret | tro<<8;
}
int advanced_check_clues(Game *g){
    int info, m;
    int i,j,k;
    
    for(i=0;i<g->n;i++){
        for(j=0; j<g->h; j++){
            for(k=0; k<g->n; k++){
                if(g->tile[i][j][k]){
                    switch_game(g,0); // save state
                    guess_tile(g,i,j,k);
                    do{ // repeat until no more information remains in clues
                        info=0;
                        for(m=0; m<g->clue_n; m++){
                            if(check_this_clue(g, &g->clue[m])){
                                info=1;
                            }
                        }
                    }while(info);
                    if(!check_panel_consistency(g)){
                        switch_game(g,1); // restore
                        hide_tile_and_check(g,i,j,k);
                        return 1;
                    } else {
                        switch_game(g,1); // restore state
                    }
                }
            }
        }
    }

    return 0;
}

int check_clues(Game *g){
    // check whether the clues add new info (within reason -- this can be tuned)
    // for now it does not combine clues (analyze each one separately)
    // if so, discover the info in g->tile
    // return 1 if new info was found, 0 if not
    int info, m, ret;
    
    ret=0;
    do{ // repeat until no more information remains in clues
        info=0;
        for(m=0; m<g->clue_n; m++){
            if(check_this_clue(g, &g->clue[m])){
                ret=1;
                info=1;
            }
        }
        if(!info && g->advanced){ // check "what if" depth 1
            while(advanced_check_clues(g)){
                info=1;
            }
        }
    }while(info);
    
    return ret;
}

void create_game_with_clues(Game *g){
    int i;
    
    init_game(g);
    create_puzzle(g);
    
    g->clue_n=0;
    for(i=0; i<100; i++){ // xxx todo add a check to see if we have found solution or not after 100
        g->clue_n++;
        do{
            get_clue(g, rand_int(g->n), rand_int(g->h), -1,  &g->clue[g->clue_n-1]);
        } while(!check_this_clue(g, &g->clue[g->clue_n-1])); // should be while !check_clues?
        check_clues(g);
        if(g->guessed == g->n*g->h) break;
    }
   
    if(!check_solution(g)) // debug
        fprintf(stderr, "ERROR: SOLUTION DOESN'T MATCH CLUES\n");
    
    filter_clues(g);
    fprintf(stdout, "%d clues\n", g->clue_n);
    
    //clean guesses and tiles
    init_game(g);
    
    // reveal reveal clues and remove them from clue list
    for(i=0;i<g->clue_n;i++){
        if(g->clue[i].rel == REVEAL){
            guess_tile(g, g->clue[i].i[0], g->clue[i].j[0], g->clue[i].k[0]);
            remove_clue(g, i);
            i--;
        }
    }

}


// checks if clue is compatible with current panel (not necessarily with solution)
int is_clue_compatible(Game *g, Clue *clue){
    int i, j, ret=0;
    int j0, k0, j1, k1, j2, k2;
    
    j0 = clue->j[0]; k0 = clue->k[0];
    j1 = clue->j[1]; k1 = clue->k[1];
    j2 = clue->j[2]; k2 = clue->k[2];
    
    ret=0;
    
    switch(clue->rel){
        case REVEAL:
            if(g->tile[clue->i[0]][j0][k0])
                ret=1;
            break;
        case ONE_SIDE:
            for(i=0;i<g->n;i++){
                if(g->tile[i][j1][k1] && (ret == -1)){
                    ret=1;
                    break; //loop
                }

                if(g->tile[i][j0][k0]){
                    ret=-1;
                }
            }
            if(ret!=1) ret=0;
            break; //switch
            
        case TOGETHER_2:
            for(i=0;i<g->n;i++){
                if(g->tile[i][j0][k0] && g->tile[i][j1][k1]){
                    ret = 1;
                    break;
                }
            }
            break;
            
        case TOGETHER_3:
            for(i=0;i<g->n;i++){
                if(g->tile[i][j0][k0] && g->tile[i][j1][k1] && g->tile[i][j2][k2]){
                    ret = 1;
                    break;
                }
            }
            break;
            
        case TOGETHER_NOT_MIDDLE:
            for(i=0;i<g->n;i++){
                if( (g->tile[i][j0][k0]) && (g->guess[i][j1] != k1) && g->tile[i][j2][k2] ){
                    ret = 1;
                    break;
                }
            }
            break;
            
        case NOT_TOGETHER:
            for(i=0;i<g->n;i++){
                if((g->guess[i][j0]!=k0) && g->tile[i][j1][k1]){
                    ret =1;
                    break;
                }
            }
            break;
            
        case NEXT_TO:
            for(i=0;i<g->n-1;i++){
                if( (g->tile[i][j0][k0] && g->tile[i+1][j1][k1]) || (g->tile[i][j1][k1] && g->tile[i+1][j0][k0])){
                    ret = 1;
                    break;
                }
            }
            break;
            
        case NOT_NEXT_TO:
            for(i=0;i<g->n;i++){
                for(j=0;j<g->n;j++){
                    if (g->tile[i][j0][k0] && g->tile[j][j1][k1]){
                        if((i-j != 1) && (j-i)!= 1){
                            ret=1; break;
                        }
                    }
                }
                if(ret) break;
            }
            break;
            
        case CONSECUTIVE:
            for(i=0;i<g->n-2;i++){
                if( (g->tile[i][j0][k0] && g->tile[i+1][j1][k1] && g->tile[i+2][j2][k2]) || (g->tile[i][j2][k2] && g->tile[i+1][j1][k1] && g->tile[i+2][j0][k0]) ){
                    ret =1;
                    break;
                }
            }
            break;
            
        case NOT_MIDDLE:
            for(i=0; i<g->n-2; i++){
                if( (g->tile[i][j0][k0] && (g->guess[i+1][j1]!=k1) && g->tile[i+2][j2][k2]) || (g->tile[i][j2][k2] && (g->guess[i+1][j1]!=k1) && g->tile[i+2][j0][k0]) ){
                    ret =1;
                    break;
                }
            }
            break;
        case TOGETHER_FIRST_WITH_ONLY_ONE:
            break;
        default:
            break;
    }
    return ret;
}

int check_panel_consistency(Game *g){
    int m;
    for(m=0; m<g->clue_n; m++){
        if(!is_clue_compatible(g, &g->clue[m]))
            return 0;
    }
    return 1;
}


int check_panel_correctness(Game *g){
    int i,j;
    for(i=0; i<g->n; i++){
        for(j=0;j<g->h; j++){
            if(!g->tile[i][j][g->puzzle[i][j]]){
                return 0;
            }
        }
    }
    return 1;
}


int check_clues_for_solution(Game *g){
    int m, ret, info;

    init_game(g);

    ret=0;
    do{ // repeat until no more information remains in clues
        info=0;
        for(m=0; m<g->clue_n; m++){
            if(check_this_clue(g, &g->clue[m])){
                ret=1;
                info=1;
            }
        }
        if(!info && g->advanced){ // check "what if" depth 1
            while(advanced_check_clues(g)){
                info=1;
            }
        }
    }while(info);
    
    if(g->guessed == g->n*g->h) return 1;
    else return 0;
}


int filter_clues(Game *g){
    int m, i, j, ret = 0;
    // revert clue order
//    for(i=0; i<g->clue_n/2; i++){
//        SWAP(g->clue[i], g->clue[g->clue_n-i-1]);
//    }
//
        for(m=0; m<g->clue_n; m++){ // test reduction
            SWAP(g->clue[g->clue_n-1], g->clue[m]);
            init_game(g);
            g->clue_n--;
            if(check_clues_for_solution(g)) {
                ret=1;
            } else{
                g->clue_n++;
            }
        }
    
    // join clues if possible
    for(i=g->clue_n-1; i>0; i--){
        if(g->clue[i].rel == TOGETHER_2){
            for(j=i-1; j>=0; j--){
                if(g->clue[j].rel == TOGETHER_2){ // check all combinations of 0, 1, 2
                    if( ((g->clue[j].j[0] == g->clue[i].j[0]) && (g->clue[j].k[0] == g->clue[i].k[0])) || ((g->clue[j].j[1] == g->clue[i].j[0]) && (g->clue[j].k[1] == g->clue[i].k[0])) ){
                        g->clue[j].j[2] = g->clue[i].j[1];
                        g->clue[j].k[2] = g->clue[i].k[1];
                        g->clue[j].rel = TOGETHER_3;
                        remove_clue(g, i);
                        break;
                    } else if( ((g->clue[j].j[0] == g->clue[i].j[1]) && (g->clue[j].k[0] == g->clue[i].k[1])) || ((g->clue[j].j[1] == g->clue[i].j[1]) && (g->clue[j].k[1] == g->clue[i].k[1])) ){
                        g->clue[j].j[2] = g->clue[i].j[0];
                        g->clue[j].k[2] = g->clue[i].k[0];
                        g->clue[j].rel = TOGETHER_3;
                        remove_clue(g, i);
                        break;
                    }
                }
            }
        }
    }
    
    //sort clues
    for(i=0;i<g->clue_n;i++){
        switch(g->clue[i].rel){
            case TOGETHER_2:
                if(g->clue[i].j[0]>g->clue[i].j[1]){
                    SWAP(g->clue[i].i[0], g->clue[i].i[1]);
                    SWAP(g->clue[i].j[0], g->clue[i].j[1]);
                    SWAP(g->clue[i].k[0], g->clue[i].k[1]);
                }
                break;
            case TOGETHER_3:
                if(g->clue[i].j[0]>g->clue[i].j[2])
                {
                    SWAP(g->clue[i].i[0], g->clue[i].i[2]);
                    SWAP(g->clue[i].j[0], g->clue[i].j[2]);
                    SWAP(g->clue[i].k[0], g->clue[i].k[2]);
                }
                if(g->clue[i].j[0]>g->clue[i].j[1]){
                    SWAP(g->clue[i].i[0], g->clue[i].i[1]);
                    SWAP(g->clue[i].j[0], g->clue[i].j[1]);
                    SWAP(g->clue[i].k[0], g->clue[i].k[1]);
                }
                if(g->clue[i].j[1]>g->clue[i].j[2]){
                    SWAP(g->clue[i].i[1], g->clue[i].i[2]);
                    SWAP(g->clue[i].j[1], g->clue[i].j[2]);
                    SWAP(g->clue[i].k[1], g->clue[i].k[2]);
                }
                
                break;
            case TOGETHER_NOT_MIDDLE:
                if(g->clue[i].j[0]>g->clue[i].j[2]){
                    SWAP(g->clue[i].i[0], g->clue[i].i[2]);
                    SWAP(g->clue[i].j[0], g->clue[i].j[2]);
                    SWAP(g->clue[i].k[0], g->clue[i].k[2]);
                }
                break;
            default:
                break;
        }
        
    }

//simplify clues that are redundant with one another
//    for(i=0;i<g->clue_n;i++){
//        for(j=i+1; j<g->clue_n+j++){
//            if((g->clue[i].rel = TOGETHER_2) && (g->clue[j].rel =)
//        }
//    }
    
    
        return ret;
}



int get_random_tile(Game *g, int i, int *j, int *k){ // random item in column i
    int m, jj, kk=0;
    
    m=0;
    for(jj=0;jj<g->h;jj++)
        for(kk=0;kk<g->n;kk++)
            if(g->guess[i][jj]<0)
                if(g->tile[i][jj][kk])
                    m++;
            
    if(m==0) return 0;
    m=rand_int(m);
    for(jj=0;jj<g->h;jj++){
        for(kk=0;kk<g->n;kk++){
            if(g->tile[i][jj][kk] && (g->guess[i][jj]<0)) {
                m--;
                if(m<0) break;
            }
        }
        if(m<0) break;
    }
    *j = jj; *k = kk;
    return 1;
};


int random_relation(void){
    int m, s, i;
    int rel;
    static int firstcall=1;
    
    if(firstcall){ // initialize probabilities
        firstcall=0;
        REL_PERCENT_MAX=0;
        for(i=0;i<NUMBER_OF_RELATIONS; i++){
            REL_PERCENT_MAX += REL_PERCENT[i];
        }
    }
    
    rel=-1;
    m = rand_int(REL_PERCENT_MAX);
    s=0;
    for(i=0;i<NUMBER_OF_RELATIONS; i++){
        s+= REL_PERCENT[i];
        if (m< s){
            rel=i; break;
        }
    }
    if(rel<0) return random_relation();
    return rel;
};

void get_random_item_col(Game *g, int i, int *j, int *k){ // get a new solved item at column ii
    *j = rand_int(g->h);
    *k = g->puzzle[i][*j];
};

void get_random_item_col_except(Game *g, int i, int *j, int *k, int ej1, int ej2){ // get a new solved item
    int m = (ej1 == ej2) ? 1 : 2;
    *j = rand_int(g->h-m);

    for(m=0; m<2; m++) // skip ej1 and ej2
        if((*j == ej1) || (*j == ej2))
            *j=(*j+1)%g->h;
    *k = g->puzzle[i][*j];
};


void get_clue(Game *g, int i, int j, int rel, Clue *clue){
    int k, ii, jj, kk, jjj, kkk, m, s;
    
    if(rel<0){
        do{ // to avoid problem when g->n is too small in the NOT_MIDDLE case
            rel = random_relation();
        } while( ((rel == NOT_MIDDLE) && (i<2) && (i>g->n-3)) || (rel == TOGETHER_FIRST_WITH_ONLY_ONE) );
    }
    clue->rel = rel;
    
    k=g->puzzle[i][j];
    
    switch(rel){
        case CONSECUTIVE:
            s = rand_int(3);
            // check that i-s, i-s+1, i-s+2 are all in range
            if(i-s+2 > g->n-1) s += (i-s+2) - (g->n-1);
            else if (i-s < 0) s = i;
        
            for(m=0;m<3;m++){
                ii = i-s+m;
                if(ii != i)
                   get_random_item_col(g, ii, &jj, &kk); // get a new solved item at column ii (solve if necessary)
                else
                { jj = j; kk  = k; }
                clue->i[m]= ii; clue->j[m] = jj; clue->k[m] = kk;
            }
            if(rand_int(2)){ // random swap of outer elements
                SWAP(clue->i[0], clue->i[2]);
                SWAP(clue->j[0], clue->j[2]);
                SWAP(clue->k[0], clue->k[2]);
            }
            break;
        case ONE_SIDE:
            s = rand_int(g->n-1);
            ii = (i + s + 1) % g->n;
            get_random_item_col(g, ii, &jj, &kk);
            if (ii< i){
                SWAP(i, ii); SWAP(j, jj); SWAP(k, kk);
            }
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = ii; clue->j[1] = jj; clue->k[1] = kk;
            clue->i[2] = ii; clue->j[2] = jj; clue->k[2] = kk; // filler
            break;
        case NEXT_TO:
            ii = i+rand_sign();
            if(ii >= g->n) ii = i-1;
            else if(ii < 0) ii = i+1;
            get_random_item_col(g, ii, &jj, &kk);
            if (rand_int(2)){
                SWAP(i, ii); SWAP(j, jj); SWAP(k, kk);
            }
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = ii; clue->j[1] = jj; clue->k[1] = kk;
            clue->i[2] = i; clue->j[2] = j; clue->k[2] = k;
            break;
        case NOT_NEXT_TO:
            if(i >= g->n-1) s=-1;
            else if(i <= 0) s=+1;
            else s = rand_sign();
            ii = i+s;
            get_random_item_col(g, ii, &jj, &kk);
            kk = (kk + 1) % g->n; // get an item that is NOT the neighbor one
            // avoid same item
            if((kk == g->puzzle[i][j]) && (j==jj)) kk = (kk + 1) % g->n;
            if((i-s>=0) && (i-s<g->n))
                if(g->puzzle[i-s][jj] == kk) // avoid the neighbor from the other side
                    kk = (kk+1) % g->n;
                            
            if (rand_int(2)){
                SWAP(i, ii); SWAP(j, jj); SWAP(k, kk);
            }
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = ii; clue->j[1] = jj; clue->k[1] = kk;
            clue->i[2] = i; clue->j[2] = j; clue->k[2] = k;
            break;
        case NOT_MIDDLE:
            if(i > g->n-3) s=-1;
            else if(i<2) s=1;
            else s=rand_sign();
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            ii = i+s;
            get_random_item_col(g, ii, &jj, &kk);
            clue->i[1] = ii; clue->j[1] = jj; clue->k[1]=(kk + 1+rand_int(g->n-1))% g->n;
            ii = i+2*s;
            get_random_item_col(g, ii, &jj, &kk);
            clue->i[2] = ii; clue->j[2] = jj; clue->k[2]=kk;
            if(rand_int(2)){ // random swap of outer elements
                SWAP(clue->i[0], clue->i[2]);
                SWAP(clue->j[0], clue->j[2]);
                SWAP(clue->k[0], clue->k[2]);
            }
            break;
        case TOGETHER_2:
            get_random_item_col_except(g, i, &jj, &kk, j, j); // except row j (and j)
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = i; clue->j[1] = jj; clue->k[1] = kk;
            clue->i[2] = i; clue->j[2] = jj; clue->k[2] = kk; // filler
            break;
        case TOGETHER_3:
            get_random_item_col_except(g, i, &jj, &kk, j, j); // except row j (and j)
            get_random_item_col_except(g, i, &jjj, &kkk, j, jj); // except row j and jj
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = i; clue->j[1] = jj; clue->k[1] = kk;
            clue->i[2] = i; clue->j[2] = jjj; clue->k[2] = kkk;
            break;
        case NOT_TOGETHER:
            get_random_item_col_except(g, i, &jj, &kk, j, j); // except row j (and j)
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = i; clue->j[1] = jj; clue->k[1] = (kk + 1+ rand_int(g->n-1))%g->n;
            clue->i[2] = i; clue->j[2] = j; clue->k[2] = k; // filler
            break;
        case TOGETHER_NOT_MIDDLE:
            get_random_item_col_except(g, i, &jj, &kk, j, j); // except row j (and j)
            get_random_item_col_except(g, i, &jjj, &kkk, j, jj); // except row j and jj
            clue->i[0] = i; clue->j[0] = j; clue->k[0] = k;
            clue->i[1] = i; clue->j[1] = jj; clue->k[1] = (kk + 1+ rand_int(g->n-1))%g->n;
            clue->i[2] = i; clue->j[2] = jjj; clue->k[2] = kkk;
            break;
        case REVEAL:
            ii = rand_int(g->n);
            get_random_item_col(g, ii, &jj, &kk);
            for(m=0; m<3;m++) {clue->i[m] = ii; clue->j[m] = jj; clue->k[m] = kk; }
            break;
        case TOGETHER_FIRST_WITH_ONLY_ONE:
            break;
    }
};


void init_game(Game *g){
    int i,j,k;
    
    for(i=0;i<g->n;i++){
        for(j=0;j<g->h;j++){
            g->guess[i][j]=-1;
            g->tile_col[j][i] = -1;
            for(k=0;k<g->n;k++){
                g->tile[i][j][k]=1;
            }
        }
    }
    g->guessed=0;
}

// return -1 if there is more than one tile left in block
// last tile number otherwise
int last_tile_in_block(Game *g, int i, int j){
    int k, m = -1, count=0;
    
    if(g->guess[i][j]>=0) return -1;
    
    for(k=0; k < g->n; k++){ // find if there is only 1 tile left
        if(g->tile[i][j][k]){
            m=k; count++;
        }
        if(count>1) return -1;
    }
    return m;
};


// return -1 if there is more than one block with given tile
 // last block number (column) otherwise
int last_tile_in_row(Game *g, int j, int k){
    int i, m = -1, count=0;
    
    for(i=0; i < g->n; i++){ // find if there is only 1 tile left
        if(g->guess[i][j]==k) return -1;
        if(g->tile[i][j][k]){
            m=i; count++;
        }
        if(count>1) return -1;
    }
    return m;
};


// check any obviously guessable clues in row
int check_row(Game *g, int j){
    int i, m,k;
    
    for(i=0; i < g->n; i++){ // find if there is only 1 tile left
        m = last_tile_in_block(g, i, j);
        if(m>=0){
            guess_tile(g, i, j, m);
            return 1;
        }
    }
    
    for(k=0; k<g->n;k++){
        m=last_tile_in_row(g, j, k); // check if there is only 1 left of this tile
        if(m>=0){
            guess_tile(g, m, j, k);
            return 1;
        }
    }
    return 0;
};

void hide_tile_and_check(Game *g, int i, int j, int k){
    g->tile[i][j][k]=0;
    check_row(g, j);
};

void guess_tile(Game *g, int i, int j, int k){
    int m;
    
    g->guess[i][j] = k;
    g->guessed++;
    for(m=0; m<g->n; m++)
        if(m!=k) g->tile[i][j][m]=0;  // hide all tiles from this block
    
    for(m=0; m<g->n; m++){
        if(m!=i) g->tile[m][j][k]=0; // hide this tile in all blocks
    }
    
    check_row(g, j);
};

int is_guessed(Game *g, int j, int k){
    int i;
    
    for(i=0; i<g->n; i++){
        if(g->guess[i][j] == k)
            return 1;
    }
    
    return 0;
};

void unguess_tile(Game *g,  int i, int j){
    int m, k;
    
    k = g->guess[i][j];
    g->guess[i][j] = -1;
    g->guessed--;
    
    for(m=0; m<g->n; m++){
        if(!is_guessed(g, j, m)) g->tile[i][j][m]=1;
        if(g->guess[m][j]<0) g->tile[m][j][k]=1;
    }
};

int is_clue_valid(Game *g, Clue *clue){
    int ret=0;
    int i0, i1, i2, j0, k0, j1, k1, j2, k2;
    
    j0 = clue->j[0]; k0 = clue->k[0];
    j1 = clue->j[1]; k1 = clue->k[1];
    j2 = clue->j[2]; k2 = clue->k[2];
    i0 = g->where[j0][k0];
    i1 = g->where[j1][k1];
    i2 = g->where[j2][k2];
    
    ret=1;
    
    switch(clue->rel){
        case ONE_SIDE:
            if(g->where[j0][k0] >= g->where[j1][k1]) ret=0;
            break;
        case TOGETHER_2:
            if(g->where[j0][k0] != g->where[j1][k1]) ret=0;
            break;
        case TOGETHER_3:
            if((g->where[j0][k0] != g->where[j1][k1]) || (g->where[j0][k0] != g->where[j2][k2]))
                ret=0;
            break;
        case TOGETHER_NOT_MIDDLE:
            if((g->where[j0][k0] == g->where[j1][k1]) || (g->where[j0][k0] != g->where[j2][k2]))
                ret=0;
            break;
        case NOT_TOGETHER:
            if((g->where[j0][k0] == g->where[j1][k1]) || (g->where[j1][k1] == g->where[j2][k2]))
                ret=0;
            break;
        case NEXT_TO:
            if((g->where[j0][k0] - g->where[j1][k1] != 1) && (g->where[j0][k0] - g->where[j1][k1] != -1))
                ret=0;
            if((g->where[j2][k2] - g->where[j1][k1] != 1) && (g->where[j2][k2] - g->where[j1][k1] != -1))
                ret=0;
            break;
    
        case NOT_NEXT_TO:
            if((g->where[j0][k0] - g->where[j1][k1] == 1) || (g->where[j0][k0] - g->where[j1][k1] == -1))
                ret=0;
            if((g->where[j2][k2] - g->where[j1][k1] == 1) || (g->where[j2][k2] - g->where[j1][k1] == -1))
                ret=0;
            break;
            
        case CONSECUTIVE:
           if( !((i1 == i0+1) && (i2 == i0+2)) && !((i1 == i2+1) && (i0 = i2+2)) )
               ret=0;
            break;
            
        case NOT_MIDDLE:
            if(i0-i2 == 2){
                if(i0-i1 == 1) ret=0;
            } else if(i2-i0 == 2){
                if(i1-i0 == 1) ret=0;
            } else
                ret=0;
            break;
        
        case TOGETHER_FIRST_WITH_ONLY_ONE:
            break;
        default:
            break;
    }
    return ret;
};

int is_vclue(RELATION rel){
    return((rel == TOGETHER_2) || (rel == TOGETHER_3) || (rel == NOT_TOGETHER) || (rel == TOGETHER_NOT_MIDDLE) || (rel == TOGETHER_FIRST_WITH_ONLY_ONE));
}