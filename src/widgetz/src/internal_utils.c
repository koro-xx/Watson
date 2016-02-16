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

/*
Title: Internal

These are functions that are useful if you need to create additional widgets
*/

/*
Function: wz_ask_parent_for_focus

Asks the parent to defocus everyone but the widget that calls this function.

Returns:
1 if the widget already has focus, or succesfully obtained the focus

0 if the widget cannot be focused
*/
int wz_ask_parent_for_focus(WZ_WIDGET* wgt)
{
	if(wgt->flags & WZ_STATE_HAS_FOCUS)
		return 1;

	if(wgt->flags & WZ_STATE_NOTWANT_FOCUS)
		return 0;

	if(wgt->flags & WZ_STATE_DISABLED)
		return 0;

	if(wgt->flags & WZ_STATE_HIDDEN)
		return 0;

	if(wgt->parent == 0)
	{
		ALLEGRO_EVENT event;
		wz_craft_event(&event, WZ_TAKE_FOCUS, wgt, 0);
		wz_send_event(wgt, &event);
	}
	else
	{
		ALLEGRO_EVENT event;

		if(!(wgt->parent->flags & WZ_STATE_HAS_FOCUS))
		{
			wz_ask_parent_for_focus(wgt->parent);
		}

		wz_craft_event(&event, WZ_WANT_FOCUS, wgt, 0);
		wz_send_event(wgt->parent, &event);
	}

	return 1;
}

/*
Function: wz_ask_parent_to_focus_next

Asks the parent to focus the next child if possible
*/
void wz_ask_parent_to_focus_next(WZ_WIDGET* wgt)
{
	WZ_WIDGET* child;

	if(wgt->parent == 0)
		return;

	child = wgt->next_sib;

	while(child)
	{
		if(wz_ask_parent_for_focus(child))
			return;

		child = child->next_sib;
	}

	child = wgt->parent->first_child;

	while(child != wgt)
	{
		if(wz_ask_parent_for_focus(child))
			return;

		child = child->next_sib;
	}
}

/*
Function: wz_ask_parent_to_focus_prev

Asks the parent to focus the previous child if possible
*/
void wz_ask_parent_to_focus_prev(WZ_WIDGET* wgt)
{
	WZ_WIDGET* child;

	if(wgt->parent == 0)
		return;

	child = wgt->prev_sib;

	while(child)
	{
		if(wz_ask_parent_for_focus(child))
			return;

		child = child->prev_sib;
	}

	child = wgt->parent->last_child;

	while(child != wgt)
	{
		if(wz_ask_parent_for_focus(child))
			return;

		child = child->prev_sib;
	}
}

/*
Function: wz_get_widget_dir

A function that returns a widget that is located the closest to the passed widget in some given direction.

Parameters:
dir - 0 is up, 1 is right, 2 is down, 3 is left

Returns:

The widget it found, or the passed widget if it found nothing
*/
WZ_WIDGET* wz_get_widget_dir(WZ_WIDGET* wgt, int dir)
{
	float least_dev = 100000;
	WZ_WIDGET* ret = wgt;
	WZ_WIDGET* child;

	if(wgt->parent == 0)
		return wgt;

	child = wgt->parent->first_child;

	while(child)
	{
		float dev = 1000000;

		switch(dir)
		{
			case 0:
			{
				if(child->y + child->h < wgt->y)
				{
					dev = wgt->y - (child->y + child->h) + fabs(wgt->x - child->x);
				}

				break;
			}
			case 1:
			{
				if(child->x > wgt->x + wgt->w)
				{
					dev = child->x - (wgt->x + wgt->w) + fabs(wgt->y - child->y);
				}

				break;
			}
			case 2:
			{
				if(child->y > wgt->y + wgt->h)
				{
					dev = child->y - (wgt->y + wgt->h) + fabs(wgt->x - child->x);
				}

				break;
			}
			default:
			{
				if(child->x + child->w < wgt->x)
				{
					dev = wgt->x - (child->x + child->w) + fabs(wgt->y - child->y);
				}

				break;
			}
		}

		if(child != wgt && dev < least_dev
		        && !(child->flags & WZ_STATE_NOTWANT_FOCUS)
		        && !(child->flags & WZ_STATE_DISABLED)
		        && !(child->flags & WZ_STATE_HIDDEN))
		{
			least_dev = dev;
			ret = child;
		}

		child = child->next_sib;
	}

	return ret;
}

/*
Function: wz_craft_event

Crafts a simple GUI event.

Parameters:
type - Type of the event
source - The widget that launched the event, or 0 if it has no source
data - data you wish to attach to the event
*/
void wz_craft_event(ALLEGRO_EVENT* event, int type, WZ_WIDGET* source, intptr_t data)
{
	event->user.type = type;
	event->user.timestamp = al_get_time();
	event->user.data1 = source == 0 ? -1 : source->id;
	event->user.data2 = (intptr_t)source;
	event->user.data3 = data;
}

/*
Function: wz_get_text_pos

Parameters:

text - the text you want to search
x - the length you want to match

Returns:

The character position such that the text length of the string up to that character
is as close as possible to the passed length.
*/
int wz_get_text_pos(ALLEGRO_FONT* font, ALLEGRO_USTR* text, float x)
{
	int ii = 0;
	int len = al_ustr_length(text);
	float width = al_get_ustr_width(font, text);

	if(x > width)
	{
		return len + 1;
	}

	if(x < 0)
	{
		return 0;
	}
	else
	{
		float old_diff = x;
		float diff;
		ALLEGRO_USTR_INFO info;

		for(ii = 0; ii <= len; ii++)
		{
			int offset = al_ustr_offset(text, ii);
			const ALLEGRO_USTR* str = al_ref_ustr(&info, text, 0, offset);
			diff = fabs(x - al_get_ustr_width(font, str));

			if(diff > old_diff)
			{
				return ii - 1;
			}

			old_diff = diff;
		}
	}

	return ii - 1;
}


