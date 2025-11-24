#include <Arduino.h>
#include "aoicalc_func.h"
#include "interpolation_func.h"

void ephInputMode(){

	aoicalc_f();
	interpolation_f();

	Serial.println("Eph input move done.");
}	