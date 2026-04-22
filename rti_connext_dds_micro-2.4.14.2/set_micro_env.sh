#!/bin/bash

# 1. Get the absolute path of the script directory
# Using 'readlink' to handle symbolic links correctly
SCRIPT_DIR="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"

# 2. Set RTI Environment Variables for the current session
export NDDSHOME="$SCRIPT_DIR"
export RTIMEHOME="$SCRIPT_DIR"

# 3. Define the architecture (Ensure this matches your $RTIMEHOME/lib folder)
# Recommended: Verify the folder name in the 'lib' directory
export RTIMEARCH="x64Linux4gcc7.3.0"

# 4. Update PATH safely for the current session
# We check if the path is already in PATH to avoid redundant entries
# if [[ ":$PATH:" != *":$RTIMEHOME/rtiddsgen/scripts:"* ]]; then
#     export PATH="$RTIMEHOME/rtiddsgen/scripts:$RTIMEHOME/lib/$RTIMEARCH:$PATH"
# fi

# 5. Persistent Configuration (Optional - use with caution)
# Instead of simple grep, we use a marker to manage the block
#MARKER="# RTI_DDS_MICRO_ENV_SET"
#if ! grep -q "$MARKER" ~/.bashrc; then
#    echo -e "\n$MARKER" >> ~/.bashrc
#    echo "export NDDSHOME=\"$NDDSHOME\"" >> ~/.bashrc
#    echo "export RTIMEHOME=\"$RTIMEHOME\"" >> ~/.bashrc
#    echo "export RTIMEARCH=\"$RTIMEARCH\"" >> ~/.bashrc
#   echo "export PATH=\"\$RTIMEHOME/rtiddsgen/scripts:\$RTIMEHOME/resource/scripts:\$RTIMEHOME/lib/\$RTIMEARCH:\$PATH\"" >> ~/.bashrc
#    echo "[INFO] Persistent environment settings added to ~/.bashrc"
#fi

# 6. Summary
echo "---------------------------------------------------"
echo "RTI Connext Micro Environment Set (Linux)"
echo "NDDSHOME  : $NDDSHOME"
echo "RTIMEHOME  : $RTIMEHOME"
echo "RTIMEARCH : $RTIMEARCH"
echo "---------------------------------------------------"