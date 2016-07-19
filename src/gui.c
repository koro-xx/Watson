// Based in example.c
// from the WidgetZ GUI library by Pavel Sountsov


#include "gui.h"
#include <stdio.h>
#include <math.h>
#include "widgetz/widgetz.h"
#include "allegro_stuff.h"
#include "main.h"
#include "text.h"
#include "game.h"

#define GUI_BG_COLOR al_map_rgb(100, 100, 100) //al_map_rgb(108, 122, 137);//(0.1, 0.6, 0.8, 1)
#define GUI_TEXT_COLOR al_map_rgb(255, 255, 255)

const char ABOUT_TEXT[] = "Watson v" PRE_VERSION " - " PRE_DATE ", by Koro.\n"
"\n"
"Watson is an open source clone of \"Sherlock\", an old game by Evertt Kaser which is itself based on the a classic puzzle known as \"Zebra puzzle\" or \"Einstein's riddle\".\n"
"\n"
"The game has been compiled in Windows, Mac OS, Linux and Android thanks to the amazing Allegro 5 library. Big thanks to the friendly folks from Freenode #allegro for all the tips and advice. The game GUI uses the WidgetZ library by SiegeLord.\n"
"\n"
"The tile set is rendered from TTF fonts obtained from fontlibrary.org. The icons are from www.icons8.com. These icons are located in <appdir>/icons and can be replaced. The sounds are from freesound.org. The text font is Linux Libertine by Philipp H. Poll. All these assets can be found in their corresponding directories."
"\n"
"Watson is licensed under the GPLv3 (this does not include the artwork cited above). The source code and some binary releases can be found at https://github.com/koro-xx/Watson"
"\n"
"\n"
"Note: the \"advanced\" mode generates (much) harder puzzles, which require indirect logic and hasn't been properly tested.\n";

const char HELP_TEXT[]="Watson is a puzzle similar to the classic \"Zebra puzzle\" or \"Einstein's Riddle\". The goal is to figure out which item goes where on the board.\n"
"The main panel has a number of columns, each dividided into blocks of same-type items. Each item in a block is secretly assigned to a different column, without repetition. Some of these assignments may be partially revealed at the beginning. Use the clues provided (right and bottom panels) to deduce which item goes where. \n"
"\n"
"QUICK REFERENCE\n"
"Press the lightbulb for a hint. You can understand the game logic from it.\n"
"Click/tap on a clue to read about its meaning.\n"
"Click/tap on a panel tile to rule it out. Hold-click on hidden tiles to bring them back.\n"
"Hold-click on a panel tile to assign it to a given column.\n"
"Double-click/tap on a clue to hide/show (if you don't need it anymore).\n"
"Drag the clues to rearrange.\n"
"\n"
"There is an undo button avialable. Go to settings to switch the tileset and other additional options."
"\n"
"Keyboard shortcuts: R to start again, ESC to quit, U to undo, C to get a hint, T to switch tiles. You can resize the window or press F to go fullscreen.\n"
"\n"
"DEBUG: S: show/hide solution.\n";

float GUI_XFACTOR = MOBILE ? 0.9 : 0.5;
float GUI_YFACTOR = MOBILE ? 0.9 : 0.5;

WZ_SKIN_THEME skin_theme;
ALLEGRO_FONT *gui_font = NULL;
int gui_font_h;
WZ_WIDGET *base_gui = NULL;

Settings nset = {0};

char hi_name[10][64];
double hi_score[10];
int hi_pos = -1;

enum {
    BUTTON_ROWS,
    BUTTON_COLS,
    BUTTON_OK,
    BUTTON_CANCEL,
    BUTTON_SOUND,
    BUTTON_ADVANCED,
    BUTTON_RESTART,
    BUTTON_ABOUT,
    BUTTON_EXIT,
    BUTTON_TILES,
    GROUP_COLS,
    GROUP_ROWS,
    BUTTON_SAVE,
    BUTTON_LOAD,
    BUTTON_ZOOM,
    BUTTON_SETTINGS,
    BUTTON_PARAMS,
    BUTTON_CLOSE,
    BUTTON_SAVE_NOW,
    BUTTON_EXIT_NOW,
    BUTTON_LOAD_NOW,
    BUTTON_RESTART_NOW,
    BUTTON_EXTRA_HARD,
    BUTTON_RESET_PARAMS,
    GUI_SETTINGS,
    GUI_PARAMS,
    EDITBOX_HISCORE,
};

// even if wgt->own = 1, the original function duplicates the string
// this avoids a memory leak
void wz_set_text_own(WZ_WIDGET* wgt, ALLEGRO_USTR* text){
    wz_set_text(wgt, text);
    al_ustr_free(text);
}

//void register_gui(Board *b, WZ_WIDGET *gui){
//        b->gui[b->gui_n] = gui;
//        b->gui_n++;
//}
//
//
//
//void unregister_gui(Board *b, WZ_WIDGET *gui){
//    int i,j;
//    
//    // find gui index
//    for(i=b->gui_n-1; b>=0; i--)
//        if(b->gui[i] == gui)
//            break;
//    
//    // in case not found
//    if(i < 0)
//        return;
//    
//    // shift remaining guis
//    b->gui_n--;
//    for(j=i; j<b->gui_n; j++)
//        b->gui[j] = b->gui[j+1];
//}


void draw_guis(void)
{
    WZ_WIDGET *gui;
    
    if(!base_gui) return;
    
    gui = base_gui->first_child;
    
    while(gui)
    {
        wz_draw(gui);
        gui = gui->next_sib;
    }
}

// unused
void init_theme_noskin(void)
{
    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_def_theme, sizeof(wz_def_theme));
    gui_font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -gui_font_h);
    skin_theme.theme.font = gui_font;
    skin_theme.theme.color1 = al_map_rgb(150, 150, 150);
    skin_theme.theme.color2 = WHITE_COLOR;
}

void init_theme(void)
{
    memset(&skin_theme, 0, sizeof(skin_theme));
    memcpy(&skin_theme, &wz_skin_theme, sizeof(skin_theme));
    gui_font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -gui_font_h);
    skin_theme.theme.font = gui_font;
    skin_theme.theme.color1 = GUI_BG_COLOR;
    skin_theme.theme.color2 = GUI_TEXT_COLOR;
    skin_theme.button_up_bitmap = al_load_bitmap("data/button_up.png");
    skin_theme.button_down_bitmap =al_load_bitmap("data/button_down.png");
    skin_theme.box_bitmap = al_load_bitmap("data/box.png");
    skin_theme.editbox_bitmap = al_load_bitmap("data/editbox.png");
    skin_theme.scroll_track_bitmap = al_load_bitmap("data/scroll_track.png");
    skin_theme.slider_bitmap = al_load_bitmap("data/slider.png");
    wz_init_skin_theme(&skin_theme);
}

void scale_gui(Board *b, float factor)
{
    gui_font_h *= factor;
    al_destroy_font(gui_font);
    gui_font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -gui_font_h);
    skin_theme.theme.font = gui_font;
    //    wz_resize(blah) resize all guis
}

void destroy_theme()
{
    ndestroy_bitmap(skin_theme.button_up_bitmap);
    ndestroy_bitmap(skin_theme.button_down_bitmap);
    ndestroy_bitmap(skin_theme.box_bitmap);
    ndestroy_bitmap(skin_theme.editbox_bitmap);
    ndestroy_bitmap(skin_theme.scroll_track_bitmap);
    ndestroy_bitmap(skin_theme.slider_bitmap);
    al_destroy_font(gui_font);
    gui_font = NULL;
    skin_theme.theme.font = NULL;
}


WZ_WIDGET* create_fill_layout(WZ_WIDGET* parent, float x, float y, float w, float h, float hspace, float vspace, int halign, int valign, int id)
{
    WZ_WIDGET *wgt = (WZ_WIDGET *)wz_create_fill_layout(parent, x, y, w, h, hspace, vspace, halign, valign, id);
    wgt->flags |= WZ_STATE_HIDDEN;
    
    return wgt;
}

WZ_WIDGET* new_widget(int id, int x, int y){
    
    WZ_WIDGET *wgt;
    wgt=wz_create_widget(0, x, y, id);
    wz_set_theme(wgt, (WZ_THEME*) &skin_theme);
    return wgt;
}

void update_guis(int x, int y, int w, int h){
    gui_font_h = h/35;
    al_destroy_font(gui_font);
    gui_font = load_font_mem(text_font_mem, TEXT_FONT_FILE, -gui_font_h);
    wz_resize(base_gui, (float)h/base_gui->h);
    base_gui->x = x;
    base_gui->y = y;
    base_gui->w = w;
    base_gui->h = h;
    
    skin_theme.theme.font = gui_font;
    wz_update(base_gui, 0);
    emit_event(EVENT_REDRAW);
}

void init_guis(int x, int y, int w, int h){
    gui_font_h = h/35;
    init_theme();
    base_gui = new_widget(-1, x, y);
    base_gui->w = w;
    base_gui->h = h;
}

void remove_all_guis(void)
{
    while(base_gui->last_child)
        remove_gui(base_gui->last_child);
}

void destroy_base_gui()
{
    remove_all_guis();
    wz_destroy(base_gui);
    destroy_theme();
}

WZ_WIDGET* create_msg_gui(int id, ALLEGRO_USTR *msg)
{
    int w = base_gui->w/2;
    int h = gui_font_h*get_multiline_text_lines(gui_font, w, al_cstr(msg));
    int i;
    int but_w = 6*gui_font_h;
    int but_h = gui_font_h*1.5;
    WZ_WIDGET *wgt, *gui;
    
    for(i=0; i<3; i++)
    {
        if(h + gui_font_h*3 + but_h > base_gui->h)
        {
            w += base_gui->w/4.1;
            h = gui_font_h*get_multiline_text_lines(gui_font, w, al_cstr(msg));
        }
        else
        {
            break;
        }
    }
    
    gui = new_widget(id, (base_gui->w - w - 2*gui_font_h)/2, (base_gui->h - (4*gui_font_h + h))/2);
    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, w + 2*gui_font_h, h + 4*gui_font_h, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    
    wz_create_textbox(gui, gui_font_h, gui_font_h, w, h, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, msg, 1, -1);
    wgt = (WZ_WIDGET*) wz_create_button(gui, gui_font_h + (w-but_w*1.5), h+gui_font_h*2, but_w, but_h, al_ustr_new("OK"), 1, BUTTON_CLOSE);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    
    return gui;
}


WZ_WIDGET* create_yesno_gui(int id, int button_ok_id, int button_cancel_id, ALLEGRO_USTR *msg){
    int but_w = 6*gui_font_h;
    int but_h = gui_font_h*1.5;
    int w = max(base_gui->w/3, 2*but_w + 3*gui_font_h);
    int h = gui_font_h*get_multiline_text_lines(gui_font, w, al_cstr(msg));
    
    WZ_WIDGET *wgt, *gui = new_widget(id, (base_gui->w - w - 2*gui_font_h)/2, (base_gui->h - (3*gui_font_h + but_h + h))/2);
    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, w + 2*gui_font_h, h + 3*gui_font_h + but_h, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    
    //    wz_create_fill_layout(gui, 0, 0, w+2*b->tsize, h+2*b->tsize, b->tsize, b->tsize, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, gui_font_h, gui_font_h, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, msg, 1, -1);
    wz_create_button(gui, gui_font_h + (w-2*but_w)/3, 2*gui_font_h + h, but_w, but_h, al_ustr_new("OK"), 1, button_ok_id);
    wgt = (WZ_WIDGET *) wz_create_button(gui, gui_font_h + but_w + 2*(w-2*but_w)/3, 2*gui_font_h + h, but_w, but_h, al_ustr_new("Cancel"), 1, button_cancel_id);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    return gui;
}


WZ_WIDGET *create_settings_gui(void)
{
    WZ_WIDGET *gui, *wgt;
    int fh = gui_font_h;
    int rh = 2.5*fh; // row height
    int rn = 0; // current row
    int gui_w = fh*20;
    int rows = 7;
    int gui_h = rows*rh;
    WZ_WIDGET *button_mute;
    int but_w = 30;
    int but_sw = 15;
    int but_h;
    int sep = 10;
    int i;
    
    nset = set;
    
    but_w = max(al_get_text_width(skin_theme.theme.font, "Save game"), but_w);
    but_w = max(al_get_text_width(skin_theme.theme.font, "Load game"), but_w);
    but_w = max(al_get_text_width(skin_theme.theme.font, "Switch tiles"), but_w);
    but_w = max(al_get_text_width(skin_theme.theme.font, "About Watson"), but_w);
    but_h = fh*2;
    
    but_sw = max(al_get_text_width(skin_theme.theme.font, "zoom"), but_sw);
    but_sw += fh;
    but_w += 2*fh;
    
    gui_w = max(gui_w, 3*(but_w+sep) + sep*3);
    
    // main gui
    gui = new_widget(GUI_SETTINGS, (base_gui->w - gui_w)/2, (base_gui->h-rows*rh)/2);
//    wz_set_theme(gui, (WZ_THEME*)&skin_theme);
    
    wgt = wz_create_box(gui, 0, 0, gui_w, gui_h+sep, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    
    // about button
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(skin_theme.theme.font, "Settings"), but_h, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new("Settings"), 1, -1);
    
    
    // xxx todo: move "advanced" to tune pane.  Make sound on/off a single button, same for advanced.
    //    wz_create_textbox(gui, 0, 0, al_get_text_width(skin_theme.theme.font, "Advanced: "), but_h, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Advanced:"), 1, -1);
    //    button_advanced = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, but_sw, but_h, set->advanced ? al_ustr_new("on") : al_ustr_new("off"), 1, -1, BUTTON_ADVANCED);
    //    ((WZ_BUTTON*) button_advanced)->down = set->advanced;
    
    
    // number of rows multitoggle
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(skin_theme.theme.font, "Columns: "), but_h, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Rows:"), 1, -1);
    for(i=0; i<5; i++)
    {
        wgt = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, but_h, but_h, al_ustr_newf("%c", '4'+i), 1, GROUP_ROWS, BUTTON_ROWS);
        if(4+i == set.h) ((WZ_BUTTON *)wgt)->down = 1;
    }

    // number of columns multitoggle
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_textbox(gui, 0, 0, al_get_text_width(skin_theme.theme.font, "Columns: "), but_h, WZ_ALIGN_RIGHT, WZ_ALIGN_CENTRE, al_ustr_new("Columns:"), 1, -1);
    for(i=0; i<5; i++)
    {
        wgt = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, but_h, but_h, al_ustr_newf("%c", '4'+i), 1, GROUP_COLS, BUTTON_COLS);
        if(4+i == set.n) ((WZ_BUTTON *)wgt)->down = 1;
    }
    
    // sound + swtich tiles + zoom
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wgt = (WZ_WIDGET*) wz_create_toggle_button(gui, 0, 0, but_w, but_h, al_ustr_new("Zoom"), 1, -1, BUTTON_ZOOM);
    ((WZ_BUTTON*) wgt)->down = set.fat_fingers;
    
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Switch tiles"), 1, BUTTON_TILES);
    button_mute = (WZ_WIDGET*) wz_create_toggle_button(gui, 0, 0, but_w, but_h, set.sound_mute ? al_ustr_new("Sound: off") : al_ustr_new("Sound: on"), 1, -1, BUTTON_SOUND);
    ((WZ_BUTTON*) button_mute)->down = !set.sound_mute;
    
    
    // advanced + save + load buttons
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Advanced"), 1, BUTTON_PARAMS);
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Save game"), 1, BUTTON_SAVE);
    wgt = (WZ_WIDGET*)wz_create_toggle_button(gui, 0, 0, but_w, but_h, al_ustr_new("Load game"), 1, -1, BUTTON_LOAD);
    ((WZ_BUTTON*) wgt)->down = !set.saved;
    
    // restart/exit/switch tiles buttons
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, sep, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("New game"), 1, BUTTON_RESTART);
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Exit game"), 1, BUTTON_EXIT);
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("About Watson"), 1, BUTTON_ABOUT);
    
    // ok/cancel buttons
    create_fill_layout(gui, 0, rh*(rn++), gui_w, rh, but_w/3, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("OK"), 1, BUTTON_OK);
    wgt = (WZ_WIDGET*) wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Cancel"), 1, BUTTON_CLOSE);
    // escape key cancels and exits
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
    
    return gui;
}

WZ_WIDGET *create_win_gui(double time)
{
    // Initialize Allegro 5 and the font routines
    WZ_WIDGET* gui;
    WZ_WIDGET* wgt;
    int gui_w, gui_h, but_w, lh;
    int i,j;
    
#ifdef ALLEGRO_ANDROID
    al_android_set_apk_file_interface();
#endif
    
    lh = 1.2*gui_font_h;
    gui_w = al_get_text_width(gui_font, "You solved the puzzle in 000:000:000") + 4*lh + 2;
    but_w = 2*lh + max(al_get_text_width(gui_font, "Settings"), al_get_text_width(gui_font, "New game"));
    
    // 13 lines of text + 1.5 for button + 2 for margin = lh*16 (+17 * vspace?)
    gui_h = 16.5*lh+2;
    
    gui = new_widget(-1, (base_gui->w - gui_w)/2, (base_gui->h - gui_h)/2);
    wz_create_fill_layout(gui, 0, 0, gui_w, gui_h, lh, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_TOP, -1);
    wz_create_textbox(gui, 0, 0, gui_w-2*lh-2, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);

    get_highscores(set.n, set.h, set.advanced, hi_name, (double *)hi_score);
    if(time > 0)
    {
        for(i=0; i<10; i++)
        {
            if(hi_score[i]>time) break;
        }
    }
    else
    {
        i = 10;
    }

    if(time > 0)
        wz_create_textbox(gui, 0, 0, gui_w-2*lh-2, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_newf("You solved the puzzle in %02d:%02d:%02d", (int)time/ 3600, (int)time/60, (int)time %60 ), 1, -1);

    wz_create_textbox(gui, 0, 0, gui_w-2*lh-2, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_newf("Best times for %d x %d board:", set.n, set.h), 1, -1);
    
    if(!(time > 0))
        wz_create_textbox(gui, 0, 0, gui_w-2*lh-2, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
    
    for(j=0;j<i;j++)
    {
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new(hi_name[j]), 1, -1);
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/5, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_newf("%02d:%02d:%02d", (int)hi_score[j]/3600, ((int)hi_score[j]/60)%60, (int)hi_score[j] % 60), 1, -1);
    }
    
    if (i>=10)
    {
        hi_pos = -1;
    }
    else
    {
        hi_pos = i;
#ifdef ALLEGRO_ANDROID
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new("You"), 1, -1); // get_highscores(g);
#else
        wgt = (WZ_WIDGET*)wz_create_editbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, al_ustr_new("your name"), 1, EDITBOX_HISCORE);
#endif
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/5, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
        wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_newf("%02d:%02d:%02d", (int) time/3600, ((int)time/60)%60, (int)time % 60), 1, -1);
        
        for(j=i;j<9;j++)
        {
            wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_new(hi_name[j]), 1, -1);
            wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/5, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
            wz_create_textbox(gui, 0, 0, (gui_w-4*lh-2)/2.5, lh, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, al_ustr_newf("%02d:%02d:%02d", (int)hi_score[j]/3600, ((int)hi_score[j]/60)%60, (int)hi_score[j] % 60), 1, -1);
            
        }
        memcpy(&hi_name[i+1], &hi_name[i], 64*sizeof(char)*(9-i));
        memcpy(&hi_score[i+1], &hi_score[i], sizeof(double)*(9-i));
        hi_score[i] = time;
        hi_name[i][0] = '\0';
    }
    
    wz_create_textbox(gui, 0, 0, gui_w*0.9, lh, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
    
    wz_create_button(gui, 0, 0, but_w, 1.5*lh, al_ustr_new("New Game"), 1, BUTTON_RESTART_NOW);
    wz_create_button(gui, 0, 0, but_w, 1.5*lh, al_ustr_new("Settings"), 1, BUTTON_SETTINGS);
    
    return gui;
}


WZ_WIDGET *create_params_gui()
{
    WZ_WIDGET *gui, *wgt;
    int i;
    ALLEGRO_USTR *rel[NUMBER_OF_RELATIONS];
    int gui_h = 0, gui_w = 0, rel_w=0, but_w=0, but_h =0;
    int lh = gui_font_h * 1.5;
    
    rel[0] = al_ustr_new("NEXT_TO");
    rel[1] = al_ustr_new("NOT_NEXT_TO");
    rel[2] = al_ustr_new("ONE_SIDE");
    rel[3] = al_ustr_new("CONSECUTIVE");
    rel[4] = al_ustr_new("NOT_MIDDLE");
    rel[5] = al_ustr_new("TOGETHER_2");
    rel[6] = al_ustr_new("TOGETHER_3");
    rel[7] = al_ustr_new("NOT_TOGETHER");
    rel[8] = al_ustr_new("TOGETHER_NOT_MIDDLE");
    rel[9] = al_ustr_new("TFWOO (disabled)");
    rel[10] = al_ustr_new("REVEAL");
    
    but_w = al_get_text_width(gui_font, "Extra hard") + 2*gui_font_h;
    
    for(i=0;i<=10;i++)
    {
        rel_w = max(rel_w, al_get_text_width(gui_font, al_cstr(rel[i])));
    }
    
    gui_w = max(base_gui->w/2.5, 2*gui_font_h + al_get_text_width(gui_font, "Clue type distribution for puzzle creation"));
    but_h = lh;
    gui_h = (NUMBER_OF_RELATIONS + 2)*lh;
    
    gui = new_widget(GUI_PARAMS, (base_gui->w - gui_w)/2, (base_gui->h - gui_h - (2*but_h + 1.5*gui_font_h))/2);
    
    wz_create_fill_layout(gui, 0, 0, gui_w, gui_h, gui_font_h, 0, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, -1);
    
    wz_create_textbox(gui, 0, 0, gui_w- 2*gui_font_h - 2, lh*2, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new("Clue type distribution for puzzle creation"), 1, -1);
    
    for(i=0; i< NUMBER_OF_RELATIONS; i++)
    {
        wz_create_textbox(gui, 0, 0, rel_w, lh, WZ_ALIGN_RIGHT, WZ_ALIGN_TOP, rel[i], 1, -1);
        wgt = (WZ_WIDGET*) wz_create_scroll(gui, 0, 0, gui_w - rel_w - 3*gui_font_h - 2, lh/2, 50, gui_font_h, i + 1024);
        ((WZ_SCROLL*)wgt)->cur_pos = REL_PERCENT[i];
    }

    // buttons
    wz_create_fill_layout(gui, 0, gui_h, gui_w, 2*but_h + 1.5*gui_font_h, (gui_w - 2*but_w)/3.1 , gui_font_h/2, WZ_ALIGN_CENTRE, WZ_ALIGN_LEFT, -1);
    
    wgt = (WZ_WIDGET *)wz_create_toggle_button(gui, 0, 0, but_w, but_h, al_ustr_new("Extra hard"), 1, -1, BUTTON_EXTRA_HARD);
    ((WZ_BUTTON *)wgt)->down = nset.advanced ? 1 : 0;
  
    wgt = (WZ_WIDGET *)wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Reset"), 1, BUTTON_RESET_PARAMS);
    
    wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("OK"), 1, BUTTON_OK);
    //    wz_create_textbox(gui, 0, 0, but_w/2, but_h, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, al_ustr_new(""), 1, -1);
    
    wgt = (WZ_WIDGET *) wz_create_button(gui, 0, 0, but_w, but_h, al_ustr_new("Cancel"), 1, BUTTON_CLOSE);
    wz_set_shortcut(wgt, ALLEGRO_KEY_ESCAPE, 0);
 
    return gui;
}



WZ_WIDGET *create_text_gui(ALLEGRO_USTR *text)
{
    int w = base_gui->w/3;
    int h = gui_font_h*get_multiline_text_lines(gui_font, w, al_cstr(text));
    int i;
    WZ_WIDGET *wgt, *gui;
    
    for(i=0; i<3; i++)
    {
        if(h + gui_font_h*2 > base_gui->h)
        {
            w += base_gui->w/6;
            h = gui_font_h*get_multiline_text_lines(gui_font, w, al_cstr(text));
        }
        else
        {
            break;
        }
    }
    
    gui = new_widget(-1, (base_gui->w - w - 2*gui_font_h)/2, (base_gui->h - 2*gui_font_h)/2);
    
    wgt = (WZ_WIDGET*) wz_create_box(gui, 0, 0, w + 2*gui_font_h, h + 2*gui_font_h, -1);
    wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
    
    wz_create_textbox(gui, gui_font_h, gui_font_h, w, h, WZ_ALIGN_LEFT, WZ_ALIGN_TOP, text, 1, -1);
    
    wz_update(gui, 0);
    return gui;
}


void confirm_restart(Settings *new_set)
{
    nset = *new_set;
    add_gui(base_gui, create_yesno_gui(-1, BUTTON_RESTART_NOW, BUTTON_CLOSE, al_ustr_newf("Start new %dx%d%s game?", nset.n, nset.h, nset.advanced ? " advanced" : "")));
}

void confirm_exit(void)
{
    add_gui(base_gui, create_yesno_gui(-1, BUTTON_EXIT_NOW, BUTTON_CLOSE, al_ustr_newf("Exit game?")));
}

void show_help(void){
    add_gui(base_gui, create_msg_gui(-1, al_ustr_new(HELP_TEXT)));
}

void show_about(void){
    add_gui(base_gui, create_msg_gui(-1, al_ustr_new(ABOUT_TEXT)));
}

void confirm_save(void)
{
    add_gui(base_gui, create_yesno_gui(-1, BUTTON_SAVE_NOW, BUTTON_CLOSE, al_ustr_new("Save game? This will overwrite\na previous save.")));
}

void confirm_load(void)
{
    add_gui(base_gui, create_yesno_gui(-1, BUTTON_LOAD_NOW, BUTTON_CLOSE, al_ustr_new("Discard current progress and load saved game?")));
}
            
void show_settings(void)
{
    add_gui(base_gui, create_settings_gui());
}

void show_win_gui(double time)
{
    add_gui(base_gui, create_win_gui(time));
}

void show_params(void)
{
    add_gui(base_gui, create_params_gui());
}

//
//void draw_center_textbox_wait(const char *text, float width_factor, Board *b, ALLEGRO_EVENT_QUEUE *queue){
////#ifndef ALLEGRO_ANDROID
////    ALLEGRO_BITMAP *keep = screenshot();
////#else
////    al_clear_to_color(BLACK_COLOR);
////#endif
//    draw_stuff(b);
//    draw_multiline_wz_box(text, b->all.x+b->xsize/2, b->all.y + b->ysize/2, width_factor*b->xsize, b->max_ysize);
//    al_wait_for_vsync();
//    al_flip_display();
//    wait_for_input(queue);
////#ifndef ALLEGRO_ANDROID
////    al_draw_bitmap(keep,0,0,0);
////    al_destroy_bitmap(keep);
////#else
////    al_clear_to_color(BLACK_COLOR);
////#endif
//}


int handle_gui_event(ALLEGRO_EVENT *event)
{
    WZ_WIDGET *wgt = (WZ_WIDGET *)event->user.data2;
    WZ_WIDGET *gui = (WZ_WIDGET *) wgt->parent;
    
    if(!gui)
    {
        deblog("Can't handle unparented gui.");
        return 0;
    }
    
    if(event->type == WZ_TEXT_CHANGED)
    {
        if(wgt->id == EDITBOX_HISCORE)
        {
            strncpy(hi_name[hi_pos], al_cstr(((WZ_TEXTBOX *) wgt)->text), 63);
            wgt->flags &= ~WZ_STATE_HAS_FOCUS;
            wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
            save_highscores(set.n, set.h, set.advanced, hi_name, hi_score);
            remove_gui(wgt);
            add_gui(base_gui, create_win_gui(-1));
        }
    }
    
    // general buttons
    if(event->type == WZ_BUTTON_PRESSED)
    {
        switch(wgt->id)
        {
            case BUTTON_CLOSE:
                remove_gui(gui);
                break;
                
            case BUTTON_EXIT_NOW:
                emit_event(EVENT_EXIT);
                break;
                
            case BUTTON_RESTART_NOW:
                emit_event(EVENT_RESTART);
                break;
                
            case BUTTON_SAVE_NOW:
                emit_event(EVENT_SAVE);
                remove_gui(gui);
                break;
                
            case BUTTON_LOAD_NOW:
                emit_event(EVENT_LOAD);
                remove_gui(gui);
                break;
            case BUTTON_SETTINGS:
                add_gui(base_gui, create_settings_gui());
                break;
        }
    }
    
    if(gui->id == GUI_SETTINGS)
    {
        if((event->type == ALLEGRO_EVENT_KEY_CHAR) && (event->keyboard.keycode == ALLEGRO_KEY_BACK))
        { // android "back" key
            remove_gui(gui);
        }
        
        if(event->type == WZ_BUTTON_PRESSED)
        {
            switch(wgt->id)
            {
                case BUTTON_EXIT:
                    confirm_exit();
                    break;
                    
                case BUTTON_OK:
                    if( (nset.n != set.n) || (nset.h != set.h) || (nset.advanced != set.advanced) )
                    {
                        confirm_restart(&nset);
                    }
                    else
                    {
                        nset = set;
                        remove_gui(gui);
                    }
                    break;
                    
                case BUTTON_RESTART:
                    confirm_restart(&nset);
                    break;
                    
                case BUTTON_TILES:
                    emit_event(EVENT_SWITCH_TILES);
                    break;
                    
                case BUTTON_ZOOM:
                    SWITCH(set.fat_fingers);
                    nset.fat_fingers = set.fat_fingers;
                    break;
                    
                case BUTTON_ABOUT:
                    show_about();
                    break;
                    
                case BUTTON_SOUND:
                    SWITCH(set.sound_mute);
                    if(set.sound_mute){
                        wz_set_text_own((WZ_WIDGET*) event->user.data2, al_ustr_new("Sound: off"));
                    } else {
                        wz_set_text_own((WZ_WIDGET*) event->user.data2, al_ustr_new("Sound: on"));
                    }
                    nset.sound_mute = set.sound_mute;
                    break;
                    
                case BUTTON_PARAMS:
                    show_params();
                    break;
                    
                case BUTTON_SAVE:
                    if(set.saved) confirm_save();
                    break;
                    
                case BUTTON_LOAD:
                    if( !set.saved )
                    { // if no saved game exists, revert button press
                        ((WZ_BUTTON*) wgt)->down = 1;
                    }
                    else
                    {
                        confirm_load();
                    }
                    break;
                    
                case BUTTON_ROWS:
                    nset.h = atoi(al_cstr(((WZ_BUTTON *)wgt)->text));
                    break;
                    
                case BUTTON_COLS:
                    nset.n = atoi(al_cstr(((WZ_BUTTON *)wgt)->text));
                    break;
            }
        }
    }
    else if(gui->id == GUI_PARAMS)
    {
        if(event->type == WZ_BUTTON_PRESSED)
        {
            if(wgt->id == BUTTON_EXTRA_HARD)
            {
                SWITCH(nset.advanced);
            }
            else if(wgt->id == BUTTON_OK)
            {
                wgt = gui->first_child;
                while(wgt)
                {
                    if(wgt->id >= 1024) // is slider
                    {
                        REL_PERCENT[wgt->id-1024] = ((WZ_SCROLL*)wgt)->cur_pos;
                    }
                    wgt = wgt->next_sib;
                }
                
                ALLEGRO_USTR *msg = al_ustr_newf("WARNING: The puzzle generation parameters are here for debug purposes and altering them may make the game unbalanced. You can reset these settings later.\n\n%s", nset.advanced ? "You also turned on extra hard mode. This mode is not recommended. Puzzle generation will be slower, and solving the puzzles may require thinking ahead many steps and very indirect reasoning." : "");
                remove_gui(gui);
                add_gui(base_gui, create_msg_gui(-1, msg));
            }
            else if(wgt->id == BUTTON_RESET_PARAMS)
            {
                wgt = gui->first_child;
                reset_rel_params();
                nset.advanced = set.advanced = 0;
                
                // update sliders and button
                while(wgt)
                {
                    if(wgt->id >= 1024) // is slider
                    {
                        ((WZ_SCROLL*)wgt)->cur_pos = REL_PERCENT[wgt->id-1024];
                    }
                    else if(wgt->id == BUTTON_EXTRA_HARD)
                    {
                        ((WZ_BUTTON*)wgt)->down = 0;
                    }
                    
                    
                    wgt = wgt->next_sib;
                }
            }
        }
    }

    return 0;
}

void update_base_gui(float dt)
{
    wz_update(base_gui, dt);
}

void gui_send_event(ALLEGRO_EVENT *event)
{
    if(wz_send_event(base_gui->last_child, event))  emit_event(EVENT_REDRAW);
    
    if(event->type == WZ_BUTTON_PRESSED || event->type == WZ_TEXT_CHANGED)
    {
        handle_gui_event(event);
    }
}

void draw_text_gui(ALLEGRO_USTR *text)
{
    WZ_WIDGET *gui = create_text_gui(text);
    
    wz_update(gui,0);
    wz_draw(gui);
    wz_destroy(gui);
}




