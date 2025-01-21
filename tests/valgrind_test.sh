#!/bin/sh

echo "testing with valgrind..."

>&2 echo -en "\e[1;31m"
valgrind --quiet --error-exitcode=1 ./build/main -s > /dev/null
exit=$?
>&2 echo -en "\e[0m"

if [ "$exit" != 0 ]; then
    >&2 echo -en "\e[1;31m"
    >&2 echo "error: valgrind test failed"
    >&2 echo -en "\e[0m"
    exit 1
fi

echo "valgrind test succeded"
