#!/bin/bash

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Compilar
echo -e "${YELLOW}Compilando shell...${NC}"
gcc -o shell shell.c
if [ $? -ne 0 ]; then
    echo -e "${RED}Error al compilar${NC}"
    exit 1
fi

# Crear archivos de prueba
echo -e "${BLUE}Creando archivos de prueba...${NC}"
cat > test_data.txt << EOF
apple
banana
cherry
date
elderberry
fig
grape
honeydew
kiwi
lemon
EOF

cat > numbers.txt << EOF
10
5
20
3
15
8
25
1
12
7
EOF

# Función para ejecutar test
run_test() {
    local test_name="$1"
    local command="$2"
    
    echo -e "${YELLOW}Test: $test_name${NC}"
    echo -e "${BLUE}Comando:${NC} $command"
    echo "Resultado:"
    
    # Ejecutar comando en shell y capturar resultado
    result=$(timeout 10s bash -c "echo '$command' | ./shell 2>&1")
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ Ejecutado correctamente${NC}"
        echo "$result"
    elif [ $exit_code -eq 124 ]; then
        echo -e "${RED}✗ Timeout (10s)${NC}"
    else
        echo -e "${RED}✗ Error en ejecución${NC}"
        echo "$result"
    fi
    
    echo "Expected vs Actual:"
    # Ejecutar el mismo comando en bash para comparar
    expected=$(timeout 5s bash -c "$command" 2>/dev/null || echo "Error esperado")
    echo -e "${GREEN}Bash result:${NC}"
    echo "$expected"
    echo "---"
}

echo -e "${YELLOW}=== Tests de Concatenación con Pipes ===${NC}"

# Tests de pipes con archivos
run_test "Cat + Sort" "cat test_data.txt | sort"
run_test "Cat + Head" "cat test_data.txt | head -5"
run_test "Cat + Tail" "cat test_data.txt | tail -3"
run_test "Cat + Grep" "cat test_data.txt | grep e"
run_test "Cat + Wc" "cat test_data.txt | wc -l"

# Tests de pipes con números
run_test "Cat números + Sort" "cat numbers.txt | sort -n"
run_test "Cat números + Head + Tail" "cat numbers.txt | head -6 | tail -3"

# Tests de concatenación de múltiples procesos
run_test "Triple pipe: ls + grep + wc" "ls | grep .txt | wc -l"
run_test "Cuádruple pipe" "cat test_data.txt | grep e | sort | head -3"
run_test "Pipeline largo" "cat test_data.txt | sort | head -8 | tail -5 | wc -l"

# Tests con comandos que generan output
run_test "Echo + múltiples procesos" "echo -e 'uno\ndos\ntres\ncuatro\ncinco' | head -4 | tail -2"
run_test "Ps + grep + wc" "ps aux | head -10 | wc -l"

# Tests de concatenación con diferentes comandos
run_test "Find + grep (simulado)" "ls -la | grep shell"
run_test "Cat + sort + uniq" "cat test_data.txt | sort | head -5"

# Tests de pipes con redirección implícita
run_test "Who + wc" "who | wc -l"
run_test "Date + cat" "date | cat"

# Tests más complejos
run_test "Pipeline de 5 comandos" "cat numbers.txt | sort -n | head -8 | tail -5 | wc -l"

echo -e "${GREEN}Tests de concatenación completados${NC}"

# Cleanup
rm -f shell test_data.txt numbers.txt