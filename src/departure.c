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

#include "locale_framework/localize.h"
#include "departure.h"
#include "settings.h"
#include "departure_layout/layouts.h"

/**
 * Deserializes departure values as received.
 *
 * The values are saved in the given departure.
 */
void departure_deserialize(DictionaryIterator *iter, struct Departure *departure) {
	Tuple *route_tuple = dict_find(iter, DEPARTURE_KEY_ROUTE);
	Tuple *destination_tuple = dict_find(iter, DEPARTURE_KEY_DESTINATION);
	Tuple *time_tuple = dict_find(iter, DEPARTURE_KEY_TIME);
	Tuple *realtime_tuple = dict_find(iter, DEPARTURE_KEY_REALTIME);
	Tuple *lowfloor_tuple = dict_find(iter, DEPARTURE_KEY_LOWFLOOR);
	Tuple *traction_tuple = dict_find(iter, DEPARTURE_KEY_TRACTION);

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
	if (realtime_tuple) {
		departure->realtime = (bool)realtime_tuple->value->data[0];
	}
	if (lowfloor_tuple) {
		departure->lowfloor = (bool)lowfloor_tuple->value->data[0];
	}
	if (traction_tuple) {
		departure->traction = (uint8_t)traction_tuple->value->data[0];
	}

	// Time string reformatting.
	if (strcmp("0", departure->time) == 0)
		strcpy(departure->time, _("now"));
	if (!departure->realtime) {
		size_t len = strlen(departure->time);
		departure->time[len++] = '*';
		departure->time[len++] = '\0';
	}
}

static struct departure_layout* get_layout() {
	if (get_setting(SETTING_COMPACT_DEPARTURES))
		return &departure_layout_oneline;
	else
		return &departure_layout_twoline;
}

int departure_height() {
	return get_layout()->departure_height;
}

/**
 * Draws a departure line.
 */
struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame) {
	return get_layout()->departure_line_create(d, frame);
}

/**
 * Updates a departure line after the underlying departure has changed.
 */
void departure_line_update(struct DepartureLine *line) {
	get_layout()->departure_line_update(line);
}

/**
 * Destroys a departure line.
 */
void departure_line_destroy(struct DepartureLine *line) {
	get_layout()->departure_line_destroy(line);
}

