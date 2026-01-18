#ifndef MOVEMENT_TASK_H
#define MOVEMENT_TASK_H

#include <Arduino.h>

void initMovementTask();

void requestMove();
void requestHome();
void requestAntiBacklash();
void requestAdjustZ();

#endif
