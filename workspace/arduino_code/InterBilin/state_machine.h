#ifndef state_machine_h
#define state_machine_h

#include <Arduino.h>

enum State {
	WAIT_CONFIG,
	STDBY;
	CONFIG,
	AUTO_MODE,
	SLEEP,
	MANUAL,
	INPUT,
	WAKE_UP,
}

extern State thisSt;
extern State nextSt;

void saveState();
void loadState();
void runMachine();
void initMachine();
void changeState(State newState);


#endif