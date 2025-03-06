#include "occupancy_grid2d.h"

namespace game_engine {
OccupancyGrid2D::~OccupancyGrid2D() {
  if (false == heap_allocated_) {
    return;
  }

  // Deallocate memory on heap
  for (size_t idx = 0; idx < size_y_; ++idx) {
    std::free(data_[idx]);
  }
  std::free(data_);
}

size_t OccupancyGrid2D::SizeX() const { return size_x_; }

size_t OccupancyGrid2D::SizeY() const { return size_y_; }

bool OccupancyGrid2D::IsOccupied(const size_t x, const size_t y) const {
  return data_[y][x];
}

const bool** OccupancyGrid2D::Data() const {
  return const_cast<const bool**>(data_);
}

bool OccupancyGrid2D::LoadFromFile(const std::string& file_path) {
  std::ifstream f(file_path);
  if (!f.is_open()) {
    std::cerr << "OccupancyGrid2D::LoadFromFile: File could not be opened."
              << std::endl;
    return false;
  }

  f >> size_y_;
  f >> size_x_;

  // Allocate memory on the heap for the file
  data_ = reinterpret_cast<bool**>(std::malloc(size_y_ * sizeof(bool*)));
  for (size_t idx = 0; idx < size_y_; ++idx) {
    data_[idx] = reinterpret_cast<bool*>(std::malloc(size_x_ * sizeof(bool)));
  }
  heap_allocated_ = true;

  // Read in file
  for (size_t row = 0; row < size_y_; ++row) {
    for (size_t col = 0; col < size_x_; ++col) {
      f >> data_[row][col];
    }
  }

  f.close();
  return true;
}

bool OccupancyGrid2D::LoadFromMap(const Map2D& map, const double sample_delta,
                                  const double safety_bound) {
  double min_x{std::numeric_limits<double>::max()},
      min_y{std::numeric_limits<double>::max()},
      max_x{std::numeric_limits<double>::min()},
      max_y{std::numeric_limits<double>::min()};

  for (const Point2D& vertex : map.Boundary().Vertices()) {
    if (vertex.x() < min_x) {
      min_x = vertex.x();
    }
    if (vertex.y() < min_y) {
      min_y = vertex.y();
    }
    if (vertex.x() > max_x) {
      max_x = vertex.x();
    }
    if (vertex.y() > max_y) {
      max_y = vertex.y();
    }
  }

  size_y_ = std::ceil((max_y - min_y) / sample_delta) + 1;
  size_x_ = std::ceil((max_x - min_x) / sample_delta) + 1;

  // Allocate memory on the heap for the file
  data_ = reinterpret_cast<bool**>(std::malloc(size_y_ * sizeof(bool*)));
  for (size_t idx = 0; idx < size_y_; ++idx) {
    data_[idx] = reinterpret_cast<bool*>(std::malloc(size_x_ * sizeof(bool)));
  }
  heap_allocated_ = true;

  const Map2D inflated_map = map.Inflate(safety_bound);

  // Read in file
  for (size_t row = 0; row < size_y_; ++row) {
    for (size_t col = 0; col < size_x_; ++col) {
      const Point2D p(min_x + col * sample_delta, min_y + row * sample_delta);
      // True indicates occupied, false indicates free
      data_[row][col] =
          !inflated_map.Contains(p) || !inflated_map.IsFreeSpace(p);
    }
  }

  return false;
}

bool OccupancyGrid2D::LoadFromBuffer(const bool** buffer, const size_t size_x,
                                     const size_t size_y) {
  size_y_ = size_y;
  size_x_ = size_x;

  // Allocate memory on the heap for the file
  data_ = reinterpret_cast<bool**>(std::malloc(size_y_ * sizeof(bool*)));
  for (size_t idx = 0; idx < size_y_; ++idx) {
    data_[idx] = reinterpret_cast<bool*>(std::malloc(size_x_ * sizeof(bool)));
    std::memcpy(&data_[idx], &buffer[idx], size_x_ * sizeof(bool));
  }
  heap_allocated_ = true;

  return true;
}

Graph2D OccupancyGrid2D::AsGraph() const {
  // Build a 2D array of nodes
  std::shared_ptr<Node2D> node_grid[size_y_][size_x_];
  for (size_t row = 0; row < size_y_; ++row) {
    for (size_t col = 0; col < size_x_; ++col) {
      node_grid[row][col] =
          std::make_shared<Node2D>(Eigen::Matrix<int, 2, 1>(row, col));
    }
  }

  // Convert node grid to directed edges and add to graph
  std::vector<DirectedEdge2D> edges;
  for (int row = 0; row < size_y_; ++row) {
    for (int col = 0; col < size_x_; ++col) {
      // If current node is unreachable, pass
      if (true == IsOccupied(col, row)) {
        continue;
      }

      constexpr double ADJACENT_COST = 1.0;
      constexpr double DIAGONAL_COST = std::sqrt(2);
      // Else, create paths from nearby nodes into this one.  There are 8 node
      // surrounding the current node.
      if (row - 1 >= 0) {
        edges.emplace_back(node_grid[row - 1][col], node_grid[row][col],
                           ADJACENT_COST);
      }
      if (col - 1 >= 0) {
        edges.emplace_back(node_grid[row][col - 1], node_grid[row][col],
                           ADJACENT_COST);
      }

      if (row + 1 < size_y_) {
        edges.emplace_back(node_grid[row + 1][col], node_grid[row][col],
                           ADJACENT_COST);
      }
      if (col + 1 < size_x_) {
        edges.emplace_back(node_grid[row][col + 1], node_grid[row][col],
                           ADJACENT_COST);
      }

      if (row - 1 >= 0 && col - 1 >= 0) {
        edges.emplace_back(node_grid[row - 1][col - 1], node_grid[row][col],
                           DIAGONAL_COST);
      }
      if (row - 1 >= 0 && col + 1 < size_x_) {
        edges.emplace_back(node_grid[row - 1][col + 1], node_grid[row][col],
                           DIAGONAL_COST);
      }
      if (row + 1 < size_y_ && col - 1 >= 0) {
        edges.emplace_back(node_grid[row + 1][col - 1], node_grid[row][col],
                           DIAGONAL_COST);
      }
      if (row + 1 < size_y_ && col + 1 < size_x_) {
        edges.emplace_back(node_grid[row + 1][col + 1], node_grid[row][col],
                           DIAGONAL_COST);
      }
    }
  }

  return Graph2D(edges);
}

}  // namespace game_engine
