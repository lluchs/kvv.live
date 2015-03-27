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

/* Stops struct and helpers. */

#include "sds/sds.h"

enum {
	PERSIST_STOPS_LENGTH,
	PERSIST_STOPS_START
};

struct stops {
	unsigned int num;
	sds *names;
	sds *ids;
	// Only set for proximity search.
	int *distances;
};

// struct stops functions
struct stops const * get_proximity_stops();
struct stops* read_favorite_stops();
void stops_destroy(struct stops *stops);

// messaging
void stops_set_favorites_num(int num);
void stops_set_proximity_num(int num);
void stops_receive_stop(DictionaryIterator *iter);
