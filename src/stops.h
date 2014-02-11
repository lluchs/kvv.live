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
};

struct stops* read_stops();
void stops_destroy(struct stops *stops);
