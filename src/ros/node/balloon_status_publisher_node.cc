#include "balloon_status_publisher_node.h"

#include <chrono>
#include <thread>

namespace game_engine {
BalloonStatusPublisherNode::BalloonStatusPublisherNode(
    const std::string& topic) {
  this->publisher_guard_ =
      std::make_shared<PublisherGuard<mg_msgs::BalloonStatus>>(topic);
}

void BalloonStatusPublisherNode::Publish(const BalloonStatus& balloon_status) {
  mg_msgs::BalloonStatus msg;
  msg.header.frame_id = "world";
  msg.header.stamp.sec = 0;
  msg.header.stamp.nsec = 0;
  msg.popped.data = balloon_status.popped;
  msg.popper.data = balloon_status.popper;
  msg.pop_time.data = balloon_status.pop_time;
  msg.set_start.data = balloon_status.set_start;

  this->publisher_guard_->Publish(msg);
}

void BalloonStatusPublisherNode::WaitForConnection() {
  while (publisher_guard_->NodeConnect() < 3) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  std::cout << "BalloonStatusPublisherNode fully connected." << std::endl;
}
}  // namespace game_engine
