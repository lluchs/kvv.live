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

#include "layouts.h"
#include "route_color.h"
#include "common.h"

#define DEPARTURE_HEIGHT 19

/**
 * Draws a departure line.
 */
static struct DepartureLine* create(const struct Departure *d, GRect frame) {
	// Allocate a departure.
	struct DepartureLine *line = (struct DepartureLine*)malloc(sizeof(struct DepartureLine));
	// Override any height.
	frame.size.h = 2 * DEPARTURE_HEIGHT;
	line->layer = layer_create(frame);

	// A departure is rendered as "route destination time".
	// route
	line->route = text_layer_create((GRect) { .origin = { 3, 0 }, .size = { 25, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->route, d->route);
	text_layer_set_text_alignment(line->route, GTextAlignmentCenter);
	struct route_color color = get_color_for_route(d->route);
	text_layer_set_background_color(line->route, color.bg);
	text_layer_set_text_color(line->route, color.fg);

	// destination
	line->destination = text_layer_create((GRect) { .origin = { 30, 0 }, .size = { 112, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->destination, d->destination);

	// time
	line->time = text_layer_create((GRect) { .origin = { 107, DEPARTURE_HEIGHT }, .size = { 37, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->time, d->time);

	// Add to the frame.
	layer_add_child(line->layer, text_layer_get_layer(line->route));
	layer_add_child(line->layer, text_layer_get_layer(line->destination));
	layer_add_child(line->layer, text_layer_get_layer(line->time));

	return line;
}


struct departure_layout departure_layout_twoline = {
	.departure_height = 2 * DEPARTURE_HEIGHT,
	.departure_line_create = create,
	.departure_line_update = departure_layout_update,
	.departure_line_destroy = departure_layout_destroy
};
