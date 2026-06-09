#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  STDBY,
  AUTO_MODE,
  MANUAL,
  EPH_INPUT,
} States;

typedef enum
{
  end_eph_input,
  begin_eph_input,
  submit_eph_input,
  begin_manual,
  submit_manual_goto,
  end_manual,
  toggle_auto_mode,
} Events;

extern States thisSt;
extern States nextSt;

void initFSM(void);
const char *stateToText(States state);
States fsmProcess(Events event, bool auto_running);
void changeState(States newState);
bool fsmPostEvent(Events event);
States fsmGetState(void);

#ifdef __cplusplus
}
#endif

#endif /* STATE_MACHINE_H */
