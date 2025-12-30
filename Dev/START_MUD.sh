#!/bin/bash

echo "=================================="
echo "  Starting Living World MUD"
echo "=================================="
echo ""

# Kill any old processes
pkill -9 rmexe 2>/dev/null
sleep 1

# Check Ollama
echo "Checking Ollama AI..."
if curl -s http://localhost:11434/api/tags > /dev/null 2>&1; then
    echo "✓ Ollama is running"

    # Check models
    echo ""
    echo "Available models:"
    ollama list 2>/dev/null || echo "  (ollama command not found, but service is running)"
else
    echo "⚠ Warning: Ollama not running"
    echo "  NPCs will use templates instead of AI"
    echo "  To enable AI:"
    echo "    ollama pull tinyllama"
    echo "    ollama pull phi3:mini"
    echo "    ollama pull qwen2.5:7b"
fi

echo ""
echo "Starting MUD..."
cd /home/user/cosMUD/Dev
./startup_fixed.sh

echo ""
echo "MUD started. Check log/smaug.log for details."
echo "To stop: pkill -9 rmexe"
