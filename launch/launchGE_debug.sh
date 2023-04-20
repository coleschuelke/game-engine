#!/bin/bash

#==========Warning===============#
localdir=$(pwd)
read -p "This script will run everything other than the autonomy protocol which you can run, after everything has launched,
manually with your debugger of choice.
Before running the script:
1) make sure no instance of ros is running
2) have a working clone of game-engine already built with your protocol. Make sure the protocol is in the ./bin folder.
3) if issues arise (i.e ballons are not popping) then CTRL+C and try again, or try manually running the commands.
4) when you CTRL+C make sure all the processes have stopped (use htop or ps -a to get the process id and kill to kill it)

All log files for mediation layer, physics layer, autonomy protocol are unbuffered and logged and the ros output is logged in:
${localdir}

Press [Enter] key to continue..."

#==========Get info from user===============#
# get game-engine working directory
filepath=
while true ; do
    read -e -r -p "Enter path to game engine (i.e. /home/user/Workspace/game-engine): " filepath
    if [ -d "$filepath" ] ; then
        break
    fi
    echo "$filepath is not a directory..."
done
# get params file
cd $filepath/run
param=
while true ; do
    read -e -r -p "ROS param file to use in /run (i.e. params.yaml): " param   
    if [ -f "$param" ] ; then
        break
    fi
    echo "$param is not in ./run"
done



#==========Setup Process===============#
# launch roscore in background
cd $localdir
echo "Launching Roscore.."
roscore > roscore.log 2>&1 &
sleep 2
echo "Success!"
# load visualizer (Optional)
cd $filepath/run
while true; do
    	read -p "Start visualizer?[y/n] " yn
    	case $yn in
        	[Yy]* ) rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 &
			echo "Success."
			sleep 2
			break;;
        	[Nn]* ) break;;
        	* ) echo "Please answer yes or no ([y/n]).";;
		    esac
done
#==========Repeatable Process===============#
while true; do
	# load roscore params
	echo "Launching $param file.."
	cd $filepath/run
	rosparam load $param /game_engine/
	echo "Success!"
	sleep 1
	# Mediation Layer
	echo "Launching Mediation Layer.."
	cd $filepath/bin
	stdbuf -o0 ./mediation_layer > $localdir/med_layer.log 2>&1 &
	sleep 1
	echo "Success!"
	# Physics Symulator
	echo "Launching Physics Simulator.."
	cd $filepath/bin
	stdbuf -o0 ./physics_simulator > $localdir/phys_sim.log 2>&1 &
	sleep 1
	echo "Success!"
	# pass to visualizer
	cd $filepath/bin
	case $yn in
	        [Yy]* ) ./visualizer &
			read -p "Press enter to continue once objects are loaded in visualizer..";;
	esac
	#get autonomy protocol
	read -p "You may now launch your autonomy protocol manually."
	read -p "Press [Enter] to kill the Mediation layer, Physics Sim and protocol and re-run them. WARNING: Logs will be wiped!"
	killall visualizer 2>/dev/null
	killall physics_simulator 2>/dev/null
	killall mediation_layer 2>/dev/null
done




