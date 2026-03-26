function [time,data,units] = flukeread(filein,mtype,dtype)
% FLUKEREAD reads out the data saved in the .csv files logged by Fluke 189
% and 289 multimeters as read out by FlukeView Forms 
%   [TIME, DATA, UNITS] = FLUKEREAD(FILEIN) returns the DATA from the
%   logged fluke data along with the correspondind TIME vector (in matlab
%   julian date numbers.  Only "Stable" and "Interval" values are returned.
%   UNITS is a string that lists the units of the data (which can be in ie
%   mV instead of V)
%
%   [TIME, DATA, UNITS] = FLUKEREAD(FILEIN,'MTYPE') allows the user to specify the
%   measurment type to be returned. Options are 'sample' (instaneous
%   values), 'average','max', and 'min'.  'sample' is only available for
%   289 records. 'average' is default.
%
%   NOTE: the folowing header values need to be verified in spanish
%   version: sample, mintime, maxtime


fin = fopen(filein,'r');
if ~exist('mtype','var') || isempty(mtype)
    mtype = 'average';
end

dateformat = 4;
ampm = false;
if ~exist('dtype','var')
    dtype = 'eu';
end



% Read multimeter type etc
%FLUKE 189;   V2.02;  0091560029
line = fgetl(fin); 

dlm = ',';
if any(line==';')
    dlm = ';';
end
lineinfo =  textscan(line,'%s','Delimiter',dlm);
model = lineinfo{1}{1};
version = deblank(lineinfo{1}{2});
% serial = str2double(lineinfo{1}{3});

% if ~strcmp(version,'V2.02')
%     warning('FLUKEREAD written for version 2.02, who knows what might happen now...')
% end

if strcmpi(model,'FLUKE 54-II') && strcmp(mtype,'average')
    warning('Average not avail for FLUKE 54-II, changing to instantaneous')
    mtype = 'sample';
end

if strcmpi(model,'FLUKE 189') && strcmp(mtype,'sample')
    warning('Instananeous samples not avail for 189s, changing to average')
    mtype = 'average';
end

% Identifiers for file navigation (bilingual)
header = {'Start Time','Hora de inicio'};
totalread = {'Total readings','Cantidad total de lecturas'};

sessionname = {'Session Name','Need to add esp.'};
savedread = {'Saved Readings','Need to add esp.'};
primary = {'Primary','Need to add esp.'};

readings =  {'Reading','Lectura'};
endlog = {'Logging Stopped','El registro terminó'};
interval = {'Interval','Intervalo'};
stable = {'Stable','Estable'};

starttime = {'Start Time','Hora de inicio'};
stoptime = {'Stop Time','Hora de terminación'};
timestamp = {'Time Stamp','Timestamp'}; %the second one is found on 289 in sample reading mode.

sample = {'Sample','Muestra'}; %CHECK
average = {'Average','Promedio'};
max = {'Max','Máx'};
maxtime = {'Max Time','Hora de máximo'};%CHECK
min = {'Min','Mín'};
mintime = {'Min Time','Hora de mínimo'}; %CHECK
description = {'Description','Descripción'};
temp1 = {'T1'};
temp2 = {'T2'};



%           1       2        3        4          5       6   7        8     9       10          11
% 289(eng): Reading;Sample;Start Time;Duration;Max Time;Max;Average;Min Time;Min;Description;Stop Time
% 189(eng): Reading;Start Time;Duration;Max;;Average;;Min;;Description;Stop Time
% 189(esp): Lectura;Hora de inicio;Duración;Máx;;Promedio;;Mín;;Descripción;Hora de terminación
% 54II(eng): Reading,T1,,T2,,T1-T2,,Time Stamp

%Find Header (or readings if no header)
line = fgetl(fin); 
if ~isempty(line), lineinfo = textscan(line,'%s','Delimiter',dlm);  end
while isempty(line) || ~(any(strcmpi(lineinfo{1}{1},header)) || any(strcmpi(lineinfo{1}{1},readings)))
    line = fgetl(fin); 
    if ~isempty(line), lineinfo = textscan(line,'%s','Delimiter',dlm);  end
end

trcol = find(or(strcmpi(lineinfo{1},totalread{1}),strcmpi(lineinfo{1},totalread{2})));
if strcmp(model,'FLUKE 289')
    sncol = find(or(strcmpi(lineinfo{1},sessionname{1}),strcmpi(lineinfo{1},sessionname{2})));
end

if any(strcmpi(lineinfo{1}{1},header))
    %Read total readings from Header Info
    % Start Time;Stop Time;Elapsed Time;Interval;Total readings;Intervals;Input Events;Session Name
    % 21/05/2010 12:25:32;21/05/2010 12:31:52;0:06:19;0:00:01;403;380;43;dataing 12
    line = fgetl(fin);  lineinfo =  textscan(line,'%s','Delimiter',dlm);
    totalread = str2double(lineinfo{1}{trcol});
    
    %For fluke 289s, we can down load saved readings as a log
    if strcmp(model,'FLUKE 289') & or(strcmp(deblank(lineinfo{1}{sncol}),savedread{1}),strcmp(deblank(lineinfo{1}{sncol}),savedread{2}))
        mtype = 'savedreadings'
    end

    % Initialize output vectors using total readings
    time = zeros(totalread,1);
    data = zeros(totalread,1);

    
    % Find Readings
    line = fgetl(fin); 
    if ~isempty(line), lineinfo = textscan(line,'%s','Delimiter',dlm);  end
    while isempty(line) || ~any([strcmpi(lineinfo{1},readings{1}); strcmpi(lineinfo{1},readings{2})] )
        line = fgetl(fin); 
        if ~isempty(line), lineinfo = textscan(line,'%s','Delimiter',dlm);  end
    end

end
%Get Column labels
collabs = lineinfo{1}; numcols = length(collabs);

% Find column indeces
descindex = find(or(strcmpi(collabs,description{1}),strcmpi(collabs,description{2})));
switch mtype
    case 'sample'
        if strcmp(model,'FLUKE 54-II')
                timeindex = find(strcmpi(collabs,timestamp{1}));
                sampleindex = [find(strcmpi(collabs,temp1)) find(strcmpi(collabs,temp2))];
                units = '°C';
                description = 'Interval';
        else
            timeindex = find(or(strcmpi(collabs,starttime{1}),strcmpi(collabs,starttime{2})));
            sampleindex = find(or(strcmpi(collabs,sample{1}),strcmpi(collabs,sample{2})));
            description = 'NULL';
        end
    case 'savedreadings'
         timeindex = find(or(strcmpi(collabs,timestamp{1}),strcmpi(collabs,timestamp{2})));
         
         %for now we only read primary reading
         sampleindex = find(or(strcmpi(collabs,primary{1}),strcmpi(collabs,primary{2})));

    case 'average'
        timeindex = [find(or(strcmpi(collabs,starttime{1}),strcmpi(collabs,starttime{2}))) find(or(strcmpi(collabs,stoptime{1}),strcmpi(collabs,stoptime{2})))];
        sampleindex = find(or(strcmpi(collabs,average{1}),strcmpi(collabs,average{2})));
    case 'max'
        switch model
            case 'FLUKE 289'
                timeindex = find(or(strcmpi(collabs,maxtime{1}),strcmpi(collabs,maxtime{2})));
            case 'FLUKE 189'
                timeindex = [find(or(strcmpi(collabs,starttime{1}),strcmpi(collabs,starttime{2}))) find(or(strcmpi(collabs,stoptime{1}),strcmpi(collabs,stoptime{2})))];
        end
        sampleindex = find(or(strcmpi(collabs,max{1}),strcmpi(collabs,max{2})));
    case 'min'
        switch model
            case 'FLUKE 289'
                timeindex = find(or(strcmpi(collabs,mintime{1}),strcmpi(collabs,mintime{2})));
            case 'FLUKE 189'
                timeindex = [find(or(strcmpi(collabs,starttime{1}),strcmpi(collabs,starttime{2}))) find(or(strcmpi(collabs,stoptime{1}),strcmpi(collabs,stoptime{2})))];
        end
        sampleindex = find(or(strcmpi(collabs,min{1}),strcmpi(collabs,min{2})));
end
% 
% unitsmixed = 1;
% if isempty(collabs{sampleindex(1)+1})
%     unitsmixed = 0;
% end

unitsmixed = 1;
if any(strcmp(collabs,'Units')) | isempty(collabs{sampleindex(1)+1})
   unitsmixed = 0;
end

k = 1;
%Read appropriate samples and times
while ~any(strcmpi(description,endlog)) & ~feof(fin)

    line = fgetl(fin);

   
    
    if isempty(line)
        description = endlog
    else
        lineinfo = textscan(line,'%s',numcols,'Delimiter',dlm); lineinfo = lineinfo{1};
    
        if k == 1
            example_time = lineinfo{timeindex(1)};
            if contains(example_time,{'AM','PM'})
                ampm = true;
            end
            example_time = split(example_time,' ');
            example_date = example_time{1};
            if length(example_date)<10
                dateformat = 'mm/dd/yy';
            end
            timeformat = [dateformat ' HH:MM:SS'];
        end

        if ~isempty(descindex)
            description = lineinfo{descindex};
            finished = any(strcmpi(description,endlog));
        else
            finished = feof(fin);
        end

        if ~finished
            try
                sampletime = datenum(lineinfo(timeindex),timeformat); %28/04/2010 15:47:33.8
                sampletime(contains(lineinfo(timeindex),'PM')) = sampletime(contains(lineinfo(timeindex),'PM')) +12 /24;
                sampletime = mean(sampletime);
            catch
                try
                    sampletime = mean(datenum(lineinfo(timeindex),'dd/mm/yyyy HH:MM')); %28/04/2010 15:47:33.8
                catch
                    sampletime = mean(datenum(lineinfo(timeindex)));
                end
            end
            for s = 1:length(sampleindex)
                sampleinfo = lineinfo{sampleindex(s)};

                if unitsmixed
                    sampleinfo = textscan(sampleinfo,'%s%s%s','Delimiter',' ');
                    sample = sampleinfo{1}{1};
                    if k == 1
                        units = sampleinfo{2}{1};
                    end
                else
                    sample = sampleinfo;
                    if k == 1
                        units = lineinfo{sampleindex(1)+1};
                    end
                end
                sample(sample == ',') = '.';

                if (any(strcmpi(description,interval)) ...
                        || any(strcmpi(description,stable))) ...
                        || strcmp(mtype,'savedreadings') ...
                        && ~isempty(sample)

                    time(k,1) = sampletime;
                    data(k,s) = str2double(sample);
                    if s == length(sampleindex)
                        k = k+1;
                    end
                end
            end
        end
    end
end

fclose(fin);

if k <= length(time);
    if time(k) == 0
        time(k:end) = [];
        data(k:end,:) = [];
    else
        time(k+1:end) = [];
        data(k+1:end,:) = [];
    end
end
