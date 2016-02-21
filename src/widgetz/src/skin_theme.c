/*
Copyright (c) 2011 Pavel Sountsov

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "../widgetz_internal.h"
#include "../widgetz_nine_patch.h"
#include <math.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>

/*
Title: Skinnable Theme Stuff
*/

/*
Function: wz_init_skin_theme

Initializes a skin theme. Do not pass wz_skin_theme to this function, but make a copy of it first. You should initialize the bitmap variables inside the <WZ_SKIN_THEME> struct before calling
this function. Don't forget to call <wz_destroy_skin_theme> on the theme when you are done with it.
*/
void wz_init_skin_theme(struct WZ_SKIN_THEME* theme)
{
	if(theme->button_up_bitmap != 0)
		theme->button_up_patch = wz_create_nine_patch_bitmap(theme->button_up_bitmap, false);
	else
		theme->button_up_patch = 0;

	if(theme->button_down_bitmap != 0)
		theme->button_down_patch = wz_create_nine_patch_bitmap(theme->button_down_bitmap, false);
	else
		theme->button_down_patch = 0;

	if(theme->box_bitmap != 0)
		theme->box_patch = wz_create_nine_patch_bitmap(theme->box_bitmap, false);
	else
		theme->box_patch = 0;

	if(theme->editbox_bitmap != 0)
		theme->editbox_patch = wz_create_nine_patch_bitmap(theme->editbox_bitmap, false);
	else
		theme->editbox_patch = 0;

	if(theme->scroll_track_bitmap != 0)
		theme->scroll_track_patch = wz_create_nine_patch_bitmap(theme->scroll_track_bitmap, false);
	else
		theme->scroll_track_patch = 0;

	if(theme->slider_bitmap != 0)
		theme->slider_patch = wz_create_nine_patch_bitmap(theme->slider_bitmap, false);
	else
		theme->slider_patch = 0;
}

/*
Function: wz_destroy_skin_theme

Destroys a skin theme that has been initialized by <wz_init_skin_theme>. Note that it does not free the passed pointer.
*/
void wz_destroy_skin_theme(struct WZ_SKIN_THEME* theme)
{
	if(theme == 0)
		return;

	wz_destroy_nine_patch_bitmap(theme->button_up_patch);
	wz_destroy_nine_patch_bitmap(theme->button_down_patch);
	wz_destroy_nine_patch_bitmap(theme->box_patch);
	wz_destroy_nine_patch_bitmap(theme->editbox_patch);
	wz_destroy_nine_patch_bitmap(theme->scroll_track_patch);
	wz_destroy_nine_patch_bitmap(theme->slider_patch);
}

/* Returns the padding corrected in case the passed rectangle was too small */
static WZ_NINE_PATCH_PADDING draw_tinted_patch(WZ_NINE_PATCH_BITMAP* p9, ALLEGRO_COLOR tint, float x, float y, float w, float h)
{
	WZ_NINE_PATCH_PADDING pad = wz_get_nine_patch_padding(p9);
	float min_w = wz_get_nine_patch_bitmap_min_width(p9);
	float min_h = wz_get_nine_patch_bitmap_min_height(p9);
	float nx = x;
	float ny = y;
	float nw = w;
	float nh = h;

	if(w < min_w)
	{
		nw = min_w;
		nx = x + w / 2 - nw / 2;
	}

	if(h < min_h)
	{
		nh = min_h;
		ny = y + h / 2 - nh / 2;
	}

	wz_draw_tinted_nine_patch_bitmap(p9, tint, nx, ny, nw, nh);
	pad.left -= x - nx;
	pad.top -= y - ny;
	pad.right -= x - nx; /* Might be wrong */
	pad.top -= y - ny;
	return pad;
}

void wz_skin_draw_box(struct WZ_THEME* theme, float x, float y, float w, float h, int style)
{
	WZ_SKIN_THEME* skin = (WZ_SKIN_THEME*)theme;
	WZ_DEF_THEME* def = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR col;

	if(skin->box_patch)
	{
		if(style & WZ_STYLE_FOCUSED)
			col = wz_scale_color(def->color1, 1.5);
		else
			col = def->color1;

		draw_tinted_patch(skin->box_patch, col, x, y, w, h);
	}
}

void wz_skin_draw_button(WZ_THEME* theme, float x, float y, float w, float h, ALLEGRO_USTR* text, int style)
{
	WZ_SKIN_THEME* skin = (WZ_SKIN_THEME*)theme;
	WZ_DEF_THEME* def = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR button_col;
	ALLEGRO_COLOR text_col;
	WZ_NINE_PATCH_PADDING pad;
	button_col = def->color1;
	text_col = def->color2;
	pad.left = 0;
	pad.right = 0;
	pad.top = 0;
	pad.bottom = 0;

#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
	if( (style & WZ_STYLE_FOCUSED) && !(style & WZ_STYLE_DOWN) )
#else
    if( (style & WZ_STYLE_FOCUSED) )
#endif
	{
		button_col = wz_scale_color(def->color1, 1.25);
	}

	if(style & WZ_STYLE_DISABLED)
	{
		button_col = wz_scale_color(def->color1, 0.5);
		text_col = wz_scale_color(def->color2, 0.5);
	}

	if(skin->button_up_patch && skin->button_down_patch)
	{
		if(style & WZ_STYLE_DOWN)
		{
			pad = draw_tinted_patch(skin->button_down_patch, button_col, x, y, w, h);
		}
		else
		{
			pad = draw_tinted_patch(skin->button_up_patch, button_col, x, y, w, h);
		}
	}

	wz_draw_multi_text(x + pad.left, y + pad.top, w - (pad.left + pad.right), h - (pad.top + pad.bottom), WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, text_col, def->font, text);
}

void wz_skin_draw_textbox(struct WZ_THEME* theme, float x, float y, float w, float h, int halign, int valign, ALLEGRO_USTR* text, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR text_col;

	if(style & WZ_STYLE_DISABLED)
		text_col = wz_scale_color(thm->color2, 0.5);
	else
		text_col = thm->color2;

	wz_draw_multi_text(x, y, w, h, halign, valign, text_col, thm->font, text);
}

void wz_skin_draw_scroll(struct WZ_THEME* theme, float x, float y, float w, float h, float fraction, float slider_size, int style)
{
	WZ_SKIN_THEME* skin = (WZ_SKIN_THEME*)theme;
	WZ_DEF_THEME* def = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR col;
	int vertical = h > w;
	float xpos;
	float ypos;
	float slider_w;
	float slider_h;

	if(style & WZ_STYLE_FOCUSED)
	{
		col = wz_scale_color(def->color1, 1.5);
	}
	else if(style & WZ_STYLE_DISABLED)
	{
		col = wz_scale_color(def->color1, 0.5);
	}
	else
	{
		col = def->color1;
	}

	if(vertical)
	{
		float max_size = 0.9f * h;
		slider_h = slider_size > max_size ? max_size : slider_size;
		slider_w = w;
		xpos = x;
		ypos = y + fraction * (h - slider_h);
	}
	else
	{
		float max_size = 0.9f * w;
		slider_h = h;
		slider_w = slider_size > max_size ? max_size : slider_size;
		xpos = x + fraction * (w - slider_w);
		ypos = y;
	}

	if(skin->scroll_track_patch)
		draw_tinted_patch(skin->scroll_track_patch, def->color1, x, y, w, h);

	if(skin->slider_patch)
		draw_tinted_patch(skin->slider_patch, col, xpos, ypos, slider_w, slider_h);
}

void wz_skin_draw_editbox(struct WZ_THEME* theme, float x, float y, float w, float h, int cursor_pos, ALLEGRO_USTR* text, int style)
{
	WZ_SKIN_THEME* skin = (WZ_SKIN_THEME*)theme;
	WZ_DEF_THEME* def = (WZ_DEF_THEME*)theme;
	int len = wz_get_text_pos(def->font, text, w - 4);
	int cx, cy, cw, ch;
	int len2 = al_ustr_length(text);
	int offset;
	ALLEGRO_USTR_INFO info;
	ALLEGRO_USTR* token;
	ALLEGRO_COLOR border_col;
	ALLEGRO_COLOR text_col;
	WZ_NINE_PATCH_PADDING pad;
	pad.left = 0;
	pad.right = 0;
	pad.top = 0;
	pad.bottom = 0;
	len = len + 1 > len2 ? len2 : len + 1;
	offset = al_ustr_offset(text, len);
	token = (ALLEGRO_USTR *)al_ref_ustr(&info, text, 0, offset);
	border_col = def->color1;
	text_col = def->color2;

	if(style & WZ_STYLE_FOCUSED)
	{
		border_col = wz_scale_color(def->color1, 1.5);
	}

	if(style & WZ_STYLE_DISABLED)
	{
		border_col = wz_scale_color(def->color1, 0.5);
		text_col = wz_scale_color(def->color2, 0.5);
	}

	if(skin->editbox_patch)
	{
		pad = draw_tinted_patch(skin->editbox_patch, border_col, x, y, w, h);
	}

	al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
	al_set_clipping_rectangle(x + pad.left, y + pad.right, w - (pad.left + pad.right), h - (pad.top + pad.bottom));
	wz_draw_single_text(x + pad.left, y + pad.right, w - (pad.left + pad.right), h - (pad.top + pad.bottom), WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, text_col, def->font, token);
	al_set_clipping_rectangle(cx, cy, cw, ch);

	if(style & WZ_STYLE_FOCUSED)
	{
		float mh = (pad.top + h - pad.bottom) / 2;

		if(((int)(al_get_time() / 0.5f)) % 2 == 0)
		{
			float len;
			float halfheight;
			offset = al_ustr_offset(text, cursor_pos);
			token = (ALLEGRO_USTR *)al_ref_ustr(&info, text, 0, offset);
			len = al_get_ustr_width(def->font, token);
			halfheight = al_get_font_line_height(def->font) / 2.0f;
			al_draw_line(x + pad.left + len, y + mh - halfheight, x + pad.left + len, y + mh + halfheight, text_col, 1);
		}
	}
}

void wz_skin_draw_image(struct WZ_THEME* theme, float x, float y, float w, float h, ALLEGRO_BITMAP* image)
{
	float ix = x + (w - al_get_bitmap_width(image)) / 2;
	float iy = y + (h - al_get_bitmap_height(image)) / 2;
	al_draw_bitmap(image, ix, iy, 0);
}

ALLEGRO_FONT* wz_skin_get_font(struct WZ_THEME* theme, int font_num)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	return thm->font;
}

/*
Variable: wz_skin_theme

A simple skinnable theme that uses nine patch images for the widgets.
*/
const WZ_SKIN_THEME wz_skin_theme =
{
	{
		{
			0,
			wz_skin_draw_button,
			wz_skin_draw_box,
			wz_skin_draw_textbox,
			wz_skin_draw_scroll,
			wz_skin_draw_editbox,
			wz_skin_draw_image,
			wz_skin_get_font,
		}
	}
};
