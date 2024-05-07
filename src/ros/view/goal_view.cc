#include "goal_view.h"

namespace game_engine {
std::vector<visualization_msgs::Marker> GoalView::Markers() const {
  visualization_msgs::Marker marker;
  marker.header.frame_id = options_.frame_id;
  marker.id = unique_id_;
  marker.ns = "Goal";
  marker.type = visualization_msgs::Marker::MESH_RESOURCE;
  marker.action = visualization_msgs::Marker::ADD;
  marker.scale.x = 0.05f;
  marker.scale.y = 0.05f;
  marker.scale.z = 0.05f;
  marker.pose.position.x = goal_position_.x();
  marker.pose.position.y = goal_position_.y();
  marker.pose.position.z = goal_position_.z(); 
  marker.pose.orientation.w = 1.0f;
  marker.pose.orientation.x = 0.0f;
  marker.pose.orientation.y = 0.0f;
  marker.pose.orientation.z = 0.0f;
  marker.color.r = options_.r;
  marker.color.g = options_.g;
  marker.color.b = options_.b;
  marker.color.a = options_.a;
  marker.mesh_resource = options_.mesh_resource;
  marker.mesh_use_embedded_materials = true;

  return {marker};
}

uint32_t GoalView::GenerateUniqueId() {
  static std::mutex mtx;
  static uint32_t id = 0;

  std::lock_guard<std::mutex> lock(mtx);
  id++;
  return id;
}
}  // namespace game_engine
