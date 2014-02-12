#include <pebble.h>
#include "stops.h"
#include "network.h"

/* Adds the given stop to the persistant memory. */
static void add_stop(int i, char *name, char *id) {
	int key = PERSIST_STOPS_START + i * 2;
	persist_write_string(key, name);
	persist_write_string(key + 1, id);
}

static void create_default_stops() {
	stops_set_num(5);
	add_stop(0, "KA Hbf Vorplatz", "de:8212:89");
	add_stop(1, "KA Durlacher Tor", "de:8212:3");
	add_stop(2, "KA Marktplatz (Kaiserstr)", "de:8212:1");
	add_stop(3, "KA ZKM", "de:8212:65");
	add_stop(4, "KA Entenfang", "de:8212:51");
}

/* Reads stops from persistent memory. */
struct stops* read_stops() {
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

void stops_destroy(struct stops *stops) {
	for (unsigned int i = 0; i < stops->num; i++) {
		sdsfree(stops->names[i]);
		sdsfree(stops->ids[i]);
	}
	free(stops->names);
	free(stops->ids);
	free(stops);
}

/* Saves the given number of stops in persistant memory. */
void stops_set_num(int num) {
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

/* Handles a stop message. */
void stops_receive_stop(DictionaryIterator *iter) {
	int index = dict_find(iter, MSG_KEY_INDEX)->value->int32;
	sds name = sdsnew(dict_find(iter, MSG_KEY_STOPNAME)->value->cstring);
	sds id = sdsnew(dict_find(iter, MSG_KEY_STOPID)->value->cstring);

	add_stop(index, name, id);

	sdsfree(name);
	sdsfree(id);
}
