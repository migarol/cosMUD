# 🌍 Guía Completa: Activar el Mundo Autónomo

## 📋 Pasos para que TODO funcione

---

## PASO 1: Compilar el MUD ✅

```bash
cd /tu/ruta/cosMUD/Dev/src

# Compilar
./compile.sh

# Verificar que funcionó
ls -lh rmexe
# Deberías ver: -rwxr-xr-x ... 6.2M ... rmexe
```

Si ves el archivo `rmexe`, ¡compiló bien! 🎉

---

## PASO 2: Configurar Ollama (IA para contenido) 🤖

Los mobs necesitan Ollama para escribir libros y generar contenido inteligente.

### Opción A: Instalar Ollama local
```bash
# Instalar Ollama
curl -fsSL https://ollama.com/install.sh | sh

# Descargar modelo (usa uno pequeño para empezar)
ollama pull llama3.2:3b
# O si quieres mejor calidad:
ollama pull mistral

# Verificar que esté corriendo
curl http://localhost:11434/api/tags
```

### Opción B: Usar Ollama remoto
Edita `Dev/src/ollama_integration.h` y cambia:
```c
#define OLLAMA_HOST "http://localhost:11434"
// A tu servidor remoto:
#define OLLAMA_HOST "http://tu-servidor:11434"
```

Luego recompila:
```bash
./compile.sh
```

---

## PASO 3: Preparar el entorno del MUD 📁

```bash
cd /tu/ruta/cosMUD/Dev

# Crear directorio para datos del mundo autónomo
mkdir -p system/memories
mkdir -p system/history
mkdir -p system/books
mkdir -p system/cultures
mkdir -p system/families
mkdir -p system/trade

# Dar permisos
chmod -R 755 system/
```

---

## PASO 4: Arrancar el MUD 🚀

```bash
cd /tu/ruta/cosMUD/Dev/src

# Opción 1: Arranque directo (para testing)
./rmexe

# Opción 2: Arranque con log (recomendado)
./rmexe > ../log/mud.log 2>&1 &

# Opción 3: Con screen (para dejarlo corriendo)
screen -S cosmud
./rmexe
# Presiona Ctrl+A luego D para salir sin cerrar
```

---

## PASO 5: Verificar que los sistemas estén activos 🔍

### 5.1 Conectarse al MUD
```bash
telnet localhost 4000
# O usa tu cliente de MUD favorito
```

### 5.2 Crear personaje o loguear como admin

### 5.3 Verificar sistemas autónomos (como IMMORTAL)

```
# Ver estado de Beeler (supervisión divina)
beeler status

# Ver estadísticas del mundo
world status

# Ver qué está pasando en tiempo real
beeler observe

# Ver culturas desarrolladas
culture list

# Ver familias
families

# Ver rutas comerciales
trade routes

# Ver historial del mundo
history recent
```

---

## PASO 6: Activar características una por una 🎯

### 6.1 Probar el sistema de libros
```bash
# Como immortal, forzar a un mob a escribir
writebook history "The Great War"

# Ver libros escritos
books
```

### 6.2 Activar IA para mobs
```bash
# Ver si Ollama está conectado
ollama status

# Probar generación de contenido
test ollama "Write a poem about dragons"
```

### 6.3 Iniciar crecimiento orgánico
```bash
# Ver áreas monitoreadas
beeler areas

# Ver evolución de un área
area analyze <nombre_area>
```

---

## PASO 7: Configurar frecuencias de actualización ⏰

Los sistemas autónomos se actualizan automáticamente. Frecuencias por defecto:

```
beeler_observation:     30 minutos (supervisa todo)
leader_ai:              1 hora     (decisiones estratégicas)
organic_creation:       2 horas    (crecimiento de pueblos)
mob_creation:           30 minutos (mobs crean NPCs)
book_writing:           1 hora     (procesa cola de escritura)
cultural_evolution:     1 hora     (culturas evolucionan)
family_lineage:         1 hora     (nacimientos/matrimonios)
global_trade:           30 minutos (caravanas viajan)
```

**Para cambiar frecuencias:** Edita `Dev/src/update.c` líneas 2157-2168

---

## PASO 8: Verificar que todo funciona ✅

### Checklist de 10 minutos:

```bash
# 1. Conectar como immortal
telnet localhost 4000

# 2. Verificar log del MUD
tail -f ../log/mud.log | grep "AUTONOMOUS\|BEELER\|BOOK\|LEADER"

# 3. Ver inicializaciones al arranque
grep "Initializing" ../log/mud.log

# Deberías ver:
# Initializing World Context...
# Initializing Beeler God Mode...
# Initializing World History...
# Initializing Leader AI...
# Initializing Organic Creation...
# Initializing Periodicos...
# Initializing Book Writing System...
# Initializing Mob Creation System...
# Initializing Cultural Evolution...
# Initializing Family Lineage System...
# Initializing Global Trade System...
```

---

## PASO 9: Primer test del mundo autónomo 🧪

### Test 1: ¿Los mobs tienen inteligencia?
```
# Crear un rey o líder
mpmset <mob> leader_type KING

# Esperar 1 hora de juego
advance_time 60

# Verificar decisiones
leader decisions <mob_name>
```

### Test 2: ¿Escriben libros?
```
# Crear un poeta o historiador
mpmset <mob> profession poet

# Forzar evento importante
beeler event "The dragon attacked the village"

# Esperar ~1 hora
# Verificar
books
```

### Test 3: ¿El mundo crece solo?
```
# Ver un pueblo pequeño
goto <pueblo>

# Esperar 6+ horas de juego (puedes acelerar con advance_time)
# El pueblo debería crecer automáticamente si:
# - Tiene población
# - Tiene recursos
# - Beeler lo aprueba
```

---

## PASO 10: Troubleshooting 🔧

### Problema: "Ollama not available"
```bash
# Verificar que Ollama esté corriendo
curl http://localhost:11434/api/tags

# Si no responde, arrancarlo:
ollama serve

# En otra terminal:
ollama pull llama3.2:3b
```

### Problema: "No se crean archivos en system/"
```bash
# Verificar permisos
ls -la system/
chmod -R 755 system/

# Verificar que el directorio exista
mkdir -p system/{memories,history,books,cultures,families,trade}
```

### Problema: "Los updates no corren"
```bash
# Ver el log en tiempo real
tail -f ../log/mud.log

# Buscar errores en las funciones de update
grep "UPDATE ERROR" ../log/mud.log

# Verificar que los pulsos se estén ejecutando
grep "pulse_" ../log/mud.log
```

### Problema: "Undefined reference" al compilar
```bash
# Ver qué falta
./compile.sh 2>&1 | grep "undefined reference" | cut -d\` -f2 | cut -d\' -f1 | sort -u

# Las más comunes:
# - olc_log       → necesitas redit.c o build.c
# - skill_table   → necesitas tables.c
# - save_char_obj → necesitas save.c
# - class_table   → necesitas tables.c

# Agregar al Makefile en O_FILES
```

---

## 📊 Comandos útiles para immortales

```bash
# Monitoreo general
beeler status          # Estado general de Beeler
world stats            # Estadísticas del mundo
autonomous report      # Reporte de todos los sistemas

# Historia del mundo
history recent         # Eventos recientes
history character <name>  # Historia de un personaje
history book <vnum>    # Qué libro menciona qué

# Cultura
culture list           # Todas las culturas
culture <area>         # Cultura de un área
traditions <area>      # Tradiciones desarrolladas

# Familias
families               # Todas las familias
lineage <name>         # Árbol genealógico

# Comercio
trade routes           # Rutas comerciales activas
trade caravans         # Caravanas en movimiento

# Libros
books                  # Todos los libros escritos
book read <vnum>       # Leer un libro

# Creación orgánica
organic pending        # Creaciones pendientes
organic history        # Historial de creaciones

# Líderes
leaders                # Todos los líderes con IA
leader thoughts <mob>  # Qué está pensando un líder
```

---

## 🎯 ¡Todo listo!

Si completaste todos los pasos:

✅ MUD compilado  
✅ Ollama funcionando  
✅ Directorios creados  
✅ Servidor corriendo  
✅ Sistemas verificados  
✅ Comandos probados  

**¡Tu mundo autónomo está VIVO!** 🌍

---

## 🆘 Si algo falla

1. **Revisa el log:** `tail -f ../log/mud.log`
2. **Verifica Ollama:** `curl http://localhost:11434/api/tags`
3. **Checa permisos:** `ls -la system/`
4. **Busca errores de compilación:** `./compile.sh 2>&1 | grep error`
5. **Lee COMPILACION_README.md** para problemas de compilación

---

## 📝 Notas importantes

- **Primer arranque:** Los sistemas tardan ~30 min en "calentar"
- **Ollama opcional:** Si no está, los mobs usan templates genéricos
- **Performance:** Con 10+ áreas activas, puede usar CPU
- **Logs:** Revisar regularmente `../log/mud.log` para errores
- **Backups:** Los archivos en `system/` son importantes, respaldar

---

**¿Preguntas?** Revisa `COMPILACION_README.md` o el código de ejemplo en cada `.h`
