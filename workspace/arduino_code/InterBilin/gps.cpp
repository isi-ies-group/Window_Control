#include <TinyGPSPlus.h>
#include <time.h>
#include "gps.h"
#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;
HardwareSerial hs(2);

void gpsInit(){
	hs.begin(9600, SERIAL_8N1, RXD2, TXD2);
	Serial.println("Waiting for GPS time (UTC)...");
	Serial.println();	

}



void setSystemTimeFromGPS() {
    struct tm t;
    t.tm_year = gps.date.year() - 1900;
    t.tm_mon  = gps.date.month() - 1;
    t.tm_mday = gps.date.day();
    t.tm_hour = gps.time.hour(); // UTC
    t.tm_min  = gps.time.minute();
    t.tm_sec  = gps.time.second();
    t.tm_isdst = 0;

    //tm to epoch
    setenv("TZ", "UTC0", 1); // UTC conversion
    tzset();
    time_t epoch = mktime(&t);

    //time to system RTC
    struct timeval now = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&now, NULL);
}

void setLocalTime(){
	unsigned long start = millis();
	bool sync = false;

	while (millis() - start < 300000 && !sync) {
	while (hs.available()) 
	gps.encode(hs.read());
	if (gps.time.isValid() && gps.date.isValid()) {
	setSystemTimeFromGPS();
	sync = true;
	}
	}

	if (sync) 
	Serial.println("System time set from GPS.");
	else 
	Serial.println("Failed to get GPS time.");

	// Spain timezone (CET/CEST)
	setenv("TZ", "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00", 1); //list of timezones can be added 
	tzset();
}
	

void printLocalTime() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    Serial.printf("Local time: %04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}