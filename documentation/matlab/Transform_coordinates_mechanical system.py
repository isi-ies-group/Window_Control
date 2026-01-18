# -*- coding: utf-8 -*-
"""
Created on Thu Jul 10 15:42:17 2025

@author: almudena.garcia
"""

import numpy as np

def solar_coord_to_module_coord(azimuth_solar, elevation_solar, tilt, pan):
    """

    Converts the sun's azimuth and elevation (defined on the horizontal plane)
    to azimuth and elevation on a plane with arbitrary tilt and pan.

    Args:

        azimuth_solar (float): Solar azimuth in degrees (0° = North, 90° = East, 180° = South, 270° = West), clockwise.
        elevation_solar (float): Solar elevation in degrees (0° = horizon, 90° = zenith).
        tilt (float): Tilt of the plane in degrees (0° = horizontal, 90° = vertical).
        pan (float): Orientation of the plane in degrees (0° = North, 90° = East, 180° = South, 270° = West), clockwise.

    Returns:
        tuple: (azimut_relativo, elevacion_relativa, AOI) in degrees.
        azimuth_relative: Azimuth of the sun relative to the inclined plane, counterclockwise from the pan direction.
        elevation_relative: Elevation of the sun above the inclined plane (angle between the solar vector and the plane).
        AOI: Angle of incidence (angle between the solar vector and the normal to the plane).
    """
    # degrees to rad
    azimuth_solar_rad = np.deg2rad(azimuth_solar)
    elevation_solar_rad = np.deg2rad(elevation_solar)
    tilt_rad = np.deg2rad(tilt)
    pan_rad = np.deg2rad(pan)
    # sferical coordinates to cartesian coordinates
    x = np.cos(elevation_solar_rad) * np.cos(azimuth_solar_rad)
    y = np.cos(elevation_solar_rad) * np.sin(azimuth_solar_rad)
    z = np.sin(elevation_solar_rad)
    # clockwise rotation around the Z axis
    x_pan = y * np.sin(pan_rad) + x * np.cos(pan_rad)
    y_pan = y * np.cos(pan_rad) - x * np.sin(pan_rad)
    z_pan = z
    # clockwise rotation around the Y axis
    y_tilt = y_pan
    x_tilt = x_pan * np.cos(tilt_rad) - z_pan * np.sin(tilt_rad)
    z_tilt = x_pan * np.sin(tilt_rad) + z_pan * np.cos(tilt_rad)
    # z_tilt clipped to the interval edges
    z_tilt = np.clip(z_tilt, -1.0, 1.0)
    
    # new elevation and azimuth
    elevacion_relativa_rad = np.arcsin(z_tilt)
    azimut_relativo_rad = np.arctan2(y_tilt, x_tilt)

    elevation_relative = np.rad2deg(elevacion_relativa_rad)
    azimuth_relative = np.rad2deg(azimut_relativo_rad)

    if tilt > 45:        
        # clockwise rotation around the Z axis
        x_tras = y_tilt * np.sin(np.pi) + x_tilt * np.cos(np.pi)         
        y_tras = y_tilt * np.cos(np.pi) - x_tilt * np.sin(np.pi)
        z_tras = z_tilt

        azimut_relativo_rad = np.arctan2(y_tras, x_tras)
        azimuth_relative = np.rad2deg(azimut_relativo_rad)
        x_tilt=x_tras
        y_tilt=y_tras
        
    AOI = np.rad2deg(np.arccos(z_tilt))
    

    return azimuth_relative, elevation_relative, AOI, x_tilt,y_tilt,z_tilt

def module_coord_to_AOIL_AOIT(azimuth_relative,elevation_relative): #azimut desde el sur, hacer previa conversion

   """
   Converts the sun's azimuth and elevation relatives of a plane with arbitrary tilt and pan into the modules' proper angles 
   projected onto the coordinate axes

   Args:
       azimuth_relative (float): Azimuth of the sun relative to the inclined plane, counterclockwise from the pan direction.
       elevation_relative (float): Elevation of the sun above the inclined plane (angle between the solar vector and the plane).
   Returns:
       tuple: (AOIL, AOIT) in degrees.
              AOIL: Indicates the projection of the sun's position on the YZ plane
              AOIT: Indicates the projection of the sun's position on the XZ plane
              
   """
   azimuth_relative = np.deg2rad(azimuth_relative)
   elevation_relative = np.deg2rad(elevation_relative)
   x = np.cos(elevation_relative)*np.cos(azimuth_relative)
   y = np.cos(elevation_relative)*np.sin(azimuth_relative)
   z = np.sin(elevation_relative)
   
   AOIL = np.rad2deg(np.arctan2(y,z))
   AOIT = np.rad2deg(np.arctan2(x,z))


   return AOIL,AOIT, x, y, z