#include <Arduino.h>
#include "aoicalcTask.h"
#include "interpolationTask.h"
#include "sync.h"
void ephInputMode(){

	AutoHandle *ah = new AutoHandle;
	ah->sem_SPA_AOI = xSemaphoreCreateBinary();
	ah->sem_AOI_Inter = xSemaphoreCreateBinary();
	ah->sem_End = xSemaphoreCreateBinary();

	xSemaphoreGive(ah->sem_SPA_AOI);
	
	xTaskCreate(aoicalcTask, "aoicalcTask", 4096, ah, 1, NULL);
	xTaskCreate(InterpolationTask, "interpolationTask", 4096, ah, 1, NULL);
	
	xSemaphoreTake(ah->sem_End, portMAX_DELAY);
	
	vSemaphoreDelete(ah->sem_End);
	vSemaphoreDelete(ah->sem_SPA_AOI);
	vSemaphoreDelete(ah->sem_AOI_Inter);
	
	delete ah;

	Serial.println("Eph input move done.");
}	