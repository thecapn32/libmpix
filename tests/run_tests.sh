#!/bin/sh

# Navigate to the base directory of the script
cd "${0%/*}"

# To give the final test result
build_error=0
runtime_error=0
success=0

# Loop over every test, build it, run it, report
for test in */; do
    if ! cmake -B "$test/build" && cmake --build "$test/build" >$test/build.log 2>&1; then
        echo "- $test: build error"
        build_error=$((build_error + 1))
    elif ! "$test/build/libmpix_test" >$test/runtime.log 2>&1
        echo "- $test: runtime error"
        runtime_error=$((runtime_error + 1))
    else
        echo "- $test: PASS"
        success=$((success + 1))
    fi
done

echo "Test results: $build_error build error - $runtime_error runtime error - $success successed"

exit "$((build_error + runtime_error))"
