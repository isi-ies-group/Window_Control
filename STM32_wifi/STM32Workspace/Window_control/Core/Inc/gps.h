#include <time.h>


void RTC_SetFromTM(struct tm *t);
void RTC_GetToTM(struct tm *t);

void setManualTime(int year, int month, int day,
                   int hour, int min, int sec);



