//
//  tutorial.c
//  watson-tests
//
//  Created by koro on 2/18/16.
//  Copyright (c) 2016 koro. All rights reserved.
//
/*
#include "tutorial.h"
#include "main.h"
#include "gui.h"
#include "allegro_stuff.h"

void tutorial(Game *g, Board *b, ALLEGRO_EVENT_QUEUE *queue){
    draw_center_textbox_wait("Welcome to the tutorial of Watson, the puzzle game.\n\n"
                             "The objective of the game is to deduce the position of each item in themain panel. Each column must have one item of each type. Each item should appear exactly once inthe panel.\n\n"
                             "There are a number of groups of items of the same type. The default number is 6, but this can be changed in the settings, as well as the number of columns.\n\n"
                             "Each column has block for each group of same-type items.\n\n"
                             "Press a key to continue.", 0.5, b, queue);

    b->highlight = &b->panel;
    show_info_text(b, "This is the main panel. Since we initially don't know the positions of the items, each block displays all possible items.");
    draw_stuff(b);
    al_flip_display();
    draw_stuff(b);
    blink_TB(&b->panel);
    wait_for_input(queue);

    b->highlight = NULL;
    switch_solve_puzzle(g,b);
    show_info_text(b, "This is what the solved puzzle looks like.");
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);
    switch_solve_puzzle(g,b);

    b->highlight = &b->vclue;
    show_info_text(b, "This is the vertical clue panel. Each clue tells us something about the relative position of items in a column.");
    draw_stuff(b);
    al_flip_display();
    draw_stuff(b);
    blink_TB(&b->vclue);
    wait_for_input(queue);

    b->highlight = &b->hclue;
    show_info_text(b, "This is the horizontal clue panel. Each clue tells us something about the relative position of the columns of some items.");
    draw_stuff(b);
    al_flip_display();
    draw_stuff(b);
    blink_TB(&b->hclue);
    wait_for_input(queue);

    b->highlight = NULL;
    show_info_text(b, "There are several types of clues. During the game, you can read an explanation of the meaning of a clue by left-clicking on it.");
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

    b->highlight = NULL;
    show_info_text(b, "To illustrate how to use the clues to solve a puzzle, I will show you the first steps for this game.");
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

    explain_clue(b, &g->clue[1]);
    b->highlight=b->clue_tiledblock[1];
    draw_stuff(b);

    draw_center_textbox_wait("This puzzle starts without any clue revealed. Since the clues only give information about relative poisitions of items, we have to rely on the columns on the edges to rule out some initial items.\n\n"
                             "We begin with the highlighted clue. You can read its description in the bottom panel." , 0.5, b, queue);

    show_info_text_b(b, "Since the last column has no columns to its left, %b cannot be there. We remove it with a right-click (or touch).", b->clue_unit_bmp[3][5]);
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

    hide_tile_and_check(g, 5, 3, 5);
    update_board(g, b);
    draw_stuff(b);
    al_flip_display();
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    wait_for_input(queue);

    b->highlight=b->clue_tiledblock[21];
    show_info_text_b(b, "This clue says that %b is in the same column as %b and %b. Thus we can rule out these last two items too.",  b->clue_unit_bmp[3][5], b->clue_unit_bmp[1][2], b->clue_unit_bmp[2][3]);
    draw_stuff(b);
    blink_TB(b->clue_tiledblock[21]);
    wait_for_input(queue);

    hide_tile_and_check(g, 5, 1, 2);
    update_board(g,b);
    draw_stuff(b);
    al_flip_display();
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    al_rest(0.4);
    hide_tile_and_check(g, 5, 2, 3);
    update_board(g,b);
    draw_stuff(b);
    al_flip_display();
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    al_rest(0.5);

    b->highlight=b->clue_tiledblock[11];
    explain_clue(b, &g->clue[11]);
    draw_stuff(b);
    draw_center_textbox_wait("This other clue says that the column of the middle item is between the columns of two other items (read description below). Thus we can remove it from the first and last columns.\n\nNote: This type of clue only says that one item is between other two, but it doesn't say which one is on the left and which on the right.", 0.5, b, queue);

    hide_tile_and_check(g, 5, 5,5);
    update_board(g,b);
    draw_stuff(b);
    al_flip_display();
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    al_rest(0.4);
    hide_tile_and_check(g, 0, 5, 5);
    update_board(g,b);
    draw_stuff(b);
    al_flip_display();
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    wait_for_input(queue);

    b->highlight=NULL;

    hide_tile_and_check(g, 0, 1,1);
    hide_tile_and_check(g, 0, 3,2);
    hide_tile_and_check(g, 0, 4,0);
    hide_tile_and_check(g, 0, 4,0);
    hide_tile_and_check(g, 5, 4,1);
    hide_tile_and_check(g, 4, 4,1);
    hide_tile_and_check(g, 4, 0,0);
    hide_tile_and_check(g, 4, 1,2);
    hide_tile_and_check(g, 4, 2,3);
    hide_tile_and_check(g, 4, 3,5);
    hide_tile_and_check(g, 5, 0,1);
    hide_tile_and_check(g, 5, 1,0);
    hide_tile_and_check(g, 5, 1,1);
    hide_tile_and_check(g, 5, 1,4);
    hide_tile_and_check(g, 5, 0,3);
    update_board(g,b);

    show_info_text_b(b, "Moving forward in the game, we reach the current situation.");
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

    b->highlight=b->clue_tiledblock[19];
    show_info_text_b(b, "Since %b must be in the same column as %b, and since %b is not in the last column, we can remove %b from there.", b->clue_unit_bmp[1][5], b->clue_unit_bmp[0][3],  b->clue_unit_bmp[0][3], b->clue_unit_bmp[1][5]);
    draw_stuff(b);
    al_flip_display();
    blink_TB(b->clue_tiledblock[19]);
    blink_TB(b->panel.b[5]->b[1]->b[5]);
    wait_for_input(queue);

    b->highlight=NULL;
    hide_tile_and_check(g, 5, 1,5);
    update_board(g,b);
    if(!set.sound_mute) play_sound(SOUND_HIDE_TILE);
    show_info_text_b(b, "Since %b was the last tile left of its type, it was marked as 'guessed'.", b->clue_unit_bmp[1][3]);
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

    b->highlight=b->clue_tiledblock[17];
    show_info_text_b(b, "But this clue tells us that %b is on the same column as %b. So we 'guess' it with a left-click (or long-press)", b->clue_unit_bmp[2][1], b->clue_unit_bmp[1][3]);
    draw_stuff(b);
    blink_TB(b->clue_tiledblock[17]);
    al_flip_display();
    wait_for_input(queue);

    guess_tile(g, 5, 2, 1);
    play_sound(SOUND_GUESS_TILE);
    update_board(g,b);
    draw_stuff(b);
    al_flip_display();
    wait_for_input(queue);

}
*/