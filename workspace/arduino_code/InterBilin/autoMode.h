#ifndef autoMode_h
#define autoMode_h

#include "global_structs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define N 86


void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa);
void autoMode ();
				
#endif	