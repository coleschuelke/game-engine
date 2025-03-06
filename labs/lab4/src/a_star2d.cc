#include "a_star2d.h"

#include <queue>

namespace game_engine {
// Anonymous namespace. Put any file-local functions and variables within this
// scope
namespace {
// The NodeWrapper object can be used to form a linked list representing a path.
struct NodeWrapper {
  // Pointer to Node2D object
  std::shared_ptr<Node2D> node_ptr;
  // Cost to reach the node pointed to by node_ptr
  double cost;
  // Parent NodeWrapper object
  std::shared_ptr<struct NodeWrapper> parent;
  // Heuristic value from this node to end node
  double heuristic;

  // Equality operator
  bool operator==(const NodeWrapper& other) const {
    return *(this->node_ptr) == *(other.node_ptr);
  }
};

// Compares the values of two NodeWrapper pointers.  Necessary for the priority
// queue.
bool NodeWrapperPtrCompare(const std::shared_ptr<NodeWrapper>& lhs,
                           const std::shared_ptr<NodeWrapper>& rhs) {
  return lhs->cost + lhs->heuristic > rhs->cost + rhs->heuristic;
}

///////////////////////////////////////////////////////////////////
// EXAMPLE HEURISTIC FUNCTION
// YOU WILL NEED TO MODIFY THIS OR WRITE YOUR OWN FUNCTION
///////////////////////////////////////////////////////////////////
double heuristic(const std::shared_ptr<Node2D>& current_ptr,
                 const std::shared_ptr<Node2D>& end_ptr) {
  return 0;
}
}  // namespace

PathInfo AStar2D::Run(const Graph2D& graph,
                      const std::shared_ptr<Node2D> start_ptr,
                      const std::shared_ptr<Node2D> end_ptr) {
  using NodeWrapperPtr = std::shared_ptr<NodeWrapper>;

  ///////////////////////////////////////////////////////////////////
  // SETUP
  // DO NOT MODIFY THIS
  ///////////////////////////////////////////////////////////////////
  Timer timer;
  timer.Start();

  // Use these data structures
  std::priority_queue<
      NodeWrapperPtr, std::vector<NodeWrapperPtr>,
      std::function<bool(const NodeWrapperPtr&, const NodeWrapperPtr&)>>
      nodes_to_explore(NodeWrapperPtrCompare);

  std::vector<NodeWrapperPtr> explored_nodes;

  ///////////////////////////////////////////////////////////////////
  // YOUR WORK GOES BELOW
  // SOME EXAMPLE CODE PROVIDED
  ///////////////////////////////////////////////////////////////////

  // Create a NodeWrapperPtr
  NodeWrapperPtr nw_ptr = std::make_shared<NodeWrapper>();
  nw_ptr->parent = nullptr;
  nw_ptr->node_ptr = start_ptr;
  nw_ptr->cost = 0;
  nw_ptr->heuristic = heuristic(start_ptr, end_ptr);
  nodes_to_explore.push(nw_ptr);

  // Create a PathInfo
  PathInfo path_info;
  path_info.details.num_nodes_explored = 0;
  path_info.details.path_length = 0;
  path_info.details.path_cost = 0;
  path_info.details.run_time = timer.Stop();
  path_info.path = {};

  // Push an example node to PathInfo.path.  Note that in your implementation,
  // path_info.path (which, as you can see in path_info.h, is just a vector of
  // pointers to Node2D objects), should contain the sequence of nodes
  // traversed from start_ptr to end_ptr.
  path_info.path.push_back(nw_ptr->node_ptr);

  // You must return a PathInfo
  return path_info;
}

}  // namespace game_engine
