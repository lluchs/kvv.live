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
#include <ctype.h>
#include "route_color.h"

#ifdef PBL_COLOR

struct route_color get_color_for_route(const char *route) {
	struct route_color color = {.bg = GColorBlack, .fg = GColorWhite};
	// Check for bus lines.
	if (isdigit((unsigned char)route[0]) && isdigit((unsigned char)route[1])) {
		color.bg = GColorPurple;
		return color;
	}
	// Nightliners have "NL" prefix.
	if (route[0] == 'N' && route[1] == 'L')
		route += 2;
	// Some kind of train.
	switch (route[0]) {
		case '1': color.bg = GColorRed; break;
		case '2': color.bg = GColorCobaltBlue; break;
		case '3': color.bg = GColorWindsorTan; break;
		case '4': color.bg = GColorYellow;
				  color.fg = GColorBlack; break;
		case '5': color.bg = GColorVividCerulean; break;
		case '6': color.bg = GColorSpringBud; break;
		case '8': color.bg = GColorChromeYellow;
				  color.fg = GColorBlack; break;
		case 'S':
		  switch (route[1]) {
			  case '1': color.bg = GColorJaegerGreen; break;
			  case '2': color.bg = GColorLavenderIndigo; break;
			  case '3': color.bg = GColorTiffanyBlue; break;
			  case '4': color.bg = GColorJazzberryJam; break;
			  case '5':
			  case '9': color.bg = GColorMelon;
						color.fg = GColorBlack; break;
			  case '6': color.bg = GColorOxfordBlue; break;
		  }
	}
	return color;
}

#else

struct route_color get_color_for_route(const char *route) {
	return (struct route_color) {.bg = GColorBlack, .fg = GColorWhite};
}

#endif
