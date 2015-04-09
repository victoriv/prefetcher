
speedups_conf_1 = [1.243, 1.043, 1.024, 1.036, 1.036, 1.031, 0.988, 1.021, 1.011, 1.002, 1.238]
speedups_conf_5 = [1.539, 1.040, 1.054, 1.058, 1.058, 1.037, 0.982, 1.079, 1.032, 0.999, 1.237]
speedups_conf_10 = [1.497, 1.041, 1.063, 1.059, 1.059, 1.032, 0.983, 1.070, 1.026, 0.997, 1.237]
speedups_conf_20 = [1.440, 1.040, 1.066, 1.057, 1.057, 1.050, 0.995, 1.048, 0.972, 0.997, 1.237]

reference = ones(1,11)
reference_x = 1:1:11
x = 1:11;

speedups_comb = zeros(11,4);

for i = 1:11
    speedups_comb(i,1) = speedups_conf_1(i);
    speedups_comb(i,2) = speedups_conf_5(i);
    speedups_comb(i,3) = speedups_conf_10(i);
    speedups_comb(i,4) = speedups_conf_20(i);
end


% Plot the data.
h = bar(speedups_comb);
%hold on;
%h = plot(x,speedups_conf_5,'-sr');
%h = plot(x,speedups_conf_10,'-sg');
%h = plot(x,speedups_conf_20,'-sm');
%h = plot(reference_x,reference,'--k');
legend({'maximum degree = 1', 'maximum degree = 5', 'maximum degree = 10', 'maximum degree = 20'}, 'Position', [0.27, 0.65, 0.3, 0.15])
ylabel('Speedup')
grid on;
%hold off;

% Reduce the size of the axis so that all the labels fit in the figure.
pos = get(gca,'Position');
set(gca,'Position',[pos(1), .2, pos(3) .65])

% Add a title.
%title('Speedup')

% Set the X-Tick locations so that every other month is labeled.
Xt = 1:1:11;
Xl = [1 11];
%set(gca,'XTick',Xt,'XLim',Xl);

% Add the months as tick labels.
benchmarks = ['          ammp'; '         applu'; '          apsi'; '        art110'; '        art470'; 'bzip2\_program'; '  bzip\_source'; '        galgel'; '          swim'; '         twolf'; '       wupwise'];

ax = axis;    % Current axis limits
axis(axis);    % Set the axis limit modes (e.g. XLimMode) to manual
Yl = ax(3:4);  % Y-axis limits

% Place the text labels
t = text(Xt,Yl(1)*ones(1,length(Xt)),benchmarks(1:1:11,:));
set(t,'HorizontalAlignment','right','VerticalAlignment','top', ...
      'Rotation',45);

% Remove the default labels
set(gca,'XTickLabel','')

% Get the Extent of each text object.  This
% loop is unavoidable.
for i = 1:length(t)
  ext(i,:) = get(t(i),'Extent');
end

% Determine the lowest point.  The X-label will be
% placed so that the top is aligned with this point.
LowYPoint = min(ext(:,2));

% Place the axis label at this point
%XMidPoint = Xl(1)+abs(diff(Xl))/2;
%tl = text(XMidPoint,LowYPoint,'X-Axis Label', ...
          %'VerticalAlignment','top', ...
          %'HorizontalAlignment','center');

