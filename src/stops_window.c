/* Shows a menu for selecting a stop. */

#include <pebble.h>

#include "sds/sds.h"

#include "stops_window.h"
#include "departures_window.h"
#include "stops.h"

static Window *window;
static MenuLayer *menu;

static struct stops *stops;

static void init_menu_layer();

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	menu = menu_layer_create(bounds);
	init_menu_layer();

	layer_add_child(window_layer, menu_layer_get_layer(menu));
}

static void window_unload(Window *window) {
	menu_layer_destroy(menu);
}

static void show_departures(int index) {
	char *stop_id = stops->ids[index];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for station %s.", stop_id);
	departures_window_show(stop_id);
}

void stops_window_init() {
	stops = read_stops();

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

	window_destroy(window);
}

void stops_window_reload() {
	stops_destroy(stops);
	stops = read_stops();
	menu_layer_reload_data(menu);
}

/* Menu callbacks */

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0: // favorites
			return stops->num;

		default:
			return 0;
	}
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	// This is a define provided in pebble.h that you may use for the default height
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0:
			// Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "Favorites");
			break;
	}
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	// Determine which section we're going to draw in
	switch (cell_index->section) {
		case 0: // favorites
			menu_cell_basic_draw(ctx, cell_layer, stops->names[cell_index->row], NULL, NULL);
			break;
	}
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	// Use the row to specify which item will receive the select action
	switch (cell_index->section) {
		case 0: // favorites
			show_departures(cell_index->row);
			break;
	}

}

static void init_menu_layer() {
	// Set all the callbacks for the menu layer
	menu_layer_set_callbacks(menu, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
	});
	menu_layer_set_click_config_onto_window(menu, window);
}
