#include <pebble.h>

#include "departure.h"
#include "network.h"

static Window *window;
static ScrollLayer *scroll_layer;

#define TITLE_LENGTH 30
static char title[TITLE_LENGTH];
static TextLayer *title_layer;

#define DEPARTURE_LINES 10
static struct DepartureLine *lines[DEPARTURE_LINES];
static struct Departure departures[DEPARTURE_LINES];

#define LINES_OFFSET_Y 23

/* Request departures of a stop. */
static void request_departures(const char *stopId) {
	text_layer_set_text(title_layer, "Loading...");

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	Tuplet value = TupletCString(DEPARTURES_REQUEST_KEY_STOPID, stopId);
	dict_write_tuplet(iter, &value);
	app_message_outbox_send();
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	scroll_layer = scroll_layer_create(bounds);
	scroll_layer_set_click_config_onto_window(scroll_layer, window);
	layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

	title_layer = text_layer_create((GRect) { .origin = { 3, 0 }, .size = { bounds.size.w - 6, 20 } });
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_layer));

	request_departures("de:8212:89");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_load");
}

static void window_unload(Window *window) {
	scroll_layer_destroy(scroll_layer);
	text_layer_destroy(title_layer);
	for (unsigned int i = 0; i < DEPARTURE_LINES && lines[i]; i++) {
		departure_line_destroy(lines[i]);
	}
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
	    layer_remove_from_parent(lines[i]->layer);
	}
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Received message");

    Tuple *length_tuple = dict_find(iter, DEPARTURES_KEY_LENGTH);
	if (length_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received length %d", (int)length_tuple->value->int32);
		create_lines((int)length_tuple->value->int32);

		Tuple *name_tuple = dict_find(iter, DEPARTURES_KEY_STOPNAME);
		strncpy(title, name_tuple->value->cstring, TITLE_LENGTH);
		title[TITLE_LENGTH - 1] = '\0';
		text_layer_set_text(title_layer, title);

		// We're done, a length message does not contain anything else.
		return;
	}

    Tuple *index_tuple = dict_find(iter, DEPARTURE_KEY_INDEX);
	if (index_tuple) {
		// This is a destination object.
		int index = index_tuple->value->int32;
		departure_deserialize(iter, &departures[index]);
		// Initiate a redraw of the given departure.
		layer_mark_dirty(lines[index]->layer);
	}
}

static void app_message_init() {
	app_message_register_inbox_received(in_received_handler);
	app_message_open(app_message_outbox_size_maximum(), APP_MESSAGE_INBOX_SIZE_MINIMUM);
}

static void init(void) {
	window = window_create();
	app_message_init();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
