#include <pebble.h>
#include "network.h"
#include "departures_window.h"
#include "stops_window.h"
#include "stops.h"

static void in_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Received message");

	// Length announcements.
	if (dict_find(iter, MSG_KEY_LENGTH)) {
		if (dict_find(iter, MSG_KEY_STOPNAME)) {
			departures_window_receive_announcement(iter);
		} else {
			int num = dict_find(iter, MSG_KEY_LENGTH)->value->int32;
			int type = dict_find(iter, MSG_KEY_TYPE)->value->int32;
			switch (type) {
				case MSG_TYPE_FAVORITES:
					stops_set_favorites_num(num);
					break;
				case MSG_TYPE_PROXIMITY:
					stops_set_proximity_num(num);
					break;
				default:
					APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid type %d", type);
			}
		}
		return;
	}

	// Content.
	if (dict_find(iter, MSG_KEY_INDEX)) {
		if (dict_find(iter, DEPARTURE_KEY_ROUTE)) {
			departures_window_receive_departure(iter);
		} else {
			stops_receive_stop(iter);
		}
		return;
	}

	// Actions.
	Tuple *tuple;
	if ((tuple = dict_find(iter, MSG_KEY_ACTION))) {
		int action = tuple->value->int32;
		switch (action) {
			case MSG_ACTION_RELOAD_FAVORITE_STOPS:
				stops_window_reload_favorite_stops();
				break;
			case MSG_ACTION_RELOAD_PROXIMITY_STOPS:
				stops_window_reload_proximity_stops();
				break;
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid action %d", action);
		}
		return;
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not send message");
	// Right now, only the departures window sends any messages.
	departures_window_handle_error();
}

static void app_message_init() {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_outbox_failed(out_failed_handler);
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
