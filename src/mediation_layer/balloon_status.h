#pragma once

#include <Eigen/Dense>
#include <chrono>
#include <string>

namespace game_engine {
// Plain-old-data structure containing information regarding a balloon's
// status
struct BalloonStatus {
  // Whether the balloon has been popped
  bool popped = false;
  // Which quad popped the balloon
  std::string popper = "null";
  // The time at which the balloon was popped, in seconds since initialization
  // in response to an asserted set_start.
  double pop_time;
  // When asserted, indicates that the timer governing balloon teleportation
  // should be started if not already started.
  bool set_start;
};
}  // namespace game_engine
