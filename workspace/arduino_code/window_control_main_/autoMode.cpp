#include <Arduino.h>
#include <time.h>
#include "autoMode.h"
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "spa_func.h"
#include "matrices.h"
#include "movement_task.h"
#include "gps.h"

// ===== Simulated time =====
static time_t sim_time = 0;

// Factor de aceleración
#define TIME_ACCELERATION 100

// Cada cuánto recalculamos SPA (ms)
#define AUTO_INTERVAL_MS 1000

void startDaySimulation(time_t sunrise_epoch) {
    sim_time = sunrise_epoch;
    use_simulated_time = true;

    struct timeval now = {
        .tv_sec = sim_time,
        .tv_usec = 0
    };
    settimeofday(&now, NULL);

    Serial.println("[SIM] Day simulation started");
}


void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa) {
    spa->year   = time_info->tm_year + 1900;
    spa->month  = time_info->tm_mon + 1;
    spa->day    = time_info->tm_mday;
    spa->hour   = time_info->tm_hour;
    spa->minute = time_info->tm_min;
    spa->second = time_info->tm_sec;
}

void printLocalTime(struct tm *time_info) {
    Serial.printf("Local time: %04d-%02d-%02d %02d:%02d:%02d\n",
        time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday,
        time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
}
void autoMode() {
    // ---- TEST-AUTO -----
    // static unsigned long last_auto = 0;
    // unsigned long now_ms = millis();

    // if (now_ms - last_auto < AUTO_INTERVAL_MS) return;
    // last_auto = now_ms;

    // if (use_simulated_time) {
    //     sim_time += (time_t)(AUTO_INTERVAL_MS / 1000.0 * TIME_ACCELERATION);

    //     struct timeval tv = {
    //         .tv_sec = sim_time,
    //         .tv_usec = 0
    //     };
    //     settimeofday(&tv, NULL);
    // }

    time_t now;
    struct tm time_info;
    time(&now);
    localtime_r(&now, &time_info);

    updateSPAInputsFromTime(&time_info, &g_SPAInputs);
    SPA_f();
    aoicalc_f();
    interpolation_f();
    requestMove();

    auto_counter++;

    if (auto_counter % 34 == 0) {
        requestAntiBacklash();
    }

    if (auto_counter == 300) {
        requestHome();
        auto_counter = 0;
    }
}


