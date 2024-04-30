#include "goal_status_subscriber_node.h"

#include <thread>

namespace game_engine {
GoalStatusSubscriberNode::GoalStatusSubscriberNode(
    const std::string& topic, std::shared_ptr<GoalStatus> goal_status) {
  goal_status_ = goal_status;
  node_handle_ = ros::NodeHandle("/game_engine/");
  subscriber_ = node_handle_.subscribe(
      topic, 1, &GoalStatusSubscriberNode::SubscriberCallback, this);
}

void GoalStatusSubscriberNode::SubscriberCallback(
    const mg_msgs::GoalStatus& msg) {
  Eigen::Vector3d position;
  position[0] = msg.pos.x;
  position[1] = msg.pos.y;
  position[2] = msg.pos.z;

  GoalStatus goal_status{.active = static_cast<bool>(msg.active.data),
                         .reached = static_cast<bool>(msg.reached.data),
                         .scorer = msg.scorer.data,
                         .reach_time = msg.reach_time.data,
                         .position = position,
                         .set_start = static_cast<bool>(msg.set_start.data)};
  *(goal_status_) = goal_status;
}

}  // namespace game_engine
