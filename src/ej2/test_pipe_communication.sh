#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

gcc -o shell shell.c
if [ $? -ne 0 ]; then
    echo -e "${RED}Error al compilar${NC}"
    exit 1
fi

echo -e "${YELLOW}=== Tests de Comunicación entre Procesos ===${NC}"

# Crear un archivo con contenido conocido
cat > pipe_test.txt << EOF
línea 1
línea 2
línea 3
línea 4
línea 5
línea 6
línea 7
línea 8
línea 9
línea 10
EOF

test_pipe_communication() {
    local test_name="$1"
    local command="$2"
    local expected_lines="$3"
    
    echo -e "${YELLOW}Test: $test_name${NC}"
    echo "Comando: $command"
    
    result=$(echo "$command" | ./shell 2>&1 | grep -v "Shell>" | wc -l)
    result=$(echo $result | tr -d ' ')  # Remove spaces
    
    if [ "$result" -eq "$expected_lines" ]; then
        echo -e "${GREEN}✓ CORRECTO: $result líneas (esperado: $expected_lines)${NC}"
    else
        echo -e "${RED}✗ ERROR: $result líneas (esperado: $expected_lines)${NC}"
        echo "Salida completa:"
        echo "$command" | ./shell 2>&1
    fi
    echo "---"
}

# Tests que validan que los pipes realmente pasan datos entre procesos
test_pipe_communication "Pipe simple: cat | head" "cat pipe_test.txt | head -3" 3
test_pipe_communication "Pipe doble: cat | head | tail" "cat pipe_test.txt | head -6 | tail -2" 2
test_pipe_communication "Pipe triple: cat | head | tail | wc" "cat pipe_test.txt | head -8 | tail -4 | wc -l" 1

# Tests que verifican orden correcto de ejecución
echo -e "${YELLOW}Test de orden de ejecución:${NC}"
echo "cat pipe_test.txt | head -5 | tail -2" | ./shell 2>&1

echo -e "${YELLOW}Comparación con bash:${NC}"
cat pipe_test.txt | head -5 | tail -2

rm -f shell pipe_test.txt