#include <Arduino.h>
#include <time.h>
#include "autoMode.h"
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "spa_func.h"
#include "matrices.h"
#include "movement.h"

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

void autoMode (){
    time_t now;
	struct tm time_info;
	time(&now);
	localtime_r(&now, &time_info);

	updateSPAInputsFromTime(&time_info, &g_SPAInputs);
    printLocalTime(&time_info);

    SPA_f();
    aoicalc_f();
    interpolation_f();
    move(g_x_val, g_z_val);
    auto_counter++;
    if (auto_counter % 60 == 0){
        antiBacklashZ(5, 40, 750); 
    }
        auto_counter++;
    if (auto_counter == 299){
        GoHomePair(g_x_val,g_z_val); 
        auto_counter = 0; 
    }
	Serial.println("autoMode successfully finished.");
}

