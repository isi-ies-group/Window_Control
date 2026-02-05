#include <TinyGPSPlus.h>
#include <time.h>
#include "gps.h"
#include "global_structs.h"

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;
HardwareSerial hs(2);

void gpsInit() {
    hs.begin(9600, SERIAL_8N1, RXD2, TXD2);
    manual_time = false;
    //hs.begin(9600, SERIAL_8N1);
    hs.setTimeout(10);
    Serial.println("Waiting for GPS time (UTC)...");
    Serial.println();
}


void debugGPS() {
    if (gps.charsProcessed() < 10) {
        Serial.println("No NMEA data, check wiring");
        return;
    }

    if (gps.date.isValid() && gps.time.isValid()) {
        Serial.printf("Date: %04d-%02d-%02d  Time: %02d:%02d:%02d UTC\n",
            gps.date.year(), gps.date.month(), gps.date.day(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
        Serial.println("Time not valid yet");
    }
}


void setSystemTimeFromGPS() {

    struct tm t;

    if (!gps.date.isValid() || !gps.time.isValid()) {
        Serial.println("[GPS] Invalid date/time, skipping system time set");
        return;
    }

    if (gps.date.year() < 2020) {
        Serial.println("[GPS] Year not valid yet");
        return;
    }

    t.tm_year = gps.date.year() - 1900;
    t.tm_mon  = gps.date.month() - 1;
    t.tm_mday = gps.date.day();
    t.tm_hour = gps.time.hour(); // UTC
    t.tm_min  = gps.time.minute();
    t.tm_sec  = gps.time.second();
    t.tm_isdst = 0;


    setenv("TZ", "UTC0", 1);
    tzset();

    time_t epoch = mktime(&t);

 
    struct timeval now = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&now, NULL);

    Serial.println("System time set from GPS.");
}


void setLocalTime() {
    if (manual_time == true) {
        Serial.println("System using manual date-time");
        return;
    }
    static unsigned long lastPrint = 0;
    unsigned long start = millis();
    bool sync = false;

    while (millis() - start < 5000 && !sync) {
        while (hs.available()) gps.encode(hs.read());
        
        if (millis() - lastPrint > 2000) {
            debugGPS();
            lastPrint = millis();
        }

        if (gps.time.isValid() && gps.date.isValid() && gps.date.year() > 2020) {
            setSystemTimeFromGPS();
            sync = true;
        }
        delay(1000);
    }

    if (!sync) {
        Serial.println("Failed to get GPS time. Using default system time.");
        return;
    }

    struct TZEntry { const char* country; const char* tz; };
    static const TZEntry tzTable[] = {
        {"Spain", "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00"},
        {"Spain_Canary", "WET0WEST,M3.5.0/01:00,M10.5.0/02:00"},
        {"UK", "GMT0BST,M3.5.0/01:00,M10.5.0/02:00"},
        {"Poland", "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00"},
        {"Argentina", "ART3"}
    };

    const char* tz = "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00";
    for (auto& entry : tzTable) {
        if (g_country.equalsIgnoreCase(entry.country)) {
            tz = entry.tz;
            break;
        }
    }

    if (!tz) {
        tz = "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00";  // España peninsular por defecto
        Serial.println("Country not found in tzTable, using default (Spain).");
    }

    Serial.print("Country: ");
    Serial.println(g_country);
    Serial.print("Time zone string: ");
    Serial.println(tz);
    setenv("TZ", tz, 1);
    tzset();
    printLocalTime();
    Serial.printf("Timezone set for %s: %s\n", g_country.c_str(), tz);
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

void setSystemTimeManualLocal(int year, int month, int day,
                              int hour, int min, int sec) {

    manual_time = true;

    setenv("TZ", "UTC0", 1);
    tzset();

    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    t.tm_isdst = 0;

    time_t epoch = mktime(&t);
    struct timeval now = {
        .tv_sec = epoch,
        .tv_usec = 0
    };

    settimeofday(&now, NULL);

    Serial.println("[TIME] Manual time set EXACT (no TZ)");
}
