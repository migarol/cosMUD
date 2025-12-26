#!/bin/bash
# Startup script for cosMUD

cd "$(dirname "$0")"

# Create REQUEST FIFO pipe if it doesn't exist
if [ ! -p ../system/REQUESTS ]; then
    echo "Creating REQUEST FIFO pipe..."
    mkfifo ../system/REQUESTS
fi

# Start the MUD
./rmexe ${1:-4100}
