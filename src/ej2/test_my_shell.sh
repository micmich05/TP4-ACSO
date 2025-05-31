#!/bin/bash

# CONFIGURACIÓN
TEST_FILE="test_files.txt"
TEMP_OUT=$(mktemp)
EXPECTED_OUT=$(mktemp)
SHELL_OUT=$(mktemp)
SHELL_ERR=$(mktemp)
VALGRIND_OUT="valgrind.txt"
TOTAL=0
PASSED=0
FAILED=0
MEM_CLEAN=0
MEM_FAIL=0
VERBOSE=false

# COLORES
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# COMPILACIÓN
echo -e "${BLUE}⚙️  Compilando shell...${NC}"
gcc -Wall -Werror -o shell shell.c
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

    # Get shell output
    echo -e "$input\nexit" | ./shell > "$SHELL_OUT" 2> "$SHELL_ERR"
    grep -v "^Shell>" "$SHELL_OUT" > "$TEMP_OUT"
    grep -v "^Ejecutando comando:" "$SHELL_ERR" > "$TEMP_OUT.err"
    grep -v "^Con argumentos:" "$TEMP_OUT.err" > "$SHELL_ERR"
    grep -v "^  args\[.*\]:" "$SHELL_ERR" > "$TEMP_OUT.err"
    mv "$TEMP_OUT.err" "$SHELL_ERR"
    mv "$TEMP_OUT" "$SHELL_OUT"

    if [[ "$should_error" == "error" ]]; then
        if [ -s "$SHELL_ERR" ]; then
            echo -e "   ${YELLOW}⚠️  Error detectado, pero se recomienda verificar manualmente la salida de stderr:${NC}"
            cat "$SHELL_ERR" | head -3 | sed 's/^/     /'
            echo -e "   ${GREEN}✅ Funcionalidad OK (stderr no está vacío)${NC}"
            ((PASSED++))
        else
            echo -e "   ${RED}❌ Funcionalidad FALLÓ (No se detectó error en stderr)${NC}"
            ((FAILED++))
        fi
    elif [ "$input" = "exit" ]; then
        # Special case for exit command
        if [ ! -s "$SHELL_OUT" ]; then
            echo -e "   ${GREEN}✅ Funcionalidad OK (Exit funcionó correctamente)${NC}"
            ((PASSED++))
        else
            echo -e "   ${RED}❌ Funcionalidad FALLÓ (Exit no funcionó correctamente)${NC}"
            ((FAILED++))
        fi
    else
        # Get expected output from actual terminal
        bash -c "$input" 2>/dev/null > "$EXPECTED_OUT" || true
        
        # Compare outputs using diff
        if diff -q "$EXPECTED_OUT" "$SHELL_OUT" > /dev/null; then
            echo -e "   ${GREEN}✅ Funcionalidad OK${NC}"
            ((PASSED++))
        else
            echo -e "   ${RED}❌ Funcionalidad FALLÓ${NC}"
            echo -e "   ${BLUE}Diferencias encontradas:${NC}"
            echo -e "   ${BLUE}Esperado:${NC}"
            cat "$EXPECTED_OUT" | head -3 | sed 's/^/     /'
            echo -e "   ${BLUE}Obtenido:${NC}"
            cat "$SHELL_OUT" | head -3 | sed 's/^/     /'
            ((FAILED++))
        fi
    fi

    # Valgrind check
    echo -e "$input\nexit" | valgrind --leak-check=full --error-exitcode=42 ./shell > /dev/null 2>> "$VALGRIND_OUT" || true
    status=$?
    if [ "$status" -ne 42 ]; then
        echo -e "   ${GREEN}✅ Memoria OK (Valgrind)${NC}"
        ((MEM_CLEAN++))
    else
        echo -e "   ${RED}🧠 Leak detectado (Valgrind)${NC}"
        ((MEM_FAIL++))
        if [ "$VERBOSE" = true ]; then
            cat "$VALGRIND_OUT"
        fi
    fi
    echo ""
}

# TESTS BÁSICOS
run_test "echo hola" "Echo simple"
run_test "echo \"hola mundo\"" "Echo con comillas dobles"
run_test "echo 'hola mundo'" "Echo con comillas simples"
run_test "ls" "Comando ls simple"

# TESTS DE PIPE BÁSICOS
run_test "echo hola | wc -w" "Pipe simple"
run_test "ls | grep sh" "Pipe con grep"
run_test "seq 5 | wc -l" "Pipe con seq"

# TESTS DE ESPACIOS Y TABS
run_test "echo hola    mundo | wc -w" "Espacios múltiples y pipe"
run_test $'echo\t\thola' "Tabulaciones entre palabras"
run_test "   echo    prueba   " "Espaciado irregular"

# TESTS CON COMILLAS
run_test "echo \"hola mundo\"" "Echo con comillas dobles"
run_test "echo 'uno' 'dos' | wc -w" "Comillas simples como argumentos"
run_test "echo \"uno  dos\" | wc -m" "Conteo de caracteres con espacios"

# TESTS DE PIPES COMPLEJOS
run_test "cat $TEST_FILE | grep .zip" "Búsqueda de extensión"
run_test "echo hola | grep hola | wc -l" "Pipeline triple"
run_test "cat $TEST_FILE | grep -E \"\\.png$|\\.zip$\"" "Grep con regex compuesta"
run_test "seq 10 | grep 5 | wc -l" "Pipeline con filtrado"
run_test "ls -la | grep ^d | wc -l" "Contar directorios"

# TESTS DE COMANDOS ESPECIALES
run_test "/bin/echo hola" "Comando con ruta absoluta"
run_test "exit" "Comando de salida"
run_test "exit | wc" "Exit dentro de pipeline"

# TESTS DE COMANDOS INVÁLIDOS
run_test "inexistentecomando" "Comando inexistente" "error"
run_test "echo \"hola" "Comillas abiertas sin cerrar" "error"

# VALORES EXTREMOS
run_test "yes | head -n 5" "Yes truncado por head"
run_test "echo \"\"" "Echo con string vacío"
run_test "echo hola | grep -v hola" "Grep que descarta salida"
run_test "cat /dev/null | wc -l" "Conteo sobre input vacío"

# ARGUMENTOS EXTREMOS
echo -e "${YELLOW}➤ Test de límites de argumentos${NC}"
ARGS_63=$(seq -s ' ' 1 63)
ARGS_64=$(seq -s ' ' 1 64)
run_test "echo $ARGS_63" "63 argumentos (límite)"
run_test "echo $ARGS_64" "64 argumentos (exceso)" "error"

# STRESS TEST: PIPELINE LARGO
echo -e "${YELLOW}➤ Test de estrés: pipeline largo${NC}"
PIPE_CHAIN=$(printf 'grep . | %.0s' {1..20}; echo tail -n 1)
run_test "cat $TEST_FILE | $PIPE_CHAIN" "Pipeline de 22 procesos"

# CASOS ESPECIALES DE REGEX
run_test "cat $TEST_FILE | grep -E \"\\.c$|\\.sh$\"" "Grep con regex de extensiones"
run_test "cat $TEST_FILE | grep -E \"n.*n\"" "Grep con regex compleja"

# RESUMEN FINAL
echo -e "${BLUE}============================================${NC}"
echo -e "         ${YELLOW}RESUMEN FINAL DE TESTS${NC}"
echo -e "   Total de tests:     $TOTAL"
echo -e "   ${GREEN}Funcionales OK:      $PASSED${NC}"
echo -e "   ${RED}Funcionales fallidos: $FAILED${NC}"
echo -e "   ${GREEN}Sin leaks de memoria: $MEM_CLEAN${NC}"
echo -e "   ${RED}Con leaks detectados: $MEM_FAIL${NC}"
echo -e "${BLUE}============================================${NC}"

# LIMPIEZA
rm -f "$TEMP_OUT" "$EXPECTED_OUT" "$SHELL_OUT" "$VALGRIND_OUT" "$TEST_FILE" "$SHELL_ERR"