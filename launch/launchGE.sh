#!/bin/bash

#=======CLI Input Handling=======#
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
			echo -e "Running in ALL-IN-ONE mode..\n"
			;;
		s)
			echo -e "Running in SHACK mode (disabled physics sim)..\n"
			shackFlag=true
			;;
		r) 	
			echo -e "\e[1m\e[35mRecording enabled.. (recordings are saved in game-engine/launch/)\e[0m\n"
			recordFlag=true
			;;
		p)
			echo -e "\n\e[1m\e[35mRunning in PLAYBACK mode..\e[0m"
			playBackFile=$OPTARG
			echo "Playing: $playBackFile"
			playbackFlag=true
			;;
		h)
            echo -e "----------------------------------------------------------------
Script for starting the game-engine and subsequent resources.

Usage: ./launchGE.sh [-a|s|r|p|h]

Options
   -a <file>   Runs the script with parameters specified
		  according to a given yaml file 
		  (see launchConfigDefault.yaml for structure).
   -s       	Runs the game-engine in 'shack' mode. roscore
		  and physics sim are not started here.
		  roscore is launched in another script
		  (for testing roscore can be launched in 
		  any terminal). The launch order is also 
		  adjusted for the real drones.
   -r        	Enables recording of the simulation. Currently 
		  this generates a rosbag file that can be played back
		  in playback mode.
   -p <file>	Runs the simulation in playback mode. Here only
		  the data from a specified rosbag is run. No additional
		  data is saved here, but it allows the ability to pause
		  and investigate a previously run simulation.
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
localdir=$(pwd)
echo -e "\n\n\e[1;33mWARNING:\e[0m Using CTRL-Z or CTRL-\\ can cause unintended behaviors. Please use CTRL-C to exit the GE.
\t If either CTRL-Z or CTRL-\\ is used, please run killGE.sh to ensure the GE is shut down\\ properly.
\n\n"

read -p "Before running the script:
1) Make sure no instance of roscore is running.
2) Have a working clone of game-engine already built with your protocol. Make sure the protocol is in the ./bin folder.
3) If issues arise (i.e ballons are not popping) then CTRL+C and try again, or try manually running the commands.
4) When you CTRL+C make sure all the processes have stopped (use htop or ps -a to get the process id and kill to kill it).

All log files for mediation layer, physics layer, autonomy protocol are unbuffered and logged and the ros output is logged in:
${localdir}

Press [Enter] key to continue... "

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
fi



#==========Setup Process===============#
# launch roscore in background (already running from shack specific script)
cd $localdir
if [ "$shackFlag" = false ] ; then
	echo "Launching roscore.."
	roscore > roscore.log 2>&1 &
	sleep 2
		echo "Success."
		echo ""
else
	while ! pgrep -x "roscore" >/tmp/null_output 2>&1; do
		echo -e "\e[1;33mWARNING:\e[0m ROSCORE IS NOT STARTED."
		read -p "PLEASE START ROSCORE IN ANOTHER TERMINAL AND PRESS [ENTER] TO CONTINUE"
	done
fi
# load visualizer (Optional when not in shack mode)
cd $filepath/run
if [[ "$shackFlag" = false && "$playbackFlag" = false ]] ; then
	while true; do
			read -p "Start visualizer? [y/n] " yn
			case $yn in
				[Yy]* ) rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 &
				echo "Success."
				echo ""
				sleep 2
				break;;
				[Nn]* ) break;;
				* ) echo "Please answer yes or no ([y/n]).";;
				esac
	done
else
	echo "Launching Visualizer.."
	rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 & echo "Success."
	echo ""
	yn="y"
	sleep 2
fi

#==========Repeatable Process===============#
while true; do
	# check that roscore and rviz are running (needed for restarting after failure)
	if [ "$shackFlag" = false ] && ! pgrep -x "roscore" >/tmp/null_output 2>&1; then
		cd $localdir
		echo "Restarting roscore.."
		roscore > roscore.log 2>&1 &
		sleep 2
		echo "Success."
		echo ""
	fi
	if [[ "$yn" == "y" || "$yn" == "Y" ]] && ! pgrep -x "rviz" >/tmp/null_output 2>&1; then
		cd $filepath/run
		echo "Restarting Visualizer.."
		rosrun rviz rviz -d config.rviz > $localdir/rviz.log 2>&1 & echo "Success."
		echo ""
		sleep 2
	fi

	# load roscore params
	echo "Launching $param file.."
	cd $filepath/run
	rosparam load $param /game_engine/
	echo "Success."
	echo ""
	sleep 1

	if [ "$shackFlag" = false ] ; then
		# Mediation Layer (launches here regularly)
		echo "Launching Mediation Layer.."
		cd $filepath/bin
		stdbuf -o0 ./mediation_layer > $localdir/med_layer.log 2>&1 &
		sleep 1
		echo "Success."
		echo ""

		# Physics Symulator (when not running in shack mode)
		echo "Launching Physics Simulator.."
		cd $filepath/bin
		stdbuf -o0 ./physics_simulator > $localdir/phys_sim.log 2>&1 &
		sleep 1
		echo "Success."
		echo ""
	fi
	# pass to visualizer
	cd $filepath/bin
	case $yn in
	        [Yy]* ) ./visualizer &
			read -p "Press [Enter] to continue once objects are loaded in visualizer... ";;
	esac

	# Playback mode handling
	if [ "$playbackFlag" = true ] ; then
		read -p "Press [Enter] to start playback... "

		cd $filepath/launch
		rosbag play $playBackFile
	fi

	# Mediation Layer (when in shack mode)
	if [ "$shackFlag" = true ] ; then

		file_contents=$(cat ../run/params.yaml)
		pos=$(echo "$file_contents" | grep -A1 "initial_quad_positions:" | grep "zeus" | sed 's/.*: "\(.*\)"/\1/')
		echo -e "\e[1;33mWARNING:\e[0m Please move quad to desired start position as stated in params.yaml -> [$pos]"
		read -p "Press [Enter] key to continue... "
		echo "Launching Mediation Layer..."
		cd $filepath/bin
		stdbuf -o0 ./mediation_layer > $localdir/med_layer.log 2>&1 &
		sleep 1
		echo "Success."
		echo ""
	fi
		
	if [ "$playbackFlag" = false ] ; then
		# get autonomy protocol
		if [ "$aioFlag" = false ] ; then
			cd $filepath/bin
			prot=
			while true ; do
				read -e -r -p "Autonomy Protocol file to use in ./bin (i.e. example_autonomy_protocol): " prot
				exe=${prot%% *}
				FILE=$filepath/bin/$exe
				if [ -f "$FILE" ] ; then
					break
				fi
				echo "$prot is not in ./bin"
			done
		else
			read -p $'\n'"Press [Enter] to run $prot... "
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

	# prompt user to restart the simulation when cancelled out
	echo ""
	echo "WARNING: Restarting will WIPE LOGS. Continue to kill the Mediation layer, Physics Sim and protocol and re-run them."
	while true; do
			read -p "Restart simulation? [y/n] " yn_sim
			case $yn_sim in
				[Yy]* ) echo "Restarting..."
					echo ""
					sleep 1
					break;;
				[Nn]* ) 
					echo "Exiting..."
					killall mediation_layer physics_simulator visualizer rviz roscore $prot 2> /dev/null
					exit;;
				* ) echo "Please answer yes or no ([y/n]).";;
				esac
	done
	killall mediation_layer physics_simulator visualizer $prot 2> /dev/null
done