% example_map_build.m
% We design the map in the first quadrant of 3D space. After the designing
% process the map (along with all the objects designed for it) is rotated 
% and translated to its ENU position. 
% Map dimensions are approximately: 
%   x: 16.40 meters
%   y: 7.25 meters
%   z: 4.27 meters 
% =========================================================================
clear;

%% design map

% to construct a rect_prism (rectangular prism) object you must specify the 
% center point and the x, y, and z edge lengths.
% see rect_prism.m for full details on object
obst1 = rect_prism([10 4 2]',2,4,4);
obst2 = rect_prism([3 3 3]',1,1,1);

% to construct a balloon object simply enter its center point and its color
b1 = balloon([14 2.3 1]','r');
b2 = balloon([13 5.5, 3.5]','b');

% to construct a point object enter its location vector
sp = point([1.5 1.5 1]');
gp = point([3 6 3]');

% plot arena with obstacles/balloons/start&goal
% specify colors for obstacles and points
f1 = figure(1); clf(1);
plot_arena()
plot(obst1,'r')
plot(obst2,'m')
plot(b1);
plot(b2);
plot(sp,'k');
plot(gp,'k');
axis equal
view(45,45)

%% build .map file

% convert obstacles to dotmap format strings 
% the set the boolean to true if converting the last obstacle
str1 = dotmap_str(obst1,false);
str2 = dotmap_str(obst2,true);

% import the string for the empty arena
fid  = fopen('arena_template.txt','r');
str0=fread(fid,'*char')';
fclose(fid);

% construct complete dotmap file by simply concatenating the strings like this:
% [str_arena str_obst1 ... str_obst_last]
fid = fopen('output_map.txt','w');
fprintf(fid,'%s',[str0 str1 str2]);
fclose(fid);

% print position for balloons and points to the command line for copy&paste
red_balloon_pos = position_str(b1)
blue_balloon_pos = position_str(b2)
start_pos = position_str(sp)
goal_pos = position_str(gp)
