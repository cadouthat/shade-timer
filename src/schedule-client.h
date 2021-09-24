#ifndef __SCHEDULE_CLIENT_H
#define __SCHEDULE_CLIENT_H

#include <stdint.h>

#include <ESP8266HTTPClient.h>

// Fetches the event schedule from a hardcoded URL and populates the provided
// array with event times, up to size.
// If there are fewer events than size, a value of kScheduleNoEvent will be
// inserted to truncate the schedule.
// Returns true on success, and false in the event that the schedule could not
// be fetched or parsed.
bool fetchSchedule(uint16_t* output_minutes, int size);
bool fetchSchedule(HTTPClient& http, uint16_t* output_minutes, int size);

#endif
