#!/bin/bash

# CONFIGURACIÓN
TEST_FILE="test_files.txt"
EXPECTED_OUT=$(mktemp)
SHELL_OUT=$(mktemp)
SHELL_ERR=$(mktemp)
TOTAL=0
PASSED=0
FAILED=0

# COLORES
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# COMPILACIÓN
echo -e "${BLUE}⚙️  Compilando shell...${NC}"
gcc -Wall -o shell shell.c
if [ ! -f ./shell ]; then
    echo -e "${RED}❌ Error: el binario 'shell' no fue generado${NC}"
    exit 1
fi

# ARCHIVO AUXILIAR
cat <<EOF > "$TEST_FILE"
imagen.png
documento.zip
imagen.jpg
archivo.c
prueba.sh
EOF

# FUNCIÓN PRINCIPAL DE TEST
run_test() {
    local input="$1"
    local description="$2"
    local should_error="$3"  # "error" if command should produce error, empty otherwise
    ((TOTAL++))

    echo -e "${YELLOW}➤ Test $TOTAL: $description${NC}"
    echo -e "   ${BLUE}Comando:${NC} $input"

    # Get expected output from bash
    bash -c "$input" 2>/dev/null > "$EXPECTED_OUT" || true
    
    # Ejecutar el shell interactivamente con script
    script -q -c "echo '$input' | ./shell" "$SHELL_OUT" >/dev/null 2>"$SHELL_ERR"
    
    # Limpiar la salida
    grep -v "Shell>" "$SHELL_OUT" > "$SHELL_OUT.tmp"
    grep -v "Ejecutando comando:" "$SHELL_OUT.tmp" > "$SHELL_OUT"
    grep -v "Con argumentos:" "$SHELL_OUT" > "$SHELL_OUT.tmp" 
    grep -v "args\[" "$SHELL_OUT.tmp" > "$SHELL_OUT"
    grep -v "Script started" "$SHELL_OUT" > "$SHELL_OUT.tmp"
    grep -v "Script done" "$SHELL_OUT.tmp" > "$SHELL_OUT"
    # Quitar líneas vacías al principio y al final
    sed -i '/./,$!d' "$SHELL_OUT"
    sed -i -e :a -e '/^\n*$/{$d;N;ba' -e '}' "$SHELL_OUT"
        
    # Compare outputs
    if diff -q "$EXPECTED_OUT" "$SHELL_OUT" > /dev/null; then
        echo -e "   ${GREEN}✅ Funcionalidad OK${NC}"
        ((PASSED++))
    else
        echo -e "   ${RED}❌ Funcionalidad FALLÓ${NC}"
        echo -e "   ${BLUE}Diferencias encontradas:${NC}"
        echo -e "   ${BLUE}Esperado:${NC}"
        cat "$EXPECTED_OUT" | head -5 | sed 's/^/     /'
        if [ ! -s "$EXPECTED_OUT" ]; then
            echo "     [salida vacía]"
        fi
        echo -e "   ${BLUE}Obtenido:${NC}"
        cat "$SHELL_OUT" | head -5 | sed 's/^/     /'
        if [ ! -s "$SHELL_OUT" ]; then
            echo "     [salida vacía]"
        fi
        ((FAILED++))
    fi
    echo ""
}

# Pruebas individuales
run_test "echo hola" "Echo simple"
run_test "echo \"hola mundo\"" "Echo con comillas dobles"
run_test "echo 'hola mundo'" "Echo con comillas simples"
run_test "echo hola | wc -w" "Pipe simple con wc"
run_test "seq 5 | grep 3" "Pipe con grep"
run_test "cat $TEST_FILE | grep -E \"\\.png$|\\.zip$\"" "Grep con regex compuesta"
run_test "ls -la | grep ^d | wc -l" "Pipeline triple"
run_test "echo \"\" | wc -c" "String vacío"

# RESUMEN FINAL
echo -e "${BLUE}============================================${NC}"
echo -e "         ${YELLOW}RESUMEN FINAL DE TESTS${NC}"
echo -e "   Total de tests:     $TOTAL"
echo -e "   ${GREEN}Funcionales OK:      $PASSED${NC}"
echo -e "   ${RED}Funcionales fallidos: $FAILED${NC}"
echo -e "${BLUE}============================================${NC}"

# LIMPIEZA
rm -f "$EXPECTED_OUT" "$SHELL_OUT" "$SHELL_OUT.tmp" "$SHELL_ERR" "$TEST_FILE"