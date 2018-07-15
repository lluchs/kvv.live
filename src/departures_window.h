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


#pragma once

// Messaging
void departures_window_receive_announcement(DictionaryIterator *iter);
void departures_window_receive_departure(DictionaryIterator *iter);
void departures_window_handle_error();

// Initialization
void departures_window_init();
void departures_window_show(char *nextStopName, char *nextStopDir);
void departures_window_deinit();
