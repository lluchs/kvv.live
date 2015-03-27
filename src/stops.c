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

#include <pebble.h>
#include "stops.h"
#include "network.h"

static struct stops proximity_stops;

/* Adds the given stop to the persistant memory. */
static void add_favorite_stop(int i, char *name, char *id) {
	int key = PERSIST_STOPS_START + i * 2;
	persist_write_string(key, name);
	persist_write_string(key + 1, id);
}

static void create_default_stops() {
	stops_set_favorites_num(5);
	add_favorite_stop(0, "KA Hbf Vorplatz", "de:8212:89");
	add_favorite_stop(1, "KA Durlacher Tor", "de:8212:3");
	add_favorite_stop(2, "KA Marktplatz (Kaiserstr)", "de:8212:1");
	add_favorite_stop(3, "KA ZKM", "de:8212:65");
	add_favorite_stop(4, "KA Entenfang", "de:8212:51");
}

/* Returns the stops requested from proximity search. */
struct stops const * get_proximity_stops() {
	return &proximity_stops;
}

/* Reads stops from persistent memory. */
struct stops* read_favorite_stops() {
	if (!persist_read_int(PERSIST_STOPS_LENGTH)) {
		APP_LOG(APP_LOG_LEVEL_INFO, "No stops found, creating default ones...");
		create_default_stops();
	}

	struct stops *stops = (struct stops*)malloc(sizeof(struct stops));
	stops->num = persist_read_int(PERSIST_STOPS_LENGTH);
	char buffer[30];

	// Allocate arrays.
	stops->names = (sds*)malloc(stops->num * sizeof(sds));
	stops->ids = (sds*)malloc(stops->num * sizeof(sds));

	for (unsigned int i = 0, key; i < stops->num; i++) {
		key = PERSIST_STOPS_START + i * 2;
		// We need both name and id.
		if (!persist_exists(key) || !persist_exists(key + 1)) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Missing stop entry in persistant memory.");
			// Initialize to empty strings and hope that nothing breaks...
			stops->names[i] = sdsnew("");
			stops->ids[i] = sdsnew("");
		}
		// Name
		persist_read_string(key, buffer, 30);
		stops->names[i] = sdsnew(buffer);
		// id
		persist_read_string(key + 1, buffer, 30);
		stops->ids[i] = sdsnew(buffer);
	}

	return stops;
}

static void stops_clear(struct stops *stops) {
	for (unsigned int i = 0; i < stops->num; i++) {
		sdsfree(stops->names[i]);
		sdsfree(stops->ids[i]);
	}
	free(stops->names);
	stops->names = NULL;
	free(stops->ids);
	stops->ids = NULL;
	if (stops->distances) {
		free(stops->distances);
		stops->distances = NULL;
	}
}

void stops_destroy(struct stops *stops) {
	stops_clear(stops);
	free(stops);
}

/* Saves the given number of stops in persistant memory. */
void stops_set_favorites_num(int num) {
	if (persist_exists(PERSIST_STOPS_LENGTH)) {
		int prevnum = persist_read_int(PERSIST_STOPS_LENGTH);
		// Remove obsolete entries.
		for (int i = prevnum; i > num; i--) {
			persist_delete(PERSIST_STOPS_START + i * 2);
			persist_delete(PERSIST_STOPS_START + i * 2 + 1);
		}
	}
	persist_write_int(PERSIST_STOPS_LENGTH, num);
}

/* Sets the number of proximity stops in memory. */
void stops_set_proximity_num(int num) {
	if (proximity_stops.names)
		stops_clear(&proximity_stops);

	proximity_stops.num = num;

	if (num > 0) {
		// Allocate arrays.
		proximity_stops.names = (sds*)malloc(num * sizeof(sds));
		proximity_stops.ids = (sds*)malloc(num * sizeof(sds));
		proximity_stops.distances = (int*)malloc(num * sizeof(int));
	}
}

void add_proximity_stop(int i, sds name, sds id, int distance) {
	proximity_stops.names[i] = sdsdup(name);
	proximity_stops.ids[i] = sdsdup(id);
	proximity_stops.distances[i] = distance;
}

/* Handles a stop message. */
void stops_receive_stop(DictionaryIterator *iter) {
	int index = dict_find(iter, MSG_KEY_INDEX)->value->int32;
	sds name = sdsnew(dict_find(iter, MSG_KEY_STOPNAME)->value->cstring);
	sds id = sdsnew(dict_find(iter, MSG_KEY_STOPID)->value->cstring);
	int distance;

	int type = dict_find(iter, MSG_KEY_TYPE)->value->int32;
	switch (type) {
		case MSG_TYPE_FAVORITES:
			add_favorite_stop(index, name, id);
			break;
		case MSG_TYPE_PROXIMITY:
			distance = dict_find(iter, STOP_KEY_DISTANCE)->value->int32;
			add_proximity_stop(index, name, id, distance);
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid type %d", type);
	}

	sdsfree(name);
	sdsfree(id);
}
