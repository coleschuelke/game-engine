#pragma once

#include <ros/ros.h>

#include <functional>
#include <memory>
#include <string>

#include "balloon_status.h"
#include "mg_msgs/BalloonStatus.h"

namespace game_engine {
class BalloonStatusSubscriberNode {
 public:
  std::shared_ptr<BalloonStatus> balloon_status_;
  BalloonStatusSubscriberNode(const std::string& topic,
                              std::shared_ptr<BalloonStatus> balloon_status);
  size_t GetNumConnections() const { return subscriber_.getNumPublishers(); }

 private:
  ros::NodeHandle node_handle_;
  ros::Subscriber subscriber_;
  void SubscriberCallback(const mg_msgs::BalloonStatus& msg);
};
}  // namespace game_engine
