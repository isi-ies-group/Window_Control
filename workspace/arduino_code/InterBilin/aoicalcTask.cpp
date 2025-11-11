#include <Arduino.h>
#include "aoicalc.h"
#include "sync.h"
#include "global_structs.h"

extern InterpolInputs g_InterpolInputs;
extern AOIInputs g_AOIInputs;


void aoicalcTask(void *pvParameters) {
	AutoHandle *ah = (AutoHandle*)pvParameters;
	
	AOI aoi_data;
	aoi_data.pan = g_AOIInputs.pan;
	aoi_data.tilt = g_AOIInputs.tilt;
	aoi_data.tilt_correction = g_AOIInputs.tilt_correction;
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

	if (xSemaphoreTake(ah->sem_SPA_AOI, portMAX_DELAY)){
		aoi_data.azimuth = g_AOIInputs.azimuth;
		aoi_data.elevation = g_AOIInputs.elevation;	
	}
	aoi_data = ephToAOI(aoi_data.azimuth, aoi_data.elevation, aoi_data.pan, aoi_data.tilt, aoi_data.tilt_correction);
	
	g_InterpolInputs.AOIl = aoi_data.AOIl;
	g_InterpolInputs.AOIt = aoi_data.AOIt;		
	xSemaphoreGive(ah->sem_AOI_Inter);


	vTaskDelete(NULL);
}	