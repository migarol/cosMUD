# Ollama Setup - Get AI Working in Your MUD

## Current Status

**Your MUD is working correctly**, but NPCs are using template responses instead of AI because Ollama is not installed.

When you see:
```
You say 'hey tsythia, how do i get a sword?'
Mistress Tsythia nods thoughtfully.
```

This is the **template fallback** - it means Ollama isn't running.

---

## Quick Install (Linux)

### Step 1: Install Ollama

```bash
curl -fsSL https://ollama.com/install.sh | sh
```

This will:
- Download and install Ollama
- Set up the systemd service
- Start Ollama automatically

### Step 2: Verify Ollama is Running

```bash
# Check service status
systemctl status ollama

# Or test the API directly
curl http://localhost:11434/api/tags
```

You should see JSON output listing models (empty at first).

### Step 3: Pull the Required Models

```bash
# Ultra-fast model for NPC dialogue (<30ms target)
ollama pull tinyllama

# Fast model for leader strategic thinking (<100ms target)
ollama pull phi3:mini

# Powerful model for Beeler world creation (background, can be slow)
ollama pull qwen2.5:7b
```

**Download sizes:**
- tinyllama: ~637 MB
- phi3:mini: ~2.3 GB
- qwen2.5:7b: ~4.7 GB

**You can start with just tinyllama** to test NPC dialogue, then add the others later.

### Step 4: Verify Models are Downloaded

```bash
ollama list
```

You should see:
```
NAME            SIZE
tinyllama       637MB
phi3:mini       2.3GB
qwen2.5:7b      4.7GB
```

### Step 5: Restart the MUD

```bash
cd /home/user/cosMUD/Dev
pkill -9 rmexe
./START_MUD.sh
```

Look for this in the output:
```
✓ Ollama is running

Available models:
tinyllama       637 MB
```

---

## Testing AI Responses

### Connect to MUD

```bash
telnet localhost 4000
```

### Test with Different NPC Tiers

**TIER 4 (Simple) - Templates Only:**
```
goto 10300
say hey guard
Guard: Move along, citizen.  (instant, <1ms)
```

**TIER 2 (Important) - AI Dialogue:**
```
goto 10399
say hey tsythia, how do i get a sword?
Mistress Tsythia: You can find weapons at the armory, young one.  (<100ms)
```

The AI response will be:
- Different each time
- Contextual and in-character
- Generated in real-time by TinyLlama

---

## Troubleshooting

### Problem: "curl: command not found"

Install curl first:
```bash
# Debian/Ubuntu
apt-get install curl

# Red Hat/CentOS
yum install curl
```

### Problem: "Ollama not running" even after install

Start the service manually:
```bash
systemctl start ollama
systemctl enable ollama  # Auto-start on boot
```

Or run in foreground for debugging:
```bash
ollama serve
```

### Problem: NPCs still use templates after installing Ollama

1. **Check Ollama is running:**
   ```bash
   curl http://localhost:11434/api/tags
   ```

2. **Check MUD logs:**
   ```bash
   tail -f /home/user/cosMUD/Dev/log/*.log | grep -i ollama
   ```

3. **Restart MUD:**
   ```bash
   pkill -9 rmexe && ./START_MUD.sh
   ```

### Problem: Timeout errors in logs

If you see timeout messages, the model might be too slow for your hardware. You can:

1. **Use smaller models:**
   ```bash
   ollama pull tinyllama  # Smallest, fastest
   ```

2. **Disable AI for NPCs** (use templates only):
   Edit `src/ollama_integration.h`:
   ```c
   #define OLLAMA_USE_FOR_NPCS       FALSE  /* Was TRUE */
   ```

3. **Increase timeout** (already set to 3 seconds):
   ```c
   #define OLLAMA_TIMEOUT_SPEAKING  5  /* Was 3 */
   ```

   Then recompile:
   ```bash
   cd src && make && cd ..
   ```

---

## What Each Model Does

### TinyLlama (637 MB) - NPC Dialogue
**When it runs:** When player talks to NPCs (TIER 2+)
**Latency:** <30-100ms
**Blocks gameplay:** YES (but so fast it's imperceptible)

Example:
```
Player: "hail king, what news?"
King: "Greetings, traveler. The realm is at peace, but bandits trouble the roads."
```

### Phi-3 Mini (2.3 GB) - Leader Strategy
**When it runs:** Background every 10 minutes
**Latency:** ~100ms
**Blocks gameplay:** NO (runs in update_handler background)

Example (in logs):
```
LEADER AI: King Aldric decided: Increase guard patrols in eastern district
LEADER AI: Queen Elara decided: Lower taxes to improve citizen morale
```

### Qwen2.5:7b (4.7 GB) - Beeler World Creation
**When it runs:** Background every 30-60 minutes
**Latency:** 10-30 seconds (doesn't matter, it's background)
**Blocks gameplay:** NO (100% background)

Example (in logs):
```
BEELER: Created new trading post 'The Golden Merchant' in northern territories
BEELER: Evolved cultural trait: Darkhaven citizens now favor blue clothing
```

---

## Performance Expectations

### With Ollama Installed

| Action | Response Time | AI Used? |
|--------|--------------|----------|
| Talk to guard | <1ms | NO (template) |
| Talk to merchant | ~50ms | YES (TinyLlama) |
| Talk to king | ~50ms | YES (TinyLlama) |
| Leader thinking | ~100ms | YES (Phi-3, background) |
| Beeler creation | ~15s | YES (Qwen, background) |

**Gameplay feels instant** - even the AI responses are faster than network latency.

### Without Ollama Installed

Everything uses templates and is instant (<1ms), but NPCs have no personality variety.

---

## Minimum Requirement

**You only need TinyLlama** to get started:

```bash
ollama pull tinyllama
```

This gives you:
- AI dialogue for important NPCs
- Real-time responses (<100ms)
- Personality and context awareness

You can add the other models later for:
- Strategic leader decisions (phi3:mini)
- Rich world evolution (qwen2.5:7b)

---

## Next Steps After Setup

Once Ollama is running and models are loaded:

1. **Restart MUD** - it will auto-detect Ollama
2. **Connect via telnet** - `telnet localhost 4000`
3. **Test AI NPCs** - talk to Mistress Tsythia (goto 10399)
4. **Watch logs** - `tail -f log/*.log | grep -E "LEADER AI|BEELER|NPC DIALOGUE"`

You should see:
- NPCs responding with varied, contextual dialogue
- Leader decisions in background (every 10 min)
- Beeler world evolution (every 30+ min)

**Your world is now ALIVE** 🌍🤖

---

## Manual Ollama Setup (Alternative)

If the install script doesn't work, you can:

1. Download from https://ollama.com/download
2. Extract to /usr/local/bin/
3. Run `ollama serve` in background
4. Pull models as shown above

---

## Verification Checklist

- [ ] Ollama service is running (`systemctl status ollama`)
- [ ] At least tinyllama is installed (`ollama list`)
- [ ] MUD shows "✓ Ollama is running" on startup
- [ ] NPCs respond with AI dialogue, not just "nods thoughtfully"
- [ ] No timeout errors in logs

If all checked, your MUD has AI! 🎉
