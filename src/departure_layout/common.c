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

#include "common.h"
#include "route_color.h"

/**
 * Updates a departure line after the underlying departure has changed.
 */
void departure_layout_update(struct DepartureLine *line) {
	const char *route = line->departure->route;
	struct route_color color = get_color_for_route(route);
	text_layer_set_background_color(line->route, color.bg);
	text_layer_set_text_color(line->route, color.fg);

	// Hack: "sofort" does not fit.
	const char *time = line->departure->time;
	static int sofort_kerning[] = { 0, -1, -1, -1, -1, -1, -2 };
	if (time[0] == 's')
		kerning_text_layer_set_kerning(line->time, sofort_kerning);
	else
		kerning_text_layer_set_kerning(line->time, NULL);
}

/**
 * Destroys a departure line.
 */
void departure_layout_destroy(struct DepartureLine *line) {
	kerning_text_layer_destroy(line->time);
	text_layer_destroy(line->destination);
	text_layer_destroy(line->route);
	layer_destroy(line->layer);
	free(line);
}
