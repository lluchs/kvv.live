/* Shows a menu for selecting a stop. */

#include <pebble.h>

#include "sds/sds.h"

#include "stops_window.h"
#include "departures_window.h"
#include "stops.h"

static Window *window;
static SimpleMenuLayer *menu;

static struct stops *stops;
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
	char *stop_id = stops->ids[index];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for station %s.", stop_id);
	departures_window_show(stop_id);
}

void stops_window_init() {
	stops = read_stops();

	menu_entries = (SimpleMenuItem*)calloc(stops->num, sizeof(SimpleMenuItem));
	menu_section.num_items = stops->num;
	menu_section.items = menu_entries;

	for (unsigned int i = 0; i < menu_section.num_items; i++) {
		menu_entries[i].title = stops->names[i];
		menu_entries[i].callback = show_departures;
	}

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

void stops_window_deinit() {
	stops_destroy(stops);
	free(menu_entries);

	window_destroy(window);
}

void stops_window_reload() {
	stops_window_deinit();
	window_stack_pop_all(true);
	stops_window_init();
}
