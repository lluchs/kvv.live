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

/* Window showing departures. */

#include <pebble.h>
#include "locale_framework/localize.h"
#include "departures_window.h"
#include "departure.h"
#include "settings.h"
#include "status_bar.h"
#include "color.h"

static Window *window;
static ScrollLayer *scroll_layer;

static char stopId[20];
static time_t request_time;

#define TITLE_LENGTH 30
static char title[TITLE_LENGTH];
static TextLayer *title_layer;

#ifdef PBL_PLATFORM_APLITE
// Unfortunately, aplite doesn't have enough RAM with PebbleOS 4 for 10 lines.
// Additionally, the tram and wheelchair graphics are disabled in twoline.c
#define DEPARTURE_LINES 9
#else
#define DEPARTURE_LINES 10
#endif
static struct DepartureLine *lines[DEPARTURE_LINES];
static struct Departure departures[DEPARTURE_LINES];

#ifdef PBL_COLOR
#define LINES_OFFSET_Y 1
#else
#define LINES_OFFSET_Y 0
#endif

/**
 * Request departures of the current stop.
 */
static void request_departures() {
	text_layer_set_text(title_layer, _("Loading..."));
	request_time = time(NULL);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	// Indirection to avoid "is always true" warning due to the
	// TupletCString macro.
	char *id = stopId;
	Tuplet value = TupletCString(MSG_KEY_STOPID, id);
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	request_departures(NULL);
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// Draws a white background for the scroll layer, overriding the red window.
static void draw_scroll_layer_bg(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	status_bar_adjust_window_bounds(&bounds);

	scroll_layer = scroll_layer_create((GRect) { .origin = { bounds.origin.x, bounds.origin.y + 23 },
	                                             .size   = { bounds.size.w, bounds.size.h - 23 } });
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks) { .click_config_provider = click_config_provider });
	IFCOLOR(scroll_layer_set_shadow_hidden(scroll_layer, true));
	layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

	title_layer = text_layer_create((GRect) { .origin = { bounds.origin.x + 3, bounds.origin.y - 1 },
	                                          .size   = { bounds.size.w - 6, 21 } });
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(title_layer));

	layer_add_child(window_layer, status_bar_layer());

	// Coloring
	IFCOLOR(text_layer_set_background_color(title_layer, COLOR_KVV));
	IFCOLOR(text_layer_set_text_color(title_layer, GColorWhite));
	IFCOLOR(window_set_background_color(window, COLOR_KVV));
	IFCOLOR(layer_set_update_proc(scroll_layer_get_layer(scroll_layer), draw_scroll_layer_bg));

	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_load");
}

static void window_unload(Window *window) {
	scroll_layer_destroy(scroll_layer);
	text_layer_destroy(title_layer);
	for (unsigned int i = 0; i < DEPARTURE_LINES && lines[i]; i++) {
		departure_line_destroy(lines[i]);
		lines[i] = NULL;
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload");
}

static void create_lines(int length) {
	if (length > DEPARTURE_LINES)
		length = DEPARTURE_LINES;
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	const int line_height = departure_height() + 1;
	int i;
	for (i = 0; i < length; i++) {
		// Lazily create departure lines.
		if (!lines[i]) {
			int ypos = LINES_OFFSET_Y + line_height * i;
			lines[i] = departure_line_create(&departures[i], (GRect) { .origin = { 0, ypos }, .size = { bounds.size.w, 20 } });
			APP_LOG(APP_LOG_LEVEL_DEBUG, "created line %d (%s)", i, departures[i].route);
		}
		scroll_layer_add_child(scroll_layer, lines[i]->layer);
		layer_mark_dirty(lines[i]->layer);
	}
	scroll_layer_set_content_size(scroll_layer, (GSize) { .h = 2 + LINES_OFFSET_Y + line_height * i, .w = bounds.size.w });

	// Remove remaining lines from the scroll layer.
	for (; i < DEPARTURE_LINES && lines[i]; i++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Removing line %d: %p %p", i, lines[i], lines[i]->layer);
		layer_remove_from_parent(lines[i]->layer);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Removal successful");
	}
}

static void window_appear(Window *window) {
	// Hide all existing lines.
	create_lines(0);
	request_departures();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_appear");
}

void departures_window_receive_announcement(DictionaryIterator *iter) {
	// The window might be closed by now.
	if (!window_stack_contains_window(window)) return;

	Tuple *length_tuple = dict_find(iter, MSG_KEY_LENGTH);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Received length %d", (int)length_tuple->value->int32);
	create_lines((int)length_tuple->value->int32);

	Tuple *name_tuple = dict_find(iter, MSG_KEY_STOPNAME);
	strncpy(title, name_tuple->value->cstring, TITLE_LENGTH);
	title[TITLE_LENGTH - 1] = '\0';
	text_layer_set_text(title_layer, title);

	// Notify the user that loading has finished.
	light_enable_interaction();
	// Also vibrate if loading took a long time. Make sure to vibrate only once.
	if (get_setting(SETTING_VIBRATE) && request_time && time(NULL) - request_time > 2)
		vibes_short_pulse();
	request_time = 0;

	// We're done, a length message does not contain anything else.
}

void departures_window_receive_departure(DictionaryIterator *iter) {
	// The window might be closed by now.
	if (!window_stack_contains_window(window)) return;

	Tuple *index_tuple = dict_find(iter, MSG_KEY_INDEX);
	// This is a destination object.
	int index = index_tuple->value->int32;

	// Make sure that the line does actually exist.
	// It might not if the user is switching stops very fast.
	if (index < DEPARTURE_LINES && lines[index]) {
		departure_deserialize(iter, &departures[index]);
		departure_line_update(lines[index]);
		// Initiate a redraw of the given departure.
		layer_mark_dirty(lines[index]->layer);
	}
}

void departures_window_handle_error() {
	if (title_layer)
		text_layer_set_text(title_layer, _("ERROR: No conn."));
}

void departures_window_init() {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.appear = window_appear,
		.unload = window_unload,
	});
}

void departures_window_show(char *nextStopId) {
	strncpy(stopId, nextStopId, 20);
	
	// Reset previous departures.
	memset(departures, 0, DEPARTURE_LINES * sizeof(struct Departure));

	const bool animated = true;
	window_stack_push(window, animated);
}

void departures_window_deinit() {
	window_destroy(window);
}
