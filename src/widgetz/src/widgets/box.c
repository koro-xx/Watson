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
Title: Box

Section: Internal

Function: wz_box_proc

See also:
<wz_widget_proc>
*/
int wz_box_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;

	switch(event->type)
	{
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			if(event->mouse.button == 1 && wz_widget_rect_test(wgt, event->mouse.x, event->mouse.y))
			{
				wz_ask_parent_for_focus(wgt);
			}

			ret = 0;
			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			if(wz_widget_rect_test(wgt, event->touch.x, event->touch.y))
			{
				wz_ask_parent_for_focus(wgt);
			}

			ret = 0;
			break;
		}
#endif
		case WZ_DRAW:
		{
			if(wgt->flags & WZ_STATE_HIDDEN)
			{
				ret = 0;
			}
			else
			{
				int flags = (wgt->flags & WZ_STATE_HAS_FOCUS) ? WZ_STYLE_FOCUSED : 0;
				wgt->theme->draw_box(wgt->theme, wgt->local_x, wgt->local_y, wgt->w, wgt->h, flags);
			}
		}
		default:
			ret = 0;
	}

	if(ret == 0)
		ret = wz_widget_proc(wgt, event);

	return ret;
}

/*
Function: wz_init_box
*/
void wz_init_box(WZ_WIDGET* wgt, WZ_WIDGET* parent, float x, float y, float w, float h, int id)
{
	wz_init_widget(wgt, parent, x, y, w, h, id);
	wgt->proc = wz_box_proc;
}

/*
Section: Public

Function: wz_create_box

Creates a box.

See Also:
<WZ_WIDGET>

<wz_create_widget>
*/
WZ_WIDGET* wz_create_box(WZ_WIDGET* parent, float x, float y, float w, float h, int id)
{
	WZ_WIDGET* wgt = malloc(sizeof(WZ_WIDGET));
	wz_init_box(wgt, parent, x, y, w, h, id);
	return wgt;
}
