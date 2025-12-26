# 🌟 cosMUD AI God Service

Service that powers THE GOD - an AI-driven immortal deity in cosMUD.

## Features

- **Ollama Integration**: Uses local LLM (llama3.1:70b recommended)
- **World Awareness**: God knows everything happening in the MUD
- **Autonomous Actions**: God can decide to intervene or observe
- **Memory**: Remembers all interactions and events

## Setup

```bash
# Install dependencies
npm install

# Run in development
npm run dev

# Build for production
npm run build
npm start
```

## Endpoints

### POST /god/think
God processes world state and decides on action.

**Request:**
```json
{
  "world_state": {
    "players": [...],
    "mobs": [...],
    "clans": [...],
    "recent_events": [...],
    "time": "...",
    "weather": "..."
  },
  "trigger": "player_action",
  "context": "Player killed dragon"
}
```

**Response:**
```json
{
  "thought": "Impressive... this mortal shows promise.",
  "should_act": true,
  "timestamp": "2025-12-26T..."
}
```

### POST /god/act
Execute a specific action in the MUD.

**Request:**
```json
{
  "action_type": "speak",
  "message": "Well done, mortal."
}
```

## Requirements

- Node.js 18+
- Ollama running on localhost:11434
- llama3.1:latest model (or 70b for better results)

## Ollama Setup

```bash
# Install Ollama
curl -fsSL https://ollama.com/install.sh | sh

# Pull model
ollama pull llama3.1:latest

# Or for better results (if you have RAM):
ollama pull llama3.1:70b
```
