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

#pragma once

#include <pebble.h>
#include "departure.h"

struct departure_layout {
	int departure_height;
	struct DepartureLine* (*departure_line_create)(const struct Departure *d, GRect frame);
	void (*departure_line_update)(struct DepartureLine *line);
	void (*departure_line_destroy)(struct DepartureLine *line);
};

extern struct departure_layout departure_layout_oneline;
/* extern struct departure_layout departure_layout_twoline; */
