# 🌍 ARQUITECTURA: MUNDO VIVO ORGÁNICO

## VISIÓN GENERAL

Un mundo donde cada mob tiene **personalidad única**, **rutinas personalizadas**,
**memoria de interacciones**, y donde la AI **crea y modifica** el mundo
orgánicamente sin cambios drásticos.

---

## 🏗️ CAPAS DE LA ARQUITECTURA

```
┌──────────────────────────────────────────────────────────────┐
│                    🧠 AI WORLD DIRECTOR                      │
│         (Orquesta todo - Llama3.1 / GPT-4 Level)            │
│  - Genera personalidades únicas                              │
│  - Crea horarios personalizados                              │
│  - Decide eventos orgánicos                                  │
│  - Escribe noticias/rumores                                  │
│  - Modifica áreas gradualmente                               │
└──────────────────────────────────────────────────────────────┘
                            ▼
┌──────────────────────────────────────────────────────────────┐
│              📊 GENERATIVE CONTENT LAYER                     │
├──────────────────────────────────────────────────────────────┤
│  mob_personality_gen.c    - Genera personalidades únicas     │
│  schedule_generator.c     - Crea horarios personalizados     │
│  news_system.c            - Sistema de noticias/rumores      │
│  area_evolution.c         - Modifica áreas orgánicamente     │
│  event_narrative.c        - Genera narrativas emergentes     │
└──────────────────────────────────────────────────────────────┘
                            ▼
┌──────────────────────────────────────────────────────────────┐
│              💾 PERSISTENCE & MEMORY LAYER                   │
├──────────────────────────────────────────────────────────────┤
│  mob_memory_db/           - Memoria de cada mob              │
│  ├─ conversations/        - Qué se dijeron                   │
│  ├─ relationships/        - Relaciones con otros mobs        │
│  ├─ schedules/           - Horarios personalizados           │
│  └─ personality/         - Rasgos de personalidad            │
│                                                               │
│  world_state_db/          - Estado del mundo                 │
│  ├─ news_archive/        - Archivo de noticias               │
│  ├─ rumors/              - Rumores activos                   │
│  ├─ events_history/      - Historia de eventos               │
│  └─ area_changes/        - Cambios graduales en áreas        │
└──────────────────────────────────────────────────────────────┘
                            ▼
┌──────────────────────────────────────────────────────────────┐
│              ⚙️ SIMULATION EXECUTION LAYER                   │
├──────────────────────────────────────────────────────────────┤
│  world_simulation.c       - Rutinas base (YA EXISTE)         │
│  npc_ai.c                 - Conversaciones AI (YA EXISTE)    │
│  living_world.c           - Sistema base (YA EXISTE)         │
│  ai_god_advanced.c        - Análisis de mundo (YA EXISTE)    │
└──────────────────────────────────────────────────────────────┘
```

---

## 📋 CASOS DE USO ESPECÍFICOS

### 1️⃣ MOBS CON PERSONALIDADES ÚNICAS

**Problema:** No todos los mobs son iguales.

**Solución:** Base de datos de personalidades generadas por AI

```json
{
  "mob_vnum": 21045,
  "name": "Tsythia",
  "personality_type": "SEDENTARY_SCHOLAR",
  "traits": {
    "mobility": 5,        // 0-100 (5 = casi nunca se mueve)
    "social": 40,         // Algo social
    "work_ethic": 95,     // Muy trabajador
    "sleep_pattern": "NOCTURNAL_PARTIAL"  // Trabaja de noche
  },
  "schedule": {
    "custom": true,
    "routines": [
      {"hour": "20:00-23:00", "activity": "RESEARCH", "location": "library"},
      {"hour": "23:00-2:00", "activity": "WRITE", "location": "study"},
      {"hour": "2:00-10:00", "activity": "SLEEP", "location": "quarters"},
      {"hour": "10:00-12:00", "activity": "EAT", "location": "quarters"},
      {"hour": "12:00-20:00", "activity": "MEDITATE", "location": "study"}
    ]
  },
  "ai_prompt": "Tsythia es un erudito nocturno obsesionado con textos antiguos.
                Rara vez sale de la biblioteca. Habla en tono académico y
                pedante. Le molestan las interrupciones."
}
```

### 2️⃣ GUARDIAS CON TURNOS ROTATORIOS

**Problema:** Los guardias no están 24/7 - tienen turnos.

**Solución:** Sistema de turnos generado dinámicamente

```json
{
  "location": "Darkhaven Main Gate",
  "guard_pool": [10234, 10235, 10236, 10237],  // 4 guardias
  "shifts": {
    "morning": {"hours": "6:00-14:00", "guards": [10234, 10235]},
    "afternoon": {"hours": "14:00-22:00", "guards": [10236, 10237]},
    "night": {"hours": "22:00-6:00", "guards": [10234, 10236]}
  },
  "transition_time": "5min",  // Overlap para cambio de turno
  "captain": 10240  // Supervisa turnos
}
```

**Comportamiento:**
- 13:55 - Guardia 10234 dice: "Mi turno casi termina, gracias a los dioses."
- 14:00 - Guardia 10236 llega: "Buenas tardes. ¿Alguna novedad?"
- 14:05 - Guardia 10234 se va a descansar

### 3️⃣ TRABAJADORES NOCTURNOS

**Problema:** No todos duermen de noche - panaderos, taberneros, guardias.

**Solución:** Profesiones con horarios nocturnos

```c
PROFESSION_SCHEDULES[] = {
  {
    profession: "BAKER",
    schedule: "2:00-10:00 WORK (preparar pan)",
              "10:00-12:00 SELL (vender en tienda)",
              "12:00-20:00 SLEEP",
              "20:00-22:00 EAT/SOCIAL"
  },
  {
    profession: "TAVERN_KEEPER",
    schedule: "18:00-2:00 WORK (atender bar)",
              "2:00-10:00 SLEEP",
              "10:00-18:00 RESTOCK/PREP"
  },
  {
    profession: "NIGHT_WATCH",
    schedule: "22:00-6:00 PATROL",
              "6:00-14:00 SLEEP",
              "14:00-22:00 TRAIN/SOCIAL"
  }
}
```

### 4️⃣ SISTEMA DE NOTICIAS/RUMORES

**Problema:** ¿Cómo se enteran de eventos en otras ciudades?

**Solución:** Sistema de noticias con mensajeros

```
EVENTO:
  - Hora 14:30 en Darkhaven
  - Jugador "Argon" mata al Dragon de Darkhaven

GENERACIÓN DE NOTICIA (AI):
  Título: "¡HÉROE DERROTA AL DRAGÓN!"
  Cuerpo: "El legendario mago Argon enfrentó al temible dragón de las
           montañas del norte y emergió victorioso. Testigos afirman..."
  Perspectiva: DARKHAVEN_OFFICIAL (heroica)

PROPAGACIÓN:
  Hora 14:45 - Mensajero mob parte de Darkhaven
  Hora 15:30 - Llega a Midgaard
  Hora 15:35 - Pregonero en Midgaard grita la noticia

VARIACIÓN POR CIUDAD:
  Darkhaven: "¡Nuestro héroe Argon salvó la ciudad!"
  Midgaard: "Dicen que un mago mató un dragón en el norte..."
  Necropolis: "Tontos humanos celebrando... el dragón era débil."

RUMOR DISTORSIONADO (hora 18:00):
  "Escuché que 5 magos mataron 3 dragones en Darkhaven..."
```

### 5️⃣ PERSPECTIVAS DIFERENTES

**Problema:** Misma historia, diferentes versiones.

**Solución:** AI genera narrativas desde múltiples ángulos

```
EVENTO: Guerra entre RDAF y Order of Chaos

PERSPECTIVA RDAF:
  "Los criminales del Order of Chaos atacaron sin provocación.
   Defendimos nuestro honor valientemente..."

PERSPECTIVA OOC:
  "RDAF llevaba meses saboteando nuestro comercio.
   Les dimos una lección que no olvidarán..."

PERSPECTIVA NEUTRAL (Mercader):
  "Dos bandas de idiotas destruyeron mi caravana.
   Me importa un bledo quién empezó..."

PERSPECTIVA NIÑO HUÉRFANO:
  "Los soldados malos mataron a mi papá.
   No sé quiénes eran, solo recuerdo el fuego..."
```

### 6️⃣ MEMORIA PERSISTENTE ENTRE MOBS

**Problema:** Mobs no recuerdan conversaciones previas.

**Solución:** Base de datos de memoria extendida

```json
{
  "mob_vnum": 21045,
  "memories": [
    {
      "type": "CONVERSATION",
      "with_mob": 21067,
      "timestamp": "2024-12-26 14:30",
      "summary": "Truhuga me pidió prestado un libro sobre dragones.
                  Dijo que lo devolvería en 3 días. No confío en él.",
      "emotional_impact": -5,  // Ligeramente molesto
      "promises": ["Devolver libro en 3 días"]
    },
    {
      "type": "WITNESSED_EVENT",
      "event": "Argon mató al dragón",
      "timestamp": "2024-12-26 14:32",
      "summary": "Vi a Argon pelear contra el dragón. Impresionante magia.",
      "emotional_impact": +20,  // Muy impresionado
      "will_tell_others": true
    },
    {
      "type": "RELATIONSHIP_CHANGE",
      "with_mob": 10240,
      "old_relation": 0,
      "new_relation": 30,
      "reason": "Me salvó de unos bandidos hace 2 semanas"
    }
  ]
}
```

**Comportamiento:**
```
3 días después, Truhuga vuelve:
Tsythia: "Ah, Truhuga. Espero que traigas MI libro."
Truhuga: "Sí, aquí está. Gracias."
Tsythia: "La próxima vez, devuélvelo a tiempo. *fruncir el ceño*"
         [Relación +5 por cumplir promesa]

Jugador pregunta a Tsythia sobre el dragón:
Jugador: "¿Qué pasó con el dragón?"
Tsythia: "Ah sí, vi a Argon derrotarlo. Magia impresionante,
          aunque un poco imprudente. Casi prende fuego a mi biblioteca."
```

---

## 🤖 AI WORLD ARCHITECT

**Sistema central que GENERA contenido dinámico:**

```python
class AIWorldArchitect:
    """
    Orquesta todo el mundo vivo.
    Crea contenido gradualmente, orgánicamente.
    """

    def generate_mob_personality(self, mob_vnum):
        """
        Genera personalidad única para cada mob.
        Considera: profesión, raza, edad, ubicación
        """
        prompt = f"""
        Genera una personalidad única para:
        Mob: {mob.name} (vnum {mob_vnum})
        Profesión: {mob.profession}
        Ubicación: {mob.home_area}

        Incluye:
        - Rasgos de personalidad
        - Horario personalizado (no genérico)
        - Manías/quirks únicos
        - Relaciones con otros mobs cercanos
        - Prompt para conversaciones AI
        """
        return ollama.generate(model="llama3.1", prompt=prompt)

    def evolve_area_organically(self, area_vnum, days_passed):
        """
        Modifica área gradualmente con el tiempo.
        Ejemplos:
        - Tienda nueva abre
        - Edificio viejo se deteriora
        - Jardín crece/muere según estación
        - NPCs envejecen
        """
        if days_passed < 30:
            return  # Cambios muy lentos

        changes = self.ai_decide_gradual_changes(area_vnum)
        self.apply_changes_slowly(changes)

    def generate_news_article(self, event):
        """
        Convierte evento en noticia con perspectiva.
        """
        perspectives = ['official', 'neutral', 'opposing', 'civilian']
        articles = []

        for perspective in perspectives:
            article = self.ai_write_news(event, perspective)
            articles.append(article)

        return articles

    def detect_emergent_narratives(self):
        """
        Encuentra historias emergentes de interacciones.
        Ejemplo: Si muchos mobs hablan de "caravana perdida",
                 genera quest espontánea.
        """
        patterns = self.analyze_mob_conversations()
        if pattern.frequency > threshold:
            self.create_dynamic_quest(pattern)
```

---

## 📐 IMPLEMENTACIÓN GRADUAL

### FASE 1: Personalidades Únicas (PRÓXIMO)
- [ ] `mob_personality_db/` - Base de datos JSON
- [ ] `personality_generator.c` - Genera personalidades
- [ ] Integrar con `world_simulation.c` para horarios custom
- [ ] Comando immortal: `genpersonality <mob_vnum>`

### FASE 2: Sistema de Noticias
- [ ] `news_system.c` - Genera y propaga noticias
- [ ] Mensajero mobs que llevan noticias
- [ ] Pregoneros en ciudades principales
- [ ] Periódico objeto: "Darkhaven Daily"

### FASE 3: Memoria Extendida
- [ ] `mob_memory_extended.c` - DB de memoria
- [ ] Integrar con `npc_ai.c` para contexto
- [ ] Mobs recuerdan conversaciones pasadas
- [ ] Relaciones dinámicas entre mobs

### FASE 4: Turnos y Horarios Custom
- [ ] `schedule_system.c` - Sistema de turnos
- [ ] Profesiones con horarios nocturnos
- [ ] Guardias rotativos
- [ ] Transiciones orgánicas

### FASE 5: Evolución de Áreas
- [ ] `area_evolution.c` - Cambios graduales
- [ ] Edificios que se deterioran/mejoran
- [ ] Vegetación que crece
- [ ] NPCs que envejecen

---

## 🎯 PRINCIPIOS DE DISEÑO

1. **GRADUAL > DRÁSTICO**
   - Cambios lentos, orgánicos
   - No spawns instantáneos de áreas
   - Evolución natural

2. **EMERGENTE > SCRIPTED**
   - Historias surgen de interacciones
   - No quest pre-escritas
   - Jugadores afectan narrativa

3. **MEMORIA > AMNESIA**
   - Mobs recuerdan TODO
   - Promesas, eventos, conversaciones
   - Relaciones evolucionan

4. **ÚNICO > GENÉRICO**
   - Cada mob diferente
   - Horarios personalizados
   - Personalidades únicas

5. **ORGÁNICO > ARTIFICIAL**
   - Mundo se siente real
   - No "respawn instantáneo"
   - Consecuencias persistentes

---

## 💡 EJEMPLOS DE EMERGENCIA

### Historia Emergente 1: La Caravana Perdida
```
Día 1: Caravana no llega a Midgaard
Día 2: Mercader en Darkhaven pregunta por ella
Día 3: 5+ mobs mencionan "caravana perdida"
Día 4: AI God detecta patrón → genera quest
Día 5: Rumor se propaga: "Bandidos en el bosque"
Día 6: Jugador investiga → encuentra caravana
Día 7: Noticias: "¡Héroe rescata caravana!"
```

### Historia Emergente 2: Romance entre NPCs
```
Mes 1: Guardia (mob A) y Mesonera (mob B) charlan frecuentemente
Mes 2: AI detecta alta frecuencia de interacción
Mes 3: Sistema genera: mob A tiene +40 relación con mob B
Mes 4: Mob A le da flores a mob B
Mes 5: Mobs se casan (evento generado por AI)
Mes 6: Jugadores descubren la historia orgánicamente
```

---

## 🔧 TECNOLOGÍAS

- **AI Engine:** Ollama (llama3.1:latest)
- **Persistence:** JSON files + SQLite para queries rápidos
- **Memory:** Hash tables + disk storage
- **Scheduling:** Cron-like system con timers
- **News Propagation:** Graph traversal entre ciudades

---

## 📊 MÉTRICAS DE ÉXITO

- ✅ Cada mob tiene personalidad única
- ✅ Horarios no genéricos (al menos 60% custom)
- ✅ Noticias se propagan entre ciudades
- ✅ Mobs recuerdan interacciones (90%+ memoria)
- ✅ Historias emergentes ocurren (1+ por semana)
- ✅ Jugadores dicen "esto se siente REAL"

---

## 🚀 SIGUIENTES PASOS

1. Implementar `mob_personality_generator.c`
2. Crear base de datos JSON de personalidades
3. Modificar `world_simulation.c` para leer horarios custom
4. Implementar sistema de noticias básico
5. Extender memoria de NPCs

¿Quieres que empiece con la **Fase 1: Personalidades Únicas**?
