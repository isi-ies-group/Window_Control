#include <Arduino.h>
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "movement_task.h"
#include "global_structs.h"
void ephInputMode(){

	aoicalc_f();
	interpolation_f();
	requestMove();
	Serial.println("Eph input move done.");
}	