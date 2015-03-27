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

/* Departure struct. */

#pragma once

#include <pebble.h>

#include "network.h"

struct Departure {
	char route[4];
	char destination[20];
	char time[10];
};

struct DepartureLine {
	Layer *layer;
	TextLayer *route;
	TextLayer *destination;
	TextLayer *time;
};

void departure_deserialize(DictionaryIterator *iter, struct Departure *departure);

int departure_height();
struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame);

void departure_line_update(struct DepartureLine *line);
void departure_line_destroy(struct DepartureLine *line);
