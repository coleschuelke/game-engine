#ifndef PRESUBMISSION_TRAJECTORY_VETTER_H_
#define PRESUBMISSION_TRAJECTORY_VETTER_H_

#include "trajectory_vetter.h"

namespace game_engine {
class PreSubmissionTrajectoryVetter : public TrajectoryVetter {
 private:
  std::shared_ptr<QuadStateWarden> quad_state_warden_;

 public:
  PreSubmissionTrajectoryVetter(
      const int& quad_safety_limits,
      const std::shared_ptr<QuadStateWarden> quad_state_warden)
      : TrajectoryVetter(quad_safety_limits),
        quad_state_warden_(quad_state_warden) {}

  // Determines if a trajectory meets the trajectory requirements
  TrajectoryCode PreVet(const std::string& quad_name,
                        const Trajectory& trajectory, const Map3D& map);
};

// This PreVet function returns the same response as the MediationLayer's
// pre-vetter.  This interface is made available for students to check their
// proposed trajectory's validity before sending it to the mediation layer.
// TrajectoryCode contains the MediationLayerCode (found in
// util/trajectory_code.h), as well as the value and index of the violation if
// it exists.
inline TrajectoryCode PreSubmissionTrajectoryVetter::PreVet(
    const std::string& quad_name, const Trajectory& trajectory,
    const Map3D& map) {
  return Vet(trajectory, map, quad_state_warden_, quad_name);
}
}  // namespace game_engine

#endif
