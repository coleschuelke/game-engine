

#pragma once

#include <ros/ros.h>

#include <functional>
#include <memory>
#include <string>

#include "balloon_status.h"
#include "mg_msgs/BalloonStatus.h"
#include "publisher_guard.h"
#include "quad_state.h"

namespace game_engine {
class BalloonStatusPublisherNode {
 public:
  BalloonStatusPublisherNode(const std::string& topic);
  void Publish(const BalloonStatus& balloon_status);
  size_t GetNumConnections() const { return publisher_guard_->NodeConnect(); }

 private:
  // A publisher guard ensures that the Publish() function may be called in a
  // thread-safe manner
  std::shared_ptr<PublisherGuard<mg_msgs::BalloonStatus>> publisher_guard_;
};
}  // namespace game_engine
