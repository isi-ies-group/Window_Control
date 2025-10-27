#include <Arduino.h>
#include "aoicalcTask.h"
#include "aoicalc.h"

extern double g_AOIl;
extern double g_AOIt;


void aoicalcTask(void *pvParameters) {
	AOIInputs *inputs = (AOIInputs *) pvParameters;
	
	AOI aoi_data;
	
	aoi_data.azimuth = inputs->azimuth;
	aoi_data.elevation = inputs->elevation;
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
	
	aoi_data = ephToAOI(aoi_data.azimuth, aoi_data.elevation, aoi_data.pan, aoi_data.tilt, aoi_data.tilt_correction);
	
	//aoi_data = ephToAOI(inputs->azimuth, inputs->elevation, inputs->pan, inputs->tilt, inputs->tilt_correction);
	g_AOIl = aoi_data.AOIl;
	g_AOIt = aoi_data.AOIt;		

	vTaskDelete(NULL);
}	