#ifndef GPS_H
#define GPS_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


void RTC_SetFromTM(struct tm *t);
void RTC_GetToTM(struct tm *t);

void setManualTime(int year, int month, int day,
                   int hour, int min, int sec);

#ifdef __cplusplus
}
#endif

#endif /* GPS_H */


