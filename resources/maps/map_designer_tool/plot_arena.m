function plot_arena()

% arena vertices from arena_rot_and_trans.m
arena_vert = [16.404 -0.0025 0 16.4073 16.404 16.4073 0 -0.0025
             7.2492 7.2449 0 -0.0056 7.2492 -0.0056 0 7.2449
             0 0 0 0 4.2680    4.2680    4.2680    4.2680]; 

hold on;

[~,sortidx] = sort(arena_vert(3,:));
arena_vert = arena_vert(:,sortidx);
sq1 = arena_vert(:,1:4);
sq2 = arena_vert(:,5:8); 
% plot paralell squares
fill3(sq1(1,:),sq1(2,:),sq1(3,:),'g');
fill3(sq2(1,:),sq2(2,:),sq2(3,:),'k','FaceAlpha',0.1);

hold off;

end % EOF plot_arena

