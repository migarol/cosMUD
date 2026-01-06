#!/bin/bash
################################################################################
# cosMUD Autonomous World System - Shutdown Script
################################################################################

MUD_DIR="/home/user/cosMUD/Dev"
PID_FILE="${MUD_DIR}/smaug.pid"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
echo -e "${YELLOW}  Shutting down cosMUD...${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check if PID file exists
if [ ! -f "$PID_FILE" ]; then
    echo -e "${YELLOW}⚠️  MUD doesn't appear to be running (no PID file)${NC}"
    exit 1
fi

# Get PID
MUD_PID=$(cat "$PID_FILE")

# Check if process exists
if ! ps -p "$MUD_PID" > /dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Process $MUD_PID not found (already stopped?)${NC}"
    rm -f "$PID_FILE"
    exit 1
fi

# Send shutdown signal
echo -e "${YELLOW}Sending shutdown signal to PID $MUD_PID...${NC}"
kill "$MUD_PID"

# Wait for graceful shutdown (max 10 seconds)
for i in {1..10}; do
    if ! ps -p "$MUD_PID" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ MUD shut down gracefully${NC}"
        rm -f "$PID_FILE"
        exit 0
    fi
    sleep 1
done

# If still running, force kill
echo -e "${YELLOW}⚠️  Graceful shutdown timeout, forcing...${NC}"
kill -9 "$MUD_PID" 2>/dev/null

sleep 1
if ! ps -p "$MUD_PID" > /dev/null 2>&1; then
    echo -e "${GREEN}✓ MUD shut down (forced)${NC}"
    rm -f "$PID_FILE"
else
    echo -e "${RED}✗ Failed to shut down MUD!${NC}"
    exit 1
fi
