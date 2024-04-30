#pragma once

#include <ros/ros.h>

#include <functional>
#include <memory>
#include <string>

#include "goal_status.h"
#include "mg_msgs/GoalStatus.h"

namespace game_engine {
class GoalStatusSubscriberNode {
 public:
  std::shared_ptr<GoalStatus> goal_status_;
  GoalStatusSubscriberNode(const std::string& topic,
                           std::shared_ptr<GoalStatus> goal_status);
  size_t GetNumConnections() const { return subscriber_.getNumPublishers(); }

 private:
  ros::NodeHandle node_handle_;
  ros::Subscriber subscriber_;
  void SubscriberCallback(const mg_msgs::GoalStatus& msg);
};
}  // namespace game_engine
