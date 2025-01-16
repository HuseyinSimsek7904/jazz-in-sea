#!/bin/sh

branch_check() {
    path="board_fen/$1"
    name=$1[@]
    branches=("${!name}")
    total=${#branches[@]}

    echo -e "testing for '$1', fen: '$(cat $path)'"

    echo -en "progress: ("
    for i in $(seq 1 $[ $total ]); do
        echo -en "."
    done
    echo -e ")"

    for i in $(seq 0 $[ $total - 1 ]); do
        expect="${branches[i]}"
        got=$(echo -e "loadfen -f '$path'\ntest -l $i\n" | ./build/main -s 2>&1)
        exit=$?

        if [ "$exit" != 0 ]; then
            echo
            >&2 echo -e "\e[1;31m"
            >&2 echo -e "jazz exit with exit code $exit"
            >&2 echo -e "$branches"
            >&2 echo -e "\e[0m"
            return
        fi

        if [ "$got" != "$expect" ]; then
            echo -en "\e[F"
            echo -en "\e[$[ 11 + $i ]C"
            echo -en '\e[1;31m?\e[0m\e[E'

            >&2 echo -en "\e[1;31m"
            >&2 echo -e "error: did not pass branch test:"
            >&2 echo -e "error: for depth $i, got $got expected $expect"
            >&2 echo -en "\e[0m"
            return
        fi

        echo -en "\e[F"
        echo -en "\e[$[ 11 + $i ]C"
        echo -en '\e[1;32m#\e[0m\e[E'
    done
}

# Since there is no other source of information
# for these values I am just using the current
# output of the program. They may be incorrect.
starting=(1 12 144 2112 30872 482432 7499456 122915700)
mate_test_1=(1 7 95 600 10481 61441 1092688 6325065 113968601)
mate_test_2=(1 3 47 137 2601 7939 142210 434478 8313911 25972105)
mate_test_3=(1 3 47 139 3023 9591 167296 533185 10779520 35160921)
mate_test_4=(1 2 4 12 36 108 324 1080 3600 11984 39888 138928 474200 1600200 5452700 19106122 65536172)

echo "testing for branch count..."

branch_check starting
branch_check mate_test_1
branch_check mate_test_2
branch_check mate_test_3
branch_check mate_test_4
