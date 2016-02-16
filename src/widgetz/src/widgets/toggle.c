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
Title: Toggle Button

Section: Internal

Function: wz_toggle_button_proc

See also:
<wz_widget_proc>
*/
int wz_toggle_button_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;
	float x, y;
	WZ_BUTTON* but = (WZ_BUTTON*)wgt;
	WZ_TOGGLE* tog = (WZ_TOGGLE*)wgt;

	switch(event->type)
	{
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
				wz_trigger(wgt);
			}
			else
				ret = 0;

			break;
		}
		case ALLEGRO_EVENT_KEY_DOWN:
		{
			switch(event->keyboard.keycode)
			{
				case ALLEGRO_KEY_ENTER:
				{
					if(wgt->flags & WZ_STATE_DISABLED)
					{
						ret = 0;
					}
					else if(wgt->flags & WZ_STATE_HAS_FOCUS)
					{
						wz_trigger(wgt);
					}
					else
						ret = 0;

					break;
				}
				default:
					ret = 0;
			}

			break;
		}
		case WZ_LOSE_FOCUS:
		{
			return wz_widget_proc(wgt, event);
			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_END:
#endif
		case ALLEGRO_EVENT_KEY_UP:
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			return wz_widget_proc(wgt, event);
			break;
		}
		case WZ_TOGGLE_GROUP:
		{
			if(event->user.data3 == tog->group)
			{
				but->down = 0;
			}

			break;
		}
		case WZ_TRIGGER:
		{
			ALLEGRO_EVENT ev;

			if(tog->group == -1)
			{
				/*
				A simple toggle button
				*/
				but->down = !but->down;
			}
			else if(wgt->parent)
			{
				/*
				Disable the group, and then enable self
				*/
				wz_craft_event(&ev, WZ_TOGGLE_GROUP, wgt, tog->group);
				wz_broadcast_event(wgt->parent, &ev);
				but->down = 1;
			}

			wz_craft_event(&ev, WZ_BUTTON_PRESSED, wgt, 0);
			al_emit_user_event(wgt->source,	&ev, 0);
			break;
		}
		default:
			ret = 0;
	}

	if(ret == 0)
		ret = wz_button_proc(wgt, event);

	return ret;
}

/*
Function: wz_init_toggle_button
*/
void wz_init_toggle_button(WZ_TOGGLE* tog, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int group, int id)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)tog;
	wz_init_button((WZ_BUTTON*)tog, parent, x, y, w, h, text, own, id);
	tog->group = group;
	wgt->proc = wz_toggle_button_proc;
}

/*
Section: Public

Function: wz_create_toggle_button

Parameters:

own - Set to 1 if you want the widget to own the text (it'll make a local copy)

See Also:
<wz_create_widget>

Inherits From:
<WZ_BUTTON>

See Also:
<WZ_TOGGLE>

<wz_create_widget>
*/
WZ_TOGGLE* wz_create_toggle_button(WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int group, int id)
{
	WZ_TOGGLE* tog = malloc(sizeof(WZ_TOGGLE));
	wz_init_toggle_button(tog, parent, x, y, w, h, text, own, group, id);
	return tog;
}
