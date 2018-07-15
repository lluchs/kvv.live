// Copyright © 2013-2015 Lukas Werling
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/* Networking keys */

#pragma once

enum {
	// Universal message keys.
	MSG_KEY_LENGTH = 0,
	MSG_KEY_INDEX,
	MSG_KEY_STOPNAME,
	MSG_KEY_STOPDIR,
	MSG_KEY_ACTION,
	MSG_KEY_TYPE,
	MSG_KEY_ERROR,

	// Single departure.
	DEPARTURE_KEY_ROUTE,
	DEPARTURE_KEY_DESTINATION,
	DEPARTURE_KEY_TIME,
	DEPARTURE_KEY_REALTIME,
	DEPARTURE_KEY_PLATFORM,
	DEPARTURE_KEY_COLOR_FG,
	DEPARTURE_KEY_COLOR_BG,

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

