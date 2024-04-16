#pragma once

#include <chrono>

#include "autonomy_protocol.h"
#include "autonomy_protocol_visualizer.h"
#include "graph.h"
#include "occupancy_grid3d.h"
#include "polynomial_sampler.h"
#include "polynomial_solver.h"

namespace game_engine {
// The ExampleAutonomyProtocol is a class that demonstrates how to create a
// trajectory for a quadcopter to follow.
class ExampleAutonomyProtocol : public AutonomyProtocol {
 public:
  // This is the class constructor. This constructor runs when a new object of
  // type ExampleAutonomyProtocol is created. It receives a set of input
  // parameters (enclosed in the parentheses), and uses these input parameters
  // to initialize some other objects. In our case, we initialize the
  // ExampleAutonomyProtocol's parent class--AutonomyProtocol--as well as the
  // occupancy grid and arena graph.
  ExampleAutonomyProtocol(
      const std::vector<std::string>& friendly_names,
      const std::vector<std::string>& enemy_names,
      const std::shared_ptr<GameSnapshot> snapshot,
      const std::shared_ptr<TrajectoryWardenClient> trajectory_warden_client,
      const std::shared_ptr<PreSubmissionTrajectoryVetter> prevetter,
      const Map3D& map3d,
      const std::shared_ptr<BalloonStatus> red_balloon_status,
      const std::shared_ptr<Eigen::Vector3d> red_balloon_position,
      const std::shared_ptr<BalloonStatus> blue_balloon_status,
      const std::shared_ptr<Eigen::Vector3d> blue_balloon_position,
      const Eigen::Vector3d& goal_position, const WindIntensity& wind_intensity,
      const AutonomyProtocolVisualizer visualizer)
      : AutonomyProtocol(friendly_names, enemy_names, snapshot,
                         trajectory_warden_client, prevetter, map3d,
                         red_balloon_status, red_balloon_position,
                         blue_balloon_status, blue_balloon_position,
                         goal_position, wind_intensity, visualizer) {
    occupancy_grid_.LoadFromMap(map3d_, cell_size_, safety_margin_);
    // Once you have built a 3D version of A*, you can run it on graph_of_arena.
    // Important hint: graph_of_arena will be large (many nodes) if cell_size is
    // small.  If you naively extend your AStar2D function to 3D, it will run
    // slowly on a large graph_of_arena.  Of course, you can keep the graph
    // small by choosing a large cell_size, but this will prevent you from
    // finding paths through areas with tight obstacle spacing.  A better
    // approach is to speed up A*.  Note that its most costly operation is the
    // search through the container of explored nodes to determine whether a
    // candidate node has already been explored.  You can speed up this search
    // dramatically by making this container an std::unordered_set rather than
    // an std::vector.  See
    // https://www.educative.io/edpresso/unordered-sets-in-cpp.
    graph_of_arena_ = occupancy_grid_.AsGraph();
  }

  std::unordered_map<std::string, Trajectory> UpdateTrajectories() override;

  void BuildHistoryVectors(std::vector<double>& x_hist,
                           std::vector<double>& y_hist,
                           std::vector<double>& z_hist,
                           const std::vector<double>& timestamps,
                           const p4::PolynomialSolver::Solution& trajectory,
                           const p4::PolynomialSampler& sampler_options);

 private:
  // Set the duration of the example trajectory.
  static constexpr int duration_sec_ = 30;

  // The OccupancyGrid3D class, which has already been written for you, divides
  // the arena space into cells of a size you specify (see cell_size below).  It
  // has convenient functions for determining whether a cell is occupied and for
  // translating between a cell's grid index triple and its 3d cell center
  // position, in meters.  Have a look at
  // game-engine/src/environment/occupancy_grid3d.h to see this class's API.
  // class offers.
  OccupancyGrid3D occupancy_grid_;
  Graph3D graph_of_arena_;

  // AutonomyProtocolVisualizer is a class that supports visualizing paths,
  // curves, points, and whole trajectories in the RVIZ display of the arena to
  // aid in your algorithm development.  Have a look at
  // autonomy_protocol_visualizer.h to see this class's API.
  bool first_time_ = true;
  bool halt_ = false;
  Eigen::Vector3d start_pos_;

  // The length of one side of the occupancy grid's cubic cells, in meters
  static constexpr double cell_size_ = 0.2;
  // Length by which obstacles are inflated to provide a safety margin, in
  // meters
  double safety_margin_ = 0.35;
};
}  // namespace game_engine
