#ifndef spaTask_h
#define spaTask_h

#ifdef __cplusplus
extern "C" {
#endif
#include <Arduino.h>

typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	double latitude;
	double longitude;
}SPAInputs;


void SPATask(void *pvParameters);
int getTimezone(int year, int month, int day);
void printTimeDecimal(double time);


#ifdef __cplusplus
}
#endif

#endif