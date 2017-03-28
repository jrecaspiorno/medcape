close all
% Read the contents back into an array
fid = fopen('data.dat');
%Status(2B) Ch0(2B) Ch1(2B)
%The data format for each channel data are twos complement and MSB first
data = fread(fid, [3, inf], 'int16', 'ieee-be');
%data = fread(fid, [3+2*8, inf], 'int16', 'ieee-be');

fclose(fid);

%Internal Vref
Vref=2.42; %ADS1192
%Vref=2.4; %ADS1298

%ADC to Volts for ADS1192
adc2v=Vref/((2^15)-1);

%ADC to Volts for ADS1298
%adc2v=Vref/((2^24)-1);

figure; hold on;
t=(1:length(data(2,:)))*(1/1000);
plot(t, data(2,:)*adc2v, 'r.-');
plot(t, data(3,:)*adc2v, 'k.-');
grid on;
