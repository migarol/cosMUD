#!/bin/bash
################################################################################
# cosMUD Autonomous World System - Startup Script
#
# This script starts the MUD server with all autonomous systems enabled:
# - Beeler God Mode (divine oversight)
# - Leader AI (strategic decisions)
# - Organic Creation (self-expanding world)
# - Player World Impact (lasting changes)
# - Universal Mob AI (intelligent NPCs)
# - World Context Analysis
# - Redis event streaming (optional)
# - Ollama AI integration (optional)
################################################################################

# Configuration
PORT=${1:-4000}
MUD_DIR="/home/user/cosMUD/Dev"
LOG_DIR="${MUD_DIR}/log"
PID_FILE="${MUD_DIR}/smaug.pid"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}  cosMUD Autonomous World System${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check if already running
if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE")
    if ps -p "$OLD_PID" > /dev/null 2>&1; then
        echo -e "${YELLOW}⚠️  MUD is already running (PID: $OLD_PID)${NC}"
        echo -e "${YELLOW}   Use ./shutdown.sh to stop it first${NC}"
        exit 1
    else
        echo -e "${YELLOW}⚠️  Removing stale PID file${NC}"
        rm -f "$PID_FILE"
    fi
fi

# Change to MUD directory
cd "$MUD_DIR" || exit 1

# Check if executable exists
if [ ! -f "src/smaug" ]; then
    echo -e "${RED}✗ Error: smaug executable not found!${NC}"
    echo -e "${YELLOW}  Run: cd src && make${NC}"
    exit 1
fi

# Create log directory if needed
mkdir -p "$LOG_DIR"

# Optional: Check Redis connection
if command -v redis-cli &> /dev/null; then
    if redis-cli ping &> /dev/null; then
        echo -e "${GREEN}✓ Redis connected (real-time events enabled)${NC}"
    else
        echo -e "${YELLOW}⚠️  Redis not running (real-time events disabled)${NC}"
        echo -e "${YELLOW}   Start with: redis-server --daemonize yes${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  Redis not installed (real-time events disabled)${NC}"
fi

# Optional: Check Ollama
if command -v ollama &> /dev/null; then
    if pgrep -x "ollama" > /dev/null; then
        echo -e "${GREEN}✓ Ollama running (AI content generation enabled)${NC}"
    else
        echo -e "${YELLOW}⚠️  Ollama not running (AI content generation disabled)${NC}"
        echo -e "${YELLOW}   Start with: ollama serve &${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  Ollama not installed (AI content generation disabled)${NC}"
fi

echo ""
echo -e "${BLUE}Starting MUD server on port ${PORT}...${NC}"
echo ""

# Start the MUD in background
nohup src/smaug $PORT > "${LOG_DIR}/smaug.log" 2>&1 &
MUD_PID=$!

# Save PID
echo $MUD_PID > "$PID_FILE"

# Wait a moment to see if it starts successfully
sleep 2

# Check if process is still running
if ps -p $MUD_PID > /dev/null; then
    echo -e "${GREEN}✓ MUD started successfully!${NC}"
    echo ""
    echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}  Server Details${NC}"
    echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
    echo -e "  ${YELLOW}PID:${NC}        $MUD_PID"
    echo -e "  ${YELLOW}Port:${NC}       $PORT"
    echo -e "  ${YELLOW}Log file:${NC}   ${LOG_DIR}/smaug.log"
    echo -e "  ${YELLOW}Connect:${NC}    telnet localhost $PORT"
    echo ""
    echo -e "${GREEN}Autonomous Systems:${NC}"
    echo -e "  ✓ Beeler God Mode (divine oversight)"
    echo -e "  ✓ Leader AI (strategic decisions)"
    echo -e "  ✓ Organic Creation (world expansion)"
    echo -e "  ✓ Player Impact (lasting changes)"
    echo -e "  ✓ Universal Mob AI (intelligent NPCs)"
    echo -e "  ✓ World Context (geography/politics)"
    echo ""
    echo -e "${BLUE}Commands:${NC}"
    echo -e "  ${YELLOW}View logs:${NC}    tail -f ${LOG_DIR}/smaug.log"
    echo -e "  ${YELLOW}Connect:${NC}      telnet localhost $PORT"
    echo -e "  ${YELLOW}Shutdown:${NC}     ./shutdown.sh"
    echo -e "${BLUE}════════════════════════════════════════════════════════════════${NC}"
else
    echo -e "${RED}✗ Failed to start MUD!${NC}"
    echo -e "${YELLOW}  Check log: ${LOG_DIR}/smaug.log${NC}"
    rm -f "$PID_FILE"
    exit 1
fi
