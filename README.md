# Game Engine
## Structure
Game Engine has four interacting components: Mediation Layer (ML),
Physics Simulator (PS), Visualizer (VZ), and Autonomy Protocol (AP). 

AP maps the quadcopter's state to an intended trajectory. ML mediates the
intended trajectories, altering them if necessary to impose boundaries,
simulate interaction with other objects, etc.  PS forward-simulates the
mediated trajectory, injecting disturbances and applying
proportional-derivative control to track the trajectory, and returns the
quadcopter's state at a subsequent time point. VZ pushes visualization data to
ROS's RVIZ program.

## Installation
### Dependencies
1. [Eigen](https://eigen.tuxfamily.org)
2. [ROS](http://www.ros.org)
3. [P4](https://gitlab.com/radionavlab/public/p4)

### Clone
```bash
cd ~/Workspace
git clone https://gitlab.com/radionavlab/public/game-engine.git
cd game-engine
git submodule update --init --recursive

```

### Build
```bash
cd ~/Workspace/game-engine
mkdir build # (do this only if the build directory hasn't already been created)
cd build
cmake ..
make -j
```

If this build procedure does not work, you may need to build and install
individual Game Engine submodules, as described [here](build_from_scratch.md).

### Configure for ROS
After the first time you build Game Engine, you'll need to add a command
to your `.zshrc` configuration file so that whenever you open a terminal
the shell will run the necessary ROS setup scripts.

First open `.zshrc` in your favorite editor; e.g., for VS Code:
```bash
code ~/.zshrc
```
Then add this line to the bottom of the file (if it's not already present):
```bash
source ~/Workspace/game-engine/build/devel/setup.zsh
```

## Running Game Engine Using Launch Script
A launch script has been provided that runs the desired autonomy protocol in
game engine, without the need for a terminal multiplexer.  Using this launch
script is simplest and safest way of running the game-engine. You can run the
script as shown below.

```bash
cd ~/Workspace/launch
./launchGE.sh -a launchConfigDefault.yaml
```

The interactive script will walk you through the steps to run your autonomy
protocol.

Note: The file `launchConfigDefault.yaml` contains the path to game-engine on your
computer and the name of the params file and the autonomy protocol that you wish
to run. It should work by default, but you may need to change the game-engine
path if game-engine is stored somewhere other than
`/home/aeronaut/Workspace/game-engine` on your computer. You will also need to
change the protocol value from `example_autonomy_protocol` to
`student_autonomy_protocol` in order to run your AP.

Note: When terminating the `launchGE.sh` script, it is important to use
Ctrl-c. Using Ctrl-z, or any other method, may keep parts of the game-engine
running in the background which will cause problems the next time you try to
start ROS. If you believe there may be lingering processes running in the
background, then run the following script to completely shutdown game-engine.

```bash
cd ~/Workspace/launch
./killGE.sh
```

## Running Game Engine Component by Component
For debugging purposes, or simply to better understand how Game Engine works
you may wish manually run each component rather than use the launch script.  For
this, you are encouraged to use a terminal multiplexer like `tmux` and start
each program in a separate pane. See [here](tmux/README.md) for further
information on `tmux`.

### ROS Core
```bash
roscore
```

### Load ROS params
The ROS parameters found in `game-engine/run/params.yaml` must be loaded after
`roscore` has been started or re-loaded if any of the parameters have changed
since loading.

First, check that the file paths in `/game-engine/run/params.yaml` (e.g.,
`map_file_path`) are correct for your system.  Then load the parameters into the
`game_engine` namespace:
```bash
cd game-engine/run
rosparam load params.yaml /game_engine/
```

### ROS Visualizer
The ROS visualizer (RVIZ) manages a 3D visualization environment in which the
arena, obstacles, balloons, and quadcopters are displayed. 
```
cd game-engine/run
rosrun rviz rviz -d config.rviz
```

### Mediation Layer (ML)
ML mediates proposed trajectories, ensuring safety.
```
cd game-engine/bin
./mediation_layer
```

### Physics Simulator (PS)
PS forward-integrates proposed quadcopter trajectories over
a short interval into the future and publishes the resulting state.  It
applies proportional-derivative control to track the trajectories.  It also
introduces disturbance forces, e.g., due to wind.
```
cd game-engine/bin
./physics_simulator
```

### Visualizer (VZ)
VZ sends arena, obstacle, balloon, and quadcopter data to RVIZ for display.
```
cd game-engine/bin
./visualizer
```
Note that it may take some time (a few tens of seconds) for all the elements of
the arena to get populated into the RVIZ display.

### Autonomy Protocol (AP)
AP takes the current quadcopter state and publishes a proposed trajectory for
the quadcopter to follow.
```
cd game-engine/bin
./example_autonomy_protocol
```

## Contributing
### Software Patterns
Please read the [software patterns](doc/software-patterns.md) document to
understand the naming convention and the purpose of components in this system.

