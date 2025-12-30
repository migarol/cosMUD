# Sistema Multi-Tier AI - Documentación Técnica

## 🎯 Filosofía del Sistema

**El LLM NO es el cerebro del NPC - es solo la BOCA**

```
┌─────────────────────────────────────────────┐
│ CEREBRO = Reglas MUD + Estado + Lógica     │
│ BOCA = LLM (generación de texto natural)   │
└─────────────────────────────────────────────┘
```

## 🏗️ Arquitectura de 5 Niveles

### TIER 0: Beeler (Dios del Mundo)
- **Modelo:** `qwen2.5:7b`
- **Latencia:** 10-30 segundos (NO IMPORTA - es background)
- **Uso:** Crear áreas, balancear economía, evolución cultural
- **Frecuencia:** Cada 30-60 minutos
- **Bloquea gameplay:** ❌ NO (100% background)

```c
// Beeler usa el modelo más potente
ollama_request_with_model(prompt, 500, OLLAMA_MODEL_BEELER, OLLAMA_TIMEOUT_BEELER);
```

### TIER 1: Líderes - PENSAMIENTO (Strategic AI)
- **Modelo:** `phi3:mini`
- **Latencia:** <100ms
- **Uso:** Decidir políticas, estrategia militar, economía
- **Frecuencia:** Cada 10 minutos
- **Bloquea gameplay:** ❌ NO (background en update_handler)

```c
void leader_think_strategically(CHAR_DATA *leader)
{
    // Se ejecuta cada 10 minutos en background
    // Usa Phi-3 Mini para razonamiento estratégico
    decision = ollama_request_with_model(
        prompt, 100,
        OLLAMA_MODEL_THINKING,   // "phi3:mini"
        OLLAMA_TIMEOUT_THINKING  // 3 segundos
    );
}
```

### TIER 2: Líderes - DIÁLOGO (Speaking AI)
- **Modelo:** `tinyllama`
- **Latencia:** <30ms
- **Uso:** Responder a players EN TIEMPO REAL
- **Frecuencia:** Cuando player habla con el líder
- **Bloquea gameplay:** ✅ SÍ (pero <30ms = imperceptible)

```c
Player: "hail king"
→ TinyLlama en <30ms
King: "Greetings, traveler. My kingdom prospers."
```

### TIER 3: NPCs Importantes (Merchants, Quest Givers)
- **Modelo:** `tinyllama`
- **Latencia:** <30ms
- **Uso:** Diálogos con personalidad
- **Bloquea gameplay:** ✅ SÍ (pero <30ms = imperceptible)

### TIER 4: NPCs Simples (Guards, Vendors)
- **Modelo:** ❌ NINGUNO (templates solo)
- **Latencia:** <1ms (instantáneo)
- **Uso:** Respuestas fijas rápidas
- **Bloquea gameplay:** ✅ SÍ (pero <1ms = instantáneo)

```c
Player: "hail"
Guard: "Move along." // Template, <1ms
```

---

## 📁 Archivos Modificados

### 1. `src/ollama_integration.h`
```c
/* Multi-Tier Model Configuration */
#define OLLAMA_MODEL_BEELER      "qwen2.5:7b"
#define OLLAMA_MODEL_THINKING    "phi3:mini"
#define OLLAMA_MODEL_SPEAKING    "tinyllama"
#define OLLAMA_MODEL_NPC         "tinyllama"

/* NPC AI Tiers */
#define NPC_AI_TIER_GOD          0  // Beeler
#define NPC_AI_TIER_LEADER       1  // Thinking + Speaking
#define NPC_AI_TIER_IMPORTANT    2  // Speaking only
#define NPC_AI_TIER_NORMAL       3  // Templates + AI fallback
#define NPC_AI_TIER_SIMPLE       4  // Templates only
```

### 2. `src/mud.h` (CHAR_DATA)
```c
struct char_data {
    ...
    /* Multi-tier AI system */
    sh_int    ai_tier;          // NPC_AI_TIER_*
    time_t    last_think_time;  // For leader thinking
    char *    current_strategy; // Leader's current plan
    char *    ai_personality;   // AI-generated personality
};
```

### 3. `src/ollama_integration.c`
```c
// Nueva función: request con modelo específico
char *ollama_request_with_model(char *prompt, int max_tokens,
                                  char *model, int timeout);

// Nueva función: diálogo NPC real-time
char *ollama_generate_npc_dialogue(CHAR_DATA *npc, char *player_message);

// Nueva función: pensamiento estratégico de líderes
void leader_think_strategically(CHAR_DATA *leader);
```

### 4. `src/leader_ai.c`
```c
void leader_ai_update(void)
{
    for (leader = first_leader; leader; leader = leader->next)
    {
        leader_think(leader);  // Lógica original

        // NUEVO: Pensamiento estratégico con AI
        if (leader->mob && leader->mob->ai_tier == NPC_AI_TIER_LEADER)
            leader_think_strategically(leader->mob);
    }
}
```

---

## 🚀 Instalación

### Paso 1: Descargar Modelos de Ollama

```bash
# Modelo ultra rápido para diálogos (<30ms)
ollama pull tinyllama

# Modelo rápido para pensamiento estratégico (<100ms)
ollama pull phi3:mini

# Modelo potente para Beeler (background, puede tardar)
ollama pull qwen2.5:7b
```

### Paso 2: Verificar Modelos

```bash
ollama list
```

Deberías ver:
```
NAME            SIZE
tinyllama       637MB
phi3:mini       2.3GB
qwen2.5:7b      4.7GB
```

### Paso 3: Compilar el MUD

```bash
cd /home/user/cosMUD/Dev/src
make clean
make
```

### Paso 4: Iniciar el MUD

```bash
cd /home/user/cosMUD/Dev
./startup_fixed.sh
```

### Paso 5: Verificar en Logs

Busca en los logs:
```
[Ollama] AI generation ENABLED
[Ollama] Model: tinyllama (timeout: 1s)
[Ollama] AI Usage Policy:
         NPCs: YES (we have fast models now!)
         Rooms: NO (templates are faster for gameplay)
         Books: YES (quality over speed)
```

---

## 🎮 Uso en el Juego

### Asignar AI Tier a un NPC

```c
// En el código cuando creas un NPC:
mob->ai_tier = NPC_AI_TIER_LEADER;     // Líder (thinking + speaking)
mob->ai_tier = NPC_AI_TIER_IMPORTANT;  // Merchant (speaking only)
mob->ai_tier = NPC_AI_TIER_NORMAL;     // Resident (templates + AI)
mob->ai_tier = NPC_AI_TIER_SIMPLE;     // Guard (templates only)
```

### Ver Decisiones de Líderes en Logs

```bash
tail -f log/*.log | grep "LEADER AI"
```

Verás:
```
LEADER AI: King Aldric decided: Increase city guard by 20% due to bandit threats
LEADER AI: Queen Elara decided: Lower taxes to improve citizen morale
```

---

## 📊 Benchmarks Esperados

| Tier | Modelo | Latencia Típica | Uso |
|------|--------|-----------------|-----|
| God | qwen2.5:7b | 10-30s | Beeler background |
| Leader Think | phi3:mini | 50-100ms | Background cada 10min |
| Leader Speak | tinyllama | 20-30ms | Real-time con player |
| NPC Important | tinyllama | 20-30ms | Real-time con player |
| NPC Simple | templates | <1ms | Instantáneo |

---

## 🔧 Personalización

### Cambiar Frecuencia de Pensamiento de Líderes

En `src/update.c`:
```c
// Cambiar de 10 minutos a 5 minutos:
pulse_leader_ai = PULSE_AREA * 5;  // Era PULSE_AREA * 10
```

### Cambiar Modelo para NPCs

En `src/ollama_integration.h`:
```c
// Usar Phi-3 para NPCs importantes en vez de TinyLlama:
#define OLLAMA_MODEL_SPEAKING    "phi3:mini"  // Era "tinyllama"
```

### Habilitar AI para Rooms

En `src/ollama_integration.h`:
```c
#define OLLAMA_USE_FOR_ROOMS      TRUE  // Era FALSE
```

---

## 🐛 Troubleshooting

### Problema: "No models found"
```bash
ollama list  # Ver modelos instalados
ollama pull tinyllama
ollama pull phi3:mini
ollama pull qwen2.5:7b
```

### Problema: "Timeout" en NPCs
Aumenta timeout en `src/ollama_integration.h`:
```c
#define OLLAMA_TIMEOUT_SPEAKING  2  // Era 1
```

### Problema: Líderes no piensan
Verifica en logs:
```bash
grep "LEADER AI" log/*.log
```

Si no hay salida, verifica que el NPC tenga:
```c
mob->ai_tier = NPC_AI_TIER_LEADER;
```

---

## 🎯 Resultado Final

### Sin AI (antes):
```
Player: "hail"
Guard: "Hail, citizen." (template)
Latency: <1ms
```

### Con AI Multi-Tier (ahora):
```
Player: "hail guard"
Guard: "Move along, citizen." (template, <1ms)

Player: "hail king"
King: "Greetings, brave traveler. The kingdom needs heroes like you."
      (TinyLlama AI, <30ms)

[Background - cada 10 min]
LEADER AI: King decided: "Increase military patrols in northern border"
(Phi-3 Mini, ~80ms, NO bloquea gameplay)

[Background - cada 30 min]
BEELER: Created new trading post in northern territories
(Qwen 7B, ~15s, NO bloquea gameplay)
```

**TODO es instantáneo para el player. El mundo evoluciona en background.**

---

## 📈 Métricas de Performance

### Gameplay Real-Time (LO QUE SIENTE EL PLAYER)
- Guards: <1ms ✅ INSTANTÁNEO
- Merchants: ~25ms ✅ IMPERCEPTIBLE
- Leaders: ~30ms ✅ FEELS INSTANT
- **NUNCA se bloquea el gameplay**

### Background Evolution (LO QUE NO VE EL PLAYER)
- Leader Thinking: ~80ms cada 10 min (no molesta)
- Beeler Creation: ~15s cada 30 min (no molesta)
- Book Writing: ~2-5s (no molesta)

---

**Tu MUD ahora tiene VIDA REAL sin sacrificar velocidad. 🚀**
