#include <Arduino.h>
#include "aoicalc_func.h"
#include "interpolation_func.h"
#include "movement.h"
#include "global_structs.h"
void ephInputMode(){

	aoicalc_f();
	interpolation_f();
	move(g_x_val, g_z_val);
	Serial.println("Eph input move done.");
}	