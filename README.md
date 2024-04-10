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
to your your `.zshrc` configuration file so that whenever you open a terminal,
the shell will run the necessary ROS setup scripts.

First open `.zshrc` in your favorite editor; e.g., for VS Code:
```bash
code ~/.zshrc
```
Then add this line to the bottom of the file:
```bash
source ~/Workspace/game-engine/build/devel/setup.zsh
```

## Running Game Engine Launch Script
A launch script has been provided that runs the desired autonomy protocol in game engine, without the need for a terminal multiplexer.  It is our reccomended way of running the game-engine as it is the best way to prevent unintended errors in ROS. You can run it as shown below.

```bash
cd ~/Workspace/launch
./launchGE.sh -a launchConfigDefault.yaml
```
After running this script, the terminal will walk you through the steps to run your autonomy protocol.

Note: The file launchConfigDefault.yaml contains the path to game-engine on your computer and the name of the params file and the autonomy protocol that you wish to run. It should work by default, but you may need to change the game-engine path if game-engine is stored somewhere other than /home/aeronaut/Workspace/game-engine on your computer. You will also need to change the protocol value from example_autonomy_protocol to student_autonomy_protocol in order to run your APl

Note: When terminating this script, it is important to use ctrl-c. Using ctrl-z, or any other method, may keep parts of the game-engine running in the background which will cause some issues the next time you try to start up ROS. If you believe this has occured, then run the following script to completely shutdown game-engine.

```bash
cd ~/Workspace/launch
./killGE.sh
```


### Tests
```bash
cd build/test
./EXECUTABLE_OF_CHOICE
```

## Contributing
### Software Patterns
Please read the [software patterns](doc/software-patterns.md) document to
understand the naming convention and the purpose of components in this system.

