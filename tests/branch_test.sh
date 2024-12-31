#!/bin/sh

echo "testing for branch count..."

branch_check() {
    branches=$(echo -e "test -l $1\n" | ./build/main -s)
    exit=$?

    if [ "$exit" != 0 ]; then
        echo -e "exit with error code $exit"
        echo $branches
        exit 1
    fi

    if [ "$branches" != "$2" ]; then
        >&2 echo -e "\e[1;31m"
        >&2 echo -e "error: did not pass branch test:"
        >&2 echo -e "error: for depth $1, got $branches expected $2"
        >&2 echo -e "\e[0m"
        exit 1
    fi

    echo -e "passed for depth $1"
}


branch_check 0 1
branch_check 1 12
branch_check 2 144
branch_check 3 2112
branch_check 4 30872
branch_check 5 482432
branch_check 6 7499456
branch_check 7 122915700
echo "passed all branch tests"
