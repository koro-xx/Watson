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

void wz_snap_editbox(WZ_EDITBOX* box)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)box;
	ALLEGRO_FONT* font = wgt->theme->get_font(wgt->theme, 0);
	int len = al_ustr_length(box->text);
	int size = al_ustr_size(box->text);
	int scroll_offset = al_ustr_offset(box->text, box->scroll_pos);
	int cursor_offset;
	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR* text = al_ref_ustr(&info, box->text, scroll_offset, size);
	int max_rel_cursor_pos = wz_get_text_pos(font, (ALLEGRO_USTR*)text, wgt->w);

	if(box->cursor_pos < box->scroll_pos)
	{
		box->scroll_pos = box->cursor_pos;
	}

	if(box->cursor_pos > box->scroll_pos + max_rel_cursor_pos)
	{
		box->scroll_pos = box->cursor_pos - max_rel_cursor_pos;
	}

	if(box->cursor_pos > 0 && box->cursor_pos - box->scroll_pos < 1)
	{
		box->scroll_pos--;
	}

	if(box->cursor_pos > len)
	{
		box->cursor_pos = len;
	}

	if(box->cursor_pos < 0)
	{
		box->cursor_pos = 0;
	}

	scroll_offset = al_ustr_offset(box->text, box->scroll_pos);
	cursor_offset = al_ustr_offset(box->text, box->cursor_pos);
	text = al_ref_ustr(&info, box->text, scroll_offset, cursor_offset);

	if(al_get_ustr_width(font, text) > wgt->w)
	{
		box->scroll_pos++;
	}
}

/*
Title: Edit Box

Section: Internal

Function: wz_editbox_proc

See also:
<wz_widget_proc>
*/
int wz_editbox_proc(WZ_WIDGET* wgt, const ALLEGRO_EVENT* event)
{
	int ret = 1;
	WZ_EDITBOX* box = (WZ_EDITBOX*)wgt;

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
				int size = al_ustr_size(box->text);
				int scroll_offset = al_ustr_offset(box->text, box->scroll_pos);
				ALLEGRO_USTR_INFO info;
				const ALLEGRO_USTR* text = al_ref_ustr(&info, box->text, scroll_offset, size);
				int pos = box->cursor_pos - box->scroll_pos;
				int flags = 0;

				if(wgt->flags & WZ_STATE_DISABLED)
					flags = WZ_STYLE_DISABLED;
				else if(wgt->flags & WZ_STATE_HAS_FOCUS)
					flags = WZ_STYLE_FOCUSED;

				wgt->theme->draw_editbox(wgt->theme, wgt->local_x, wgt->local_y, wgt->w, wgt->h, pos, (ALLEGRO_USTR*)text, flags);
			}

			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else if(event->mouse.button == 1 && wz_widget_rect_test(wgt, event->mouse.x, event->mouse.y))
			{
				int len = al_ustr_length(box->text);
				ALLEGRO_USTR_INFO info;
				const ALLEGRO_USTR* text = al_ref_ustr(&info, box->text, box->scroll_pos, len - 1);
				ALLEGRO_FONT* font = wgt->theme->get_font(wgt->theme, 0);
				wz_ask_parent_for_focus(wgt);
				box->cursor_pos = wz_get_text_pos(font, (ALLEGRO_USTR*)text, event->mouse.x - wgt->x) + box->scroll_pos;
			}
			else
				ret = 0;

			break;
		}
#if (ALLEGRO_SUB_VERSION > 0)
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}
			else if(wz_widget_rect_test(wgt, event->touch.x, event->touch.y))
			{
				int len = al_ustr_length(box->text);
				ALLEGRO_USTR_INFO info;
				const ALLEGRO_USTR* text = al_ref_ustr(&info, box->text, box->scroll_pos, len - 1);
				ALLEGRO_FONT* font = wgt->theme->get_font(wgt->theme, 0);
				wz_ask_parent_for_focus(wgt);
				box->cursor_pos = wz_get_text_pos(font, (ALLEGRO_USTR*)text, event->touch.x - wgt->x) + box->scroll_pos;
			}
			else
				ret = 0;

			break;
		}
#endif
		case WZ_HANDLE_SHORTCUT:
		{
			wz_ask_parent_for_focus(wgt);
			break;
		}
		case WZ_DESTROY:
		{
			if(box->own)
				al_ustr_free(box->text);

			ret = 0;
			break;
		}
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			int len;

			if(wgt->flags & WZ_STATE_DISABLED || !(wgt->flags & WZ_STATE_HAS_FOCUS))
			{
				ret = 0;
				break;
			}
			else if(event->keyboard.modifiers & ALLEGRO_KEYMOD_CTRL || event->keyboard.modifiers & ALLEGRO_KEYMOD_ALT)
			{
				ret = 0;
			}

			len = al_ustr_length(box->text);

			if((int)(event->keyboard.unichar) > 31 && (int)(event->keyboard.unichar) != 127)
			{
				al_ustr_insert_chr(box->text, al_ustr_offset(box->text, box->cursor_pos), event->keyboard.unichar);
				box->cursor_pos++;
			}
			else
			{
				switch(event->keyboard.keycode)
				{
					case ALLEGRO_KEY_BACKSPACE:
					{
						if(len > 0 && box->cursor_pos > 0)
						{
							al_ustr_remove_chr(box->text, al_ustr_offset(box->text, box->cursor_pos - 1));
							box->cursor_pos--;
						}

						break;
					}
					case ALLEGRO_KEY_DELETE:
					{
						if(len > 0 && box->cursor_pos < len)
						{
							al_ustr_remove_chr(box->text, al_ustr_offset(box->text, box->cursor_pos));
						}

						break;
					}
					case ALLEGRO_KEY_LEFT:
					{
						if(box->cursor_pos > 0)
						{
							box->cursor_pos--;
						}
						else
							ret = 0;

						break;
					}
					case ALLEGRO_KEY_RIGHT:
					{
						if(box->cursor_pos < len)
						{
							box->cursor_pos++;
						}
						else
							ret = 0;

						break;
					}
					case ALLEGRO_KEY_HOME:
					{
						box->cursor_pos = 0;
						break;
					}
					case ALLEGRO_KEY_END:
					{
						len = al_ustr_length(box->text);
						box->cursor_pos = len;
						break;
					}
					case ALLEGRO_KEY_ENTER:
					{
						wz_trigger(wgt);
						break;
					}
					default:
						ret = 0;
				}
			}

			wz_snap_editbox(box);
			break;
		}
		case WZ_SET_CURSOR_POS:
		{
			box->cursor_pos = event->user.data3;
			wz_snap_editbox(box);
		}
		case WZ_SET_TEXT:
		{
			if(box->own)
			{
				al_ustr_assign(box->text, (ALLEGRO_USTR*)event->user.data3);
			}
			else
				box->text = (ALLEGRO_USTR*)event->user.data3;

			wz_snap_editbox(box);
			break;
		}
		case WZ_TRIGGER:
		{
			ALLEGRO_EVENT ev;
			wz_craft_event(&ev, WZ_TEXT_CHANGED, wgt, 0);
			al_emit_user_event(wgt->source,	&ev, 0);
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			if(wgt->flags & WZ_STATE_DISABLED)
			{
				ret = 0;
			}

			if(wz_widget_rect_test(wgt, event->mouse.x, event->mouse.y))
			{
				wz_ask_parent_for_focus(wgt);
			}

			return wz_widget_proc(wgt, event);
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
Function: wz_init_editbox
*/
void wz_init_editbox(WZ_EDITBOX* box, WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int id)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)box;
	wz_init_widget(wgt, parent, x, y, w, h, id);
	box->own = own;

	if(!text)
	{
		box->text = al_ustr_new("");
		al_ustr_assign(box->text, text);
		box->own = 1;
	}
	else
	{
		box->text = text;
	}

	box->cursor_pos = 0;
	box->scroll_pos = 0;
	wgt->proc = wz_editbox_proc;
}

/*
Section: Public

Function: wz_create_editbox

Creates an edit box.

Parameters:

own - Set to 1 if you want the widget to own the text

Inherits From:
<WZ_WIDGET>

See Also:
<WZ_EDITBOX>

<wz_create_widget>
*/
WZ_EDITBOX* wz_create_editbox(WZ_WIDGET* parent, float x, float y, float w, float h, ALLEGRO_USTR* text, int own, int id)
{
	WZ_EDITBOX* box = malloc(sizeof(WZ_EDITBOX));
	wz_init_editbox(box, parent, x, y, w, h, text, own, id);
	return box;
}
