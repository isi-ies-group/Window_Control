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
	begin_eph_input,
	begin_manual,
	toggle_auto_mode,
	go_sleep,
	wake_up,
	woke_up,
}Events;

extern States thisSt;
extern States nextSt;
extern Events event;

void saveState();
void loadState();
void runMachine();
States fsmProcess();
void initMachine();
void changeState(States newState);


#endif