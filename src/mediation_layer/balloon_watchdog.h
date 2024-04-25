#pragma once

#include <Eigen/Core>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "balloon_position_publisher_node.h"
#include "balloon_status_publisher_node.h"
#include "balloon_status_subscriber_node.h"
#include "warden.h"

namespace game_engine {
// BalloonWatchdog monitors quad positions and determines if a quad has popped
// the assigned balloon. If so, it updates the status of the balloon over ROS.
class BalloonWatchdog {
 public:
  struct Options {
    Options() {}
    // Distance from the center of the balloon within which a quad must be to
    // pop the balloon, in meters.
    double pop_distance = 0.30;
  };

  BalloonWatchdog(const Options& options = Options()) : options_(options) {}
  void Run(
      const bool curve_flag, const bool polyFlag,
      std::shared_ptr<BalloonStatusPublisherNode> balloon_status_publisher,
      std::shared_ptr<BalloonStatusSubscriberNode> balloon_status_subscriber,
      std::shared_ptr<BalloonPositionPublisherNode> balloon_position_publisher,
      std::shared_ptr<QuadStateWarden> quad_state_warden,
      const std::vector<std::string>& quad_names,
      Eigen::Vector3d& balloon_position, Eigen::Vector3d& new_balloon_position,
      double max_move_time, std::mt19937& gen, const std::string& topic);
  void Stop();
  void ExternalPopIndicatorCallback(const std_msgs::Bool& msg);

 private:
  volatile std::atomic_bool external_pop_indicator_{false};
  volatile std::atomic_bool ok_{true};
  Options options_;
};
}  // namespace game_engine
