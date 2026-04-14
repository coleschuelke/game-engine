#include "student_autonomy_protocol.h"

namespace game_engine {

void BuildHistoryVectors(const std::vector<double> &timestamps,
                         const p4::PolynomialSolver::Solution &trajectory,
                         const p4::PolynomialSampler &sampler_options,
                         std::vector<double> &x_hist,
                         std::vector<double> &y_hist,
                         std::vector<double> &z_hist) {
  // Use this object to sample the trajectory
  p4::PolynomialSampler sampler(sampler_options);
  const Eigen::MatrixXd samples = sampler.Run(timestamps, trajectory);

  for (size_t i = 0; i < samples.cols(); ++i) {
    x_hist.push_back(samples(1, i));
    y_hist.push_back(samples(2, i));
    z_hist.push_back(samples(3, i));
  }
}

std::unordered_map<std::string, Trajectory>
StudentAutonomyProtocol::UpdateTrajectories() {
  // STUDENTS: Fill out this function.  To make your code manageable, you are
  // encouraged to add to the StudentAutonomyProtocol class whatever data
  // members and member functions you find convenient. You can also add
  // non-member functions to this file.  In fact, feel free to create your own
  // classes in separate files. (You'll need to include the name of the *.cc
  // file for each class you create in the list of SOURCE_FILES in
  // ../CMakeLists.txt.)
  // ============BEGIN THE COPIED CODE==============
  // ===== Access the Quad and Environment States =====

  // Retrieve your quad's name
  const std::string &quad_name = friendly_names_[0];

  // Access the current quad state with the snapshot_ pointer.  You can retrieve
  // its position, velocity, and orientation.
  Eigen::Vector3d current_pos;
  snapshot_->Position(quad_name, current_pos);

  // Obtain current balloon positions and popped states
  const Eigen::Vector3d red_balloon_pos = *red_balloon_position_;
  const Eigen::Vector3d blue_balloon_pos = *blue_balloon_position_;
  const bool red_balloon_popped = red_balloon_status_->popped;
  const bool blue_balloon_popped = blue_balloon_status_->popped;

  // Set some variables on the first call to this function
  if (first_time_) {
    first_time_ = false;
    start_pos_ = current_pos;
    // May need to add something back if halt_pos_ must be set
    halt_pos_ << 1000, 1000, 1000;
    visualizer_.startVisualizing("/game_engine/environment");
  }

  // For now, just pop the blue balloon first, then red balloon
  // Can later impose a criteria
  Eigen::Vector3d target_pos;
  if (!blue_balloon_popped) {
    target_pos = blue_balloon_pos;
  } else if (!red_balloon_popped) {
    target_pos = red_balloon_pos;
  } else {
    target_pos = goal_position_;
    halt_pos_ = goal_position_;
  }

  // Wind intensity will change between maps. Condition actions or parameters on
  // wind intensity.  For example, consider increasing safety_margin_ with wind
  // intensity.
  switch (wind_intensity_) {
  case WindIntensity::Zero:
    // Do something zero-ish
    break;
  case WindIntensity::Mild:
    // Do something mild
    break;
  case WindIntensity::Stiff:
    // Do something stiff
    break;
  case WindIntensity::Intense:
    // Do something intense
    break;
  case WindIntensity::Ludicrous:
    // Do something ludicrous
    break;
  }

  // ===== Handle Rejections from the Mediation Layer =====

  // The following section is an example of how you might handle error codes
  // returned by the Mediation Layer (ML). For example, the default example
  // autonomy protocol initially submits trajectories where the sampling
  // interval is too long: Instead of submitting at 20-ms intervals or shorter,
  // it submits at 40-ms intervals.  The ML rejects the trajectory, returing
  // MediationLayerCode::TimeBetweenPointsExceedsMaxTime.  Below you can see an
  // example of how to handle this code. You can fill out this switch statement
  // with case statements tailored to each possible MediationLayerCode.
  //
  // For more information on handling time in C++ see the section on creating a
  // TrajectoryVector below.
  static std::chrono::milliseconds dt_chrono_u_ = std::chrono::milliseconds(15);
  switch (trajectoryCodeMap_[quad_name].code) {
  case MediationLayerCode::Success:
    // You won't need to do anything in response to Success.
    break;
  case MediationLayerCode::TimeBetweenPointsExceedsMaxTime: {
    // Suppose your AP initially submits a trajectory with a time that exceeds
    // the maximum allowed time between points. You could fix the problem as
    // shown below.  Using std::cerr ensures debugging statements are printed
    // when desired.
    std::cerr << "Replanning trajectory: "
                 "Shortening time between trajectory points.\n"
                 "Value: "
              << trajectoryCodeMap_[quad_name].value << std::endl;
    dt_chrono_u_ = dt_chrono_u_ - std::chrono::milliseconds(15);
    break;
  }
  default:
    // If you want to see a numerical MediationLayerCode value, you can cast
    // and print the code as shown below.
    std::cerr << "MediationLayerCode: "
              << static_cast<int>(trajectoryCodeMap_[quad_name].code) << '\n'
              << "Value: " << trajectoryCodeMap_[quad_name].value << '\n'
              << "Index: " << trajectoryCodeMap_[quad_name].index << std::endl;
  }

  // ===== Pathfinding =====

  // In this section one would typically employ pathfinding algorithms to
  // traverse the graph_of_arena_ and plot a safe path to avoid any obstacles.
  // For now we will assume an empty arena and create a simple linear path.
  //
  // Here we create at least one waypoint per meter from the quad to the red
  // balloon ensuring at least two waypoints, one for the current and one for
  // the goal position.
  double path_length = (target_pos - current_pos).norm();
  double num_wp = floor(path_length) + 2;
  std::vector<Eigen::Vector3d> waypoints;
  for (int i{}; i < num_wp; i++) {
    waypoints.push_back(current_pos +
                        i / (num_wp - 1) * (target_pos - current_pos));
  }

  // Assign waypoint times based on distance (not optimal!)
  std::vector<double> times{};
  double seconds_per_meter = 1; // WARN: could cause issues with acc??
  for (auto wp : waypoints) {
    double distance = abs((wp - current_pos).norm());
    times.push_back(distance * seconds_per_meter);
  }

  // ===== 3D Polynomial Pathplanning and Sampling =====

  // You are already familiar with node equality bounds from lab 4.
  // Recall the syntax:
  //   NodeEqualityBound(dimension_idx, node_idx, derivative_idx, value)
  // The P4 polynomial must start at the current quad position.
  // While it is possible to achive good results with only node equality
  // bounds for each waypoint, using some of the tools introduced below
  // can help in finding a faster trajectory.
  std::vector<p4::NodeEqualityBound> node_equality_bounds = {
      p4::NodeEqualityBound(0, 0, 0, waypoints[0].x()),
      p4::NodeEqualityBound(1, 0, 0, waypoints[0].y()),
      p4::NodeEqualityBound(2, 0, 0, waypoints[0].z()),
  };

  // A Node inequality bound defines a range of acceptable values for a
  // constraint instead of a single value.  The syntax for these is
  //   NodeInequalityBound(dim_idx, node_idx, derivative_idx,l_bound, u_bound)
  // In this example, node inequality bounds are used to constrain the
  // trajectory positions in order to hit the balloons.  These bounds specify
  // 0.2m x 0.2m x 0.2m boxes centered around the balloon centers.
  const double p_bound = 0.1;
  std::vector<p4::NodeInequalityBound> node_inequality_bounds;
  for (size_t w = 1; w < waypoints.size(); w++) {
    for (size_t d{}; d < 3; d++) {
      node_inequality_bounds.push_back(p4::NodeInequalityBound(
          d, w, 0, waypoints[w](d) - p_bound, waypoints[w](d) + p_bound));
    }
  }

  // A segment is defined to be the path connecting two consecutive nodes.  A
  // segment inequality bound the following linear inequality as a constraing
  // on the segment: dot(a,x) < b.  The x vector is the derivative vector, the
  // a vector is its corresponding coefficient vector, and the scalar b is a
  // chosen maximum.  The syntax is:
  //   SegmentInequalityBound(segment_idx, deriv_idx, coeff_vector(a),
  //   value(b)).
  // In this example, velocity is constrained to a magnitude between -1.5 m/s
  // and +1.5 m/s along each axis for every segment in the trajectory.
  const double v_bound = 1.5;
  std::vector<Eigen::Vector3d> coeff_vectors{
      {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1}};
  std::vector<p4::SegmentInequalityBound> segment_inequality_bounds;
  // push back segment bounds for each waypoint and direction
  for (int w = 0; w < waypoints.size() - 1; w++) {
    for (size_t d{}; d < coeff_vectors.size(); d++) {
      segment_inequality_bounds.push_back(
          p4::SegmentInequalityBound(w, 1, coeff_vectors[d], v_bound));
    }
  }

  // The remaining operations should be familiar from lab 4.

  // Set options for polynomial solver
  p4::PolynomialSolver::Options solver_options;
  solver_options.num_dimensions = 3;   // 3D
  solver_options.polynomial_order = 8; // Fit an 8th-order polynomial
  solver_options.continuity_order = 4; // Require continuity to the 4th order
  solver_options.derivative_order = 2; // Minimize 2nd order (acceleration)

  // Set settings for polynomial solver
  osqp_set_default_settings(&solver_options.osqp_settings);
  // Polish the solution, getting the best answer possible
  solver_options.osqp_settings.polish = true;
  // Suppress printout of solver information
  solver_options.osqp_settings.verbose = false;
  // Set time limit for P4 solver, in seconds.  A longer time limit allows P4 to
  // expend more effort to seek a solution, a shorter limit ensures that the
  // UpdateTrajectories() function finishes executing within the
  // AutonomyProtocol's nominal update period.  Note that it's OK for your
  // UpdateTrajectories function to take longer than
  // AutonomyProtocol::nominal_update_period_ms_; the quad will simply continue
  // following the most recently accepted trajectory until your function finally
  // submits an admissible trajectory that overwrites it.
  solver_options.osqp_settings.time_limit =
      AutonomyProtocol::nominal_update_period_ms_ / 2;

  // Invoke p4::PolynomialSolver to solve for polynomial trajectories
  p4::PolynomialSolver solver(solver_options);
  const p4::PolynomialSolver::Solution path =
      solver.Run(times, node_equality_bounds, node_inequality_bounds,
                 segment_inequality_bounds);

  // Check whether solution was feasible and accurate.  If not, do something
  // here to reduce the number of constraints that P4 has to meet (e.g., prune
  // waypoints, shorten the trajectory, etc.), and then call
  // P4::PolynomialSolver again, or abandon the effort for this cycle and return
  // an empty trajectory.
  if (path.status ==
      p4::PolynomialSolver::Solution::SolutionStatus::infeasible) {
    std::cerr << "P4 problem overly constrained or not well defined.\n";
  } else if (path.status ==
             p4::PolynomialSolver::Solution::SolutionStatus::inaccurate) {
    std::cerr << "P4 indicates an accurate solution was not found.";
  }

  // Declare options container for configuring the polynomial sampler
  p4::PolynomialSampler::Options sampler_options;

  // Change sampling interval from chrono time to a double value.
  // See the Creating a PVAYT Trajectory Vector section for more details.
  const double dt =
      std::chrono::duration_cast<std::chrono::duration<double>>(dt_chrono_u_)
          .count();
  // Number of samples per second
  sampler_options.frequency = 1 / dt;

  // Sample position, velocity, and acceleration using the file-scope function
  sampler_options.derivative_order = 0;
  std::vector<double> x_hist, y_hist, z_hist;
  BuildHistoryVectors(times, path, sampler_options, x_hist, y_hist, z_hist);

  sampler_options.derivative_order = 1;
  std::vector<double> vx_hist, vy_hist, vz_hist;
  BuildHistoryVectors(times, path, sampler_options, vx_hist, vy_hist, vz_hist);

  sampler_options.derivative_order = 2;
  std::vector<double> ax_hist, ay_hist, az_hist;
  BuildHistoryVectors(times, path, sampler_options, ax_hist, ay_hist, az_hist);

  // ===== Creating a PVAYT Trajectory Vector =====

  // The following code generates and returns a new trajectory each time it
  // runs.  It creates a trajectory specifying position, velocity, and
  // acceleration (PVA) points spaced by dt seconds. Note that its strategy is
  // not optimal for covering the greatest distance in the allotted time in the
  // presence of disturbance accelerations.  Each PVA point needs an associated
  // timestamp to let the system know when the quad is expected to achieve the
  // specified state.  The agreed convention for trajectories passed to the
  // Mediation Layer requires time points measured in floating point seconds
  // from the Unix epoch.  Epoch time means Jan 1st of 1970, which is a standard
  // time of Unix-derived OSs. Please read the comment below for an introduction
  // to the std::chrono library, which we use to create time points and time
  // durations.

  // The best way to get good timing without diving into ROS timers is to use
  // the C++ std::chrono library. Here are some basic pointers for using the
  // library. There are three basic types of std::chrono variables:
  //
  // 1. The "timestamp" type. Represents an absolute time point, usually
  //   relative to epoch. Generated with std::chrono::system_clock::now().
  //
  //       const std::chrono::time_point<std::chrono::system_clock>
  //       timepointVar
  //         = std::chrono::system_clock::now();
  //
  // 2. The "duration" type.  Something you can use to add to and subtract from
  //   timestamps.  Pretty useful.  Can be generated by taking the difference
  //   between "timestamp" types, using std::chrono::duration<double>::
  //   time_since_epoch(), and std::chrono::duration_cast<...>(unitVar).  You
  //   can convert these back and forth to normal C++ doubles using count() as
  //   well.
  //
  //       const std::chrono::duration<double> durationVar
  //         = timepointVar1 - timepointVar2;
  //
  //       const std::chrono::duration<double> durationVar
  //         = timepointVar.time_since_epoch();
  //
  // 3. The "unit time" type. Must be set with integer, and is confusing because
  //   it acts a lot like the "duration" type.  Typically useful only if you
  //   need a quickhand way to generate some finite amount of time that isn't
  //   really related to anything in variable names.
  //
  //       std::chrono::milliseconds unitVar == std::chrono::milliseconds(15);
  //
  // For more information, visit
  // https://cplusplus.com/reference/chrono/.

  // You can construct a time_point using now()
  const std::chrono::time_point<std::chrono::system_clock>
      current_chrono_time_t_ = std::chrono::system_clock::now();

  // Yaw is arbitrary since we are not using the camera
  const double yaw = 0.0;

  // The TrajectoryVector3D object holds the PVAYT points.  It is a std::vector
  // object defined in the trajectory.h file.
  // ** I think this is just converting the sampled trajectory from relative
  // time to absolute time
  TrajectoryVector3D trajectory_vector;
  for (size_t time_idx = 0; time_idx < x_hist.size(); ++time_idx) {
    // Chrono time at the trajectory point
    const std::chrono::duration<double> flight_chrono_time_d_ =
        current_chrono_time_t_.time_since_epoch() + time_idx * dt_chrono_u_;

    // Time must be specified as a floating point number that measures the
    // number of seconds since the Unix epoch.  To get how many ticks are
    // counted in a chrono duration time, count() should be used. Effectively,
    // this converts from a from a unit to a duration.
    const double flight_time = flight_chrono_time_d_.count();

    // Push matrix packed with a trajectory point onto the trajectory vector
    trajectory_vector.push_back(
        (Eigen::Matrix<double, 11, 1>() << x_hist[time_idx], y_hist[time_idx],
         z_hist[time_idx], vx_hist[time_idx], vy_hist[time_idx],
         vz_hist[time_idx], ax_hist[time_idx], ay_hist[time_idx],
         az_hist[time_idx], yaw, flight_time)
            .finished());
  }

  // ===== Returning a Trajectory to the ML  =====

  // Create an empty quad-to-trajectory map.  This map object associates a quad
  // name (expressed as a std::string) with the corresponding Trajectory object
  // and is the return object of this function.
  std::unordered_map<std::string, Trajectory> quad_to_trajectory_map;

  // If there are too few samples for a valid trajectory, return an empty
  // quad_to_trajectory_map.  The quad will continue following the last valid
  // trajectory submitted.
  constexpr size_t min_required_samples_in_trajectory = 2;
  if (trajectory_vector.size() < min_required_samples_in_trajectory) {
    std::cerr << "Too few trajectory samples. Returning empty trajectory.\n";
    return quad_to_trajectory_map;
  }

  // If close to the red balloon or halt_ is true return a stationary trajectory
  constexpr double arrival_threshold_meters = 0.3;
  const Eigen::Vector3d dv = current_pos - halt_pos_;

  if (halt_ || dv.norm() < arrival_threshold_meters) { // WARN: should double
                                                       // check what sets halt_
    halt_ = true;
    std::cerr << "\nHalting quad at position: [" << halt_pos_(0) << halt_pos_(1)
              << halt_pos_(2) << "]\n";

    TrajectoryVector3D halt_trajectory_vector;
    for (size_t idx = 0; idx < 20; ++idx) {
      const std::chrono::duration<double> flight_chrono_time_d_ =
          current_chrono_time_t_.time_since_epoch() + idx * dt_chrono_u_;
      const double flight_time = flight_chrono_time_d_.count();
      halt_trajectory_vector.push_back(
          (Eigen::Matrix<double, 11, 1>() << halt_pos_.x(), halt_pos_.y(),
           halt_pos_.z(), 0, 0, 0, 0, 0, 0, 0, flight_time)
              .finished());
    }

    // The Trajectory object is created from a trajectory vector input
    Trajectory trajectory(halt_trajectory_vector);
    // Associate the quad name with the trajectory
    quad_to_trajectory_map[quad_name] = trajectory;
    return quad_to_trajectory_map;
  } else {
    // Otherwise, return the normal trajectory computed in the last section

    Trajectory trajectory(trajectory_vector);

    // Invoke the visualizer to see the proposed trajectory, which will be
    // displayed in violet.  See autonomy_protocol_visualizer.h for other
    // visualization options: you can visualize a short path, a single point,
    // etc.  It will be helpful to get such visual feedback on candidate
    // trajectories.  Note that there is a built-in visualizer called
    // "ViewManager" implemented elsewhere in the game-engine code, but you
    // don't have full control over what it displays like you do with the
    // visualizer invoked below.
    visualizer_.drawTrajectory(trajectory);
    quad_to_trajectory_map[quad_name] = trajectory;
    std::cerr << "Returning nominal trajectory.\n";
    return quad_to_trajectory_map;
  }

  // ===== Using the Prevetter for Trajectory Fixes =====

  // Note: this section never executes in this script since we already returned
  // the trajectory to the quad in the last section.
  //
  // Before submitting the trajectory, if you wish, you can use this
  // prevetting interface to determine if the trajectory violates any
  // mediation layer constraints.  It works very similarly to the mediation
  // layer error handling much further up in the script, with the only
  // difference being that you can use this to double check your submitted
  // trajectory before mediation layer ever has to see it.  Assuming it
  // doesn't pass the final check, you can jump back up to a certain part of
  // the code through a variety of means, where you can then compensate for
  // the error and try again.
  //
  // Further details that might help you understand how to utilize all
  // features of the prevetting: This interface's source code can be found
  // in presubmission_trajectory_vetter.h.  The PreVet function returns type
  // TrajectoryCode which contains three values: (1) The mediation layer
  // code that specifies success or the failure; (2) The failure value
  // (e.g., if the velocity limit is 4.0 m/s and you submit 5.0 m/s, the
  // value is returned as 5.0); (3) The failure index.  This is the
  // trajectory sample index that caused the mediation layer to kick back an
  // error code.  The index is the sampled index (specified from the P4
  // sampling process). You can figure out the waypoint index by a simple
  // math transformation.

  // Construct a trajectory from the trajectory vector
  Trajectory trajectory(trajectory_vector);

  // Send the trajectory through the prevetter
  TrajectoryCode prevetter_response =
      prevetter_->PreVet(quad_name, trajectory, map3d_);

  // Adjust trajectory based on prevetter output code
  switch (prevetter_response.code) {
  case MediationLayerCode::Success: {
    // You probably won't need to do anything in response to Success.
    break;
  }
  case MediationLayerCode::TimeBetweenPointsExceedsMaxTime: {
    // Suppose your AP intends to submit a trajectory with a time that
    // exceeds the maximum allowed time between points.  The prevetter
    // would catch this before you submit to the mediation layer.
    std::cerr << "Prevet: Shorten time between trajectory points. \n"
                 "Prevet: Time violation: "
              << prevetter_response.value << std::endl;

    // Resample P4 to reduce the time between PVAYT trajectory points

    break;
  }
  case MediationLayerCode::StartPointFarFromCurrentPosition: {
    std::cerr << "Prevet: Start point far from current position \n"
                 "Prevet: Position violation: "
              << prevetter_response.value << std::endl;

    // Fix trajectory starting position

    break;
  }
  default:
    std::cerr << "Prevet code: " << static_cast<int>(prevetter_response.code)
              << std::endl;
    std::cerr << "Prevet value: " << prevetter_response.value << std::endl;
    std::cerr << "Prevet index: " << prevetter_response.index << std::endl;
  }
}
} // namespace game_engine
