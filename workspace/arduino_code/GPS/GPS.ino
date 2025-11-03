#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <time.h>

TinyGPSPlus gps;
HardwareSerial hs(2);
#define RXD2 16
#define TXD2 17


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

void printLocalTime() {
    time_t now;
    struct tm time_info;
    time(&now);
    localtime_r(&now, &timeinfo);

    Serial.printf("Local time: %04d-%02d-%02d %02d:%02d:%02d\n",
                    time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday,
                    time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
}

void setup() {
    Serial.begin(115200);
    hs.begin(9600, SERIAL_8N1, RXD2, TXD2);
    Serial.println("Waiting for GPS time (UTC)...");

    unsigned long start = millis();
    bool sync = false;

    while (millis() - start < 300000 && !sync) { //wait ti
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
    setenv("TZ", "CET-1CEST,M3.5.0/02:00,M10.5.0/03:00", 1);
    tzset();
    printLocalTime();
}

void loop() {
    delay(10000);
    printLocalTime();
    autoMode()
}

