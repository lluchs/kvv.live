/* Departure struct. */

#pragma once

#include <pebble.h>

#define DEPARTURE_HEIGHT 19

enum {
	DEPARTURE_KEY_LENGTH = 0,
	DEPARTURE_KEY_STOPNAME,
	DEPARTURE_KEY_INDEX,
	DEPARTURE_KEY_ROUTE,
	DEPARTURE_KEY_DESTINATION,
	DEPARTURE_KEY_TIME,
};

struct Departure {
	char route[4];
	char destination[20];
	char time[10];
};

struct DepartureLine {
	Layer *layer;
	TextLayer *route;
	TextLayer *destination;
	TextLayer *time;
};

void departure_deserialize(DictionaryIterator *iter, struct Departure *departure);

struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame);

void departure_line_destroy(struct DepartureLine *line);
