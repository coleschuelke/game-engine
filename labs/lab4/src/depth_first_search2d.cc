#include "depth_first_search2d.h"

#include <stack>

namespace game_engine
{
  // Anonymous namespace. Put any file-local functions and variables within this
  // scope
  namespace
  {
    // The NodeWrapper object can be used to form a linked list representing a path.
    struct NodeWrapper
    {
      // Pointer to Node2D object
      std::shared_ptr<Node2D> node_ptr;
      // Cost to reach the node pointed to by node_ptr
      double cost;
      // Parent NodeWrapper object
      std::shared_ptr<struct NodeWrapper> parent;

      // Equality operator function
      bool operator==(const NodeWrapper &other) const
      {
        return *(this->node_ptr) == *(other.node_ptr);
      }
    };

    // Check for existence in list
    bool contains(const std::vector<std::shared_ptr<NodeWrapper>> &nwptr_list, const std::shared_ptr<NodeWrapper> nw_ptr)
    {
      for (const auto list_node_ptr : nwptr_list)
      {
        if (*list_node_ptr == *nw_ptr)
        {
          return true;
        }
      }
      return false;
    }

    // Assemble the path from the end node
    PathInfo create_path(const std::shared_ptr<NodeWrapper> end_node, const std::vector<std::shared_ptr<NodeWrapper>> &explored, Timer timer)
    {
      PathInfo path;
      path.details.path_cost = end_node->cost;
      path.details.num_nodes_explored = explored.size();
      std::shared_ptr<NodeWrapper>
          parent = end_node->parent;
      path.path.push_back(end_node->node_ptr);
      while (parent != nullptr)
      {
        path.path.push_back(parent->node_ptr);
        parent = parent->parent;
      }
      std::reverse(path.path.begin(), path.path.end());
      path.details.path_length = path.path.size();
      path.details.run_time = timer.Stop();

      return path;
    }

    // Wrap a node
    std::shared_ptr<NodeWrapper> wrap_node(const std::shared_ptr<NodeWrapper> parent, const std::shared_ptr<Node2D> node, const double cost)
    {
      auto wrapped_node = std::make_shared<NodeWrapper>();
      wrapped_node->parent = parent;
      wrapped_node->node_ptr = node;
      wrapped_node->cost = cost;

      return wrapped_node;
    }
  } // namespace

  PathInfo DepthFirstSearch2D::Run(const Graph2D &graph,
                                   const std::shared_ptr<Node2D> start_ptr,
                                   const std::shared_ptr<Node2D> end_ptr)
  {
    using NodeWrapperPtr = std::shared_ptr<NodeWrapper>;

    ///////////////////////////////////////////////////////////////////
    // SETUP
    // DO NOT MODIFY THIS
    ///////////////////////////////////////////////////////////////////
    Timer timer;
    timer.Start();

    // Use these data structures
    std::stack<NodeWrapperPtr> nodes_to_explore;
    std::vector<NodeWrapperPtr> explored_nodes;

    ///////////////////////////////////////////////////////////////////
    // YOUR WORK GOES BELOW
    // SOME EXAMPLE CODE PROVIDED
    ///////////////////////////////////////////////////////////////////

    // Wrap the start node with the function
    NodeWrapperPtr start_nwptr = wrap_node(nullptr, start_ptr, 0);

    // Push start node to nodes_to_explore
    nodes_to_explore.push(start_nwptr);

    // while nte not empty
    while (!nodes_to_explore.empty())
    {

      // Pop a node to explore
      const NodeWrapperPtr node_to_explore = nodes_to_explore.top();
      nodes_to_explore.pop();

      // Check for existence in explored_nodes
      if (contains(explored_nodes, node_to_explore))
      {
        continue;
      }

      // Check if we found the end
      if (*(node_to_explore->node_ptr) == *end_ptr) // Not the same memory location, but the same value
      {
        return create_path(node_to_explore, explored_nodes, timer);
      }

      // Explore and add neighbors to explore
      explored_nodes.push_back(node_to_explore);
      for (const auto edge : graph.Edges(node_to_explore->node_ptr))
      {
        const auto source_ptr = edge.Source();
        const auto sink_ptr = edge.Sink();
        const auto cost = edge.Cost();

        NodeWrapperPtr new_nwptr = wrap_node(node_to_explore, sink_ptr, node_to_explore->cost + cost);

        nodes_to_explore.push(new_nwptr);
      }
    }
    // Failed without finding the end node
    PathInfo failed_path = create_path(start_nwptr, explored_nodes, timer);
    return failed_path;
  }

} // namespace game_engine
