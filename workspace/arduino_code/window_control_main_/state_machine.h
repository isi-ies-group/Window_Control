#ifndef state_machine_h
#define state_machine_h

#include <Arduino.h>

typedef enum {
	CONFIG,
	STDBY,
	AUTO_MODE,
	SLEEP,
	MANUAL,
	EPH_INPUT,
	AWAKENING,
}States;

typedef enum{
	begin_config,
	end_config,
	end_eph_input,
	begin_eph_input,
	begin_manual,
	end_manual,
	toggle_auto_mode,
	go_sleep,
	wake_up,
	woke_up,
}Events;

extern States thisSt;
extern States nextSt;
extern Events event;
extern bool auto_on;
String stateToText(States s);
void runMachine();
States fsmProcess(Events event, bool auto_running);
void initFSM();
void changeState(States newState);


#endif