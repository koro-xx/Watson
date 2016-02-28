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

#include "../../widgetz_internal.h"

/* Returns 1 if the position changed, 0 otherwise*/
static int set_scroll_pos(WZ_SCROLL* scl, float x, float y)
{
	float fraction;
	int old_pos;
	WZ_WIDGET* wgt = (WZ_WIDGET*)scl;

	if(wgt->h > wgt->w)
	{
		float max_size = 0.9f * wgt->h;
		float slider_size = scl->slider_size > max_size ? max_size : scl->slider_size;
		fraction = ((float)(y - wgt->local_y - slider_size / 2)) / ((float)wgt->h - slider_size);
	}
	else
	{
		float max_size = 0.9f * wgt->w;
		float slider_size = scl->slider_size > max_size ? max_size : scl->slider_size;
		fraction = ((float)(x - wgt->local_x - slider_size / 2)) / ((float)wgt->w - slider_size);
	}

	old_pos = scl->cur_pos;
	scl->cur_pos = (int)(((float)scl->max_pos) * fraction + 0.5f);

	if(scl->cur_pos < 0)
		scl->cur_pos = 0;

	if(scl->cur_pos > scl->max_pos)
		scl->cur_pos = scl->max_pos;

	return old_pos != scl->cur_pos;
}

/*
Title: Scroll Bar

Section: Internal

Function: wz_scroll_proc

See also:
<wz_widget_proc>
*/
int wz_scroll_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;
	WZ_SCROLL* scl = (WZ_SCROLL*)wgt;
	float x, y;
	int vertical = wgt->h > wgt->w;

	switch(event->type)
	{
		case WZ_DRAW:
		{
			int flags = 0;
			float fraction;

			if(wgt->flags & WZ_STATE_HIDDEN)
			{
				ret = 0;
			}
			else if(wgt->flags & WZ_STATE_DISABLED)
			{
				flags = WZ_STYLE_DISABLED;
			}
			else if(wgt->flags & WZ_STATE_HAS_FOCUS)
			{
				flags = WZ_STYLE_FOCUSED;
			}

			fraction = ((float)scl->cur_pos) / ((float)scl->max_pos);
			wgt->theme->draw_scroll(wgt->theme, wgt->local_x, wgt->local_y, wgt->w, wgt->h, fraction, scl->slider_size, flags);
			break;
		}
		case WZ_SET_SCROLL_POS:
		{
			scl->cur_pos = event->user.data3;

			if(scl->cur_pos < 0)
				scl->cur_pos = 0;

			if(scl->cur_pos > scl->max_pos)
				scl->cur_pos = scl->max_pos;

			break;
		}
		case WZ_SET_SCROLL_MAX_POS:
		{
			scl->max_pos = event->user.data3;

			if(scl->max_pos < 0)
				scl->max_pos = 0;

			if(scl->cur_pos > scl->max_pos)
				scl->cur_pos = scl->max_pos;

			break;
		}
		case WZ_HANDLE_SHORTCUT:
		{
			wz_ask_parent_for_focus(wgt);
			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_MOVE:
			x = event->touch.x;
			y = event->touch.y;
#endif
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			if(event->type == ALLEGRO_EVENT_MOUSE_AXES)
			{
				x = event->mouse.x;
				y = event->mouse.y;
			}

			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else if(event->mouse.dx != 0 || event->mouse.dy != 0)
			{
				ret = 0;
				if(wz_widget_rect_test(wgt, x, y) && !(wgt->flags & WZ_STATE_HAS_FOCUS))
				{
					wz_ask_parent_for_focus(wgt);
					ret = 1;
				}

				if(scl->down)
				{
					if(set_scroll_pos(scl, x, y))
					{
						wz_trigger(wgt);
						ret = 1;
					}
				}

			}
			else
			{
				ret = 0;
			}

			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_BEGIN:
			x = event->touch.x;
			y = event->touch.y;
#endif
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			if(event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
			{
				x = event->mouse.x;
				y = event->mouse.y;
			}

			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else if(wz_widget_rect_test(wgt, x, y))
			{
				wz_ask_parent_for_focus(wgt);
				wgt->hold_focus = 1;

				if(set_scroll_pos(scl, x, y))
					wz_trigger(wgt);

				scl->down = 1;
			}
			else
				ret = 0;

			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_END:
#endif
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else
			{
				if(scl->down)
				{
					scl->down = 0;
					wgt->hold_focus = 0;
				}
				else
				{
					ret = 0;
				}
			}
			break;
		}
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			int old_pos = scl->cur_pos;;

			switch(event->keyboard.keycode)
			{
				case ALLEGRO_KEY_LEFT:
				{
					if(!vertical && scl->cur_pos > 0)
						scl->cur_pos--;
					else
						ret = 0;

					break;
				}
				case ALLEGRO_KEY_RIGHT:
				{
					if(!vertical && scl->cur_pos < scl->max_pos)
						scl->cur_pos++;
					else
						ret = 0;

					break;
				}
				case ALLEGRO_KEY_UP:
				{
					if(vertical && scl->cur_pos > 0)
						scl->cur_pos--;
					else
						ret = 0;

					break;
				}
				case ALLEGRO_KEY_DOWN:
				{
					if(vertical && scl->cur_pos < scl->max_pos)
						scl->cur_pos++;
					else
						ret = 0;

					break;
				}
				default:
					ret = 0;
			}

			if(old_pos != scl->cur_pos)
			{
				wz_trigger(wgt);
			}

			break;
		}
		case WZ_TRIGGER:
		{
			ALLEGRO_EVENT ev;

			if(!(wgt->flags & WZ_STATE_HAS_FOCUS))
			{
				wz_ask_parent_for_focus(wgt);
			}

			wz_craft_event(&ev, WZ_SCROLL_POS_CHANGED, wgt, scl->cur_pos);
			al_emit_user_event(wgt->source,	&ev, 0);
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
Function: wz_init_scroll
*/
void wz_init_scroll(WZ_SCROLL* scl, WZ_WIDGET* parent, float x, float y, float w, float h, int max_pos, int slider_size, int id)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)scl;
	wz_init_widget(wgt, parent, x, y, w, h, id);
	wgt->proc = wz_scroll_proc;
	scl->cur_pos = 0;
	scl->max_pos = max_pos;
	scl->down = 0;
	scl->slider_size = slider_size;
}

/*
Section: Public

Function: wz_create_scroll

Creates a scroll bar.

Inherits From:
<WZ_WIDGET>

See Also:
<WZ_SCROLL>

<wz_create_widget>
*/
WZ_SCROLL* wz_create_scroll(WZ_WIDGET* parent, float x, float y, float w, float h, int max_pos, int slider_size, int id)
{
	WZ_SCROLL* scl = malloc(sizeof(WZ_SCROLL));
	wz_init_scroll(scl, parent, x, y, w, h, max_pos, slider_size, id);
	return scl;
}
