#ifndef autoMode_h
#define autoMode_h

#include "global_structs.h"

#define N 86

void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa);
void autoMode (double lon, double lat, double pan, double tilt, bool tilt_correction,
				const float (*matrix_X)[N], const float (*matrix_Z)[N]);
				
#endif	