% example_map_build.m
clear;

%% design map

% to construct a rect_prism (rectangular prism) object you must specify the 
% center point and the x, y, and z edge lengths.
% see rect_prism.m for full details on object
obst1 = rect_prism([10 4 2]',2,4,4);
obst2 = rect_prism([3 3 3]',1,1,1);

% plot arena with obstacles
f1 = figure(1); clf(1);
plot_arena()
plot(obst1,'r')
plot(obst2,'m')
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
