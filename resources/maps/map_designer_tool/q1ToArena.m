function enu_vecs = q1ToArena(quadrant1_vecs)
% arena_location: rotates vectors in the 1st quadrant representation of the
%   arena to the corresponding locations in the ASE roof ENU frame.
%
% inputs: 
%   quadrant1_vecs --- 3-by-N; each column is a vector 
%
% outputs:
%   enu_vecs --- 3-by-N; each column holds the rotated and translated version of
%                the vectors in quadrant1_vecs
%
% ==============================================================================

% rotate and translate the vertices to match the arena
RAI = [-0.9965 -0.0832 0;0.0832 -0.9965 0;0 0 1.0000];
arena_translation = [10.37 -19.58 5.15]';
temp_vecs = pagemtimes(quadrant1_vecs',RAI')';
enu_vecs = temp_vecs - arena_translation;

end