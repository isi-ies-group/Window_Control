#include "stateMachine.h"
#include <Arduino.h>

State thisSt;
State nextSt;
Event event;

void initFSM() {
	thisSt = STDBY;
	nextSt = STDBY;
	Serial.println("Machine initialized. Current State: WAIT_CONFIG");	
}

void runMachine(){
	switch (thisSt){	
		case CONFIG:
			break;
		case STDBY:
			break;
		case MANUAL:
			break;
		case SLEEP:
			break;
		case AWAKENING:
			break;
		case INPUT:
			break;
		case AUTO_MODE:
			autoMode(lon, lat, pan, tilt, tilt_correction, matrix_X, matrix_Z);
			break;
	}

}

State fsmProcess(Events event, bool auto_running){
	if (event == begin_config) return CONFIG;
	if (event == begin_eph_input) return INPUT;
	if (event == begin_manual) return MANUAL;
	if (event == toggle_auto_mode) return AUTO_MODE;
	if (event == go_sleep) return SLEEP;
	if (event == wake_up) return AWAKENING;
	if (event == woke_up) 
		if (auto_running == true) return AUTO_MODE;
		else return STDBY;
}


void changeState(State newSt) {
    bool valid = true;
    State aux = thisSt;

    switch (thisSt) {
        case STDBY:
            if (newSt == AUTO_MODE || newSt == CONFIG || newSt == MANUAL || newSt == SLEEP)
                aux = newSt;
            else valid = false;
            break;
        case AUTO_MODE:
            if (newSt == STDBY || newSt == SLEEP)
                aux = newSt;
            else valid = false;
            break;
        case SLEEP:
            if (newSt == AWAKENING)
                aux = newSt;
            else valid = false;
            break;
        case AWAKENING:
            if (newSt == STDBY || newSt == AUTO_MODE)
                aux = newSt;
            else valid = false;
            break;
    }

    if (valid) thisSt = aux;
}
