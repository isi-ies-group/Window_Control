#ifndef sync_h
#define sync_h

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t sem_SPA_AOI;
extern SemaphoreHandle_t sem_AOI_Inter;

#endif
