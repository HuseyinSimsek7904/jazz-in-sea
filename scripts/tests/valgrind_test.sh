#!/bin/sh

EXECUTABLE=./bin/jazzinsea

echo "testing with valgrind..."

>&2 echo -en "\e[1;31m"
valgrind --quiet --error-exitcode=1 $EXECUTABLE -sn "aidepth 3" "evaluate" > /dev/null
exit=$?
>&2 echo -en "\e[0m"

if [ "$exit" != 0 ]; then
    >&2 echo -en "\e[1;31m"
    >&2 echo "error: valgrind test failed"
    >&2 echo -en "\e[0m"
    exit 1
fi

echo "valgrind test succeded"
