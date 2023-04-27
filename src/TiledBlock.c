#include "TiledBlock.h"
#include <allegro5/allegro_primitives.h>

// find the tile at x,y. Returns in p[] an array of integers starting at p[0] representing the
// nested sequence of subblocks that leads to it, and d = depth + 1 (d=0 means the main tile doesn't match)

int get_TiledBlock_tile(TiledBlock *t, int x, int y, int *p)
{
    int m;
    int d = 0;

    if ((x >= t->x) && (x < t->x + t->w) && (y >= t->y) && (y < t->y + t->h))
    {
        for (m = 0; m < t->sb; m++)
        {
            d = get_TiledBlock_tile(t->b[m], x - t->x, y - t->y, p + 1);
            if (d)
            {
                *p = m;
                break;
            }
        }
        return d + 1;
    }
    return 0;
};

// get pointer to tiledblock at position x,y
TiledBlock *get_TiledBlock(TiledBlock *t, int x, int y)
{
    int m;
    TiledBlock *rt = NULL;

    if ((x >= t->x) && (x < t->x + t->w) && (y >= t->y) && (y < t->y + t->h))
    {
        for (m = 0; m < t->sb; m++)
        {
            rt = get_TiledBlock(t->b[m], x - t->x, y - t->y);
            if (rt && (rt->hidden != -1))
                return rt;
        }
        return t;
    }
    else
        return NULL;
};

// Draw the tiled block in the target allegro display
void draw_TiledBlock(TiledBlock *t, int x, int y)
{
    int i;

    if (t->bmp && (t->hidden != -1))
    {
        if (t->hidden)
        {
            al_draw_tinted_bitmap(*(t->bmp), al_map_rgba_f(0.1, 0.1, 0.1, 0.1), t->x + x, t->y + y, 0);
        }
        else
        {
            al_draw_bitmap(*(t->bmp), t->x + x, t->y + y, 0);
        }
    }
    else
    {
        al_draw_filled_rectangle(t->x + x, t->y + y, t->x + x + t->w, t->y + y + t->h, t->bg_color);
    }

    if (t->bd)
        al_draw_rectangle(t->x + x, t->y + y, t->x + x + t->w, t->y + y + t->h, t->bd_color, t->bd);

    for (i = 0; i < t->sb; i++)
    {
        if (t->b[i])
            draw_TiledBlock(t->b[i], t->x + x, t->y + y);
    }
};

void get_TiledBlock_offset(TiledBlock *t, int *x, int *y)
{
    *x = t->x;
    *y = t->y;

    while (t->parent)
    {
        t = t->parent;
        *x += t->x;
        *y += t->y;
    }
}

void highlight_TiledBlock(TiledBlock *t)
{
    int x, y, i;
    get_TiledBlock_offset(t, &x, &y);
    //    al_draw_rectangle(x-2,y-2, x+t->w+2, y+t->h+2, (ALLEGRO_COLOR){1,0,0,0.5}, 4);
    for (i = 0; i < 8; i++)
    {
        al_draw_rectangle(x, y, x + t->w, y + t->h, al_premul_rgba_f(1, 0, 0, 0.2), i);
    }
    al_draw_filled_rectangle(x, y, x + t->w, y + t->h, al_premul_rgba_f(1, 1, 1, 0.3));
}

TiledBlock *new_TiledBlock(void)
{
    TiledBlock *t = malloc(sizeof(*t));
    *t = (TiledBlock){0};
    return t;
}
