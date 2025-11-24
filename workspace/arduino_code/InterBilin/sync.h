#ifndef sync_h
#define sync_h

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "global_def.h"

typedef struct {
  SemaphoreHandle_t sem_SPA_AOI;
  SemaphoreHandle_t sem_AOI_Inter;
	SemaphoreHandle_t sem_End;
	SemaphoreHandle_t sem_AOI_Motors;

  const float (*matrix_X)[N];
  const float (*matrix_Z)[N];
}AutoHandle;

#endif
