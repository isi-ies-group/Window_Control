#include <Arduino.h>
#include "aoicalcTask.h"
#include "aoicalc.h"
#include "sync.h"
#include "interpolationTask.h"

extern InterpolInputs g_InterpolInputs;
extern AOIInputs g_AOIInputs;
extern SemaphoreHandle_t sem_AOI_Inter;
extern SemaphoreHandle_t sem_SPA_AOI;


void aoicalcTask(void *pvParameters) {
	AOIInputs *inputs = (AOIInputs *) pvParameters;
	
	AOI aoi_data;
	aoi_data.pan = inputs->pan;
	aoi_data.tilt = inputs->tilt;
	aoi_data.tilt_correction = inputs->tilt_correction;
	aoi_data._AOI = 0;
	aoi_data.AOIl = 0;
	aoi_data.AOIt = 0;
	aoi_data.phi = 0;
	aoi_data.eps = 0;
	aoi_data.pan_rad = 0;
	aoi_data.tilt_rad = 0;
	aoi_data.x = 0;
	aoi_data.y = 0;
	aoi_data.z = 0;

	if (xSemaphoreTake(sem_SPA_AOI, portMAX_DELAY)){
		aoi_data.azimuth = g_AOIInputs.azimuth;
		aoi_data.elevation = g_AOIInputs.elevation;	
	}
	aoi_data = ephToAOI(aoi_data.azimuth, aoi_data.elevation, aoi_data.pan, aoi_data.tilt, aoi_data.tilt_correction);
	
	//aoi_data = ephToAOI(inputs->azimuth, inputs->elevation, inputs->pan, inputs->tilt, inputs->tilt_correction);
	g_InterpolInputs.AOIl = aoi_data.AOIl;
	g_InterpolInputs.AOIt = aoi_data.AOIt;		
	xSemaphoreGive(sem_AOI_Inter);


	vTaskDelete(NULL);
}	