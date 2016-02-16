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
Title: Layout Stop
*/

/*
Function: wz_init_layout_stop
*/
void wz_init_layout_stop(WZ_WIDGET* box, WZ_WIDGET* parent, int id)
{
	WZ_WIDGET* wgt = (WZ_WIDGET*)box;
	wz_init_widget(wgt, parent, 0, 0, 0, 0, id);
	wgt->proc = wz_widget_proc;
	wgt->flags |= WZ_STATE_LAYOUT;
	wgt->flags |= WZ_STATE_NOTWANT_FOCUS;
}

/*
Section: Public

Function: wz_create_layout_stop

Creates a layout stop meta widget. Any widgets after this one will no longer be subject
to any layout control

Inherits From:
<WZ_WIDGET>

See Also:
<WZ_FILL_LAYOUT>

<wz_create_widget>
*/
WZ_WIDGET* wz_create_layout_stop(WZ_WIDGET* parent, int id)
{
	WZ_WIDGET* box = malloc(sizeof(WZ_WIDGET));
	wz_init_layout_stop(box, parent, id);
	wz_ask_parent_to_focus_next(box);
	return box;
}

