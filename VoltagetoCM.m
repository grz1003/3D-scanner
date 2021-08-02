%This function read the data from the file. The data inside the file is the
%voltage output of the inferred sensor and the data is needed to be convert
%to centermeter.

function Distance = VoltagetoCM(filename)
%filename = 'sample.txt';

x = [0.3, 0.4, 0.59, 1.06, 2.09, 2.45];
y = [20, 15, 10, 5, 2, 1.3];

p = polyfit(x, y, 5);
v = polyval(p, x);

t = 1:numel(x);
xy = [x;y];
s = spline(t,xy);
tStep = linspace(1,numel(x));
xyPloy = ppval(s, tStep);

%Conversion Curve Version 1:Straight line between points
% figure(1)
% plot(x,y,'x','MarkerEdgeColor', 'blue')
% hold on
% plot(x, v)
% hold off
% grid on;
% xlabel('Voltage');
% ylabel('cm');

%Conversion Curve Version 2: Curve line between points
%We are using this version for better accuracy
% figure(2)
% plot(x,y,'x','MarkerEdgeColor', 'blue')
% hold on
% plot(xyPloy(1,:),xyPloy(2,:))
% hold off
% grid on;
% xlabel('Voltage');
% ylabel('cm');

%The working range of inferred sensor
%maxDistance = 15;%limited by the sensor
%minDistance = 2;

Source = readmatrix(filename);
Distance = interp1(xyPloy(1,:),xyPloy(2,:), Source);

%clean up workspace
clear p s Source t tStep v x xy xyPloy y;
end
