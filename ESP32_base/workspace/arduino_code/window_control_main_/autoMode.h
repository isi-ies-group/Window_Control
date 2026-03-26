#ifndef autoMode_h
#define autoMode_h

#include "global_structs.h"
void startDaySimulation(time_t sunrise_epoch);
void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa);
void autoMode();
				
#endif	