#!/bin/bash

#=======CLI Handling=======#
aioFlag=false
shackFlag=false
recordFlag=false
playbackFlag=false

while getopts "a:s r p:h" opt; do
	case $opt in
		a)
			aioFlag=true
			configFile=$OPTARG
			echo "Config file to use: $configFile"
			echo -e "Running in ALL-IN-ONE mode ...\n"
			;;
		s)
			echo -e "Running in SHACK mode (disabled physics sim) ...\n"
			shackFlag=true
			;;
		r) 	
			echo -e "\e[1m\e[35mRecording enabled ... (recordings are saved in game-engine/launch/)\e[0m\n"
			recordFlag=true
			;;
		p)
			echo -e "\n\e[1m\e[35mRunning in PLAYBACK mode ...\e[0m"
			playBackFile=$OPTARG
			echo "Playing: $playBackFile"
			playbackFlag=true
			;;
		h)
            echo -e "----------------------------------------------------------------
Script for starting the game-engine and supporting resources.

Usage: ./launchGE.sh [-a|s|r|p|h]

Options
   -a <file>    Runs the script with parameters specified
		  according to a given yaml file 
		  (see launchConfigDefault.yaml for structure).
   -s       	Runs the game-engine in 'shack' mode. In this mode,
                  roscore and physics_simulator are not started, and
                  the launch order is adjusted for best results 
                  with the real quads.
   -r        	Enables recording of the simulation. Currently, 
		  this generates a rosbag file that can be played back
		  in playback mode.
   -p <file>	Runs the simulation in playback mode. Only the data from 
                  a specified rosbag are processed. No additional
		  data are saved.  This mode allows pausing to
                  investigate a previously-run simulation.
   -h        	Prints usage.
----------------------------------------------------------------"
            exit 1
            ;;
		\?)		
			echo "Invalid option: -$OPTARG" >&2
    	    exit 1
        	;;
	esac
done

#==========Warning===============#
clear
localdir=$(pwd)
echo -e "\n\e[1;33mWARNING:\e[0m Using Ctrl-Z or Ctrl-\\ can cause unintended behaviors. 
Use only Ctrl-C to exit the Game Engine.  If Ctrl-Z or Ctrl-\\ is
accidentally used, or if you have any reason to believe some 
Game-Engine-related process has not shut down properly, run killGE.sh.  \n\n"

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
if [ "$aioFlag" = true ] ; then
	# get information from config file
	filepath=$(awk '/^path_to_GE: / {print $2}' $configFile)
	echo -e "\nGame-Engine Path: $filepath"
	param=$(awk '/^param_file: / {print $2}' $configFile)
	echo -e "Param file: $param"
	prot=$(awk '/^protocol: / {print $2}' $configFile)
	echo -e "Protocol to run: $prot\n"
else
	# get game-engine working directory
	filepath="$(dirname `pwd`)"
	echo ${filepath}
	CMAKEFILE=
	while true ; do
		CMAKEFILE=$filepath/src/autonomy_protocol/CMakeLists.txt
		if [ -f $CMAKEFILE ] ; then
			break
		fi
		echo "$filepath is not a valid game-engine directory... auto dir detection failed. Enter manually:"
		read -e -r -p "Enter path to game engine (i.e. /home/user/Workspace/game-engine): " filepath
	done
	# get params file
	cd $filepath/run
	param=
	while true ; do
		read -e -r -p "ROS param file to use in /run (e.g., params.yaml): " param   
		if [ -f "$param" ] ; then
			break
		fi
		echo "$param is not in ./run"
	done
fi

#==========Setup Process===============#
# launch roscore in background (assumed already running in shack mode)
cd $localdir
if [ "$shackFlag" = false ] ; then
    rosclean purge -y
    echo -n "Launching roscore ... "
	roscore > roscore.log 2>&1 &
	sleep 2
		echo "success."
		echo ""
else
	while ! pgrep -x "rosmaster" >/tmp/null_output 2>&1; do
		echo -e "\e[1;33mWARNING:\e[0m ROSCORE IS NOT STARTED."
		read -p "PLEASE START ROSCORE IN ANOTHER TERMINAL AND PRESS [ENTER] TO CONTINUE"
	done
fi
# load visualizer
cd $filepath/run
echo -n "Launching Visualizer ... "
rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 & echo "success."
echo ""
yn="y"
sleep 2

#==========Repeatable Process===============#
while true; do
	# check that roscore and rviz are running (needed for restarting after failure)
	if [ "$shackFlag" = false ] && ! pgrep -x "roscore" >/tmp/null_output 2>&1; then
		cd $localdir
		echo -n "Restarting roscore ... "
		roscore > roscore.log 2>&1 &
		sleep 2
		echo "success."
		echo ""
	fi
	if [[ "$yn" == "y" || "$yn" == "Y" ]] && ! pgrep -x "rviz" >/tmp/null_output 2>&1; then
		cd $filepath/run
		echo -n "Restarting Visualizer ... "
		rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 & echo "success."
		echo ""
		sleep 2
	fi

	# load roscore params
	echo -n "Loading $param file ... "
	cd $filepath/run
	rosparam load $param /game_engine/
	echo "success."
	echo ""
	sleep 1

	if [ "$shackFlag" = false ] ; then
		# Mediation Layer (launches here regularly)
		echo -n "Launching Mediation Layer ... "
		cd $filepath/bin
		stdbuf -o0 ./mediation_layer > $localdir/med_layer.log 2>&1 &
		sleep 1
		echo "success."
		echo ""

		# Physics Symulator (when not running in shack mode)
		echo -n "Launching Physics Simulator ... "
		cd $filepath/bin
		stdbuf -o0 ./physics_simulator > $localdir/phys_sim.log 2>&1 &
		sleep 1
		echo "success."
		echo ""
	fi
	# pass to visualizer
	cd $filepath/bin
	case $yn in
	        [Yy]* ) ./visualizer &
			read -p "Press [Enter] once environment objects appear.";;
	esac

	# Playback mode handling
	if [ "$playbackFlag" = true ] ; then
		read -p "Press [Enter] to start playback."

		cd $filepath/launch
		rosbag play $playBackFile
	fi

	# Mediation Layer (when in shack mode)
	if [ "$shackFlag" = true ] ; then

		file_contents=$(cat ../run/$param)
		pos=$(echo "$file_contents" | grep -A1 "initial_quad_positions:" | grep ":" | sed 's/.*: "\(.*\)"/\1/')
		echo -e "\e[1;33mWARNING:\e[0m Move quad to start position specified in params.yaml -> [$pos]"
		read -p "Press [Enter] to continue."
		echo -n "Launching Mediation Layer ... "
		cd $filepath/bin
		stdbuf -o0 ./mediation_layer > $localdir/med_layer.log 2>&1 &
		sleep 1
		echo "success."
		echo ""
	fi
		
	if [ "$playbackFlag" = false ] ; then
		# get autonomy protocol
		if [ "$aioFlag" = false ] ; then
			cd $filepath/bin
			prot=
			while true ; do
				read -e -r -p "Autonomy Protocol file to use in ./bin (e.g., student_autonomy_protocol): " prot
				exe=${prot%% *}
				FILE=$filepath/bin/$exe
				if [ -f "$FILE" ] ; then
					break
				fi
				echo "$prot is not in ./bin"
			done
		else
		    read -p $'\n'"Press [Enter] to run $prot."
                    echo ""
		    sleep 1
		fi

		# start recording before running autonomy protocol
		if [ "$recordFlag" = true ] ; then
			rosbag record -a -o ../launch/testBag __name:=my_bag > /dev/null 2>&1 & $filepath/bin/$prot | tee $localdir/autonomy_protocol.log 
		else
			$filepath/bin/$prot | tee $localdir/autonomy_protocol.log 
		fi

		# stop recording
		if [ "$recordFlag" = true ] ; then
			rosnode kill /my_bag > /dev/null 2>&1
		fi
	fi

	# output protocol time to the command line if the course was completed
	cd $localdir
	medout=`grep "Goal reached at elapsed time" med_layer.log`
	value=${medout#*"Goal reached at elapsed time"}
	value=`echo $value | grep -m 1 -Eo '[0-9]+([.][0-9]+)?'`
	value=`echo $value | cut -d' ' -f1`
	echo ""
    echo "-----"
	if [[ $value =~ ^[+-]?[0-9]+([.][0-9]+)?$ ]]; then
		echo "$exe, ${value%%??}"
	else
		echo "$exe, NaN"
	fi
    echo "-----"
	echo ""

	# prompt user to restart the simulation when cancelled out
	# Wait for protocol to end
	echo "Restart to kill the Mediation layer, Physics Simulator, and protocol and re-run them."
        echo "Note: Restarting will overwrite any existing log files." 
	while true; do
			read -p "Restart simulation? [y/n] " yn_sim
			case $yn_sim in
				[Yy]* ) echo "Restarting ... "
					echo ""
					sleep 1
					break;;
				[Nn]* ) 
					echo -n "Exiting ... "
					killall mediation_layer physics_simulator visualizer rviz roscore $prot 2> /dev/null
                                        echo "success."
					exit;;
				* ) echo "Please answer yes or no ([y/n]).";;
				esac
	done
	killall mediation_layer physics_simulator visualizer $prot 2> /dev/null
done
