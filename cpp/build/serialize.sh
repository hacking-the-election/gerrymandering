#!/bin/bash

# takes a state name and runs according to build_data.list
# just useful for minor testing where I want to run `serialize.sh "wyoming"`
# and not worry about actual keys or files

# export DYLD_LIBRARY_PATH=/usr/local/boost/lib/:$DYLD_LIBRARY_PATH

#==================================
# hardcoded paths for serialization
#==================================
BINARY="bin/serialize_state"
DATA="../../data/"
RAW="raw/"
BIN="bin/cpp/"
BUILDFILE="build/build_data.list"

CWD=$(pwd | rev | cut -d '/' -f -2 | rev)

# has to be run from the right dir
# or the paths will not make sense
if [ "$CWD" != "gerrymandering/cpp" ]; then
    echo "Not in right dir, please cd into gerrymandering/cpp"
    echo "exitting..."
    exit 1;
fi

# read the correct line from the build file
CMD=$(cat "$BUILDFILE" | grep "\"$1\"" | cut -d ':' -f 2- | tr ' ' '\n' | grep . | tr '\n' ' ')
# start building the command string
CMDSTR="$BINARY "

# for each space-delimited argument
for (( i = 1; i <= "$(echo "$CMD" | tr ' ' '\n' | grep . | wc -l)"; i++)); do
    
    ARG=$(echo "$CMD" | cut -d ' ' -f "$i")
    END=$(echo "$ARG" | rev | cut -d '.' -f 1 | rev)

    if [ "$END" == "json" ] || [ "$END" == "tab" ]; then
        # is a raw data argument
        CMDSTR="${CMDSTR}${DATA}${RAW}${ARG} "
    elif [ "$(echo "$ARG" | cut -d '=' -f 1)" == "--keys" ]; then
        # is a keys argument
        CMDSTR="${CMDSTR}$ARG "
    elif [ "$END" == "state" ]; then
        # is an output argument
        CMDSTR="${CMDSTR}${DATA}${BIN}${ARG}"
    fi
done

echo ${CMDSTR}
exec ${CMDSTR}
