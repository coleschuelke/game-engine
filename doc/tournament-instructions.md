# Tournament Instructions

## Goals
Vision Test: Estimate the red and blue balloon center locations to within 10
cm based on a set of images and their corresponding metadata.

Obstacle Course: Command a quadcopter to pop two balloons and arrive at the goal position as quickly as possible.

## Obstacle Course Notes

### Interface
Your team will complete the function
`StudentAutonomyProtocol::UpdateTrajectories` located in the file
`game-engine/src/autonomy_protocol/aerial_robotics/student_autonomy_protocol.cc`. This function is a member of the `StudentAutonomyProtocol` class, which inherits from the `AutonomyProtocol` parent class.  These classes provide access to map data, balloon data, the current state of the quadcopter, etc.  The function `UpdateTrajectories` returns a proposed quadcopter trajectory.

Note that the function interface only specifies the input and output (quad state ->
quad trajectory), but not how to accomplish the path planning task. It is up to your team
to determine how best to complete the function.

The entire obstacle course challenge boils down to writing a single
function in one file.  But understand that the problem set before you is not
trivial. The problem of prescribing a time-optimal trajectory through a
cluttered environment in the presence of disturbances is difficult.

### Code Modifications
You may add any additional header (*.h) or source (*.cc) files you wish to the `game-engine/src/autonomy_protocol/aerial_robotics` directory, and you may add any data members or member functions you'd like to the `StudentAutonomyProtocol` class.  But do not alter the code or add files in any directory other than `aerial_robotics`, and do not remove any of the existing functions and data members of the `StudentAutonomyProtocol` class.  

Source (*.cc) files that you add to `aerial_robotics` are not compiled by default in the code building process.  You'll need to add them to the list of source files in [this](https://gitlab.com/radionavlab/public/game-engine/-/blob/master/src/autonomy_protocol/aerial_robotics/CMakeLists.txt?ref_type=heads#L9) section of `aerial_robotics/CMakeLists.txt`.  

### Running the Game Engine
To run `game-engine`, follow the build/run instructions in `README.md` in the
top-level `game-engine` directory. Your autonomy protocol will be compiled
into an executable called `student_autonomy_protocol`.

### Tagging releases for evaluation
During the pre-tournament and tournament, your team will want the TA to
evaluate only your *approved* `student_autonomy_protocol` code, which may not
be the code in the master branch of your team's GitLab repository.  You'll
want to *tag* an approved code release so that the TA knows which one to test.
The TA will test whichever master-branch revision is tagged `release`.

Suppose you're happy with the way your autonomy protocol is running on your
local machine and you'd like to tag the current master-branch version as
`release`.  Perform the following steps.

First, commit your current version and push to `origin` on branch `master`:
```bash
git commit -am "WRITE A COMMIT MESSAGE"
git push origin master
```

Second, if you already have a tag named `release`, then delete this
tag locally and at `origin`:
```bash
cd ~/Workspace/game-engine
git tag -d release
git push origin --delete release
```

Third, tag your current commit as `release` and push the tag to `origin`.  In
the example below, the annotated tag is labeled with example message
"pre-tournament day 3."
```bash
git tag -a release -m "pre-tournament day 3"
git push origin release
```

The TA will pull your `release` tag every evaluation day at 9 am and evaluate
your pre-tournament standing.
