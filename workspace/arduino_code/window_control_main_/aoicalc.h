#ifndef aoicalc_h
#define aoicalc_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h> 

typedef struct{
	double _AOI;
	double AOIl;
	double AOIt;
	double azimuth, phi;
	double elevation, eps;
	double pan, pan_rad;
	double tilt, tilt_rad;
	double x, y, z;
	bool tilt_correction;  
} AOI;



// sferical coordinates to cartesian coordinates
AOI degToCartesian(double azimuth, double elevation);
//clockwise rotation around the Z axis
AOI applyPan(double pan, double _x, double _y);  
double zClip(double _z);
// Tilt (y rotation)
AOI applyTilt(double tilt, double _x, double _z);

 // Tilt correction when tilt > max_tilt
AOI applyTiltCorrection(double _x, double _y, bool tilt_correction) ;

// AOIl AOIt 
AOI cartesianToAngles(double _x, double _y, double _z);

AOI cartesianToNewEph(double _x, double _y, double _z);

AOI ephToAOI(double azimuth, double elevation, double pan, double tilt, bool tilt_correction);

#ifdef __cplusplus
}
#endif

#endif
