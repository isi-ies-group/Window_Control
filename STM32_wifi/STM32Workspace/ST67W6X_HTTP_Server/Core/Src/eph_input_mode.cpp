#include "eph_input_mode.h"
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "movement_task.h"
#include "global_structs.h"

void ephInputMode(void)
{
	/*
	 * What: run one ephemeris-input movement cycle from web azimuth/elevation.
	 * How: AOI uses g_AOIInputs.azimuth/elevation, interpolation updates g_x_val/g_z_val,
	 *      then movement_task receives the non-blocking move request.
	 * Why: ephemeris mode already receives sun angles, so it must not call SPA.
	 */
	aoicalc_f();
	interpolation_f();
	requestMove();
}	
