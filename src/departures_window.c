/* Window showing departures. */

#include <pebble.h>
#include "departures_window.h"
#include "departure.h"

static Window *window;
static ScrollLayer *scroll_layer;

static char stopId[20];

#define TITLE_LENGTH 30
static char title[TITLE_LENGTH];
static TextLayer *title_layer;

#define DEPARTURE_LINES 10
static struct DepartureLine *lines[DEPARTURE_LINES];
static struct Departure departures[DEPARTURE_LINES];

#define LINES_OFFSET_Y 23

/**
 * Request departures of the current stop.
 */
static void request_departures() {
	text_layer_set_text(title_layer, "Loading...");

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

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	scroll_layer = scroll_layer_create(bounds);
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks) { .click_config_provider = click_config_provider });
	layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

	title_layer = text_layer_create((GRect) { .origin = { 3, 0 }, .size = { bounds.size.w - 6, 21 } });
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_layer));

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
	const int line_height = DEPARTURE_HEIGHT + 1;
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
	if (lines[index]) {
		departure_deserialize(iter, &departures[index]);
		// Initiate a redraw of the given departure.
		layer_mark_dirty(lines[index]->layer);
	}
}

void departures_window_handle_error() {
	text_layer_set_text(title_layer, "ERROR: No conn.");
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
