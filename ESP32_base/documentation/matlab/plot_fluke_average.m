clc
clear
close all

% Leer archivos (AVERAGE por default)
[t1, d1, u1] = flukeread('Save 1.csv');
[t2, d2, u2] = flukeread('Save 4.csv');

% Convertir tiempo a datetime
t1 = datetime(t1, 'ConvertFrom', 'datenum');
t2 = datetime(t2, 'ConvertFrom', 'datenum');

% ---- TIP: tiempo iniciando en cero ----
t1 = t1 - t1(1);
t2 = t2 - t2(1);

%% FIGURA 1 - Save 1
figure
plot(t1, d1, '-o', 'MarkerSize', 4)
grid on
xlabel('Tiempo desde inicio')
ylabel(['Average (' u1 ')'])
title('Save 1 - Medición corriente promedio MCU')

%% FIGURA 2 - Save 2
figure
plot(t2, d2, '-o', 'MarkerSize', 4)
grid on
xlabel('Tiempo desde inicio')
ylabel(['Average (' u2 ')'])
title('Save 2 - Medición corriente promedio fuente alimentación')
