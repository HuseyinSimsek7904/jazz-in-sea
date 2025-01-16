#!/bin/sh

eval_check() {
    path="board_fen/$1"
    depth="$2"
    eval="$3"

    echo -e "[    ] testing for '$1', fen: '$(cat $path)' with depth $depth -> '$eval'"

    got=$(echo -e "loadfen -f '$path'\naidepth $depth\nevaluate\n" | ./build/main -s 2>&1)
    exit=$?

    if [ "$exit" != 0 ]; then
        echo -e "\e[1;31m\e[F\e[CERR\e[0m"

        >&2 echo -e "\e[1;31m"
        >&2 echo -e "jazz exit with exit code $exit"
        >&2 echo -e "$got"
        >&2 echo -e "\e[0m"
        return
    fi

    if [ "$got" != "$eval" ]; then
        echo -e "\e[1;31m\e[F\e[CERR\e[0m"

        >&2 echo -en "\e[1;31m"
        >&2 echo -e "error: did not pass eval test:"
        >&2 echo -e "error: for depth $depth, got $got expected $eval"
        >&2 echo -en "\e[0m"
        return
    fi

    echo -e "\e[1;32m\e[F\e[CDONE\e[0m"
}

echo "testing for evaluation..."

eval_check mate_test_2 8  WM#8
eval_check mate_test_2 9  WM#8
eval_check mate_test_2 10 WM#8
eval_check mate_test_2 11 WM#8

# eval_check mate_test_3 12 WM#12

eval_check mate_test_4 11 WM#11
eval_check mate_test_4 12 WM#11
eval_check mate_test_4 13 WM#11
