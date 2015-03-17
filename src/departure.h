/* Departure struct. */

#pragma once

#include <pebble.h>

#include "network.h"

#define DEPARTURE_HEIGHT 19

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

void departure_line_update(struct DepartureLine *line);
void departure_line_destroy(struct DepartureLine *line);
