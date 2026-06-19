#include <cmath>
#include <time.h>
#include "autoMode.h"
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "spa_func.h"
#include "matrices.h"
#include "gps.h"
#include "movement_task.h"


#define AUTOMODE_MIN_MOVE_MM 1.0f


static bool autoModeTargetNeedsMove(float current_x, float current_z,
                                    float target_x, float target_z)
{
    /*
     * What: decide if an automatic target is large enough to move the mechanics.
     * How: accepts the target only when X or Z changes at least 1 mm from the last accepted position.
     * Why: sub-millimetre automatic corrections create accumulated step error without useful panel motion.
     */
    if ((!std::isfinite(target_x)) || (!std::isfinite(target_z)))
    {
        return false;
    }

    return (std::fabs(target_x - current_x) >= AUTOMODE_MIN_MOVE_MM) ||
           (std::fabs(target_z - current_z) >= AUTOMODE_MIN_MOVE_MM);
}




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
     * How: reads the same local RTC shown in the web status, calculates SPA/AOI/XZ, then queues movement only above 1 mm.
     * Why: automatic mode must follow the configured local time source without blocking the FSM task.
     */
    struct tm time_info = {0};
    RTC_GetToTM(&time_info);

    updateSPAInputsFromTime(&time_info, &g_SPAInputs);

    SPA_f();
    aoicalc_f();
    interpolation_f();

    if (autoModeTargetNeedsMove(g_x_val, g_z_val, g_x_target, g_z_target))
    {
        requestMove();
    }

}

