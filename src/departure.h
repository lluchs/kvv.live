/* Departure struct. */

#pragma once

#include <pebble.h>

struct Departure {
	char *route;
	char *destination;
	char *direction;
	char *time;
	bool lowfloor;
	bool realtime;
	int traction;
};

struct DepartureLine {
	Layer *layer;
	TextLayer *route;
	TextLayer *destination;
	TextLayer *time;
};

struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame);

void departure_line_destroy(struct DepartureLine *line);
