#!/bin/bash
# Script rápido de compilación para mundo autónomo

echo "=== Compilando Mundo Autónomo ==="

# Compilar archivos críticos primero
echo "1. Compilando world_context y world_history_tracker..."
gcc -c -g3 -O -Wall -DI3 -DI3SMAUG -DREQUESTS -DSMAUG \
    world_context.c world_history_tracker.c || {
    echo "❌ Error compilando archivos world"
    exit 1
}

echo "✓ Archivos world compilados"

# Compilar con make
echo "2. Ejecutando make..."
make

# Verificar
if [ -f rmexe ]; then
    echo ""
    echo "🎉 ¡ÉXITO! Ejecutable creado:"
    ls -lh rmexe
else
    echo ""
    echo "❌ Falló. Errores:"
    make 2>&1 | grep "undefined reference" | cut -d\` -f2 | cut -d\' -f1 | sort -u | head -20
fi
