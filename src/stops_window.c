/* Shows a menu for selecting a stop. */

#include <pebble.h>

#include "sds/sds.h"

#include "stops_window.h"
#include "departures_window.h"
#include "stops.h"
#include "network.h"

static Window *window;
static MenuLayer *menu;

static struct stops const *proximity_stops;
static struct stops *favorite_stops;

static sds proximity_status;

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

static void show_departures(char *stop_id) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for station %s.", stop_id);
	departures_window_show(stop_id);
}

/* Request stops from the phone. */
static void request_proxmity_stops() {
	proximity_status = sdscpy(proximity_status, "Loading...");
	// Clear existing stops to prevent invalid menu entries.
	stops_set_proximity_num(0);
	menu_layer_reload_data(menu);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	Tuplet value = TupletInteger(MSG_KEY_ACTION, MSG_ACTION_RELOAD_PROXIMITY_STOPS);
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
}

void stops_window_init() {
	proximity_stops = get_proximity_stops();
	favorite_stops = read_favorite_stops();

	proximity_status = sdsnew("Select to update");

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

void stops_window_deinit() {
	stops_set_proximity_num(0);
	stops_destroy(favorite_stops);
	sdsfree(proximity_status);

	window_destroy(window);
}

void stops_window_reload_favorite_stops() {
	stops_destroy(favorite_stops);
	favorite_stops = read_favorite_stops();
	menu_layer_reload_data(menu);
}

void stops_window_reload_proximity_stops() {
	static char time[10];
	APP_LOG(APP_LOG_LEVEL_DEBUG, "reload_proximity_stops");
	clock_copy_time_string(time, 10);
	proximity_status = sdscpy(proximity_status, "Updated at ");
	proximity_status = sdscatlen(proximity_status, time, 10);
	// The proximity stops are implicitly updated...
	menu_layer_reload_data(menu);
}

// Shows the given error as status below the Search button.
void show_proximity_error(char *error) {
	proximity_status = sdscpy(proximity_status, error);
	menu_layer_reload_data(menu);
}

/* Menu callbacks */

// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 2;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0: // proximity search
			return 1 + proximity_stops->num;
		case 1: // favorites
			return favorite_stops->num;

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
			menu_cell_basic_header_draw(ctx, cell_layer, "Proximity search");
			break;
		case 1:
			menu_cell_basic_header_draw(ctx, cell_layer, "Favorites");
			break;
	}
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	static char dist_buf[10];
	// Determine which section we're going to draw in
	switch (cell_index->section) {
		case 0: // proximity search
			if (cell_index->row == 0) {
				menu_cell_basic_draw(ctx, cell_layer, "Search", proximity_status, NULL);
			} else {
				snprintf(dist_buf, 10, "%dm", proximity_stops->distances[cell_index->row - 1]);
				menu_cell_basic_draw(ctx, cell_layer, proximity_stops->names[cell_index->row - 1], dist_buf, NULL);
			}
			break;
		case 1: // favorites
			menu_cell_basic_draw(ctx, cell_layer, favorite_stops->names[cell_index->row], NULL, NULL);
			break;
	}
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	// Use the row to specify which item will receive the select action
	switch (cell_index->section) {
		case 0: // proximity search
			if (cell_index->row == 0)
				request_proxmity_stops();
			else
				show_departures(proximity_stops->ids[cell_index->row - 1]);
			break;
		case 1: // favorites
			show_departures(favorite_stops->ids[cell_index->row]);
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
	// Select the first favorite station.
	menu_layer_set_selected_index(menu, (MenuIndex){.section = 1, .row = 0}, MenuRowAlignNone, false);
}
