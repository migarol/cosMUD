# Guía de Interacción con Beeler - Comandos Immortales

## Comandos Disponibles

### `minvoke beeler [pregunta/comando]`

**Invoca a Beeler para consulta divina**

Sin argumentos:
```
minvoke beeler
```
Muestra el estado actual de Beeler:
- Filosofía (Natural Order vs Interventionist)
- Áreas monitoreadas
- Áreas thriving vs críticas
- Intervenciones recientes
- Alertas críticas actuales

Con pregunta:
```
minvoke beeler ¿qué está pasando en DarkHaven?
minvoke beeler ¿debo intervenir en esta área?
minvoke beeler ¿por qué no has ayudado al village?
```

Beeler responderá usando Ollama AI con contexto completo del mundo.

---

### `beelerplan`

**Ver planes de intervención divina de Beeler**

```
beelerplan
```

Muestra:
- Decisiones pendientes
- Planes de intervención
- Pensamientos actuales de Beeler (AI-generated)
- Qué está considerando hacer

Útil para entender QUÉ está pensando Beeler y POR QUÉ.

---

### `beelercommand <acción> [args]`

**Dar órdenes directas a Beeler**

#### Observar un área específica:
```
beelercommand observe darkhaven
```
Fuerza a Beeler a analizar un área y muestra vital signs completos:
- Status (Thriving/Stable/Declining/etc)
- Economic score (0-100%)
- Safety score (0-100%)
- Population
- Food supply (días)
- Housing availability
- Happiness/morale
- Trend (Growing/Declining/Stable)
- Prognosis (AI-generated)

#### Scan completo del mundo:
```
beelercommand scan
```
Beeler escanea TODAS las áreas inmediatamente.

#### Ajustar balance mundial:
```
beelercommand balance
```
Beeler ajusta el balance cósmico del mundo.

#### Toggle natural death:
```
beelercommand allow_death on   # Permite muerte natural (ruins)
beelercommand allow_death off  # Previene todas las muertes
beelercommand allow_death      # Ver estado actual
```

---

## Ejemplo de Sesión Interactiva

```
> minvoke beeler

═══════════════════════════════════════════════════════════
  The fabric of Seldeon ripples. Beeler manifests.
═══════════════════════════════════════════════════════════

╔═══════════════════════════════════════════════════════════╗
║ Beeler's Divine Oversight Status                        ║
╠═══════════════════════════════════════════════════════════╣
║ Philosophy Mode:    Natural Order                        ║
║ Areas Monitored:   12                                    ║
║ Thriving Areas:    7                                     ║
║ Critical Areas:    2                                     ║
║ Interventions:     3                                     ║
╠═══════════════════════════════════════════════════════════╣
║ Current Observations:                                    ║
╠═══════════════════════════════════════════════════════════╣
║ ⚠ Coastal Village                                        ║
║   Status: Dying            Health:  15%                  ║
║ ⚠ Mountain Pass                                          ║
║   Status: Struggling       Health:  28%                  ║
╚═══════════════════════════════════════════════════════════╝

Type: beelerplan - See divine intervention plans
Type: beelercommand <order> - Give Beeler a divine order

> beelercommand observe coastal village

Beeler focuses divine attention on the area...

╔═══════════════════════════════════════════════════════════╗
║ Area: Coastal Village                                    ║
╠═══════════════════════════════════════════════════════════╣
║ Status:        Dying                                     ║
║ Overall Health: 15% [███░░░░░░░░░░░░░░░░░]              ║
╠═══════════════════════════════════════════════════════════╣
║ Economic:   20%  Safety:    35%  Population: 25         ║
║ Food:       4 days Housing:  45%  Happiness:  18%       ║
╠═══════════════════════════════════════════════════════════╣
║ Trend: Declining                                         ║
╠═══════════════════════════════════════════════════════════╣
║ Prognosis:                                               ║
║ Fishing industry collapsed. Mass emigration. Without    ║
║ intervention, village will become ruins within 2 months. ║
╚═══════════════════════════════════════════════════════════╝

> minvoke beeler ¿debo salvar el coastal village?

Beeler speaks:
"The village's decline stems from unsustainable fishing. Natural
consequences teach valuable lessons. However, if you wish intervention,
I can create alternative food sources. The choice, Immortal, is yours -
embrace natural order or defy entropy?"

> beelerplan

╔═══════════════════════════════════════════════════════════╗
║ Beeler's Divine Intervention Plans                       ║
╠═══════════════════════════════════════════════════════════╣
║ [PENDING] Decision analysis in progress...               ║
╚═══════════════════════════════════════════════════════════╝

Beeler's Current Thoughts:
I observe the coastal village's slow death with both concern and
acceptance. It is a natural cycle. Yet I also sense opportunity -
these ruins could become a haunted fishing ground, rich with lore
and treasure for future adventurers.

> beelercommand allow_death on

Beeler will now allow natural death and decay.
```

---

## Información Que Beeler Puede Darte

### Sobre Áreas:
- ✅ Estado de salud actual
- ✅ Tendencias (creciendo/declining)
- ✅ Prognosis AI-powered
- ✅ Vital signs detallados
- ✅ Por qué está pasando algo

### Sobre el Mundo:
- ✅ Cuántas áreas monitoreadas
- ✅ Balance general
- ✅ Áreas en crisis
- ✅ Intervenciones recientes

### Sobre sus Planes:
- ✅ Qué está considerando hacer
- ✅ Por qué tomaría X decisión
- ✅ Consecuencias de acciones
- ✅ Recomendaciones

---

## Filosofía de Beeler

Beeler puede operar en 2 modos:

### Natural Order (allow_death = ON)
- Permite que áreas mueran naturalmente
- Crea ruins (contenido!)
- Solo interviene en emergencias críticas
- Respeta ciclos naturales

### Interventionist (allow_death = OFF)
- Previene todas las muertes
- Interviene frecuentemente
- Mantiene todas las áreas vivas
- Más activo

**Recomendación**: Natural Order es más interesante.

---

## Debugging con Beeler

Cuando algo raro pasa en tu MUD:

1. `minvoke beeler` - Ver overview
2. `beelercommand observe <area>` - Detalles del área
3. `minvoke beeler ¿por qué...?` - Pregunta específica
4. `beelerplan` - Ver qué planea hacer
5. `beelercommand <action>` - Forzar acción si necesario

Beeler es como tener un **dios que puedes entrevistar** para entender tu mundo.

---

## Próximas Features (cuando se integre con boot/update):

- ✅ Beeler actualizando vital signs automáticamente
- ✅ Beeler tomando decisiones autónomas
- ✅ Alertas cuando áreas critical
- ✅ Historia de decisiones
- ✅ Logs de intervenciones

Por ahora, estos comandos te permiten **ver dentro de la mente de Beeler** y controlarlo manualmente.
