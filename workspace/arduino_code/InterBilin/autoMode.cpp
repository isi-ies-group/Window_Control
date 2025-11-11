#include <Arduino.h>
#include <time.h>
#include "autoMode.h"
#include "sync.h"
#include "spaTask.h"
#include "aoicalcTask.h"
#include "interpolationTask.h"

void updateSPAInputsFromTime(struct tm *time_info, SPAInputs *spa) {
    spa->year   = time_info->tm_year + 1900;
    spa->month  = time_info->tm_mon + 1;
    spa->day    = time_info->tm_mday;
    spa->hour   = time_info->tm_hour;
    spa->minute = time_info->tm_min;
    spa->second = time_info->tm_sec;
}


void autoMode (const float (*matrix_X)[N], const float (*matrix_X)[N]){
	AutoHandle ah;				

	ah.sem_SPA_AOI = xSemaphoreCreateBinary();
	ah.sem_AOI_Inter = xSemaphoreCreateBinary();
	ah.sem_End = xSemaphoreCreateBinary();

	if (!ah.sem_SPA_AOI || !ah.sem_AOI_Inter) {
    Serial.println("Error: semaphores not created.");
    return;
  }
	time_t now;
	struct tm time_info;
	time(&now);
	localtime_r(&now, &time_info);


	updateSPAInputsFromTime(&time_info, &g_SPAInputs);


	xTaskCreate(
	SPATask,        // Function
	"SPATask",      // Name
	4096,           // Stack size
	&ah,   // Parameters
	1,              // Priority
	NULL            // Handle
	);

	Serial.print("spaTask created: \n");  
	

	xTaskCreate(
	aoicalcTask,     // Function
	"aoicalcTask",   // Name
	4096,            // Stack size
	&ah,    // Parameters
	1,               // Priority
	NULL             // Handle
	);
	Serial.print("aoicalcTask created: \n");  

	
	// Interpolation
	// AOIt (rows)
	// AOIl (columns)

	xTaskCreate(
	InterpolationTask,     // Function
	"interpolationTask",   // Name
	4096,            // Stack size
	&ah,    // Parameters
	1,               // Priority
	NULL             // Handle
	);
	Serial.print("interpolTask created: \n");

	xSemaphoreTake(ah.sem_End, portMAX_DELAY);

	vSemaphoreDelete(ah.sem_End);
	vSemaphoreDelete(ah.sem_SPA_AOI);
  vSemaphoreDelete(ah.sem_AOI_Inter);
	
	Serial.print("sems deleted: \n");

	Serial.println("autoMode successfully finished.");
}

