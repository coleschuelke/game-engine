classdef rect_prism

  properties
    center_point; % 3-by-1
    xlen; % scalar
    ylen; % scalar
    zlen; % scalar
    color; % hex color code
  end

  methods

    function obj = rect_prism(center_point,xlen,ylen,zlen)
      % constructor
      obj.center_point = center_point;
      obj.xlen = xlen;
      obj.ylen = ylen;
      obj.zlen = zlen;
    end % EOF constructor

    function vertex_points = vertices(obj)
      % find all possible combos of -1 or 1 (each col vec holds one possibility)
      binary_combos = ff2n(3)'*2-1;
      % compute vertecies coordinates
      edge_lengths = [obj.xlen obj.ylen obj.zlen]';
      vertex_points = obj.center_point + binary_combos.*(0.5*edge_lengths);
    end % EOF vertices

    function plot(obj,color_str)
      hold on;
      rpvert = vertices(obj);
      for i = 1:size(rpvert,1)
        % separate vertices in rpvert into two parallel 
        [~,sortidx] = sort(rpvert(i,:));
        rpvert = rpvert(:,sortidx);
        sq1 = rpvert(:,[2 1 3 4]);
        sq2 = rpvert(:,[6 5 7 8]); 
        % plot paralell squares
        fill3(sq1(1,:),sq1(2,:),sq1(3,:),color_str,'FaceAlpha',.8);
        fill3(sq2(1,:),sq2(2,:),sq2(3,:),color_str,'FaceAlpha',.8);
      end
      hold off;
    end % EOF plot

    function obstacle_str = dotmap_str(obj,last)
      rpvert = vertices(obj);
      % put the vertices of the object in the right order for the template
      [~,sortidx] = sort(rpvert(3,:));
      rpvert = rpvert(:,sortidx);
      rpvert = rpvert(:,[2 1 3 4 6 5 7 8]); % [A B C D E F G H] 
      % rotate and translate the vertices to match the arena
      RAI = [-0.9965 -0.0832 0;0.0832 -0.9965 0;0 0 1.0000];
      arena_translation = [10.37 -19.58 5.15]';
      rpvert = pagemtimes(rpvert',RAI')';
      rpvert = rpvert - arena_translation;
      % create a string that is the template replaced with the values in rpvert
      if last
        fid  = fopen('last_obstacle_template.txt','r');
      else
        fid  = fopen('obstacle_template.txt','r');
      end
      obstacle_str=fread(fid,'*char')';
      fclose(fid);
      alph = 'ABCDEFGH';
      for i = 1:8
        ostr = [alph(i) 'X,' alph(i) 'Y,' alph(i) 'Z'];
        nstr = sprintf('%.2f,%.2f,%2.2f',rpvert(:,i)');
        obstacle_str = strrep(obstacle_str,ostr,nstr);
      end
    end % EOF dotmap_str

  end

end % classdef rect_prism