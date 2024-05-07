#!/bin/bash

#==========Warning===============#
localdir=$(pwd)
echo -e "This script will run everything other than the autonomy protocol which
you can run, after everything has launched, within your debugger of choice.

read -p "Before proceeding with the script ...
1) Ensure no instance of roscore is running.
2) Have a working clone of game-engine already built with your target protocol
   in the ./bin folder.
3) If problems arise (i.e., ballons are not popping), then hit Ctrl+C and try again, 
   or try manually running the Game Engine modules.
4) After you apply Ctrl-C, make sure no game-engine-related processes remain running 
  (use htop or ps -a to see running processes); if so, run killGE.sh.

Note that log files for mediation layer, physics layer, autonomy protocol will
be logged together with all ROS output in ${localdir}

Press [Enter] to continue."

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




