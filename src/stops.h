/* Stops struct and helpers. */

#include "sds/sds.h"

enum {
	PERSIST_STOPS_LENGTH,
	PERSIST_STOPS_START
};

struct stops {
	unsigned int num;
	sds *names;
	sds *ids;
	// Only set for proximity search.
	int *distances;
};

// struct stops functions
struct stops const * get_proximity_stops();
struct stops* read_favorite_stops();
void stops_destroy(struct stops *stops);

// messaging
void stops_set_favorites_num(int num);
void stops_set_proximity_num(int num);
void stops_receive_stop(DictionaryIterator *iter);
