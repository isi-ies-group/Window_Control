#include <time.h>
#include "autoMode.h"
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "spa_func.h"
#include "matrices.h"
#include "gps.h"
#include "movement_task.h"




void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa) {
    /*
     * What: copy local calendar time into the SPA input structure.
     * How: converts struct tm offsets into normal year/month/day/hour fields.
     * Why: SPA expects explicit local date/time values, not an RTC or epoch object.
     */
    spa->year   = time_info->tm_year + 1900;
    spa->month  = time_info->tm_mon + 1;
    spa->day    = time_info->tm_mday;
    spa->hour   = time_info->tm_hour;
    spa->minute = time_info->tm_min;
    spa->second = time_info->tm_sec;
}


void autoMode() {
    /*
     * What: run one automatic tracking cycle.
     * How: reads the same local RTC shown in the web status, calculates SPA/AOI/XZ, then queues movement.
     * Why: automatic mode must follow the configured local time source without blocking the FSM task.
     */
    struct tm time_info = {0};

    RTC_GetToTM(&time_info);

    updateSPAInputsFromTime(&time_info, &g_SPAInputs);

    SPA_f();
    aoicalc_f();
    interpolation_f();
    requestMove();

}

