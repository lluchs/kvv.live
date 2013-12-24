#include <pebble.h>
#include <departure.h>

static Window *window;

static struct DepartureLine *line;
static struct Departure departure;

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	departure.route = "S41";
	departure.destination = "Tullastrasse";
	departure.time = "12:35";

	line = departure_line_create(&departure, (GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
	layer_add_child(window_layer, line->layer);
}

static void window_unload(Window *window) {
	departure_line_destroy(line);
}

static void init(void) {
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
