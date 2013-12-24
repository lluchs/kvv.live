
#include "departure.h"

/**
 * Draws a departure line.
 */
struct DepartureLine* departure_line_create(const struct Departure *d, GRect frame) {
	// Allocate a departure.
	struct DepartureLine *line = (struct DepartureLine*)malloc(sizeof(struct DepartureLine));
	// Override any height.
	frame.size.h = DEPARTURE_HEIGHT;
	line->layer = layer_create(frame);

	// A departure is rendered as "route destination time".
	// route
	line->route = text_layer_create((GRect) { .origin = { 3, 0 }, .size = { 25, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->route, d->route);
	text_layer_set_text_alignment(line->route, GTextAlignmentCenter);
	text_layer_set_background_color(line->route, GColorBlack);
	text_layer_set_text_color(line->route, GColorWhite);

	// destination
	line->destination = text_layer_create((GRect) { .origin = { 30, 0 }, .size = { 70, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->destination, d->destination);

	// time
	line->time = text_layer_create((GRect) { .origin = { 105, 0 }, .size = { 39, DEPARTURE_HEIGHT } });
	text_layer_set_text(line->time, d->time);

	// Add to the frame.
	layer_add_child(line->layer, text_layer_get_layer(line->route));
	layer_add_child(line->layer, text_layer_get_layer(line->destination));
	layer_add_child(line->layer, text_layer_get_layer(line->time));

	return line;
}

/**
 * Destroys a departure line.
 */
void departure_line_destroy(struct DepartureLine *line) {
	text_layer_destroy(line->time);
	text_layer_destroy(line->destination);
	text_layer_destroy(line->route);
	layer_destroy(line->layer);
	free(line);
}

