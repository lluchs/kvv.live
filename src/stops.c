#include <pebble.h>
#include "stops.h"

/* Adds the given stop to the persistant memory. */
static void add_stop(int i, char *name, char *id) {
	int key = PERSIST_STOPS_START + i * 2;
	persist_write_string(key, name);
	persist_write_string(key + 1, id);
}

static void create_default_stops() {
	persist_write_int(PERSIST_STOPS_LENGTH, 5);
	add_stop(0, "Durmersheim Nord", "de:8216:35109");
	add_stop(1, "Durlacher Tor", "de:8212:3");
	add_stop(2, "Albtalbahnhof", "de:8212:1201");
	add_stop(3, "ZKM", "de:8212:65");
	add_stop(4, "Ebertstraße", "de:8212:91");
}

/* Reads stops from persistent memory. */
struct stops* read_stops() {
	if (!persist_exists(PERSIST_STOPS_LENGTH)) {
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
