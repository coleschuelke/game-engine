#!/usr/bin/env bash

PROTOCOL_TIMEOUT=300

#==========Warning===============#
read -p "Before running the script:
1) Ensure a valid .csv file is present as described in README
2) Have a working clone of game-engine already built. Make sure the build folder is called build.

This will remove the previous leaderboard.

Press [Enter] to continue ..."

#==========Get info from user===============#
# Get Current Working Directory and CSV paths
CWD=`(pwd)`
CSV=tournament.csv
TEAMS=${CWD}/$CSV

# Find CSV file, or exit if not found
if [ ! -f "$TEAMS" ]; then
    echo "$CSV does not exist in the ./tournament directory. Please create it before running the script."
    exit 1
fi

# Get game-engine working directory
GAME_ENGINE=
CMAKEFILE=
while true ; do
    read -e -r -p "Enter path to game engine (i.e. /home/user/Workspace/game-engine): " GAME_ENGINE
    CMAKEFILE=$GAME_ENGINE/src/autonomy_protocol/CMakeLists.txt
    if [ -f $CMAKEFILE ] ; then
        break
    fi
    echo "$GAME_ENGINE is not a valid game-engine directory..."
done

if [ "$GAME_ENGINE" != "/" ]; then
    GAME_ENGINE="${GAME_ENGINE%/}"
fi
GAME_ENGINE=$(realpath "$GAME_ENGINE")


# get .yaml file to be used
cd $GAME_ENGINE/run
param=
while true ; do
    read -e -r -p "ROS param file to use in game-engine/run (i.e. params.yaml): " param   
    if [ -f "$param" ] ; then
        break
    fi
    echo "$param is not in ./run"
done

#==========Clean up===============#
# To make sure it remembers the credentials
git config --global credential.helper store

# remove last and create new leaderboard
cd $CWD
leaderboard=leaderboard.csv
echo Team Name, Time to Goal, Date >> $leaderboard

git config --global advice.detachedHead false

#==========Run protocols===============#
# For each team copy over to game engine, run and save
while read line || [ -n "$line" ];
do  
    # Read team name and git link from CSV
    dt=$(date '+%F %T')
    GIT_LINK=`awk '{split($0, array, ","); print array[2]}' <<< $line `
    TEAM_NAME=`awk '{split($0, array, ","); print array[1]}' <<< $line `
    
    if [ "$TEAM_NAME" == "Team Name" ] || [ "$GIT_LINK" == "Clone URL" ]; then
        continue
    fi

    # Get team source code
    cd $CWD
    GIT_LINK=`sed -e 's/^"//' -e 's/"$//' <<<"$GIT_LINK"`
    echo "Trying team $TEAM_NAME with repo at $GIT_LINK"
    TEAM_NAME="${TEAM_NAME// /_}"
    # -branch can also take tags and detaches the HEAD at that commit in the
    # resulting repository as of 1.8.0 
    rm -rf $TEAM_NAME > /dev/null 2>&1
    if ! (git clone --quiet --branch release $GIT_LINK "$TEAM_NAME" > /dev/null) then
        echo "[ERROR] Unable to read from '$GIT_LINK'"
        echo $TEAM_NAME, "Nan", $dt >> $leaderboard
        continue
    fi
    echo "Repo cloned successfully!"
    echo "Searching for student_autonomy_protocol.."
    
    #----------------------------------------------#
    #----------------In student repo---------------#
    #----------------------------------------------#
    cd $TEAM_NAME

    # Find code to copy over
    # find student_autonomy_protocol.cc
    SAPcc=`(find -path "*autonomy_protocol/aerial_robotics/student_autonomy_protocol.cc" -not -path "./build/*")`
    if [ ! -f "$SAPcc" ]; then
        echo "$SAPcc not found. Default to large time."
        echo $TEAM_NAME, "Nan", $dt >> $leaderboard
        continue
    fi
    echo $SAPcc
    # find student_autonomy_protocol.h
    SAPh=`(find -path "*autonomy_protocol/aerial_robotics/student_autonomy_protocol.h" -not -path "./build/*")`
    if [ ! -f "$SAPh" ]; then
        echo "$SAPh not found. Default to large time."
        echo $TEAM_NAME, "Nan", $dt >> $leaderboard
        continue
    fi
    echo "student_autonomy_protocol found!"
    echo "Copying to game engine.."
    # Copy files over to game engine
    aerial_robotics_dir="$(dirname "$SAPcc")"

    cp -R "$aerial_robotics_dir/"* "$GAME_ENGINE"/src/autonomy_protocol/aerial_robotics/
    
    echo "Copying successful!"
    # Delete repo
    cd $CWD
    rm -rf $TEAM_NAME
    # remove previous logs
    cd $CWD
    rm -f med_layer.log
    rm -f roscore.log
    rm -f phys_sim.log
    # #==========Game Engine here===============#
    # Build game engine with new binaries
    echo "Generating makefiles..."
    cd $GAME_ENGINE
    cd build
    if cmake .. > /dev/null 2>&1 ; then
        echo "Cmake succeeded"
    else
        echo "Cmake failed"
        echo $TEAM_NAME, "Nan", $dt >> $leaderboard
        continue
    fi
    echo "Compiling game engine..."
    if make -j > /dev/null 2>&1 ; then
        echo "make succeeded"
    else
        echo "make failed"
        echo $TEAM_NAME, "Nan", $dt >> $leaderboard
        continue
    fi
      
    # launch roscore in background
    cd $CWD
    echo "launching Roscore"
    roscore > roscore.log 2>&1 &
    sleep 2
    echo "Success. Terminal output in roscore.log..."
    # load roscore params
    cd $GAME_ENGINE/run
    rosparam load $param /game_engine/
    sleep 2
    rosrun rviz rviz -d config.rviz > $GAME_ENGINE/rviz.log 2>&1 & echo "Success."
	echo ""
	sleep 2
    # Mediation Layer
    sleep 0.5
    cd $GAME_ENGINE/bin
    stdbuf -o0 ./mediation_layer > $CWD/med_layer.log 2>&1 &
    echo "Mediation Layer started. Terminal output in med_layer.log.."
    sleep 1
    # Physics Symulator
    sleep 0.5
    cd $GAME_ENGINE/bin
    ./physics_simulator > $CWD/phys_sim.log 2>&1 & 
    echo "Physics sim started..."
    sleep 1
    # pass to visualizer
	cd $GAME_ENGINE/bin
	./visualizer & 
    echo "Visualizer started..."
    sleep 1
    # Run protocol
    echo "Running $TEAM_NAME's protocol"
    cd $GAME_ENGINE/bin
    ./student_autonomy_protocol > $CWD/protocol.log 2>&1 &


    # Wait for protocol to end
    cd $CWD
    SECONDS=0
    # while goal not eached and seconds < 300
    while ! grep -q "Goal reached" med_layer.log && [[ "$SECONDS" -lt $PROTOCOL_TIMEOUT ]]; do
        sleep 1
    done
    killall student_autonomy_protocol
    killall mediation_layer
    killall physics_simulator
    killall roscore
    tm="Nan"
    cd $CWD
    if [[ "$SECONDS" -lt $PROTOCOL_TIMEOUT ]]; then
        medout=`grep "Goal reached" med_layer.log`
        value=${medout#*"Goal reached"}
        value=`echo $value | grep -m 1 -Eo '[0-9]+([.][0-9]+)?'`
        echo $value
        tm=$value
    fi
    echo $tm
    echo $TEAM_NAME, "Nan", $dt >> $leaderboard
    echo $TEAM_NAME, $tm, $dt
    cd $CWD
done < $TEAMS

#==========Clean up===============#
git config --global advice.detachedHead true


