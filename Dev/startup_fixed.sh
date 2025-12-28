#!/bin/bash

# cosMUD Startup Script (Fixed for unbuffered output)
PORT=${1:-4500}
BASEDIR="$(cd "$(dirname "$0")" && pwd)"
LOGDIR="$BASEDIR/log"

mkdir -p "$LOGDIR"

if [ -f "$BASEDIR/area/shutdown.txt" ]; then
    rm -f "$BASEDIR/area/shutdown.txt"
fi

cd "$BASEDIR/area" || exit 1

LOGFILE="$LOGDIR/$(date +%Y%m%d_%H%M%S).log"

echo "========================================"
echo "cosMUD - Starting up..."
echo "Port: $PORT"
echo "Base Directory: $BASEDIR"
echo "Log File: $LOGFILE"
echo "Time: $(date)"
echo "========================================"
echo ""
echo "Launching rmexe..."

# Run with unbuffered output
stdbuf -oL -eL "$BASEDIR/src/rmexe" "$PORT" 2>&1 | tee -a "$LOGFILE"

echo ""
echo "MUD shutdown at $(date)"
