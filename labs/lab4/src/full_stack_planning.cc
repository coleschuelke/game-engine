#include <cstdlib>
#include <vector>
#include <iostream>

#include "a_star2d.h"
#include "gnuplot-iostream.h"
#include "gui2d.h"
#include "occupancy_grid2d.h"
#include "path_info.h"
#include "polynomial_sampler.h"
#include "polynomial_solver.h"

using namespace game_engine;

int main(int argc, char **argv)
{
  if (argc != 6)
  {
    std::cerr << "Usage: ./full_stack_planning occupancy_grid_file row1 col1 "
                 "row2 col2"
              << std::endl;
    return EXIT_FAILURE;
  }

  // Parsing input
  const std::string occupancy_grid_file = argv[1];
  const std::shared_ptr<Node2D> start_ptr = std::make_shared<Node2D>(
      Eigen::Vector2i(std::stoi(argv[2]), std::stoi(argv[3])));
  const std::shared_ptr<Node2D> end_ptr = std::make_shared<Node2D>(
      Eigen::Vector2i(std::stoi(argv[4]), std::stoi(argv[5])));

  // Load an occupancy grid from a file
  OccupancyGrid2D occupancy_grid;
  occupancy_grid.LoadFromFile(occupancy_grid_file);

  // Transform an occupancy grid into a graph
  const Graph2D graph = occupancy_grid.AsGraph();

  /////////////////////////////////////////////////////////////////////////////
  // RUN A STAR
  // TODO: Run your A* implementation over the graph and nodes defined above.
  //       This section is intended to be more free-form. Using previous
  //       problems and examples, determine the correct commands to complete
  //       this problem. You may want to take advantage of some of the plotting
  //       and graphing utilities in previous problems to check your solution on
  //       the way.
  /////////////////////////////////////////////////////////////////////////////
  AStar2D a_star;
  PathInfo astar_path = a_star.Run(graph, start_ptr, end_ptr);
  astar_path.details.Print();
  Gui2D gui;
  gui.LoadOccupancyGrid(&occupancy_grid);
  gui.LoadPath(astar_path.path);
  gui.Display("AStar");

  /////////////////////////////////////////////////////////////////////////////
  // RUN THE POLYNOMIAL PLANNER
  // TODO: Convert the A* solution to a problem the polynomial solver can
  //       solve. Solve the polynomial problem, sample the solution, figure out
  //       a way to export it to Matlab.
  /////////////////////////////////////////////////////////////////////////////

  // Convert the path to waypoints
  std::vector<double> times;

  std::vector<p4::NodeEqualityBound> node_equality_bounds;

  // Constrain the starting velocity and acceleration
  node_equality_bounds.push_back(p4::NodeEqualityBound(0, 0, 1, 0));
  node_equality_bounds.push_back(p4::NodeEqualityBound(1, 0, 1, 0));
  node_equality_bounds.push_back(p4::NodeEqualityBound(0, 0, 2, 0));
  node_equality_bounds.push_back(p4::NodeEqualityBound(1, 0, 2, 0));

  // Push each node as a waypoint at 1 second increments
  for (size_t i = 0; i < astar_path.path.size(); ++i)
  {
    times.push_back(i);
    node_equality_bounds.push_back(p4::NodeEqualityBound(0, i, 0, astar_path.path[i]->Data().y()));
    node_equality_bounds.push_back(p4::NodeEqualityBound(1, i, 0, -1 * astar_path.path[i]->Data().x()));
  }

  // Configure the polynomial solver
  p4::PolynomialSolver::Options solver_options;
  solver_options.num_dimensions = 2;
  solver_options.polynomial_order = 8;
  solver_options.continuity_order = 4;
  solver_options.derivative_order = 2;

  osqp_set_default_settings(&solver_options.osqp_settings);
  solver_options.osqp_settings.polish = true;
  solver_options.osqp_settings.verbose = false;

  // Create the solver
  p4::PolynomialSolver solver(solver_options);
  const p4::PolynomialSolver::Solution path = solver.Run(
      times,
      node_equality_bounds,
      {},
      {});

  // Sample and plot
  {
    p4::PolynomialSampler::Options pos_sampler_options;
    pos_sampler_options.frequency = 200;
    pos_sampler_options.derivative_order = 0;
    p4::PolynomialSampler::Options vel_sampler_options;
    vel_sampler_options.frequency = 200;
    vel_sampler_options.derivative_order = 1;
    p4::PolynomialSampler::Options acc_sampler_options;
    acc_sampler_options.frequency = 200;
    acc_sampler_options.derivative_order = 2;

    p4::PolynomialSampler pos_sampler(pos_sampler_options);
    Eigen::MatrixXd pos_samples = pos_sampler.Run(times, path);
    p4::PolynomialSampler vel_sampler(vel_sampler_options);
    Eigen::MatrixXd vel_samples = vel_sampler.Run(times, path);
    p4::PolynomialSampler acc_sampler(acc_sampler_options);
    Eigen::MatrixXd acc_samples = acc_sampler.Run(times, path);

    // Write the results to a csv
    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");
    std::ofstream pos_file("/home/aeronaut/Workspace/results/position.csv");
    pos_file << pos_samples.format(CSVFormat);
    std::ofstream vel_file("/home/aeronaut/Workspace/results/velocity.csv");
    vel_file << vel_samples.format(CSVFormat);
    std::ofstream acc_file("/home/aeronaut/Workspace/results/acceleration.csv");
    acc_file << acc_samples.format(CSVFormat);

    // Plotting setup
    std::vector<double> t_hist, x_hist, y_hist;
    for (size_t time_idx = 0; time_idx < pos_samples.cols(); ++time_idx)
    {
      t_hist.push_back(pos_samples(0, time_idx));
      x_hist.push_back(pos_samples(1, time_idx));
      y_hist.push_back(pos_samples(2, time_idx));
    }

    // gnu-iostream plotting library
    // Utilizes gnuplot commands with a nice stream interface
    {
      Gnuplot gp;
      gp << "plot '-' using 1:2 with lines title 'Trajectory'" << std::endl;
      gp.send1d(boost::make_tuple(x_hist, y_hist));
      gp << "set grid" << std::endl;
      gp << "set xlabel 'X'" << std::endl;
      gp << "set ylabel 'Y'" << std::endl;
      gp << "replot" << std::endl;
    }
    {
      Gnuplot gp;
      gp << "plot '-' using 1:2 with lines title 'X-Profile'" << std::endl;
      gp.send1d(boost::make_tuple(t_hist, x_hist));
      gp << "set grid" << std::endl;
      gp << "set xlabel 'Time (s)'" << std::endl;
      gp << "set ylabel 'X-Profile'" << std::endl;
      gp << "replot" << std::endl;
    }
    {
      Gnuplot gp;
      gp << "plot '-' using 1:2 with lines title 'Y-Profile'" << std::endl;
      gp.send1d(boost::make_tuple(t_hist, y_hist));
      gp << "set grid" << std::endl;
      gp << "set xlabel 'Time (s)'" << std::endl;
      gp << "set ylabel 'Y-Profile'" << std::endl;
      gp << "replot" << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
