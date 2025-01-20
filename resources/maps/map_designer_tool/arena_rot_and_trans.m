% arena_rot_and_trans: finds the 3D translation and rotation matrix to go from
% the ENU arena vertices to the following: one corner is the origin and the 
% majority of arena is in the first quadrant of the plane. 

% load arena vertices from empty_arena.map
arena_bot =      [-27.32,  13.72, -05.15;
                  -10.97,  12.36, -05.15;
                  -10.37,  19.58, -05.15;
                  -26.72,  20.95, -05.15]';
arena_top =      [-27.32,  13.72,  -0.882;
                  -26.72,  20.95,  -0.882;
                  -10.37,  19.58,  -0.882;
                  -10.97,  12.36,  -0.882]';

% translate such that one corner is touching the origin
arena_translation = [10.37 -19.58 5.15]';
arena_bot = arena_bot + arena_translation;
arena_top = arena_top + arena_translation;

% find the rotation matrix relating the arena alignment to the axes RAI
vIMat = [1 0 0; 0 1 0; 0 0 1];
vAMat = [-16.35 1.37 0; -0.6 -7.22 0; 0 0 4.268];
RAI = mm.wahba(ones(3,1),vIMat,vAMat);

% rotate the bottom vertices to the first quadrant and back
arena_bot
arena_bot = pagemtimes(arena_bot',RAI)'
arena_bot = pagemtimes(arena_bot',RAI')'

% rotate the bottom vertices to the first quadrant and back
arena_top
arena_top = pagemtimes(arena_top',RAI)'
arena_top = pagemtimes(arena_top',RAI')'