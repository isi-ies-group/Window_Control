#include <Arduino.h>
#include "interpolation.h"
#include "global_structs.h"
#include "sync.h"



extern float g_x_val;
extern float g_z_val;
extern InterpolInputs g_InterpolInputs;


void InterpolationTask(void *pvParameters) {
	AutoHandle *ah = (AutoHandle*)pvParameters;

	float query_points[2];

	xSemaphoreTake(ah->sem_AOI_Inter, portMAX_DELAY);

	query_points[0] = (float)fabs(g_InterpolInputs.AOIt);
	query_points[1] = (float)fabs(g_InterpolInputs.AOIl);
	

	for (int i = 0; i < 2; i++){
		if (query_points[i] < 0) query_points[i] = 0;
		if (query_points[i] > N - 1) query_points[i] = N - 1;
	}

	static float x_coords[N], y_coords[N];
	for (int k = 0; k < N; k++) {
	x_coords[k] = k;
	y_coords[k] = k;
	}

	const float* coords[2] = {x_coords, y_coords}; 
	int n[2] = {N, N};


	g_x_val = interpolate(coords, n, ah->matrix_X, query_points);
	g_z_val = interpolate(coords, n, ah->matrix_Z, query_points);

	
	Serial.print("Interpolated x value: ");
	Serial.println(g_x_val, 6);
	Serial.print("Interpolated z value: ");
	Serial.println(g_z_val, 6);
	xSemaphoreGive(ah->sem_End);
	
	vTaskDelete(NULL); 
}