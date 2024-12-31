#!/bin/sh

echo "testing with valgrind..."
echo "aidepth 6\nevaluate\n" | valgrind --quiet --error-exitcode=1 ./build/main -s
exit=$?

if [ "$exit" != 0 ]; then
    echo "valgrind test failed"
    exit 1
fi

echo "valgrind test succeded"
