#pragma once

#include <geometry_msgs/Point.h>
#include <visualization_msgs/Marker.h>

#include "marker_publisher_node.h"
#include "occupancy_grid3d.h"
#include "trajectory.h"

// This class supports visualizing paths, curves, points, and whole trajectories
// in the RVIZ display of the arena to aid in autonomy protocol development.
class AutonomyProtocolVisualizer {
  public:
  AutonomyProtocolVisualizer() {}
  void startVisualizing(std::string msg_name);
  void spin();
  void drawPath(std::vector<Eigen::Vector3d> path, int id);
  void drawDot(Eigen::Vector3d pt, int id, bool good);
  void drawBalloonPosition(Eigen::Vector3d pt, std::string balloon_color);
  void drawTrajectory(game_engine::Trajectory traj, int id_offset = 0);
  void drawCurve(std::vector<Eigen::Vector3d> pts, int id, Eigen::Vector3d rgb);

  private:
  ros::Publisher publisher_;
};
