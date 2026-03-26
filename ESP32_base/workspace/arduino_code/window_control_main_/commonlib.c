#include <math.h>
#include <Arduino.h>
#include "commonlib.h"

double rad2deg(double radians)
{
    return (180.0/PI)*radians;
}

double deg2rad(double degrees)
{
    return (PI/180.0)*degrees;
}
RTC_DATA_ATTR time_t rtc_sunrise_epoch = 0;
RTC_DATA_ATTR time_t rtc_sunset_epoch  = 0;
RTC_DATA_ATTR bool rtc_auto_mode_on = false;


