#ifndef aoicalcTask_h
#define aoicalcTask_h

#ifdef __cplusplus
extern "C" {
#endif
#include <Arduino.h>

typedef struct {
	int tilt_correction;
	double azimuth;
	double elevation;
	double pan;
	double tilt;
}AOIInputs;


void aoicalcTask(void *pvParameters);



#ifdef __cplusplus
}
#endif

#endif