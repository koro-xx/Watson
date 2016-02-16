/*
Taken from Allegro Nine Patch library. See LICENSE for copying information.
*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "widgetz_nine_patch.h"

typedef struct nine_patch_mark_tag
{
	int offset;
	int length;
	int dest_offset;
	int dest_length;
	float ratio;
} NINE_PATCH_MARK;

typedef struct nine_patch_side_tag
{
	NINE_PATCH_MARK* m;
	int count;
	int fix;
} NINE_PATCH_SIDE;

struct nine_patch_bitmap_tag
{
	ALLEGRO_BITMAP* bmp;
	NINE_PATCH_SIDE h, v;
	WZ_NINE_PATCH_PADDING padding;
	bool destroy_bmp;
	int width, height;
	int cached_dw, cached_dh;
	ALLEGRO_MUTEX* mutex;
};

static bool init_nine_patch_side(NINE_PATCH_SIDE* ps, ALLEGRO_BITMAP* bmp, int vertical)
{
	const int len = vertical ? al_get_bitmap_height(bmp) : al_get_bitmap_width(bmp);
	int i, s, t, n, z;
	ALLEGRO_COLOR c;
	int alloc = 8;
	ps->m = al_malloc(alloc * sizeof(*ps->m));

	for(i = 1, s = -1, t = 0, n = 0, z = -1; i < len; ++i)
	{
		int zz;
		uint8_t r, g, b, a;
		c = vertical ? al_get_pixel(bmp, 0, i) : al_get_pixel(bmp, i, 0);
		al_unmap_rgba(c, &r, &g, &b, &a);

		if(i == len - 1)
			zz = -1;
		else if(r == 0 && g == 0 && b == 0 && a == 255)
			zz = 0;
		else if(a == 0 || r + g + b + a == 255 * 4)
			zz = 1;
		else
			return false;

		if(z != zz)
		{
			if(s != -1)
			{
				ps->m[n].offset = s;
				ps->m[n].length = i - s;

				if(z == 0)
				{
					ps->m[n].ratio = 1;
					t += ps->m[n].length;
				}
				else
				{
					ps->m[n].ratio = 0;
				}

				++n;
			}

			s = i;
			z = zz;
		}

		if(n == alloc)
		{
			alloc *= 2;
			ps->m = al_realloc(ps->m, alloc * sizeof(*ps->m));
		}
	}

	if(n != alloc)
	{
		ps->m = al_realloc(ps->m, n * sizeof(*ps->m));
	}

	ps->count = n;
	ps->fix = len - 2 - t;

	for(i = 0; i < n; ++i)
	{
		if(ps->m[i].ratio)
			ps->m[i].ratio = ps->m[i].length / (float) t;
	}

	return true;
}

WZ_NINE_PATCH_BITMAP* wz_create_nine_patch_bitmap(ALLEGRO_BITMAP* bmp, bool owns_bitmap)
{
	int i;
	WZ_NINE_PATCH_BITMAP* p9;
	ALLEGRO_COLOR c;
	p9 = al_malloc(sizeof(*p9));
	p9->bmp = bmp;
	p9->destroy_bmp = owns_bitmap;
	p9->h.m = NULL;
	p9->v.m = NULL;
	p9->cached_dw = 0;
	p9->cached_dh = 0;
	p9->mutex = al_create_mutex();
	p9->width = al_get_bitmap_width(bmp) - 2;
	p9->height = al_get_bitmap_height(bmp) - 2;
	al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	if(p9->width <= 0 || p9->height <= 0)
		goto bad_bitmap;

	/* make sure all four corners are transparent */
#define _check_pixel(x, y) \
	c = al_get_pixel(bmp, x, y); \
  if (c.a != 0 && c.r + c.g + c.b + c.a != 4) goto bad_bitmap;
	_check_pixel(0, 0);
	_check_pixel(al_get_bitmap_width(bmp) - 1, 0);
	_check_pixel(0, al_get_bitmap_height(bmp) - 1);
	_check_pixel(al_get_bitmap_width(bmp) - 1, al_get_bitmap_height(bmp) - 1);
#undef _check_pixel
	p9->padding.top = p9->padding.right = p9->padding.bottom = p9->padding.left = -1;
	i = 1;

	while(i < al_get_bitmap_width(bmp))
	{
		c = al_get_pixel(bmp, i, al_get_bitmap_height(bmp) - 1);

		if(c.r + c.g + c.b == 0 && c.a == 1)
		{
			if(p9->padding.left == -1)
				p9->padding.left = i - 1;
			else if(p9->padding.right != -1)
				goto bad_bitmap;
		}
		else if(c.a == 0 || c.r + c.g + c.b + c.a == 4)
		{
			if(p9->padding.left != -1 && p9->padding.right == -1)
				p9->padding.right = al_get_bitmap_width(bmp) - i - 1;
		}

		++i;
	}

	i = 1;

	while(i < al_get_bitmap_height(bmp))
	{
		c = al_get_pixel(bmp, al_get_bitmap_width(bmp) - 1, i);

		if(c.r + c.g + c.b == 0 && c.a == 1)
		{
			if(p9->padding.top == -1)
				p9->padding.top = i - 1;
			else if(p9->padding.bottom != -1)
				goto bad_bitmap;
		}
		else if(c.a == 0 || c.r + c.g + c.b + c.a == 4)
		{
			if(p9->padding.top != -1 && p9->padding.bottom == -1)
				p9->padding.bottom = al_get_bitmap_height(bmp) - i - 1;
		}

		++i;
	}

	if(!init_nine_patch_side(&p9->h, bmp, 0) || !init_nine_patch_side(&p9->v, bmp, 1))
	{
bad_bitmap:
		al_destroy_mutex(p9->mutex);

		if(p9->h.m) al_free(p9->h.m);

		if(p9->v.m) al_free(p9->v.m);

		al_free(p9);
		p9 = NULL;
	}

	al_unlock_bitmap(bmp);
	return p9;
}

void calc_nine_patch_offsets(NINE_PATCH_SIDE* ps, int len)
{
	int i, j;
	int dest_offset = 0;
	int remaining_stretch = len - ps->fix;

	for(i = 0, j = 0; i < ps->count; ++i)
	{
		ps->m[i].dest_offset = dest_offset;

		if(ps->m[i].ratio == 0)
		{
			ps->m[i].dest_length = ps->m[i].length;
		}
		else
		{
			ps->m[i].dest_length = (len - ps->fix) * ps->m[i].ratio;
			remaining_stretch -= ps->m[i].dest_length;
			j = i;
		}

		dest_offset += ps->m[i].dest_length;
	}

	if(remaining_stretch)
	{
		ps->m[j].dest_length += remaining_stretch;

		if(j + 1 < ps->count)
			ps->m[j + 1].dest_offset += remaining_stretch;
	}
}

void wz_draw_tinted_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9, ALLEGRO_COLOR tint, int dx, int dy, int dw, int dh)
{
	int i, j;
	bool release_drawing = false;

	/* don't draw bitmaps that are smaller than the fixed area */
	if(dw < p9->h.fix || dh < p9->v.fix) return;

	/* if the bitmap is the same size as the origin, then draw it as-is */
	if(dw == p9->width && dh == p9->height)
	{
		al_draw_tinted_bitmap_region(p9->bmp, tint, 1, 1, dw, dh, dx, dy, 0);
		return;
	}

	/* due to the caching mechanism, multiple threads cannot draw this image at the same time */
	al_lock_mutex(p9->mutex);

	/* only recalculate the offsets if they have changed since the last draw */
	if(p9->cached_dw != dw || p9->cached_dh != dh)
	{
		calc_nine_patch_offsets(&p9->h, dw);
		calc_nine_patch_offsets(&p9->v, dh);
		p9->cached_dw = dw;
		p9->cached_dh = dh;
	}

	if(!al_is_bitmap_drawing_held())
	{
		release_drawing = true;
		al_hold_bitmap_drawing(true);
	}

	/* draw each region */
	for(i = 0; i < p9->v.count; ++i)
	{
		for(j = 0; j < p9->h.count; ++j)
		{
			al_draw_tinted_scaled_bitmap(p9->bmp, tint,
			                             p9->h.m[j].offset, p9->v.m[i].offset,
			                             p9->h.m[j].length, p9->v.m[i].length,
			                             dx + p9->h.m[j].dest_offset, dy + p9->v.m[i].dest_offset,
			                             p9->h.m[j].dest_length, p9->v.m[i].dest_length,
			                             0
			                            );
		}
	}

	al_unlock_mutex(p9->mutex);

	if(release_drawing)
		al_hold_bitmap_drawing(false);
}

void wz_draw_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9, int dx, int dy, int dw, int dh)
{
	wz_draw_tinted_nine_patch_bitmap(p9, al_map_rgb_f(1, 1, 1), dx, dy, dw, dh);
}

ALLEGRO_BITMAP* wz_create_bitmap_from_nine_patch(WZ_NINE_PATCH_BITMAP* p9, int w, int h)
{
	ALLEGRO_BITMAP* bmp = al_create_bitmap(w, h);
	ALLEGRO_STATE s;

	if(!bmp) return NULL;

	al_store_state(&s, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(bmp);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	wz_draw_nine_patch_bitmap(p9, 0, 0, w, h);
	al_restore_state(&s);
	return bmp;
}

WZ_NINE_PATCH_BITMAP* wz_load_nine_patch_bitmap(const char* filename)
{
	ALLEGRO_BITMAP* bmp = al_load_bitmap(filename);
	return bmp ? wz_create_nine_patch_bitmap(bmp, true) : NULL;
}

int wz_get_nine_patch_bitmap_width(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->width;
}

int wz_get_nine_patch_bitmap_height(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->height;
}

int wz_get_nine_patch_bitmap_min_width(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->h.fix;
}

int wz_get_nine_patch_bitmap_min_height(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->v.fix;
}

ALLEGRO_BITMAP* wz_get_nine_patch_bitmap_source(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->bmp;
}

WZ_NINE_PATCH_PADDING wz_get_nine_patch_padding(const WZ_NINE_PATCH_BITMAP* p9)
{
	return p9->padding;
}

void wz_destroy_nine_patch_bitmap(WZ_NINE_PATCH_BITMAP* p9)
{
	if(p9 == 0)
		return;

	if(p9->destroy_bmp)
		al_destroy_bitmap(p9->bmp);

	al_destroy_mutex(p9->mutex);
	al_free(p9->h.m);
	al_free(p9->v.m);
	al_free(p9);
}
