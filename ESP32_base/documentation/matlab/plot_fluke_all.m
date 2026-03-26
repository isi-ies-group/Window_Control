clc
clear
close all

% === LEER DATOS ===
[t_avg, d_avg, u] = flukeread('Save 1.csv', 'average');
[t_max, d_max, ~] = flukeread('Save 1.csv', 'max');
[t_min, d_min, ~] = flukeread('Save 1.csv', 'min');

% === TIEMPO EN SEGUNDOS ===
t0 = t_avg(1);
t = (t_avg - t0) * 24 * 3600;

% Asegurar vectores columna
d_avg = d_avg(:);
d_max = d_max(:);
d_min = d_min(:);

% === FIGURA ===
figure('Color','w')
hold on
grid on
box on

% --- Preparar t_max en segundos (misma referencia que t_avg si lo deseas) ---
t0_max = t_max(1);
tmax_s = (t_max - t0_max) * 24 * 3600;
tmax_s = tmax_s(:);
d_max = d_max(:);

% Encontrar picos en d_max (no en d_min)
[pks_max, locs_max] = findpeaks(d_max);

if isempty(pks_max)
    warning('No se encontraron picos en d_max.');
else
    [~, idx_sort] = sort(pks_max, 'descend');
    nShow = min(5, numel(pks_max));
    topIdx = idx_sort(1:nShow);
    topLocs = locs_max(topIdx);
    topPks = pks_max(topIdx);
    topTimes = tmax_s(topLocs);

    plot(topTimes, topPks, 'rv', 'MarkerFaceColor','r', 'MarkerSize',8, ...
         'DisplayName','Top 5 peaks (max)')
    for k = 1:nShow
        text(topTimes(k), topPks(k), sprintf('  %.3g', topPks(k)), ...
             'VerticalAlignment','bottom', 'FontSize',10, 'Color','r')
    end
    legend('Location','best')
end
% Banda Min–Max
fill([t; flipud(t)], ...
     [d_min; flipud(d_max)], ...
     [0.85 0.85 0.85], ...
     'EdgeColor','none', ...
     'DisplayName','Min–Max')

% Línea Average
plot(t, d_avg, 'k', 'LineWidth', 2, ...
     'DisplayName','Average')

xlabel('Tiempo desde el inicio (s)','Interpreter','latex')
ylabel(['Corriente (' u ')'],'Interpreter','latex')
title('Evolución temporal de la corriente medida','Interpreter','latex')

legend('Location','best')
set(gca,'FontSize',12)
