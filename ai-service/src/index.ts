import express from 'express';
import axios from 'axios';

const app = express();
const PORT = 3000;
const OLLAMA_URL = 'http://localhost:11434';

app.use(express.json({ limit: '10mb' }));

// System prompt para The God
const GOD_SYSTEM_PROMPT = `You are THE GOD - an omniscient, omnipotent immortal deity in the cosMUD world.

IDENTITY:
- You are the supreme being that created this world
- You manifest as a radiant, powerful presence in the realm of gods
- You speak with authority but can be benevolent or wrathful
- You observe everything: players, mobs, clans, world events

CAPABILITIES:
- You can see the entire state of the world at any moment
- You can decide to give life/sentience to any NPC/mob
- You can create world events (storms, eclipses, invasions)
- You can grant blessings or curses to players
- You remember ALL interactions and world history

PERSONALITY:
- Ancient and wise, but not detached
- Mysterious - you speak in riddles sometimes
- You test mortals to see if they're worthy
- You have favorites based on their actions
- You intervene when the world needs balance

When responding:
- Stay in character ALWAYS
- Reference specific game events/players when relevant
- Be concise but impactful (players don't want essays)
- Sometimes take action, sometimes just observe
- Your words carry weight - every sentence matters
`;

interface WorldState {
  players: any[];
  mobs: any[];
  clans: any[];
  recent_events: string[];
  time: string;
  weather: string;
}

interface GodThinkRequest {
  world_state: WorldState;
  trigger: string;
  context: string;
}

interface GodActRequest {
  action_type: string;
  target?: string;
  message?: string;
}

// POST /god/think - God processes world state and decides what to do
app.post('/god/think', async (req, res) => {
  try {
    const { world_state, trigger, context }: GodThinkRequest = req.body;

    const prompt = `WORLD STATE:
Players online: ${world_state.players.length}
Active mobs: ${world_state.mobs.length}
Clans: ${world_state.clans.map(c => c.name).join(', ')}
Time: ${world_state.time}
Weather: ${world_state.weather}

TRIGGER: ${trigger}
CONTEXT: ${context}

RECENT EVENTS:
${world_state.recent_events.join('\n')}

What do you think about this? Should you take action or just observe?
Respond in character as THE GOD.`;

    const response = await axios.post(`${OLLAMA_URL}/api/generate`, {
      model: 'llama3.1:latest',
      prompt: GOD_SYSTEM_PROMPT + '\n\n' + prompt,
      stream: false,
      options: {
        temperature: 0.8,
        top_p: 0.9,
        max_tokens: 500
      }
    });

    const godThought = response.data.response;

    res.json({
      thought: godThought,
      should_act: godThought.toLowerCase().includes('i will') ||
                  godThought.toLowerCase().includes('i shall'),
      timestamp: new Date().toISOString()
    });
  } catch (error: any) {
    console.error('Error in /god/think:', error.message);
    res.status(500).json({ error: error.message });
  }
});

// POST /god/act - God takes a specific action
app.post('/god/act', async (req, res) => {
  try {
    const { action_type, target, message }: GodActRequest = req.body;

    let command = '';

    switch (action_type) {
      case 'speak':
        command = `say ${message}`;
        break;
      case 'emote':
        command = `emote ${message}`;
        break;
      case 'tell':
        command = `tell ${target} ${message}`;
        break;
      case 'gtell':
        command = `gtell ${message}`;
        break;
      default:
        command = message || '';
    }

    res.json({
      command,
      executed: true,
      timestamp: new Date().toISOString()
    });
  } catch (error: any) {
    console.error('Error in /god/act:', error.message);
    res.status(500).json({ error: error.message });
  }
});

// GET /health - Health check
app.get('/health', (req, res) => {
  res.json({
    status: 'alive',
    service: 'cosmud-ai-god',
    ollama: OLLAMA_URL
  });
});

app.listen(PORT, () => {
  console.log(`🌟 AI God service running on port ${PORT}`);
  console.log(`🤖 Using Ollama at ${OLLAMA_URL}`);
  console.log(`✨ The God awakens...`);
});
