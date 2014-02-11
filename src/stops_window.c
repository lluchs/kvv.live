/* Shows a menu for selecting a stop. */

#include <pebble.h>

#include "sds/sds.h"

#include "stops_window.h"
#include "departures_window.h"

static Window *window;
static SimpleMenuLayer *menu;

static sds *stop_ids;
static SimpleMenuItem *menu_entries;
static SimpleMenuSection menu_section;

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	menu = simple_menu_layer_create(bounds, window, &menu_section, 1, NULL);
	layer_add_child(window_layer, simple_menu_layer_get_layer(menu));
}

static void window_unload(Window *window) {
	simple_menu_layer_destroy(menu);
}

static void show_departures(int index, void *context) {
	char *stop_id = stop_ids[index];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for station %s.", stop_id);
	departures_window_show(stop_id);
}

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
static void read_stops() {
	if (!persist_exists(PERSIST_STOPS_LENGTH)) {
		APP_LOG(APP_LOG_LEVEL_INFO, "No stops found, creating default ones...");
		create_default_stops();
	}

	char buffer[30];
	int length = persist_read_int(PERSIST_STOPS_LENGTH);

	// Allocate arrays.
	stop_ids = (sds*)malloc(length * sizeof(sds));
	menu_entries = (SimpleMenuItem*)calloc(length, sizeof(SimpleMenuItem));
	menu_section.num_items = length;
	menu_section.items = menu_entries;

	for (int i = 0, key; i < length; i++) {
		key = PERSIST_STOPS_START + i * 2;
		// We need both name and id.
		if (!persist_exists(key) || !persist_exists(key + 1)) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Missing stop entry in persistant memory.");
			// Initialize to empty strings and hope that nothing breaks...
			menu_entries[i].title = sdsnew("");
			stop_ids[i] = sdsnew("");
		}
		// Name
		persist_read_string(key, buffer, 30);
		menu_entries[i].title = sdsnew(buffer);
		// id
		persist_read_string(key + 1, buffer, 30);
		stop_ids[i] = sdsnew(buffer);
	}
}

void stops_window_init() {
	read_stops();

	for (unsigned int i = 0; i < menu_section.num_items; i++)
		menu_entries[i].callback = show_departures;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

void stops_window_deinit() {
	// Free all menu entries.
	for (unsigned int i = 0; i < menu_section.num_items; i++) {
		sdsfree((sds)menu_entries[i].title);
		sdsfree(stop_ids[i]);
	}
	free(menu_entries);
	free(stop_ids);

	window_destroy(window);
}
