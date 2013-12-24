#include <pebble.h>
#include <departure.h>

static Window *window;

#define DEPARTURE_LINES 5
static struct DepartureLine *lines[DEPARTURE_LINES];
static struct Departure departures[DEPARTURE_LINES];

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// Render lines.
	const int line_height = DEPARTURE_HEIGHT + 1;
	for (unsigned int i = 0; i < DEPARTURE_LINES; i++) {
		int ypos = 3 + line_height * i;
		lines[i] = departure_line_create(&departures[i], (GRect) { .origin = { 0, ypos }, .size = { bounds.size.w, 20 } });
		layer_add_child(window_layer, lines[i]->layer);
	}
}

static void window_unload(Window *window) {
	for (unsigned int i = 0; i < DEPARTURE_LINES; i++) {
		departure_line_destroy(lines[i]);
	}
}

static void init(void) {
	// Initialize test departures.
	departures[0].route       = "S4";
	departures[0].destination = "Tullastrasse";
	departures[0].time        = "3min";
	departures[1].route       = "S41";
	departures[1].destination = "Freudenstadt Hbf";
	departures[1].time        = "12:25";
	departures[2].route       = "S41";
	departures[2].destination = "Tullastrasse";
	departures[2].time        = "12:35";
	departures[3].route       = "S4";
	departures[3].destination = "Achern";
	departures[3].time        = "12:45";
	departures[4].route       = "S4";
	departures[4].destination = "Tullastrasse";
	departures[4].time        = "13:15";

	window = window_create();
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
