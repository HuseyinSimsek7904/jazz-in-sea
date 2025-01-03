#!/bin/sh

echo "testing with valgrind..."
echo "aidepth 6\nevaluate\n" | valgrind --quiet --error-exitcode=1 ./build/main -s
exit=$?

if [ "$exit" != 0 ]; then
    >&2 echo -e "\e[1;31m"
    >&2 echo "error: valgrind test failed"
    >&2 echo -e "\e[0m"
    exit 1
fi

echo "valgrind test succeded"
