#!/bin/bash

# cosMUD Startup Script
# Puerto por defecto: 4500

PORT=${1:-4500}
BASEDIR="/home/user/cosMUD/Dev"
LOGDIR="$BASEDIR/log"

# Crear directorio de logs si no existe
mkdir -p "$LOGDIR"

# Limpiar shutdown.txt si existe
if [ -f "$BASEDIR/area/shutdown.txt" ]; then
    rm -f "$BASEDIR/area/shutdown.txt"
fi

# Cambiar al directorio de área
cd "$BASEDIR/area" || exit 1

# Generar nombre de log con timestamp
LOGFILE="$LOGDIR/$(date +%Y%m%d_%H%M%S).log"

echo "========================================" | tee "$LOGFILE"
echo "cosMUD - Starting up..." | tee -a "$LOGFILE"
echo "Port: $PORT" | tee -a "$LOGFILE"
echo "Base Directory: $BASEDIR" | tee -a "$LOGFILE"
echo "Log File: $LOGFILE" | tee -a "$LOGFILE"
echo "Time: $(date)" | tee -a "$LOGFILE"
echo "========================================" | tee -a "$LOGFILE"
echo "" | tee -a "$LOGFILE"

# Ejecutar el MUD
echo "Launching rmexe..." | tee -a "$LOGFILE"
"$BASEDIR/src/rmexe" "$PORT" 2>&1 | tee -a "$LOGFILE"

# Si termina, mostrar mensaje
echo "" | tee -a "$LOGFILE"
echo "MUD shutdown at $(date)" | tee -a "$LOGFILE"
