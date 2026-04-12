#pragma once

#include <chrono>

#include "autonomy_protocol.h"
#include "autonomy_protocol_visualizer.h"
#include "graph.h"
#include "occupancy_grid3d.h"
#include "polynomial_sampler.h"
#include "polynomial_solver.h"

namespace game_engine {
class StudentAutonomyProtocol : public AutonomyProtocol {
private:
  // The OccupancyGrid3D class, which has already been written for you, divides
  // the arena space into cells of a size you specify in cell_size below.  It
  // has convenient functions for determining whether a cell is occupied and for
  // translating between a cell's grid index triple and its 3D cell center
  // position, in meters.  Have a look at its header file
  // game-engine/src/environment/occupancy_grid3d.h to learn more about its API.
  OccupancyGrid3D occupancy_grid_;
  Graph3D graph_of_arena_;

  // AutonomyProtocolVisualizer is a class that supports visualizing paths,
  // curves, points, and whole trajectories in the RVIZ display of the arena,
  // which will aid your algorithm development.  Have a look at
  // autonomy_protocol_visualizer.h to see its API.
  AutonomyProtocolVisualizer visualizer_;

  // Length of one side of the occupancy grid's cubic cells, in meters
  static constexpr double cell_size_ = 0.2;
  // Length by which obstacles are inflated to provide a safety margin, in
  // meters
  double safety_margin_ = 0.35;
  bool first_time_ = true;
  bool halt_ = false;
  Eigen::Vector3d start_pos_, halt_pos_;

public:
  StudentAutonomyProtocol(
      const std::vector<std::string> &friendly_names,
      const std::vector<std::string> &enemy_names,
      const std::shared_ptr<GameSnapshot> snapshot,
      const std::shared_ptr<TrajectoryWardenClient> trajectory_warden_client,
      const std::shared_ptr<PreSubmissionTrajectoryVetter> prevetter,
      const Map3D &map3d,
      const std::shared_ptr<BalloonStatus> red_balloon_status,
      const std::shared_ptr<Eigen::Vector3d> red_balloon_position,
      const std::shared_ptr<BalloonStatus> blue_balloon_status,
      const std::shared_ptr<Eigen::Vector3d> blue_balloon_position,
      const Eigen::Vector3d &goal_position, const WindIntensity &wind_intensity,
      const AutonomyProtocolVisualizer visualizer)
      : AutonomyProtocol(friendly_names, enemy_names, snapshot,
                         trajectory_warden_client, prevetter, map3d,
                         red_balloon_status, red_balloon_position,
                         blue_balloon_status, blue_balloon_position,
                         goal_position, wind_intensity, visualizer) {}

  std::unordered_map<std::string, Trajectory> UpdateTrajectories() override;
};
} // namespace game_engine
