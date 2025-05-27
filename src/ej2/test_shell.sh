#!/bin/bash

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compilar
echo -e "${YELLOW}Compilando shell...${NC}"
gcc -o shell shell.c
if [ $? -ne 0 ]; then
    echo -e "${RED}Error al compilar${NC}"
    exit 1
fi

# Función para ejecutar test
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_behavior="$3"
    
    echo -e "${YELLOW}Test: $test_name${NC}"
    echo "Comando: $command"
    echo "Resultado:"
    
    # Ejecutar comando en shell y capturar resultado
    result=$(timeout 5s bash -c "echo '$command' | ./shell 2>&1")
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ Ejecutado correctamente${NC}"
        echo "$result"
    elif [ $exit_code -eq 124 ]; then
        echo -e "${RED}✗ Timeout (5s)${NC}"
    else
        echo -e "${RED}✗ Error en ejecución${NC}"
        echo "$result"
    fi
    
    echo "---"
}

echo -e "${YELLOW}=== Tests Automatizados para Shell ===${NC}"

# Tests básicos
run_test "Comando simple" "echo hello" "Debe imprimir 'hello'"
run_test "Pipe básico" "echo hello | cat" "Debe imprimir 'hello'"
run_test "Conteo de líneas" "ls | wc -l" "Debe contar archivos en directorio"
run_test "Grep básico" "echo -e 'line1\nline2' | grep line1" "Debe filtrar línea"
run_test "Sort básico" "echo -e 'c\nb\na' | sort" "Debe ordenar a,b,c"

# Tests intermedios
run_test "Triple pipe" "echo -e 'a\nb\nc\nd\ne' | head -4 | tail -2" "Debe mostrar b,c"
run_test "Word count" "echo 'one two three' | wc -w" "Debe contar 3 palabras"
run_test "Character count" "echo 'hello' | wc -c" "Debe contar caracteres"

# Tests de edge cases
run_test "Entrada vacía" "" "Debe continuar esperando input"
run_test "Solo espacios" "   " "Debe ignorar y continuar"
run_test "Pipe al final" "echo hello |" "Debe manejar pipe incompleto"

# Tests de robustez
run_test "Comando inexistente" "comandoinexistente" "Debe mostrar error"
run_test "Pipe con comando inexistente" "echo hello | comandoinexistente" "Debe manejar error"

echo -e "${GREEN}Tests completados${NC}"

# Cleanup
rm -f shell