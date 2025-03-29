#include "gui2d.h"

namespace game_engine {
namespace {
struct PolygonView {
  struct Options {
    bool fill{false};
    std::string fill_color{"black"};
    bool transparent{false};
    std::string line_color{"black"};
    Options() {}
  };

  Polygon polygon_;
  Options options_;

  PolygonView(const Polygon& polygon, const Options& options = Options())
      : options_(options), polygon_(polygon) {}

  std::vector<Gui2D::CommandUnit> Load() const {
    std::vector<Gui2D::CommandUnit> command_units;

    if (polygon_.Vertices().empty()) {
      return command_units;
    }

    std::vector<std::pair<double, double>> data;
    data.reserve(polygon_.Vertices().size() + 1);
    for (const Point2D& vertex : polygon_.Vertices()) {
      data.emplace_back(vertex.x(), vertex.y());
    }
    data.emplace_back(polygon_.Vertices()[0].x(), polygon_.Vertices()[0].y());

    // For a polygon with a difference color for fill and lines, must draw
    // the polygon twice: once filled and one not filled
    if (options_.fill) {
      std::string command = "'-' ";
      command +=
          "with filledcurves fillcolor rgb '" + options_.fill_color + "' ";
      command += options_.transparent ? "fillstyle transparent solid 0.3 " : "";
      command += ",";
      command_units.emplace_back(command, data);
    }

    {
      std::string command = "'-' ";
      command += "with lines linetype rgb '" + options_.line_color + "' ";
      command += ",";
      command_units.emplace_back(command, data);
    }

    return command_units;
  };
};

struct PathView {
  struct Options {
    std::string line_color{"black"};
    Options() {}
  };

  std::vector<std::shared_ptr<Node2D>> path_;
  Options options_;

  PathView(const std::vector<std::shared_ptr<Node2D>>& path,
           const Options& options = Options())
      : options_(options), path_(path) {}

  std::vector<Gui2D::CommandUnit> Load() const {
    std::vector<Gui2D::CommandUnit> command_units;

    std::vector<std::pair<double, double>> data;
    data.reserve(path_.size());
    for (const std::shared_ptr<Node2D>& node : path_) {
      // This is flipped because of the x,y -> row,col correspondence
      data.emplace_back(node->Data().y(), node->Data().x());
    }

    std::string command = "'-' ";
    // command += "with lines linecolor rgb '" + options_.line_color + "'
    // linetype 1 linewidth 2";
    command +=
        "with linespoints linecolor rgb '" + options_.line_color +
        "' linetype 1 linewidth 2 pointtype 7 pointinterval -1 pointsize 1.5";
    command += ",";
    command_units.emplace_back(command, data);

    return command_units;
  };
};

struct OccupancyGridView {
  struct Options {
    Options() {}
  };

  const OccupancyGrid2D* occupancy_grid_;
  const Options options_;

  OccupancyGridView(const OccupancyGrid2D* occupancy_grid,
                    const Options options = Options())
      : options_(options), occupancy_grid_(occupancy_grid) {}

  std::vector<Gui2D::CommandUnit> Load() const {
    std::vector<Gui2D::CommandUnit> command_units;

    const double num_rows = occupancy_grid_->SizeY();
    const double num_cols = occupancy_grid_->SizeX();

    for (int yy = 0; yy < num_rows; ++yy) {
      for (int xx = 0; xx < num_cols; ++xx) {
        PolygonView::Options poly_options;
        poly_options.fill = occupancy_grid_->IsOccupied(xx, yy);
        poly_options.fill_color = "red";
        poly_options.transparent = true;
        Polygon poly;
        poly.ConstructFromPoints(
            {Point2D(xx - 0.5, yy - 0.5), Point2D(xx + 0.5, yy - 0.5),
             Point2D(xx + 0.5, yy + 0.5), Point2D(xx - 0.5, yy + 0.5)});
        std::vector<Gui2D::CommandUnit> cell_command_units =
            PolygonView(poly, poly_options).Load();
        command_units.insert(command_units.end(), cell_command_units.begin(),
                             cell_command_units.end());
      }
    }

    return command_units;
  };
};
}  // namespace

bool Gui2D::LoadPath(const std::vector<std::shared_ptr<Node2D>>& path) {
  PathView::Options options;
  const std::vector<CommandUnit> command_units = PathView(path, options).Load();
  command_units_.insert(command_units_.end(), command_units.begin(),
                        command_units.end());
  return true;
}

bool Gui2D::LoadOccupancyGrid(const OccupancyGrid2D* occupancy_grid) {
  OccupancyGridView::Options options;
  const std::vector<CommandUnit> command_units =
      OccupancyGridView(occupancy_grid, options).Load();
  command_units_.insert(command_units_.end(), command_units.begin(),
                        command_units.end());
  return true;
}

bool Gui2D::Display(const std::string title) {
  // Turn off the key
  gp_ << "set key off" << std::endl;

  // Plot all the data
  gp_ << "plot ";
  for (const CommandUnit cu : command_units_) {
    gp_ << cu.command_ << " ";
  }
  gp_ << std::endl;

  // Send all the data
  for (const CommandUnit cu : command_units_) {
    gp_.send1d(cu.data_);
  }

  // Reverse y-direction to match the occupancy grid file
  gp_ << "set yrange [*:*] reverse" << std::endl;
  // set title of window
  gp_ << "set term qt title \"" << title << '\"' << std::endl;
  // replot
  gp_ << "replot" << std::endl;

  return true;
}
}  // namespace game_engine
