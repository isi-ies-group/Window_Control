# -*- coding: utf-8 -*-
"""
Created on Wed Jun  4 13:23:01 2025

@author: almudena.garcia
"""
import matplotlib.pyplot as plt  # for visualization
import pvlib as pvlib
import numpy as np
import pandas as pd
import os

import Transform_coordinates_AG as TC

# Función que devuelve una lista con todas las fechas horas y minutos necesarios
def times(day, start, end, frec_min):
    dia = str(day)
    horas = np.arange(start, end, 1)
    # print(horas)
    fechas = []
    aux = 0
    for i in horas:
        # print(i)
        minutos = np.arange(0, 60, frec_min)
        for j in minutos:
            # print(j)
            if int(i) < 10:
                i = str(i)
                hora = '0'+ i
            else:
                hora = str(i)
            if j < 10:
                j = str(j)
                minutos = '0'+ j
            else:
                minutos = str(j)
            # minutos=str(j)
            segundos = '00'
            fecha = str(dia + ' ' + hora + ':' + minutos + ':' + segundos + '+00:00')
            fechas.insert(aux, fecha)
            aux = aux+1
            # print(fecha)
    return fechas


# Location's coordinates
latitude, longitude = 40.42, -3.70 # Madrid (Spain)
location = pvlib.location.Location(latitude=latitude, longitude=longitude)


tiempo=times('20/03/2020',8,15,30) # fecha, hora inicial, hora final, intervalo de tiempo en minutos
idx = pd.DatetimeIndex(tiempo)
solar_position = location.get_solarposition(idx)


tilt=0
pan=0

# Después de obtener los resultados de solar_coord_to_module_coord
H, delta, AOI, x_tilt, y_tilt, z_tilt = TC.solar_coord_to_module_coord(solar_position.azimuth, solar_position.elevation, tilt, pan)
print("Resultados de solar_coord_to_module_coord:")
print("Azimut relativo:", H)
print("Elevación relativa:", delta)
print("Ángulo de incidencia (AOI):", AOI)

# Después de obtener los resultados de module_coord_to_AOIL_AOIT
AOIL, AOIT, x, y, z = TC.module_coord_to_AOIL_AOIT(H, delta)
print("Resultados de module_coord_to_AOIL_AOIT:")
print("AOIL:", AOIL)
print("AOIT:", AOIT)
