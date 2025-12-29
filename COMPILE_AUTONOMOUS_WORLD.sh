#!/bin/bash
# Script to compile cosMUD with autonomous world systems

cd Dev/src

echo "=== Compiling Autonomous World Systems ==="

# Ensure world_context and world_history_tracker are compiled first
echo "Step 1: Compiling world context and history tracker..."
gcc -c -g3 -O -Wall -DI3 -DI3SMAUG -DREQUESTS -DSMAUG world_context.c world_history_tracker.c

if [ ! -f world_context.o ] || [ ! -f world_history_tracker.o ]; then
    echo "ERROR: Failed to compile world files!"
    exit 1
fi

echo "Step 2: Building main executable..."
make

if [ -f rmexe ]; then
    echo ""
    echo "=== SUCCESS! ==="
    echo "Autonomous world MUD compiled successfully!"
    echo "Executable: Dev/src/rmexe"
    ls -lh rmexe
else
    echo ""
    echo "=== Build failed. Checking for missing functions... ==="
    make 2>&1 | grep "undefined reference" | head -20
fi
