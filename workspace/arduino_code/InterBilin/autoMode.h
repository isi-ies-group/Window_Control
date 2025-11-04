#ifndef autoMode_h
#define autoMode_h

#define N 86

void autoMode (double lon, double lat, double pan, double tilt, bool tilt_correction,
				const float (*matrix_X)[N], const float (*matrix_Z)[N]);
				
#endif	