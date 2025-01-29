classdef balloon

  properties (Constant)
    xlen = .4; % scalar
    ylen = .4; % scalar
    zlen = .5; % scalar
  end

  properties
    center_point; % 3-by-1
    color; % hex color code or matlab predefined color string
  end

  methods

    function obj = balloon(center_point,color_str)
      % constructor
      obj.center_point = center_point;
      obj.color = color_str;
    end % EOF constructor

    function vertex_points = vertices(obj)
      % find all possible combos of -1 or 1 (each col vec holds one possibility)
      binary_combos = ff2n(3)'*2-1;
      % compute vertecies coordinates
      edge_lengths = [obj.xlen obj.ylen obj.zlen]';
      vertex_points = obj.center_point + binary_combos.*(0.5*edge_lengths);
    end % EOF vertices

    function plot(obj)
      color_str = obj.color;
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

    function balloon_str = position_str(obj)
      balloon_pos = q1ToArena(obj.center_point);
      balloon_str = sprintf('%s_balloon_position: [%.2f,%.2f,%.2f]',...
        obj.color,balloon_pos);
    end % EOF position_str

  end

end % classdef balloon