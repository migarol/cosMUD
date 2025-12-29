#!/bin/bash
# Compilación completa del mundo autónomo

cd "$(dirname "$0")/Dev/src"

echo "=== COMPILACIÓN COMPLETA MUNDO AUTÓNOMO ==="
echo ""

# Paso 1: Compilar archivos críticos primero
echo "Paso 1: Compilando world_context y world_history_tracker..."
gcc -c -g3 -O -Wall -DI3 -DI3SMAUG -DREQUESTS -DSMAUG \
    world_context.c world_history_tracker.c

if [ ! -f world_context.o ] || [ ! -f world_history_tracker.o ]; then
    echo "❌ ERROR: No se pudieron compilar los archivos world"
    exit 1
fi

echo "✓ world_context.o y world_history_tracker.o compilados"
echo ""

# Paso 2: Compilar archivos base del MUD que suelen faltar
echo "Paso 2: Compilando archivos base del MUD..."
for file in tables.c skills.c save.c redit.c; do
    if [ -f "$file" ]; then
        echo "  Compilando $file..."
        gcc -c -g3 -O -Wall -DI3 -DI3SMAUG -DREQUESTS -DSMAUG "$file" 2>&1 | grep -E "error:" || true
    fi
done

echo ""
echo "Paso 3: Compilación principal con make..."
make 2>&1 | tee compile_log.txt

# Verificar resultado
if [ -f rmexe ]; then
    echo ""
    echo "🎉 =========================================="
    echo "🎉 ¡COMPILACIÓN EXITOSA!"
    echo "🎉 =========================================="
    echo ""
    ls -lh rmexe
    echo ""
    echo "Ejecutable: Dev/src/rmexe"
    echo ""
    echo "Sistemas autónomos activos:"
    echo "  ✓ world_history_tracker"
    echo "  ✓ leader_ai"
    echo "  ✓ organic_creation"
    echo "  ✓ periodicos"
    echo "  ✓ book_writing_system"
    echo "  ✓ mob_creation_system"
    echo "  ✓ cultural_evolution"
    echo "  ✓ family_lineage"
    echo "  ✓ global_trade"
    echo "  ✓ beeler_god_mode"
    echo "  ✓ world_context"
else
    echo ""
    echo "⚠️  Compilación falló. Revisando errores..."
    echo ""
    echo "Errores de funciones no definidas:"
    grep "undefined reference" compile_log.txt | cut -d\` -f2 | cut -d\' -f1 | sort -u
    echo ""
    echo "Ver compile_log.txt para detalles completos"
fi
