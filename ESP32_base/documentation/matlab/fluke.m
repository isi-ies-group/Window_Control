clc; clear; close all;

T = readtable('MCU_current.csv','Delimiter','\t');

avg = str2double(erase(T.Average,' A DC'));
t   = seconds(duration(T.("Start Time"),'InputFormat','mm:ss.S'));

plot(t, avg)
grid on
xlabel('Tiempo (s)')
ylabel('Average (A)')

