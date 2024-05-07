#pragma once

#include <ros/ros.h>
#include <visualization_msgs/Marker.h>

#include <memory>

#include "publisher_guard.h"

namespace game_engine {
// ROS node that publishes marker messages
class MarkerPublisherNode {
 private:
  // A publisher guard ensures that the Publish() function may be called in
  // a thread-safe manner
  std::shared_ptr<PublisherGuard<visualization_msgs::Marker>> publisher_guard_;

 public:
  MarkerPublisherNode(const std::string& topic);
  void Publish(const visualization_msgs::Marker& msg);
};
}  // namespace game_engine
