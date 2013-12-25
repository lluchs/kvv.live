#include <pebble.h>
#include "network.h"
#include "departures_window.h"
#include "stops_window.h"

static void in_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Received message");

	if (dict_find(iter, DEPARTURES_KEY_LENGTH)) {
		departures_window_receive_announcement(iter);
		return;
	}

	if (dict_find(iter, DEPARTURE_KEY_INDEX)) {
		departures_window_receive_departure(iter);
		return;
	}
}

static void app_message_init() {
	app_message_register_inbox_received(in_received_handler);
	app_message_open(app_message_outbox_size_maximum(), APP_MESSAGE_INBOX_SIZE_MINIMUM);
}

static void init(void) {
	app_message_init();
	stops_window_init();
	departures_window_init();
}

static void deinit(void) {
	stops_window_deinit();
	departures_window_deinit();
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

	app_event_loop();
	deinit();
}
