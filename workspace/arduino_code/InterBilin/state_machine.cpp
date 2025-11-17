#include "state_machine.h"
#include "autoMode.h"
#include "global_structs.h"
#include "matrices.h"
#include <Arduino.h>

States thisSt;
States nextSt;
Events event;



void initFSM() {
	nextSt = thisSt;
	Serial.println("Machine initialized. Current State: STDBY");	
}

void runMachine(){
	switch (thisSt){	
		case CONFIG:
			Serial.println("[FSM]: CONFIG");
			break;
		case STDBY:
			Serial.println("[FSM]: STDBY");
			break;
		case MANUAL:
			Serial.println("[FSM]: MANUAL");
			break;
		case SLEEP:
			Serial.println("[FSM]: SLEEP");
			break;
		case AWAKENING:
			Serial.println("[FSM]: AWAKENING");
			break;
		case EPH_INPUT:
			Serial.println("[FSM]: EPH_INPUT");
			break;
		case AUTO_MODE:
			Serial.println("[FSM]: AUTO_MODE");
			autoMode();
			break;
	}
}

States fsmProcess(Events event, bool auto_running){
	if (event == begin_config) return CONFIG;
	if (event == end_config) return STDBY;
	if (event == end_eph_input) return STDBY;
	if (event == end_manual) return STDBY;
	if (event == begin_eph_input) return EPH_INPUT;
	if (event == begin_manual) return MANUAL;
	if (event == toggle_auto_mode && auto_running == false) return AUTO_MODE;
	if (event == toggle_auto_mode && auto_running == true) return STDBY;
	if (event == go_sleep) return SLEEP;
	if (event == wake_up) return AWAKENING;
	if (event == woke_up) 
		if (auto_running == true) return AUTO_MODE;
		else return STDBY;
}


void changeState(States newSt) {
    bool valid = true;

    switch (thisSt) {
			case STDBY:
				if (newSt == AUTO_MODE || newSt == CONFIG || newSt == MANUAL || newSt == SLEEP
						|| newSt == EPH_INPUT)
					nextSt = newSt;
				else valid = false;
				break;
			case CONFIG:
					nextSt = newSt;
				break;						
			case AUTO_MODE:
				if (newSt == STDBY || newSt == SLEEP)
					nextSt = newSt;
				else valid = false;
				break;
			case SLEEP:
				if (newSt == AWAKENING)
					nextSt = newSt;
				else valid = false;
				break;
			case AWAKENING:
				if (newSt == STDBY || newSt == AUTO_MODE)
					nextSt = newSt;
				else valid = false;
			break;
			case EPH_INPUT:
					nextSt = newSt;
			break;		
			case MANUAL:
					nextSt = newSt;
			break;
	}

    if (valid) thisSt = nextSt;
}
