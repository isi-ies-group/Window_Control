#include <math.h>
#include <Arduino.h>
#include "commonlib.h"

double rad2deg(double radians)
{
    return (180.0/PI)*radians;
}

double deg2rad(double degrees)
{
    return (PI/180.0)*degrees;
}


