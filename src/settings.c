// Copyright © 2015 Lukas Werling
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

#include <pebble.h>
#include "settings.h"

// Default values / cache.
static bool settings[] = {
	false, // ignored
	true,  // SETTING_VIBRATE
};

bool get_setting(int which) {
	const int index = which - _SETTING_FIRST;
	if (persist_exists(which))
		settings[index] = persist_read_bool(which);
	return settings[index];
}

void set_setting(int which, bool to) {
	persist_write_bool(which, to);
	settings[which - _SETTING_FIRST] = to;
}
