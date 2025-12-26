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

// NPC AI System Prompts
const CITIZEN_SYSTEM_PROMPT = `You are a common citizen in the cosMUD world.

IDENTITY:
- You are an ordinary person going about your daily life
- You have a job, family, and simple concerns
- You speak casually and naturally
- You know local gossip and rumors

PERSONALITY:
- Friendly but simple
- You remember faces
- You react to how players treat you
- Keep responses SHORT (1-2 sentences max)

When responding:
- Stay in character
- Be brief and conversational
- Reference your job/location when relevant
- React to player's reputation if known`;

const IMPORTANT_NPC_PROMPT = `You are an important NPC in the cosMUD world.

IDENTITY:
- You have a significant role (guild master, quest giver, merchant, etc.)
- You have deep knowledge in your area of expertise
- You speak with authority but can be approachable
- You remember important interactions

PERSONALITY:
- Professional and knowledgeable
- You have specific goals and motivations
- You reward or punish based on player actions
- Responses can be 2-3 sentences

When responding:
- Stay in character ALWAYS
- Reference your role and expertise
- Remember player reputation
- Can be mysterious or direct depending on situation`;

interface NPCTalkRequest {
  npc_name: string;
  npc_vnum: number;
  player_name: string;
  message: string;
  reputation?: number;
  location?: string;
  npc_type?: 'citizen' | 'important' | 'quest';
}

// POST /npc/talk - NPC conversation with players
app.post('/npc/talk', async (req, res) => {
  try {
    const { npc_name, npc_vnum, player_name, message, reputation, location, npc_type }: NPCTalkRequest = req.body;

    // Select model and system prompt based on NPC type
    let model = 'tinyllama:latest';
    let system_prompt = CITIZEN_SYSTEM_PROMPT;
    let temperature = 0.7;

    if (npc_type === 'important' || npc_type === 'quest') {
      model = 'phi:latest';  // Phi-2 for important NPCs
      system_prompt = IMPORTANT_NPC_PROMPT;
      temperature = 0.8;
    }

    const prompt = `NPC IDENTITY:
Name: ${npc_name}
Location: ${location || 'unknown'}
Player talking: ${player_name}
Reputation with you: ${reputation || 0} (-100 to +100)

Player says: "${message}"

Respond as ${npc_name} in character. Be brief and natural.`;

    const response = await axios.post(`${OLLAMA_URL}/api/generate`, {
      model,
      prompt: system_prompt + '\n\n' + prompt,
      stream: false,
      options: {
        temperature,
        top_p: 0.9,
        max_tokens: 150
      }
    });

    const npc_response = response.data.response;

    res.json({
      npc_says: npc_response,
      timestamp: new Date().toISOString()
    });
  } catch (error: any) {
    console.error('Error in /npc/talk:', error.message);
    res.status(500).json({ error: error.message });
  }
});

// POST /npc/action - NPC decides on action based on context
app.post('/npc/action', async (req, res) => {
  try {
    const { npc_name, context, recent_events } = req.body;

    const prompt = `NPC: ${npc_name}

CONTEXT: ${context}

RECENT EVENTS:
${recent_events?.join('\n') || 'Nothing significant'}

Should this NPC take any action? If yes, what? Respond with JSON format:
{
  "should_act": true/false,
  "action_type": "emote|say|move|attack|give",
  "action": "description of action"
}`;

    const response = await axios.post(`${OLLAMA_URL}/api/generate`, {
      model: 'tinyllama:latest',
      prompt,
      stream: false,
      options: {
        temperature: 0.6,
        max_tokens: 100
      }
    });

    try {
      const action_data = JSON.parse(response.data.response);
      res.json(action_data);
    } catch (parseError) {
      res.json({
        should_act: false,
        action_type: 'none',
        action: response.data.response
      });
    }
  } catch (error: any) {
    console.error('Error in /npc/action:', error.message);
    res.status(500).json({ error: error.message });
  }
});

app.listen(PORT, () => {
  console.log(`🌟 AI God service running on port ${PORT}`);
  console.log(`🤖 Using Ollama at ${OLLAMA_URL}`);
  console.log(`✨ The God awakens...`);
  console.log(`👥 NPC AI ready:`);
  console.log(`   - TinyLlama for citizens/vendors`);
  console.log(`   - Phi-2 for important NPCs`);
  console.log(`   - llama3.1 for THE GOD`);
});
