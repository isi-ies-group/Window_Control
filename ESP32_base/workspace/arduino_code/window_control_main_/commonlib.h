#ifndef commonlib_h
#define commonlib_h

#ifdef __cplusplus
extern "C" {
#endif
#include "Arduino.h"
#include "time.h"

#define PI 3.1415926535897932384626433832795028841971

double rad2deg(double radians);
double deg2rad(double radians);

extern RTC_DATA_ATTR time_t rtc_sunrise_epoch;
extern RTC_DATA_ATTR time_t rtc_sunset_epoch;
extern RTC_DATA_ATTR bool   rtc_auto_mode_on;

#ifdef __cplusplus
}
#endif

#endif
