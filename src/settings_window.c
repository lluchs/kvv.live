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

#include <pebble.h>
#include "locale_framework/localize.h"
#include "settings_window.h"
#include "settings.h"

static bool initialized = false;
static Window *window;
static MenuLayer *menu;

static const char* settings[] = {
	"Vibration"
};

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
void settings_window_init() {
	if (initialized) return;
	initialized = true;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void settings_window_show() {
	const bool animated = true;
	window_stack_push(window, animated);
}

void settings_window_deinit() {
	if (!initialized) return;
	initialized = false;
	window_destroy(window);
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
		case 0:
			return _SETTING_LAST - _SETTING_FIRST - 1;

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
			menu_cell_basic_header_draw(ctx, cell_layer, _("Settings"));
			break;
	}
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	char *state;
	switch (cell_index->section) {
		case 0:
			state = get_setting(_SETTING_FIRST + cell_index->row + 1) ? _("Enabled") : _("Disabled");
			menu_cell_basic_draw(ctx, cell_layer, settings[cell_index->row], state, NULL);
			break;
	}
}

// Here we capture when a user selects a menu item
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			toggle_setting(_SETTING_FIRST + cell_index->row + 1);
			menu_layer_reload_data(menu_layer);
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
