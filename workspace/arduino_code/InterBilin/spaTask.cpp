#include <Arduino.h>
#include "spaTask.h"
#include "sync.h"
#include "spa.h"
#include <time.h>
#include "global_structs.h"

extern AOIInputs g_AOIInputs;
extern SemaphoreHandle_t sem_SPA_AOI;

int getTimezone(int year, int month, int day) {
	int lastSundayMarch = 31;
	while (true) {
		tm t = {};
		t.tm_year = year - 1900;
		t.tm_mon = 2;
		t.tm_mday = lastSundayMarch;
		mktime(&t);
		if (t.tm_wday == 0) break;
		lastSundayMarch--;
	}


	int lastSundayOctober = 31;
	while (true) {
		tm t = {};
		t.tm_year = year - 1900;
		t.tm_mon = 9;
		t.tm_mday = lastSundayOctober;
		mktime(&t);
		if (t.tm_wday == 0) break;
		lastSundayOctober--;
	}

	if ((month > 3 && month < 10) ||
			(month == 3 && day >= lastSundayMarch) ||
			(month == 10 && day < lastSundayOctober)) {
		return 2;
	} 
	else
		return 1;
}

void printTimeDecimal(double time) {
  int h = (int)time;
  double min = 60.0 * (time - h);
  double sec = 60.0 * (min - (int)min);

  if (h < 10) Serial.print('0');
  Serial.print(h);
  Serial.print(':');

  if ((int)min < 10) Serial.print('0');
  Serial.print((int)min);
  Serial.print(':');

  if ((int)sec < 10) Serial.print('0');
  Serial.println((int)sec);
}

void SPATask(void *pvParameters) {
	SPAInputs *inputs = (SPAInputs *) pvParameters;

	spa_data spa;
	spa.year = inputs->year;
	spa.month = inputs->month;
	spa.day = inputs->day;
	spa.hour = inputs->hour;
	spa.minute = inputs->minute;
	spa.second = inputs->second;
	spa.latitude = inputs->latitude;
	spa.longitude = inputs->longitude;
	spa.timezone      = 2;
	spa.delta_ut1     = 0;
	spa.delta_t       = 67;
	spa.elevation     = 670;
	spa.pressure      = 820;
	spa.temperature   = 20;
	spa.slope         = 30;
	spa.azm_rotation  = -10;
	spa.atmos_refract = 0.5667;
	spa.function      = SPA_ZA_RTS;

	
	int result = spa_calculate(&spa);

	if (result == 0) {

		g_AOIInputs.azimuth = spa.azimuth;
		g_AOIInputs.elevation = spa.e;
		Serial.print("g_elvation: "); 
		Serial.println(g_AOIInputs.elevation, 6);
		Serial.print("Zenith: "); 
		Serial.println(spa.zenith, 6);

		Serial.print("Azimuth: "); 
		Serial.println(spa.azimuth, 6);

		Serial.print("Elevation: "); 
		Serial.println(spa.e, 6);		

		Serial.print("Sunrise: ");	
		printTimeDecimal(spa.sunrise);

		Serial.print("Sunset: ");
		printTimeDecimal(spa.sunset);
		xSemaphoreGive(sem_SPA_AOI);
	} 
	else
		Serial.print("SPA Error Code: "); Serial.println(result);

	vTaskDelete(NULL); 
}
