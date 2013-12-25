
#pragma once

// Messaging
void departures_window_receive_announcement(DictionaryIterator *iter);
void departures_window_receive_departure(DictionaryIterator *iter);

// Initialization
void departures_window_init();
void departures_window_show(char *nextStopId);
void departures_window_deinit();
