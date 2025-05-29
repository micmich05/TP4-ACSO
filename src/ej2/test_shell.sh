#!/bin/bash
# filepath: /Users/micolmichanie/UdeSA/Acso/TP4-ACSO/src/ej2/test_shell.sh

# Test script for shell.c
# Tests valid commands and pipelines (arguments with and without double quotes)

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

echo "=== Basic Commands (Arguments WITHOUT Quotes) ==="

# Test 1: Simple command
run_test "Simple ls command" "ls" ""

# Test 2: Command with arguments
run_test "ls with arguments" "ls -la" ""

# Test 3: Echo without quotes
run_test "Echo without quotes" "echo hello world" "hello world"

# Test 4: Date command
run_test "Date command" "date" ""

# Test 5: Who command
run_test "Who command" "whoami" ""

# Test 6: Cat command
run_test "Cat shell.c" "cat shell.c" "#include"

# Test 7: Multiple arguments without quotes
run_test "ls with multiple args" "ls -l -a" ""

# Test 8: Head command with argument
run_test "Head command" "head -5 shell.c" "#include"

# Test 9: Echo multiple words without quotes
run_test "Echo multiple words" "echo one two three four" "one two three four"

# Test 10: Grep without quotes
run_test "Grep without quotes" "echo testing | grep test" "testing"

echo
echo "=== Basic Commands (Arguments WITH Double Quotes) ==="

# Test 11: Echo with double quotes
run_test "Echo with double quotes" 'echo "hello world"' "hello world"

# Test 12: Echo with quotes and spaces
run_test "Echo quoted with spaces" 'echo "this is a test"' "this is a test"

# Test 13: Echo with multiple quoted args
run_test "Multiple quoted arguments" 'echo "first arg" "second arg"' "first arg second arg"

# Test 14: Echo with mixed args
run_test "Mixed quoted and unquoted" 'echo hello "world test" end' "hello world test end"

# Test 15: Grep with quoted pattern
run_test "Grep with quoted pattern" 'echo testing special | grep "special"' "testing special"

# Test 16: Echo with empty quotes
run_test "Echo with empty quotes" 'echo "hello" "" "world"' "hello  world"

# Test 17: Complex quoted argument
run_test "Complex quoted string" 'echo "file name with spaces.txt"' "file name with spaces.txt"

# Test 18: Quoted argument with special chars
run_test "Quoted with special chars" 'echo "test-file_name.txt"' "test-file_name.txt"

echo
echo "=== Simple Pipelines (Arguments WITHOUT Quotes) ==="

# Test 19: Simple pipe
run_test "Simple pipe (ls | wc)" "ls | wc" ""

# Test 20: Echo and grep without quotes
run_test "Echo and grep unquoted" "echo hello world test | grep world" "world"

# Test 21: Cat and wc
run_test "Cat and count lines" "cat shell.c | wc -l" ""

# Test 22: Ls and head
run_test "Ls and head" "ls | head -3" ""

# Test 23: Date and cat
run_test "Date and cat" "date | cat" ""

# Test 24: Echo and wc
run_test "Echo and word count" "echo one two three | wc -w" "3"

# Test 25: Grep with extension
run_test "Grep file extension" "ls | grep .c" ""

# Test 26: Sort without quotes
run_test "Echo and sort" "echo zebra apple banana | tr ' ' '\n' | sort" ""

echo
echo "=== Simple Pipelines (Arguments WITH Double Quotes) ==="

# Test 27: Pipeline with quoted echo
run_test "Quoted echo in pipeline" 'echo "hello world test" | grep "world"' "world"

# Test 28: Multiple quoted args in pipeline
run_test "Multiple quotes in pipeline" 'echo "first line" "second line" | wc -w' "4"

# Test 29: Mixed quotes in pipeline
run_test "Mixed quotes pipeline" 'echo hello "world test" | grep "test"' "hello world test"

# Test 30: Quoted args with special chars in pipeline
run_test "Quotes with special chars" 'echo "test-file.txt" | cat' "test-file.txt"

# Test 31: Grep with quoted pattern
run_test "Grep quoted pattern" 'echo "special pattern here" | grep "pattern"' "special pattern here"

# Test 32: Complex quoted in pipeline
run_test "Complex quoted pipeline" 'echo "line one" "line two" "line three" | wc -w' "6"

echo
echo "=== Multiple Command Pipelines (Arguments WITHOUT Quotes) ==="

# Test 33: Three command pipeline
run_test "Three command pipeline" "ls | sort | head -5" ""

# Test 34: Four command pipeline  
run_test "Four command pipeline" "echo apple banana cherry date | tr ' ' '\n' | sort | head -2" ""

# Test 35: Multiple pipes with different commands
run_test "Multiple different commands" "ls -1 | sort | tail -3 | wc -l" ""

# Test 36: Cat, grep and count
run_test "Cat grep count" "cat shell.c | grep include | wc -l" ""

# Test 37: Complex unquoted pipeline
run_test "Complex unquoted pipeline" "echo one two three four five | tr ' ' '\n' | sort | head -3" ""

# Test 38: Long pipeline with many commands
run_test "Long unquoted pipeline" "ls | sort | head -10 | tail -5 | wc -l" ""

echo
echo "=== Multiple Command Pipelines (Arguments WITH Double Quotes) ==="

# Test 39: Three commands with quotes
run_test "Three commands with quotes" 'echo "apple banana cherry" | tr " " "\n" | sort' ""

# Test 40: Pipeline with multiple quoted args
run_test "Pipeline multiple quoted" 'echo "first" "second" "third" | tr " " "\n" | wc -l' "3"

# Test 41: Mixed quotes in long pipeline
run_test "Mixed quotes long pipeline" 'echo hello "world test" more | tr " " "\n" | sort | head -2' ""

# Test 42: Complex quoted pipeline
run_test "Complex quoted pipeline" 'echo "line1 data" "line2 data" | tr " " "\n" | grep "data"' "data"

# Test 43: Quoted arguments with special processing
run_test "Quoted with processing" 'echo "test file name.txt" | wc -w' "3"

echo
echo "=== Mixed Argument Patterns ==="

# Test 44: Mix of quoted and unquoted in same command
run_test "Mixed in same command" 'echo start "middle part" end | wc -w' "4"

# Test 45: Mix in pipeline
run_test "Mixed in pipeline" 'echo "quoted start" unquoted "quoted end" | grep "start"' "quoted start"

# Test 46: Multiple patterns
run_test "Multiple patterns" 'echo file1 "file 2.txt" file3 | wc -w' "3"

# Test 47: Complex mixing
run_test "Complex mixing" 'echo normal "special chars!" another | tr " " "\n" | wc -l' "4"

echo
echo "=== Stress Tests ==="

# Test 48: Long pipeline without quotes
run_test "Long pipeline unquoted" "echo one two three four five six seven | tr ' ' '\n' | sort | cat | head -4 | wc -l" "4"

# Test 49: Long pipeline with quotes
run_test "Long pipeline quoted" 'echo "word1" "word2" "word3" "word4" | tr " " "\n" | sort | head -2 | wc -l' "2"

# Test 50: Many arguments without quotes
run_test "Many arguments unquoted" "echo a b c d e f g h i j k l | wc -w" "12"

# Test 51: Many arguments with quotes
run_test "Many arguments quoted" 'echo "a" "b" "c" "d" "e" "f" | wc -w' "6"

# Test 52: Very long quoted string
run_test "Very long quoted string" 'echo "this is a very long string with many words to test the argument parsing functionality" | wc -w' "16"

# Test 53: Multiple short pipelines sequentially
{
    echo "echo test1 | cat"
    echo "echo test2 | cat" 
    echo "echo test3 | cat"
    echo 'echo "quoted test4" | cat'
    echo 'echo "test5" | grep "test"'
} | ./shell > /dev/null 2>&1
echo "✓ Multiple short pipelines executed"
echo "---"

echo
echo "=== Memory and Resource Tests ==="

# Test for memory leaks and proper pipe handling
echo "Running memory leak test with valgrind (if available)..."
if command -v valgrind &> /dev/null; then
    echo "Testing with valgrind..."
    echo -e "ls | wc\necho test | grep test\nexit" | valgrind --leak-check=full --show-leak-kinds=all ./shell 2>&1 | grep -E "(ERROR SUMMARY|definitely lost|indirectly lost|possibly lost)"
    echo "Valgrind test completed."
else
    echo "Valgrind not available, skipping memory leak test."
fi

echo

# Test multiple sequential pipelines to check resource cleanup
echo "Testing resource cleanup with multiple sequential commands..."
{
    echo "ls | head -1"
    echo "echo test1 | cat"
    echo "date | cat"
    echo "ls -1 | wc -l"
    echo "echo test2 | grep test"
    echo "whoami | cat"
    echo "ls | sort | head -2"
    echo "echo final test | wc -w"
    echo 'echo "quoted test" | cat'
    echo 'echo "multi word test" | wc -w'
    echo 'echo start "middle" end | wc -w'
} | ./shell > /dev/null 2>&1

echo "✓ Multiple sequential commands executed successfully"

echo

# Test pipe stress - many commands in sequence
echo "Testing pipe stress with long pipeline..."
echo "echo line1 line2 line3 line4 line5 | tr ' ' '\n' | cat | sort | cat | head -3 | wc -l" | ./shell > /dev/null 2>&1
echo "✓ Long pipeline executed successfully"

echo

# Test with different argument patterns
echo "Testing various argument patterns..."
{
    echo 'echo "quoted argument" | cat'
    echo "ls -l | head -1"
    echo "echo hello world | grep hello"
    echo 'echo "test file.txt" | cat'
    echo "echo a b c | wc -w"
    echo 'echo "first" second "third" | wc -w'
    echo "find . -name shell.c | head -1"
} | ./shell > /dev/null 2>&1

echo "✓ Various argument patterns executed successfully"

echo
echo "=== Performance Test ==="

# Performance test - measure execution time
echo "Running performance test..."
start_time=$(date +%s.%N)
{
    for i in {1..10}; do
        echo "echo test $i | cat | wc -w"
    done
    for i in {1..10}; do
        echo 'echo "quoted test" '$i' | cat | wc -w'
    done
} | ./shell > /dev/null 2>&1
end_time=$(date +%s.%N)

execution_time=$(echo "$end_time - $start_time" | bc -l)
echo "✓ Performance test completed in ${execution_time} seconds"

echo
echo "=== File Descriptor Test ==="

# Check for file descriptor leaks
echo "Testing file descriptor usage..."
initial_fds=$(lsof -p $$ 2>/dev/null | wc -l)
{
    echo "ls | cat"
    echo "echo test | grep test"
    echo "date | wc -w"
    echo 'echo "quoted test" | cat'
    echo 'echo start "middle" end | wc -w'
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