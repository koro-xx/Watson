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
Title: Widget

Section: Internal

Function: wz_widget_proc

Callback function that handles the operations of a general widget.
All widget procs call this function as their default handler.

Returns:

1 if the event was handled by the widget, 0 otherwise
*/
int wz_widget_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;

	switch(event->type)
	{
		case WZ_HIDE:
		{
			wgt->flags |= WZ_STATE_HIDDEN;
			break;
		}
		case WZ_SHOW:
		{
			wgt->flags &= ~WZ_STATE_HIDDEN;
			break;
		}
		case WZ_DISABLE:
		{
			wgt->flags |= WZ_STATE_DISABLED;
			break;
		}
		case WZ_ENABLE:
		{
			wgt->flags &= ~WZ_STATE_DISABLED;
			break;
		}
		case WZ_UPDATE_POSITION:
		{
			if(wgt->parent)
			{
				wgt->local_x = wgt->parent->local_x + wgt->x;
				wgt->local_y = wgt->parent->local_y + wgt->y;
			}
			else
			{
				wgt->local_x = wgt->x;
				wgt->local_y = wgt->y;
			}

			break;
		}
		case WZ_DESTROY:
		{
			al_destroy_user_event_source(wgt->source);
			free(wgt->source);
			free(wgt);
			break;
		}
		case WZ_LOSE_FOCUS:
		{
			wgt->flags &= ~WZ_STATE_HAS_FOCUS;
			break;
		}
		case WZ_TAKE_FOCUS:
		{
			wz_focus(wgt, 0);
			wgt->flags |= WZ_STATE_HAS_FOCUS;

			if(wgt->first_child)
				wz_focus(wgt->first_child, 1);

			break;
		}
		case WZ_WANT_FOCUS:
		{
			WZ_WIDGET* child = wgt->first_child;
			ALLEGRO_EVENT ev;
			int all_unfocused = 1;

			while(child)
			{
				if(child->hold_focus)
				{
					all_unfocused = 0;
					break;
				}

				wz_focus(child, 0);
				child = child->next_sib;
			}

			if(all_unfocused)
			{
				wz_craft_event(&ev, WZ_TAKE_FOCUS, wgt, 0);
				wz_send_event((WZ_WIDGET*)event->user.data2, &ev);
			}

			break;
		}
		/* Switch through elements on Touch:
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			if (wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else if (wgt->first_child == 0 && wgt->parent != 0)
			{
				wz_ask_parent_to_focus_next(wgt);
			}
			else
			{
				ret = 0;
			}
			break;
		}
		*/
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			if(event->keyboard.keycode == wgt->shortcut.keycode && ((event->keyboard.modifiers & wgt->shortcut.modifiers) || wgt->shortcut.modifiers == 0))
			{
				ALLEGRO_EVENT ev;
				wz_craft_event(&ev, WZ_HANDLE_SHORTCUT, wgt, 0);
				wz_send_event(wgt, &ev);
			}
			else
			{
				switch(event->keyboard.keycode)
				{
					case ALLEGRO_KEY_TAB:
					{
						if(wgt->first_child != 0)
						{
							ret = 0;
						}
						else if(event->keyboard.modifiers & 1)
						{
							wz_ask_parent_to_focus_prev(wgt);
						}
						else
						{
							wz_ask_parent_to_focus_next(wgt);
						}

						break;
					}
					case ALLEGRO_KEY_UP:
					{
						if(wgt->first_child != 0)
						{
							ret = 0;
						}
						else if(wgt->parent != 0)
						{
							wz_ask_parent_for_focus(wz_get_widget_dir(wgt, 0));
						}
						else
							ret = 0;

						break;
					}
					case ALLEGRO_KEY_RIGHT:
					{
						if(wgt->first_child != 0)
						{
							ret = 0;
						}
						else if(wgt->parent != 0)
						{
							wz_ask_parent_for_focus(wz_get_widget_dir(wgt, 1));
						}
						else
							ret = 0;

						break;
					}
					case ALLEGRO_KEY_DOWN:
					{
						if(wgt->first_child != 0)
						{
							ret = 0;
						}
						else if(wgt->parent != 0)
						{
							wz_ask_parent_for_focus(wz_get_widget_dir(wgt, 2));
						}
						else
							ret = 0;

						break;
					}
					case ALLEGRO_KEY_LEFT:
					{
						if(wgt->first_child != 0)
						{
							ret = 0;
						}
						else if(wgt->parent != 0)
						{
							wz_ask_parent_for_focus(wz_get_widget_dir(wgt, 3));
						}
						else
							ret = 0;

						break;
					}
					default:
						ret = 0;
				}
			}

			break;
		}
		default:
			ret = 0;
	}

	return ret;
}

/*
Function: wz_init_widget

Initializes a generic widget to some sane values
*/
void wz_init_widget(WZ_WIDGET* wgt, WZ_WIDGET* parent, float x, float y, float w, float h, int id)
{
	wgt->x = x;
	wgt->y = y;
	wgt->h = h;
	wgt->w = w;
	wgt->flags = 1;
	wgt->theme = (WZ_THEME*) & wz_def_theme;
	wgt->proc = wz_widget_proc;
	wgt->parent = 0;
	wgt->last_child = 0;
	wgt->first_child = 0;
	wgt->next_sib = 0;
	wgt->prev_sib = 0;
	wgt->id = id;
	wgt->hold_focus = 0;

	wgt->source = malloc(sizeof(ALLEGRO_EVENT_SOURCE));
	al_init_user_event_source(wgt->source);
	wgt->shortcut.modifiers = 0;
	wgt->shortcut.keycode = -1;
	wz_attach(wgt, parent);
	wz_ask_parent_for_focus(wgt);
}

/*
Section: Public

Function: wz_create_widget

Creates a standard widget. You generally don't need to use this, unless you want a do-nothing dummy widget.

Parameters:
id - The id of this widget, which will be used to identify it when it sends events to the user's queue.
Pass -1 to automatically assign an id. If automatically assigned, id will equal the id of its previous sibling
plus one, or if it has none, the id of its parent plus one. If it has no parent, its id will be 0.

See Also:
<WZ_WIDGET>
*/
WZ_WIDGET* wz_create_widget(WZ_WIDGET* parent, float x, float y, int id)
{
	WZ_WIDGET* wgt = malloc(sizeof(WZ_WIDGET));
	wz_init_widget(wgt, parent, x, y, 0, 0, id);
	return wgt;
}
