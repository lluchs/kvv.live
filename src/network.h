/* Networking keys */

#pragma once

enum {
	// Universal message keys.
	MSG_KEY_LENGTH = 0,
	MSG_KEY_INDEX,
	MSG_KEY_STOPNAME,
	MSG_KEY_STOPID,
	MSG_KEY_ACTION,

	// Single departure.
	DEPARTURE_KEY_ROUTE,
	DEPARTURE_KEY_DESTINATION,
	DEPARTURE_KEY_TIME,
};

// Actions
enum {
	MSG_ACTION_RELOAD_STOPS = 0,
};

