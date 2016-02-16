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

/*
Title: Text Box

Section: Internal

Function: wz_textbox_proc

See also:
<wz_widget_proc>
*/
int wz_textbox_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;
	WZ_TEXTBOX* box = (WZ_TEXTBOX*)wgt;

	switch(event->type)
	{
		case WZ_DRAW:
		{
			if(wgt->flags & WZ_STATE_HIDDEN)
			{
				ret = 0;
			}
			else
			{
				int flags = (wgt->flags & WZ_STATE_DISABLED) ? WZ_STYLE_DISABLED : 0;
				wgt->theme->draw_textbox(wgt->theme, wgt->local_x, wgt->local_y, wgt->w, wgt->h, box->h_align, box->v_align, box->text, flags);
			}

			break;
		}
		case WZ_DESTROY:
		{
			if(box->own)
				al_ustr_free(box->text);

			ret = 0;
			break;
		}
		case WZ_SET_TEXT:
		{
			if(box->own)
			{
				al_ustr_free(box->text);
				box->text = al_ustr_dup((ALLEGRO_USTR*)event->user.data3);
			}
			else
			{
				box->text = (ALLEGRO_USTR*)event->user.data3;
			}

			break;
		}
		default:
			ret = 0;
	}

	if(ret == 0)
		ret = wz_widget_proc(wgt, event);

	return ret;
}

/*
Function: wz_init_textbox
*/
void wz_init_textbox(WZ_TEXTBOX* box, WZ_WIDGET* parent, float x, float y, float w, float h, int halign, int valign, ALLEGRO_USTR* text, int own, int id)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)box;
	wz_init_widget(wgt, parent, x, y, w, h, id);
	wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
	wgt->proc = wz_textbox_proc;
	box->own = own;
	box->h_align = halign;
	box->v_align = valign;
	box->text = text;
}

/*
Section: Public

Function: wz_create_textbox

Creates a text box.

Parameters:

halign - Horizontal text alignment, can be one of: <WZ_ALIGN_LEFT>, <WZ_ALIGN_CENTRE>, <WZ_ALIGN_RIGHT>
valign - Vertical text alignment, can be one of: <WZ_ALIGN_TOP>, <WZ_ALIGN_CENTRE>, <WZ_ALIGN_BOTTOM>
own - Set to 1 if you want the widget to own the text

Inherits From:
<WZ_WIDGET>

See Also:
<WZ_TEXTBOX>

<wz_create_widget>
*/
WZ_TEXTBOX* wz_create_textbox(WZ_WIDGET* parent, float x, float y, float w, float h, int halign, int valign, ALLEGRO_USTR* text, int own, int id)
{
	WZ_TEXTBOX* box = malloc(sizeof(WZ_TEXTBOX));
	wz_init_textbox(box, parent, x, y, w, h, halign, valign, text, own, id);
	return box;
}
