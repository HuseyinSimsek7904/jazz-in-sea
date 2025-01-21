white_wins=$(mktemp)
black_wins=$(mktemp)
draw=$(mktemp)

white_player=$1
black_player=$2

iter=$3

fight() {
    result=$($white_player -ns \
                           "aitime 100" \
                           "test -f '$black_player'" \
                           "status" \
                 | cut -d' ' -f1)
    if [ $? != 0 ]; then
        echo -e "white player returned with error\n"
        exit 1
    fi

    while [ $(($(date +%s%N) / 50000000 % 20 )) != $(( $1 * 2 )) ]; do
        sleep 0.03
    done

    case $result in
        "white")
            echo $(( $(cat $white_wins) + 1 )) > $white_wins
        ;;
        "black")
            echo $(( $(cat $black_wins) + 1 )) > $black_wins
        ;;
        "draw")
            echo $(( $(cat $draw) + 1 )) > $draw
        ;;
        *)
            echo -e "got invalid status, '$result'"
            exit 1
        ;;
    esac
}

echo 0 > $white_wins
echo 0 > $black_wins
echo 0 > $draw

>&2 echo -e "white: '$white_player' against black: '$black_player'"
>&2 echo -en "round 0 -- 0 | 0 | 0"
for i in $(seq 1 "$3"); do
    for j in $(seq 0 9); do
        fight $j &
    done
    wait
    >&2 echo -en "\e[1F\e[1Eround $i -- "
    >&2 echo -en "$(cat $white_wins) | "
    >&2 echo -en "$(cat $black_wins) | "
    >&2 echo -en "$(cat $draw)"
done
>&2 echo -e

echo "white: '$white_player' against black: '$black_player'"
echo "white won $(cat $white_wins) ($(( $(cat $white_wins) * 10 / $3 ))%) times"
echo "black won $(cat $black_wins) ($(( $(cat $black_wins) * 10 / $3 ))%) times"
echo "game ended in draw $(cat $draw) ($(( $(cat $draw) * 10 / $3 ))%) times"

rm $white_wins
rm $black_wins
rm $draw
