#include <Arduino.h>
#include <time.h>
#include "autoMode.h"
#include "sync.h"
#include "global_structs.h"
#include "spaTask.h"
#include "aoicalcTask.h"
#include "interpolationTask.h"

void autoMode (double lon, double lat, double pan, double tilt, bool tilt_correction,
				const float (*matrix_X)[N], const float (*matrix_Z)[N]){
					
	sem_SPA_AOI = xSemaphoreCreateBinary();
	sem_AOI_Inter = xSemaphoreCreateBinary();
	
	time_t now;
	struct tm time_info;
	time(&now);
	localtime_r(&now, &time_info);


	g_SPAInputs.year          = time_info.tm_year;
	g_SPAInputs.month         = time_info.tm_mon;
	g_SPAInputs.day           = time_info.tm_mday;
	g_SPAInputs.hour          = time_info.tm_hour;
	g_SPAInputs.minute        = time_info.tm_min;
	g_SPAInputs.second        = time_info.tm_sec;
	g_SPAInputs.longitude     = lon;
	g_SPAInputs.latitude      = lat;


	xTaskCreate(
	SPATask,        // Function
	"SPATask",      // Name
	4096,           // Stack size
	&g_SPAInputs,   // Parameters
	1,              // Priority
	NULL            // Handle
	);

	Serial.print("spaTask created: \n");  
	
	//Ephemerids to AOI
	g_AOIInputs.pan = pan;
	g_AOIInputs.tilt = tilt;
	g_AOIInputs.tilt_correction = tilt_correction;

	xTaskCreate(
	aoicalcTask,     // Function
	"aoicalcTask",   // Name
	4096,            // Stack size
	&g_AOIInputs,    // Parameters
	1,               // Priority
	NULL             // Handle
	);
	Serial.print("aoicalcTask created: \n");  

	// Interpolation
	g_InterpolInputs.matrix_X = matrix_X;
	g_InterpolInputs.matrix_Z = matrix_Z;
	// AOIt (rows)
	// AOIl (columns)

	xTaskCreate(
	InterpolationTask,     // Function
	"interpolationTask",   // Name
	4096,            // Stack size
	&g_InterpolInputs,    // Parameters
	1,               // Priority
	NULL             // Handle
	);
	Serial.print("interpolTask created: \n");

}
