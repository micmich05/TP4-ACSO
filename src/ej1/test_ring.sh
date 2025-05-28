#!/bin/bash
# filepath: ring_tests.sh
# Enhanced test script for ring program with memory leak and pipe checks

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Path to the ring executable
RING="./ring"

# Check if ring is available
if [ ! -x "$RING" ]; then
    echo -e "${RED}Error: $RING not found or not executable${NC}"
    echo "Make sure to compile the ring program first"
    exit 1
fi

# Check if valgrind is available
if ! command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}Warning: valgrind not found. Memory leak checks will be skipped.${NC}"
    VALGRIND_AVAILABLE=0
else
    VALGRIND_AVAILABLE=1
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

# Function to check for memory leaks using valgrind
check_memory_leaks() {
    local n=$1
    local c=$2
    local s=$3
    
    if [ $VALGRIND_AVAILABLE -eq 0 ]; then
        echo -e "${YELLOW}Skipping memory leak check - valgrind not available${NC}"
        return 0
    fi
    
    echo "Running memory leak check for n=$n, c=$c, s=$s..."
    
    # Run with valgrind
    valgrind_output=$(valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --error-exitcode=1 $RING $n $c $s 2>&1)
    
    # Check if valgrind detected any issues
    if [ $? -ne 0 ]; then
        echo -e "${RED}Memory leak or file descriptor leak detected:${NC}"
        echo "$valgrind_output" | grep -A 5 -B 5 "LEAK SUMMARY\|FILE DESCRIPTORS"
        return 1
    else
        echo -e "${GREEN}No memory leaks or file descriptor leaks detected${NC}"
        return 0
    fi
}

# Function to check for open file descriptors before and after
check_pipe_leaks() {
    local n=$1
    local c=$2
    local s=$3
    
    echo "Checking for pipe fd leaks for n=$n, c=$c, s=$s..."
    
    # Get initial count of open FDs for this process
    if [ "$(uname)" == "Darwin" ]; then
        # macOS
        fd_before=$(lsof -p $$ | wc -l)
    else
        # Linux
        fd_before=$(ls -1 /proc/$$/fd | wc -l)
    fi
    
    # Run the program
    $RING $n $c $s > /dev/null 2>&1
    
    # Get count after program execution
    if [ "$(uname)" == "Darwin" ]; then
        # macOS
        fd_after=$(lsof -p $$ | wc -l)
    else
        # Linux
        fd_after=$(ls -1 /proc/$$/fd | wc -l)
    fi
    
    # Compare
    if [ "$fd_before" -eq "$fd_after" ]; then
        echo -e "${GREEN}No file descriptor leaks detected${NC}"
        return 0
    else
        echo -e "${RED}Possible file descriptor leak: before=$fd_before, after=$fd_after${NC}"
        return 1
    fi
}

# Count passed tests
passed=0
passed_memory=0
passed_pipes=0
total=15

echo "Running basic functionality tests for ring program..."

# Run the standard tests
run_test 1 3 0 1
[ $? -eq 0 ] && ((passed++))

run_test 2 3 10 1
[ $? -eq 0 ] && ((passed++))

run_test 3 3 0 2
[ $? -eq 0 ] && ((passed++))

run_test 4 3 0 3
[ $? -eq 0 ] && ((passed++))

run_test 5 5 0 1
[ $? -eq 0 ] && ((passed++))

run_test 6 5 0 3
[ $? -eq 0 ] && ((passed++))

run_test 7 5 0 5
[ $? -eq 0 ] && ((passed++))

run_test 8 10 0 1
[ $? -eq 0 ] && ((passed++))

run_test 9 10 0 5
[ $? -eq 0 ] && ((passed++))

run_test 10 10 0 10
[ $? -eq 0 ] && ((passed++))

run_test 11 5 1000 1
[ $? -eq 0 ] && ((passed++))

run_test 12 5 -1000 1
[ $? -eq 0 ] && ((passed++))

run_test 13 20 0 1
[ $? -eq 0 ] && ((passed++))

run_test 14 20 0 10
[ $? -eq 0 ] && ((passed++))

run_test 15 50 0 25
[ $? -eq 0 ] && ((passed++))

# Display functionality test summary
echo ""
echo "Functionality test summary: $passed out of $total tests passed"

# Now run memory leak checks on a subset of tests
echo ""
echo "Running memory leak checks..."

check_memory_leaks 3 0 1
[ $? -eq 0 ] && ((passed_memory++))

check_memory_leaks 5 0 3
[ $? -eq 0 ] && ((passed_memory++))

check_memory_leaks 10 0 5
[ $? -eq 0 ] && ((passed_memory++))

# Display memory leak test summary
echo ""
echo "Memory leak test summary: $passed_memory out of 3 tests passed"

# Run pipe leak checks
echo ""
echo "Running pipe leak checks..."

check_pipe_leaks 3 0 1
[ $? -eq 0 ] && ((passed_pipes++))

check_pipe_leaks 5 0 3
[ $? -eq 0 ] && ((passed_pipes++))

check_pipe_leaks 10 0 5
[ $? -eq 0 ] && ((passed_pipes++))

# Display pipe leak test summary
echo ""
echo "Pipe leak test summary: $passed_pipes out of 3 tests passed"

# Final summary
echo ""
echo "Overall summary:"
echo "- Functionality: $passed/$total"
echo "- Memory leaks: $passed_memory/3"
echo "- Pipe leaks: $passed_pipes/3"

if [ $passed -eq $total ] && [ $passed_memory -eq 3 ] && [ $passed_pipes -eq 3 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi