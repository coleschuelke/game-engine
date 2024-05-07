#include "balloon_view.h"

namespace game_engine {
std::vector<visualization_msgs::Marker> BalloonView::Markers() const {
  visualization_msgs::Marker marker;
  marker.header.frame_id = options_.frame_id;
  marker.id = unique_id_;
  marker.ns = "Balloon";
  marker.type = visualization_msgs::Marker::MESH_RESOURCE;
  marker.action = action_;
  marker.scale.x = 12.0f;
  marker.scale.y = 12.0f;
  marker.scale.z = 12.0f;
  marker.pose.position.x = balloon_position_.x();
  marker.pose.position.y = balloon_position_.y();
  // Shift z value for better alignment in visualization
  marker.pose.position.z = balloon_position_.z() - 0.4;
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

uint32_t BalloonView::GenerateUniqueId() {
  static std::mutex mtx;
  static uint32_t id = 0;

  std::lock_guard<std::mutex> lock(mtx);
  id++;
  return id;
}
}  // namespace game_engine
