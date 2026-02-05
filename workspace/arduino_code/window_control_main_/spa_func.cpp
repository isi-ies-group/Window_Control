#include <Arduino.h>
#include "spa_func.h"
#include "spa.h"
#include <time.h>
#include "global_structs.h"

extern AOIInputs g_AOIInputs;

// int getTimezone(int year, int month, int day) {
// 	int lastSundayMarch = 31;
// 	while (true) {
// 		tm t = {};
// 		t.tm_year = year - 1900;
// 		t.tm_mon = 2;
// 		t.tm_mday = lastSundayMarch;
// 		mktime(&t);
// 		if (t.tm_wday == 0) break;
// 		lastSundayMarch--;
// 	}


// 	int lastSundayOctober = 31;
// 	while (true) {
// 		tm t = {};
// 		t.tm_year = year - 1900;
// 		t.tm_mon = 9;
// 		t.tm_mday = lastSundayOctober;
// 		mktime(&t);
// 		if (t.tm_wday == 0) break;
// 		lastSundayOctober--;
// 	}

// 	if ((month > 3 && month < 10) ||
// 			(month == 3 && day >= lastSundayMarch) ||
// 			(month == 10 && day < lastSundayOctober)) {
// 		return 2;
// 	} 
// 	else
// 		return 1;
// }
int getTimezoneForCountry(const String& country, int year, int month, int day) {

    // --- Base offset--
    int baseOffset = 0;
    bool hasDST = false;

    if (country.equalsIgnoreCase("Spain")) {
        baseOffset = 1; hasDST = true;
    }
    else if (country.equalsIgnoreCase("Spain_Canary")) {
        baseOffset = 0; hasDST = true;
    }
    else if (country.equalsIgnoreCase("UK")) {
        baseOffset = 0; hasDST = true;
    }
    else if (country.equalsIgnoreCase("Poland")) {
        baseOffset = 1; hasDST = true;
    }
    else if (country.equalsIgnoreCase("Argentina")) {
        baseOffset = -3; hasDST = false;
    }
    else {
        baseOffset = 0; hasDST = false;
    }

 
    if (hasDST) {
        int lastSundayMarch = 31;
        while (true) {
            struct tm t = {};
            t.tm_year = year - 1900;
            t.tm_mon = 2; 
            t.tm_mday = lastSundayMarch;
            mktime(&t);
            if (t.tm_wday == 0) break;
            lastSundayMarch--;
        }

        int lastSundayOctober = 31;
        while (true) {
            struct tm t = {};
            t.tm_year = year - 1900;
            t.tm_mon = 9;
            t.tm_mday = lastSundayOctober;
            mktime(&t);
            if (t.tm_wday == 0) break;
            lastSundayOctober--;
        }

        bool isDST = false;
        if ((month > 3 && month < 10) ||
            (month == 3 && day >= lastSundayMarch) ||
            (month == 10 && day < lastSundayOctober)) {
            isDST = true;
        }

        if (isDST) baseOffset += 1;
    }

    return baseOffset;
}

void printTimeDecimal(double time) {
  int h = (int)time;
  double min = 60.0 * (time - h);
  double sec = 60.0 * (min - (int)min);

  if (h < 10) Serial.print('0');
  Serial.print(h);
  Serial.print(':');

  if ((int)min < 10) Serial.print('0');
  Serial.print((int)min);
  Serial.print(':');

  if ((int)sec < 10) Serial.print('0');
  Serial.println((int)sec);
}
void SPA_f() {

    spa_data spa;

    spa.year   = g_SPAInputs.year;
    spa.month  = g_SPAInputs.month;
    spa.day    = g_SPAInputs.day;
    spa.hour   = g_SPAInputs.hour;
    spa.minute = g_SPAInputs.minute;
    spa.second = g_SPAInputs.second;

    spa.latitude  = g_SPAInputs.latitude;
    spa.longitude = g_SPAInputs.longitude;

    spa.timezone = getTimezoneForCountry(
        g_country,
        spa.year,
        spa.month,
        spa.day
    );

    spa.delta_ut1     = 0;
    spa.delta_t       = 67;
    spa.elevation     = 670;
    spa.pressure      = 820;
    spa.temperature   = 20;
    spa.slope         = 30;
    spa.azm_rotation  = -10;
    spa.atmos_refract = 0.5667;
    spa.function      = SPA_ZA_RTS;

    Serial.printf(
        "[SPA INPUT] %04d-%02d-%02d %02d:%02d:%02d  tz=%d\n",
        spa.year, spa.month, spa.day,
        spa.hour, spa.minute, spa.second,
        spa.timezone
    );

    int result = spa_calculate(&spa);

    if (result != 0) {
        Serial.print("SPA Error Code: ");
        Serial.println(result);
        return;
    }

    g_AOIInputs.azimuth   = spa.azimuth;
    g_AOIInputs.elevation = spa.e;

    Serial.print("Azimuth: ");
    Serial.println(spa.azimuth, 6);
    Serial.print("Elevation: ");
    Serial.println(spa.e, 6);

    if (spa.sunrise > 0 && spa.sunset > 0) {

        struct tm t;
        time_t now;
        time(&now);
        localtime_r(&now, &t);

        t.tm_hour = 0;
        t.tm_min  = 0;
        t.tm_sec  = 0;

        time_t midnight = mktime(&t);

        time_t sunrise = midnight + (time_t)(spa.sunrise * 3600.0);
        time_t sunset  = midnight + (time_t)(spa.sunset  * 3600.0);

        if (sunrise <= now) sunrise += 24 * 3600;
        if (sunset  <= now) sunset  += 24 * 3600;

        g_sunrise_epoch = sunrise;
        g_sunset_epoch  = sunset;

        Serial.printf("Sunrise epoch: %ld\n", g_sunrise_epoch);
        Serial.printf("Sunset  epoch: %ld\n", g_sunset_epoch);
    }
}
