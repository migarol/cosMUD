# Optimización de Rendimiento de Ollama

## Problema Identificado
Ollama funciona correctamente pero es **demasiado lento** para uso en tiempo real durante el gameplay del MUD.

## Solución Implementada

### 1. Modelo Más Pequeño y Rápido
- **Antes:** `llama3.2:latest` (probablemente 3b - más lento)
- **Ahora:** `llama3.2:1b` (modelo 1 billion params - mucho más rápido)

### 2. Timeout Reducido
- **Antes:** 5 segundos de timeout
- **Ahora:** 2 segundos de timeout (fail-fast para no bloquear gameplay)

### 3. Política de Uso Selectivo
El sistema ahora usa AI **SOLO** para contenido NO crítico:

| Tipo de Contenido | Usa AI? | Razón |
|-------------------|---------|-------|
| NPCs (personalidades) | **NO** | Las plantillas son instantáneas, mejor para gameplay |
| Rooms (descripciones) | **NO** | Las plantillas son instantáneas, mejor para gameplay |
| Libros (contenido) | **SÍ** | Los libros se escriben en background, pueden esperar |
| Cultura/Historia | **SÍ** | Evolución cultural es proceso de fondo |

### 4. Graceful Fallback
- Si Ollama está habilitado pero tarda más de 2 segundos → usa plantillas
- Si Ollama no está disponible → usa plantillas
- **El gameplay NUNCA se bloquea esperando AI**

## Instrucciones para el Usuario

### Paso 1: Descargar el Modelo Rápido
```bash
ollama pull llama3.2:1b
```

### Paso 2: Verificar que el modelo se descargó
```bash
ollama list
```

Deberías ver:
```
NAME             SIZE
llama3.2:1b      ...
llama3.2:latest  ...  (este es el lento que ya tenías)
```

### Paso 3: Reiniciar el MUD
```bash
cd /home/user/cosMUD/Dev
./startup_fixed.sh
```

### Paso 4: Verificar la Configuración
En los logs de inicio verás:

```
[Ollama] AI generation ENABLED
[Ollama] Connected to http://localhost:11434
[Ollama] Model: llama3.2:1b (timeout: 2s)
[Ollama] AI Usage Policy:
         NPCs: NO (templates are faster for gameplay)
         Rooms: NO (templates are faster for gameplay)
         Books: YES (quality over speed)
         Culture/History: YES (background processing)
```

## Resultado Esperado

### Gameplay en Tiempo Real
- Creación de NPCs: **INSTANTÁNEA** (usa plantillas)
- Descripciones de rooms: **INSTANTÁNEAS** (usa plantillas)
- Sin lag, sin esperas

### Contenido de Calidad en Background
- Libros escritos por el sistema: **Usa AI** (pueden tardar, no afecta gameplay)
- Evolución cultural: **Usa AI** (proceso de fondo, no bloquea nada)

## Archivos Modificados

1. `src/ollama_integration.h` - Configuración y políticas
2. `src/ollama_integration.c` - Implementación con políticas
3. `src/book_writing_system.c` - Respeta política de libros

## Personalización

Si quieres ajustar qué usa AI, edita en `src/ollama_integration.h`:

```c
/* AI Usage Policy - when to use AI vs templates */
#define OLLAMA_USE_FOR_NPCS       FALSE  /* Cambia a TRUE si quieres AI para NPCs */
#define OLLAMA_USE_FOR_ROOMS      FALSE  /* Cambia a TRUE si quieres AI para rooms */
#define OLLAMA_USE_FOR_BOOKS      TRUE   /* Cambia a FALSE si quieres templates */
#define OLLAMA_USE_FOR_CULTURE    TRUE   /* Background AI */
#define OLLAMA_USE_FOR_HISTORY    TRUE   /* Background AI */
```

Después de cambiar, recompila:
```bash
cd /home/user/cosMUD/Dev/src
make
```

## Benchmarks Esperados

Con llama3.2:1b deberías ver:
- Respuestas en 1-2 segundos (vs 10-30 segundos con 3b)
- Calidad ligeramente menor pero aceptable
- **Sin impacto en gameplay porque solo se usa en background**

---

**Resumen:** Ollama ahora es una mejora OPCIONAL para contenido de fondo, no un cuello de botella para gameplay.
