// Copyright © 2015 Lukas Werling
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <pebble.h>

static Layer *status_bar = NULL;

static void init_status_bar() {
#ifdef PBL_SDK_3
	StatusBarLayer *bar = status_bar_layer_create();
	status_bar = status_bar_layer_get_layer(bar);
#else
	GRect r;
	status_bar = layer_create(r);
#endif
}

Layer* status_bar_layer() {
	if (!status_bar) init_status_bar();
	return status_bar;
}

void status_bar_adjust_window_bounds(GRect *bounds) {
#ifdef PBL_SDK_3
	bounds->origin.y += STATUS_BAR_LAYER_HEIGHT;
	bounds->size.h -= STATUS_BAR_LAYER_HEIGHT;
#endif
}
