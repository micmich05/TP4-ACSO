#!/bin/bash
# filepath: ring_tests.sh
# Test script for ring program

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Path to the ring executable
RING="./ring"

# Check if ring is available
if [ ! -x "$RING" ]; then
    echo -e "${RED}Error: $RING not found or not executable${NC}"
    echo "Make sure to compile the ring program first"
    exit 1
fi

# Function to run a test
run_test() {
    local test_num=$1
    local n=$2
    local c=$3
    local s=$4
    local expected=$((c + n))
    
    echo "Test $test_num: n=$n, c=$c, s=$s"
    
    # Run the command and capture output
    output=$($RING $n $c $s 2>&1)
    
    # Check for errors in the output
    if echo "$output" | grep -q "Entradas inválidas\|Error\|error"; then
        echo -e "${RED}Failed - Invalid inputs or error${NC}"
        echo "$output"
        return 1
    fi
    
    # Extract the final result
    result=$(echo "$output" | grep "Resultado final:" | awk '{print $3}')
    
    # Check if result matches expected
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}Passed - Result: $result (Expected: $expected)${NC}"
        return 0
    else
        echo -e "${RED}Failed - Result: $result (Expected: $expected)${NC}"
        echo "$output"
        return 1
    fi
}

# Count passed tests
passed=0
total=15

echo "Running tests for ring program..."

# Test 1: Basic test with minimum processes
run_test 1 3 0 1
[ $? -eq 0 ] && ((passed++))

# Test 2: Basic test with non-zero starting value
run_test 2 3 10 1
[ $? -eq 0 ] && ((passed++))

# Test 3: Starting from second process
run_test 3 3 0 2
[ $? -eq 0 ] && ((passed++))

# Test 4: Starting from last process
run_test 4 3 0 3
[ $? -eq 0 ] && ((passed++))

# Test 5: More processes, starting from first
run_test 5 5 0 1
[ $? -eq 0 ] && ((passed++))

# Test 6: More processes, starting from middle
run_test 6 5 0 3
[ $? -eq 0 ] && ((passed++))

# Test 7: More processes, starting from last
run_test 7 5 0 5
[ $? -eq 0 ] && ((passed++))

# Test 8: 10 processes, starting from first
run_test 8 10 0 1
[ $? -eq 0 ] && ((passed++))

# Test 9: 10 processes, starting from middle
run_test 9 10 0 5
[ $? -eq 0 ] && ((passed++))

# Test 10: 10 processes, starting from last
run_test 10 10 0 10
[ $? -eq 0 ] && ((passed++))

# Test 11: Large initial value
run_test 11 5 1000 1
[ $? -eq 0 ] && ((passed++))

# Test 12: Negative initial value
run_test 12 5 -1000 1
[ $? -eq 0 ] && ((passed++))

# Test 13: Many processes
run_test 13 20 0 1
[ $? -eq 0 ] && ((passed++))

# Test 14: Many processes with different start
run_test 14 20 0 10
[ $? -eq 0 ] && ((passed++))

# Test 15: Maximum reasonable test
run_test 15 50 0 25
[ $? -eq 0 ] && ((passed++))

# Display summary
echo ""
echo "Test summary: $passed out of $total tests passed"
if [ $passed -eq $total ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi