/* Shows a menu for selecting a stop. */

#include <pebble.h>

#include "stops_window.h"
#include "departures_window.h"

static Window *window;
static SimpleMenuLayer *menu;

#define MENU_ENTRIES 5
static char *stop_ids[MENU_ENTRIES];
static SimpleMenuItem menu_entries[MENU_ENTRIES];
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
	departures_window_show(stop_id);
}

void stops_window_init() {
	menu_section.items = menu_entries;
	menu_section.num_items = MENU_ENTRIES;

	menu_entries[0].title = "Durmersheim Nord";
	stop_ids[0] = "de:8216:35109";
	menu_entries[1].title = "Durlacher Tor";
	stop_ids[1] = "de:8212:3";
	menu_entries[2].title = "Albtalbahnhof";
	stop_ids[2] = "de:8212:1201";
	menu_entries[3].title = "ZKM";
	stop_ids[3] = "de:8212:65";
	menu_entries[4].title = "Ebertstraße";
	stop_ids[4] = "de:8212:91";

	for (int i = 0; i < MENU_ENTRIES; i++)
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
	window_destroy(window);
}
