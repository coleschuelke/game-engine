#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "graph.h"
#include "path_info.h"
#include "timer.h"

namespace game_engine {

struct Dijkstra2D {
  // Students will implement this function
  PathInfo Run(const Graph2D& graph, const std::shared_ptr<Node2D> start_ptr,
               const std::shared_ptr<Node2D> end_ptr);
};
}  // namespace game_engine
