# Launchers for Game Engine
This directory holds scripts for conveniently launching Game Engine. They automate, in one form or another, the process described in the project root README file.

# Launch Scripts
## launch.sh
This script simply creates tabs for each component of Game Engine that needs execution. It also launches the vizualizer.

## launchGE.sh
This script is meant to streamline launching Game Engine by providing the user with choices of the parameter file and autonomy protocol. You can also choose to disable the vizualizer. All output (mediation layer/protocol) is saved for you to inspect afterwards.

## launchGE_debug.sh
This script is identical to `launchGE.sh`, with the exception of launching the protocol. It leaves this up to the user. This way, the user can launch their protocol with a debugger attatched and step through their code. It should help streamline debugging an autonomy protocol.

## tournament.sh
This script is meant to run protocols in succession and keep track of the time they take. It outputs the mediation layer and protocol outputs to individual logs, and generates a .csv (it appends to if it exists) with the time each team took. To ensure the script runs smoothly,
make sure of the following:
- In the same directory as `tournament.sh`, you should have a `tournament.csv` with the structure below. It can handle spaces, but it is probably best to limit that in case there are edgecases I haven't checked.
~~~csv
Team Name, Clone URL
Wenkai's team, git@gitlab.com:wenkaiqin/game-engine.git
~~~
- All code that will be tested should be in the `autonomy_protocol/aerial_robotics/` directory. Spceifically, the code finds the directory that holds `student_autonomy_protocol.cc` and copies the content over. Also, the script copies the CMakeLists.txt in the directory outside the `aerial_robotics` directory which determinces the source files for the `student_autonomy_protocol.cc`.
- The script looks for the output `Goal reached` from the mediation layer. If Game Engine updates changing this, the script should also be updated.

