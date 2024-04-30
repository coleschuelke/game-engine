#ifndef BALLOON_POSITION_SUBSCRIBER_NODE_H
#define BALLOON_POSITION_SUBSCRIBER_NODE_H

#include <geometry_msgs/Point.h>
#include <ros/ros.h>

#include <Eigen/StdVector>
#include <functional>
#include <memory>
#include <string>

namespace game_engine {
class BalloonPositionSubscriberNode {
 public:
  // Pointer to balloon status. No multiple-access protections.
  std::shared_ptr<Eigen::Vector3d> balloon_position_;
  BalloonPositionSubscriberNode(
      const std::string& topic,
      std::shared_ptr<Eigen::Vector3d> balloon_position);

 private:
  ros::NodeHandle node_handle_;
  ros::Subscriber subscriber_;
  void SubscriberCallback(const geometry_msgs::Point& msg);
};
}  // namespace game_engine

#endif
