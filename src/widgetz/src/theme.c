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

#include "widgetz_internal.h"
#include <math.h>

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

/*
Title: Theme Stuff
*/

static void wz_draw_3d_rectangle(float x1, float y1, float x2, float y2, float border, ALLEGRO_COLOR col, bool invert)
{
	ALLEGRO_VERTEX vtx[6];
	ALLEGRO_COLOR hi, lo;
	int ii;

	if(invert)
	{
		lo = wz_scale_color(col, 1.5);
		hi = wz_scale_color(col, 0.5);
	}
	else
	{
		hi = wz_scale_color(col, 1.5);
		lo = wz_scale_color(col, 0.5);
	}

	for(ii = 0; ii < 6; ii++)
	{
		vtx[ii].color = hi;
		vtx[ii].z = 0;
	}

	vtx[0].x = x1 + border;
	vtx[0].y = y1 + border;
	vtx[1].x = x1 + border;
	vtx[1].y = y2 - border;
	vtx[2].x = x1;
	vtx[2].y = y2;
	vtx[3].x = x1;
	vtx[3].y = y1;
	vtx[4].x = x2;
	vtx[4].y = y1;
	vtx[5].x = x2 - border;
	vtx[5].y = y1 + border;
	al_draw_prim(vtx, 0, 0, 0, 6, ALLEGRO_PRIM_TRIANGLE_FAN);
	vtx[0].x = x2 - border;
	vtx[0].y = y2 - border;
	vtx[1].x = x1 + border;
	vtx[1].y = y2 - border;
	vtx[2].x = x1;
	vtx[2].y = y2;
	vtx[3].x = x2;
	vtx[3].y = y2;
	vtx[4].x = x2;
	vtx[4].y = y1;
	vtx[5].x = x2 - border;
	vtx[5].y = y1 + border;

	for(ii = 0; ii < 6; ii++)
	{
		vtx[ii].color = lo;
	}

	al_draw_prim(vtx, 0, 0, 0, 6, ALLEGRO_PRIM_TRIANGLE_FAN);
	al_draw_filled_rectangle(x1 + border, y1 + border, x2 - border, y2 - border, col);
}

//My version of it
//returns the token length, and skips the first occurence if it is the first character
//char *wz_ustrpbrk(AL_CONST char *s, AL_CONST char *set, int* index)
//{
//	AL_CONST char *setp;
//	int c, d;
//	int first = 1;
//	ALLEGRO_ASSERT(s);
//	ALLEGRO_ASSERT(set);
//
//	*index = 0;
//	while ((c = ugetc(s)) != 0)
//	{
//		setp = set;
//
//		while ((d = ugetxc(&setp)) != 0)
//		{
//			if (c == d && !first)
//				return (char *)s;
//		}
//		(*index)++;
//		s += uwidth(s);
//		first = 0;
//	}
//
//	return NULL;
//}

//return the new start
int wz_find_eol(ALLEGRO_USTR* text, ALLEGRO_FONT* font, float max_width, int start, int* end)
{
	int a, b;
	int first = 1;
	int last = 0;
	a = start;

	while(1)
	{
		ALLEGRO_USTR_INFO info;
		const ALLEGRO_USTR* token;
		float len;
		/*
		Find the end of current token
		*/
		b = al_ustr_find_set_cstr(text, a, "\t\n ");

		if(b == -1) //found nothing
		{
			b = al_ustr_size(text); //this is the last whole word
			last = 1;
		}

		/*
		Check to see if the token fits
		*/
		token = al_ref_ustr(&info, text, start, b);
		len = al_get_ustr_width(font, token);

		if(len < max_width || first)
		{
			if(last)
			{
				*end = b + 1;
				return -1;
			}
		}
		else   //we return the last num
		{
			*end = a - 1;
			return a;
		}

		/*
		Check what character we found
		*/
		{
			int character = al_ustr_get(text, b);

			if(character == '\n')
			{
				*end = b;
				return b + 1;
			}
		}
		a = b + 1;
		first = 0;
	}
}

/*
Function: wz_draw_single_text

Draws a single line of text
*/
void wz_draw_single_text(float x, float y, float w, float h, int halign, int valign, ALLEGRO_COLOR color, ALLEGRO_FONT* font, ALLEGRO_USTR* text)
{
	float xpos;
	float ypos;
	float height = al_get_font_line_height(font);

	if(valign == WZ_ALIGN_TOP)
	{
		ypos = y;
	}
	else if(valign == WZ_ALIGN_BOTTOM)
	{
		ypos = y + h - height;
	}
	else
	{
		ypos = y + h / 2 - height / 2;
	}

	if(halign == WZ_ALIGN_LEFT)
	{
		xpos = x;
		al_draw_ustr(font, color, floorf(xpos), floorf(ypos), ALLEGRO_ALIGN_LEFT, text);
	}
	else if(halign == WZ_ALIGN_RIGHT)
	{
		xpos = x + w;
		al_draw_ustr(font, color, floorf(xpos), floorf(ypos), ALLEGRO_ALIGN_RIGHT, text);
	}
	else
	{
		xpos = x + w / 2;
		al_draw_ustr(font, color, floorf(xpos), floorf(ypos), ALLEGRO_ALIGN_CENTRE, text);
	}
}

/*
Function: wz_draw_multi_text

Draws multiple lines of text, wrapping it as necessary
*/
void wz_draw_multi_text(float x, float y, float w, float h, int halign, int valign, ALLEGRO_COLOR color, ALLEGRO_FONT* font, ALLEGRO_USTR* text)
{
	float cur_y = y;
	float text_height = al_get_font_line_height(font);
	float total_height = 0;

	if(valign == WZ_ALIGN_BOTTOM || valign == WZ_ALIGN_CENTRE)
	{
		int ret = 0;

		do
		{
			int start = ret;
			int end;
			ret = wz_find_eol(text, font, w, start, &end);
			total_height += text_height;
		}
		while(ret > 0);
	}

	if(valign == WZ_ALIGN_BOTTOM)
	{
		cur_y = y + h - total_height;
	}
	else if(valign == WZ_ALIGN_CENTRE)
	{
		cur_y = y + (h - total_height) / 2;
	}

	{
		int ret = 0;

		do
		{
			int start = ret;
			int end;
			ret = wz_find_eol(text, font, w, start, &end);
			{
				ALLEGRO_USTR_INFO info;
				const ALLEGRO_USTR* token = al_ref_ustr(&info, text, start, end);
				//printf("%f %f %s\n", x, cur_y, al_cstr(token));
				wz_draw_single_text(x, cur_y, w, h, halign, WZ_ALIGN_TOP, color, font, (ALLEGRO_USTR*)token);
			}
			cur_y += text_height;
		}
		while(ret > 0);
	}
}

void wz_def_draw_box(struct WZ_THEME* theme, float x, float y, float w, float h, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	al_draw_filled_rectangle(x, y, x + w, y + h, wz_scale_color(thm->color1, 0.5));

	if(style & WZ_STYLE_FOCUSED)
		al_draw_rectangle(x, y, x + w, y + h, wz_scale_color(thm->color1, 1.5), 1);
	else
		al_draw_rectangle(x, y, x + w, y + h, thm->color1, 1);
}

void wz_def_draw_button(WZ_THEME* theme, float x, float y, float w, float h, ALLEGRO_USTR* text, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR button_col;
	ALLEGRO_COLOR text_col;
	bool invert = false;
	button_col = thm->color1;
	text_col = thm->color2;

	if(style & WZ_STYLE_FOCUSED)
	{
		button_col = wz_scale_color(thm->color1, 1.25);
	}

	if(style & WZ_STYLE_DISABLED)
	{
		button_col = wz_scale_color(thm->color1, 0.5);
		text_col = wz_scale_color(thm->color2, 0.5);
	}

	if(style & WZ_STYLE_DOWN)
	{
		invert = true;
	}

	wz_draw_3d_rectangle(x, y, x + w, y + h, 2, button_col, invert);
	wz_draw_multi_text(x, y, w, h, WZ_ALIGN_CENTRE, WZ_ALIGN_CENTRE, text_col, thm->font, text);
}

void wz_def_draw_textbox(struct WZ_THEME* theme, float x, float y, float w, float h, int halign, int valign, ALLEGRO_USTR* text, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	ALLEGRO_COLOR text_col;

	if(style & WZ_STYLE_DISABLED)
		text_col = wz_scale_color(thm->color2, 0.5);
	else
		text_col = thm->color2;

	wz_draw_multi_text(x, y, w, h, halign, valign, text_col, thm->font, text);
}

void wz_def_draw_scroll(struct WZ_THEME* theme, float x, float y, float w, float h, float fraction, float slider_size, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	int vertical = h > w;
	ALLEGRO_COLOR col;
	float xpos;
	float ypos;
	float slider_w;
	float slider_h;

	if(style & WZ_STYLE_FOCUSED)
	{
		col = wz_scale_color(thm->color1, 1.5);
	}
	else if(style & WZ_STYLE_DISABLED)
	{
		col = wz_scale_color(thm->color1, 0.5);
	}
	else
	{
		col = thm->color1;
	}

	if(vertical)
	{
		float max_size = 0.9f * h;
		slider_h = slider_size > max_size ? max_size : slider_size;
		slider_w = w;
		xpos = x;
		ypos = y + fraction * (h - slider_h);
		wz_draw_3d_rectangle(xpos - 4 + w / 2, y, xpos + 4 + w / 2, y + h, 2, wz_scale_color(thm->color1, 0.75), true);
	}
	else
	{
		float max_size = 0.9f * w;
		slider_h = h;
		slider_w = slider_size > max_size ? max_size : slider_size;
		xpos = x + fraction * (w - slider_w);
		ypos = y;
		wz_draw_3d_rectangle(x, ypos - 4 + h / 2, x + w, ypos + 4 + h / 2, 2, wz_scale_color(thm->color1, 0.75), true);
	}

	wz_draw_3d_rectangle(xpos, ypos, xpos + slider_w, ypos + slider_h, 1, col, false);
}

void wz_def_draw_editbox(struct WZ_THEME* theme, float x, float y, float w, float h, int cursor_pos, ALLEGRO_USTR* text, int style)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	int len = wz_get_text_pos(thm->font, text, w - 4);
	int cx, cy, cw, ch;
	int len2 = al_ustr_length(text);
	int offset;
	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR* token;
	ALLEGRO_COLOR border_col;
	ALLEGRO_COLOR text_col;
	len = len + 1 > len2 ? len2 : len + 1;
	offset = al_ustr_offset(text, len);
	token = al_ref_ustr(&info, text, 0, offset);
	border_col = thm->color1;
	text_col = thm->color2;

	if(style & WZ_STYLE_FOCUSED)
	{
		border_col = wz_scale_color(thm->color1, 1.5);
	}

	if(style & WZ_STYLE_DISABLED)
	{
		border_col = wz_scale_color(thm->color1, 0.5);
		text_col = wz_scale_color(thm->color2, 0.5);
	}

	wz_draw_3d_rectangle(x, y, x + w, y + h, 1, border_col, true);
	al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
	al_set_clipping_rectangle(x + 2, y + 2, w - 4, h - 4);
	wz_draw_single_text(x + 2, y + 2, w - 4, h - 4, WZ_ALIGN_LEFT, WZ_ALIGN_CENTRE, text_col, thm->font, (ALLEGRO_USTR*)token);
	al_set_clipping_rectangle(cx, cy, cw, ch);

	if(style & WZ_STYLE_FOCUSED)
	{
		if(((int)(al_get_time() / 0.5f)) % 2 == 0)
		{
			float len;
			float halfheight;
			offset = al_ustr_offset(text, cursor_pos);
			token = al_ref_ustr(&info, text, 0, offset);
			len = al_get_ustr_width(thm->font, token);
			halfheight = al_get_font_line_height(thm->font) / 2.0f;
			al_draw_line(x + 2 + len, y + 2 + h / 2 - halfheight, x + 2 + len, y + 2 + h / 2 + halfheight, text_col, 1);
		}
	}
}

void wz_def_draw_image(struct WZ_THEME* theme, float x, float y, float w, float h, ALLEGRO_BITMAP* image)
{
	float ix = x + (w - al_get_bitmap_width(image)) / 2;
	float iy = y + (h - al_get_bitmap_height(image)) / 2;
	al_draw_bitmap(image, ix, iy, 0);
}

ALLEGRO_FONT* wz_def_get_font(struct WZ_THEME* theme, int font_num)
{
	WZ_DEF_THEME* thm = (WZ_DEF_THEME*)theme;
	return thm->font;
}

/*
Variable: wz_def_theme

The default theme.
*/
const WZ_DEF_THEME wz_def_theme =
{
	{
		0,
		wz_def_draw_button,
		wz_def_draw_box,
		wz_def_draw_textbox,
		wz_def_draw_scroll,
		wz_def_draw_editbox,
		wz_def_draw_image,
		wz_def_get_font,
	}
};
