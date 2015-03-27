// Copyright © 2013-2015 Lukas Werling
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

#include <ctype.h>
#include "departure.h"

#define DEPARTURE_HEIGHT 19

/**
 * Deserializes departure values as received.
 *
 * The values are saved in the given departure.
 */
void departure_deserialize(DictionaryIterator *iter, struct Departure *departure) {
	Tuple *route_tuple = dict_find(iter, DEPARTURE_KEY_ROUTE);
	Tuple *destination_tuple = dict_find(iter, DEPARTURE_KEY_DESTINATION);
	Tuple *time_tuple = dict_find(iter, DEPARTURE_KEY_TIME);

	if (route_tuple) {
		strncpy(departure->route, route_tuple->value->cstring, sizeof(departure->route));
		departure->route[sizeof(departure->route) - 1] = '\0';
	}
	if (destination_tuple) {
		strncpy(departure->destination, destination_tuple->value->cstring, sizeof(departure->destination));
		departure->destination[sizeof(departure->destination) - 1] = '\0';
	}
	if (time_tuple) {
		strncpy(departure->time, time_tuple->value->cstring, sizeof(departure->time));
		departure->time[sizeof(departure->time) - 1] = '\0';
	}
}

/**
 * Determines color for a given route.
 */
#ifdef PBL_COLOR
struct route_color {
	GColor8 bg;
	GColor8 fg;
};
static struct route_color get_color_for_route(const char *route) {
	struct route_color color = {.bg = GColorBlack, .fg = GColorWhite};
	// Check for bus lines.
	if (isdigit((unsigned char)route[0]) && isdigit((unsigned char)route[1])) {
		color.bg = GColorPurple;
		return color;
	}
	// Nightliners have "NL" prefix.
	if (route[0] == 'N' && route[1] == 'L')
		route += 2;
	// Some kind of train.
	switch (route[0]) {
		case '1': color.bg = GColorRed; break;
		case '2': color.bg = GColorCobaltBlue; break;
		case '3': color.bg = GColorWindsorTan; break;
		case '4': color.bg = GColorYellow;
				  color.fg = GColorBlack; break;
		case '5': color.bg = GColorVividCerulean; break;
		case '6': color.bg = GColorSpringBud; break;
		case '8': color.bg = GColorChromeYellow;
				  color.fg = GColorBlack; break;
		case 'S':
		  switch (route[1]) {
			  case '1': color.bg = GColorJaegerGreen; break;
			  case '2': color.bg = GColorLavenderIndigo; break;
			  case '3': color.bg = GColorTiffanyBlue; break;
			  case '4': color.bg = GColorJazzberryJam; break;
			  case '5':
			  case '9': color.bg = GColorMelon;
						color.fg = GColorBlack; break;
			  case '6': color.bg = GColorOxfordBlue; break;
		  }
	}
	return color;
}

#else

struct route_color {
	GColor bg;
	GColor fg;
};
static struct route_color get_color_for_route(const char *route) {
	return (struct route_color) {.bg = GColorBlack, .fg = GColorWhite};
}
#endif

int departure_height() {
	return DEPARTURE_HEIGHT;
}

/**
 * Draws a departure line.
 */
struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame) {
	// Allocate a departure.
	struct DepartureLine *line = (struct DepartureLine*)malloc(sizeof(struct DepartureLine));
	// Override any height.
	frame.size.h = DEPARTURE_HEIGHT;
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
	line->destination = text_layer_create((GRect) { .origin = { 30, 0 }, .size = { 75, DEPARTURE_HEIGHT } });
	text_layer_set_font(line->destination, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text(line->destination, d->destination);

	// time
	line->time = text_layer_create((GRect) { .origin = { 107, 0 }, .size = { 37, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->time, d->time);

	// Add to the frame.
	layer_add_child(line->layer, text_layer_get_layer(line->route));
	layer_add_child(line->layer, text_layer_get_layer(line->destination));
	layer_add_child(line->layer, text_layer_get_layer(line->time));

	return line;
}

/**
 * Updates a departure line after the underlying departure has changed.
 */
void departure_line_update(struct DepartureLine *line) {
	const char *route = text_layer_get_text(line->route);
	struct route_color color = get_color_for_route(route);
	text_layer_set_background_color(line->route, color.bg);
	text_layer_set_text_color(line->route, color.fg);
}

/**
 * Destroys a departure line.
 */
void departure_line_destroy(struct DepartureLine *line) {
	text_layer_destroy(line->time);
	text_layer_destroy(line->destination);
	text_layer_destroy(line->route);
	layer_destroy(line->layer);
	free(line);
}

