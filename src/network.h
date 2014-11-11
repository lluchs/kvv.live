/* Networking keys */

#pragma once

enum {
	// Universal message keys.
	MSG_KEY_LENGTH = 0,
	MSG_KEY_INDEX,
	MSG_KEY_STOPNAME,
	MSG_KEY_STOPID,
	MSG_KEY_ACTION,
	MSG_KEY_TYPE,
	MSG_KEY_ERROR,

	// Single departure.
	DEPARTURE_KEY_ROUTE,
	DEPARTURE_KEY_DESTINATION,
	DEPARTURE_KEY_TIME,

	// Extra info for stops.
	STOP_KEY_DISTANCE,
};

// Actions
enum {
	MSG_ACTION_RELOAD_FAVORITE_STOPS = 0,
	MSG_ACTION_RELOAD_PROXIMITY_STOPS,
};

// Types
enum {
	MSG_TYPE_FAVORITES = 0,
	MSG_TYPE_PROXIMITY = 1,
};

