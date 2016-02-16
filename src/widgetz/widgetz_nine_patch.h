/*
Taken from Allegro Nine Patch library. See LICENSE for copying information.
*/

#ifndef __included_nine_patch_h
#define __included_nine_patch_h

#include <allegro5/allegro.h>

typedef struct nine_patch_bitmap_tag WZ_NINE_PATCH_BITMAP;

typedef struct nine_patch_padding_tag
{
	int top, right, bottom, left;
} WZ_NINE_PATCH_PADDING;

WZ_NINE_PATCH_BITMAP* wz_create_nine_patch_bitmap(ALLEGRO_BITMAP* bmp, bool owns_bitmap);
WZ_NINE_PATCH_BITMAP* wz_load_nine_patch_bitmap(const char* filename);
void wz_draw_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9, int dx, int dy, int dw, int dh);
void wz_draw_tinted_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9, ALLEGRO_COLOR tint, int dx, int dy, int dw, int dh);
ALLEGRO_BITMAP* wz_create_bitmap_from_nine_patch(WZ_NINE_PATCH_BITMAP* p9, int w, int h);

int wz_get_nine_patch_bitmap_width(const WZ_NINE_PATCH_BITMAP* p9);
int wz_get_nine_patch_bitmap_height(const WZ_NINE_PATCH_BITMAP* p9);

int wz_get_nine_patch_bitmap_min_width(const WZ_NINE_PATCH_BITMAP* p9);
int wz_get_nine_patch_bitmap_min_height(const WZ_NINE_PATCH_BITMAP* p9);

ALLEGRO_BITMAP* wz_get_nine_patch_bitmap_source(const WZ_NINE_PATCH_BITMAP* p9);
WZ_NINE_PATCH_PADDING wz_get_nine_patch_padding(const WZ_NINE_PATCH_BITMAP* p9);

void wz_destroy_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9);

#endif
