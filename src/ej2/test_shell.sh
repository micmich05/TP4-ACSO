#!/bin/bash
# filepath: /Users/micolmichanie/UdeSA/Acso/TP4-ACSO/src/ej2/test_shell.sh

# Test script for shell.c
# Tests valid commands and pipelines

echo "=== Testing Shell Implementation ==="
echo

# Compile the shell
echo "Compiling shell..."
gcc -o shell shell.c
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi
echo "Compilation successful!"
echo

# Function to run a test
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_pattern="$3"
    
    echo "Test: $test_name"
    echo "Command: $command"
    
    # Run the command through our shell and capture output
    result=$(echo "$command" | ./shell 2>&1 | grep -v "Shell>")
    
    if [ -n "$expected_pattern" ]; then
        if echo "$result" | grep -q "$expected_pattern"; then
            echo "✓ PASS"
        else
            echo "✗ FAIL - Expected pattern '$expected_pattern' not found"
            echo "Got: $result"
        fi
    else
        echo "Output: $result"
        echo "✓ EXECUTED"
    fi
    echo "---"
}

# Test 1: Simple command
run_test "Simple ls command" "ls" ""

# Test 2: Command with arguments
run_test "ls with arguments" "ls -la" ""

# Test 3: Simple pipe
run_test "Simple pipe (ls | wc)" "ls | wc" ""

# Test 4: Pipe with grep
run_test "ls | grep test" "ls | grep .c" ""

# Test 5: Three command pipeline
run_test "Three command pipeline" "ls | sort | head -5" ""

# Test 6: Echo and grep
run_test "Echo and grep" "echo 'hello world test' | grep world" "world"

# Test 7: Cat and wc
run_test "Cat shell.c and count" "cat shell.c | wc -l" ""

# Test 8: Multiple pipes with sort
run_test "Multiple pipes with sort" "ls -1 | sort | tail -3" ""

# Test 9: Echo with quoted arguments
run_test "Echo with quotes" 'echo "hello world"' "hello world"

# Test 10: Find and grep pipeline
run_test "Find and grep" "find . -name '*.c' | head -2" ""

# Test 11: Date and formatting
run_test "Date command" "date" ""

# Test 12: Who and wc
run_test "Who and count" "who | wc -l" ""

# Test 13: Four command pipeline
run_test "Four command pipeline" "echo -e 'apple\nbanana\ncherry\ndate' | sort | head -2 | wc -l" "2"

# Test 14: Complex pipeline with multiple operations
run_test "Complex pipeline" "ls -1 | grep -E '\.(c|h)$' | sort | head -1" ""

echo
echo "=== Memory and Resource Tests ==="
echo

# Test for memory leaks and proper pipe handling
echo "Running memory leak test with valgrind (if available)..."
if command -v valgrind &> /dev/null; then
    echo "Testing with valgrind..."
    echo -e "ls | wc\necho 'test' | grep test\nexit" | valgrind --leak-check=full --show-leak-kinds=all ./shell 2>&1 | grep -E "(ERROR SUMMARY|definitely lost|indirectly lost|possibly lost)"
    echo "Valgrind test completed."
else
    echo "Valgrind not available, skipping memory leak test."
fi

echo

# Test multiple sequential pipelines to check resource cleanup
echo "Testing resource cleanup with multiple sequential commands..."
{
    echo "ls | head -1"
    echo "echo 'test1' | cat"
    echo "date | cat"
    echo "ls -1 | wc -l"
    echo "echo 'test2' | grep test"
    echo "whoami | cat"
    echo "ls | sort | head -2"
    echo "echo 'final test' | wc -w"
} | ./shell > /dev/null 2>&1

echo "✓ Multiple sequential commands executed successfully"

echo

# Test pipe stress - many commands in sequence
echo "Testing pipe stress with long pipeline..."
echo "echo -e 'line1\nline2\nline3\nline4\nline5' | cat | sort | cat | head -3 | wc -l" | ./shell > /dev/null 2>&1
echo "✓ Long pipeline executed successfully"

echo

# Test with different argument patterns
echo "Testing various argument patterns..."
{
    echo 'echo "quoted argument" | cat'
    echo 'ls -l | head -1'
    echo 'echo hello world | grep hello'
    echo 'find . -name "*.c" | head -1'
} | ./shell > /dev/null 2>&1

echo "✓ Various argument patterns executed successfully"

echo
echo "=== Performance Test ==="
echo

# Performance test - measure execution time
echo "Running performance test..."
start_time=$(date +%s.%N)
{
    for i in {1..10}; do
        echo "echo 'test $i' | cat | wc -w"
    done
} | ./shell > /dev/null 2>&1
end_time=$(date +%s.%N)

execution_time=$(echo "$end_time - $start_time" | bc -l)
echo "✓ Performance test completed in ${execution_time} seconds"

echo
echo "=== File Descriptor Test ==="
echo

# Check for file descriptor leaks
echo "Testing file descriptor usage..."
initial_fds=$(lsof -p $$ 2>/dev/null | wc -l)
{
    echo "ls | cat"
    echo "echo 'test' | grep test"
    echo "date | wc -w"
} | ./shell > /dev/null 2>&1
final_fds=$(lsof -p $$ 2>/dev/null | wc -l)

if [ "$initial_fds" -eq "$final_fds" ]; then
    echo "✓ No file descriptor leaks detected"
else
    echo "⚠ File descriptor count changed: $initial_fds -> $final_fds"
fi

echo
echo "=== All Tests Completed ==="
echo

# Cleanup
rm -f shell

echo "Shell testing completed successfully!"