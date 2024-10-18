BINARY=./build/main

DEPTH_TEST_ITER=1000

# Try to finding a random branch for DEPTH_TEST_ITER times.
echo 'Attempting to do depth tests...'

for i in $(seq 1 $DEPTH_TEST_ITER); do
    if ! $BINARY 0 5 > /dev/null; then
        echo 'Depth test failed'
        exit 1
    fi
done

echo 'Depth tests succeeded!'

# Try counting all of the branches that can be reached in COUNT_TEST_DEPTH moves.
echo 'Attempting to do count tests...'
result=$($BINARY 1 5)
if [ $? != 0 ]; then
    echo 'Branch count test failed'
    exit 1
fi

if [ $result != 482432 ]; then
    echo 'Branch count test failed; expected 482432, got' $result
    exit 1
fi

echo 'Count tests succeeded!'

# Try checking the get_board_state function.
echo 'Attempting to do state tests...'
STATE_TEST_BOARDS=(
    '8/8/8/8/8/8/8/8 w'
    '8/8/8/8/8/8/p7/8 w'
    '8/8/8/8/8/8/P7/8 w'
    'np4PN/pp4PP/8/8/8/8/PP4pp/NP4pn w'
    '7p/8/PPPPPPP1/3P4/8/8/8/8 w'
    '7P/8/ppppppp1/3p4/8/8/8/8 w'
    '7p/8/PPPPPPP1/3P4/8/8/8/7P w'
    '7p/8/ppppppp1/3p4/8/8/8/8 w'
)

STATE_TEST_RESULTS=(
    'draw by both sides have insufficient material'
    'black won as opponent has insufficient material'
    'white won as opponent has insufficient material'
    'continue'
    'white won by islands'
    'black won by islands'
    'continue'
    'black won as opponent has insufficient material'
)

for i in ${!STATE_TEST_BOARDS[@]}
do
    board="${STATE_TEST_BOARDS[i]}"
    expect="${STATE_TEST_RESULTS[i]}"

    result=$($BINARY 2 "$board")

    if [ "$?" != 0 ]; then
        echo "Getting state for #$i failed:"
        echo $result
        exit 1
    fi

    if [ "$result" != "$expect" ]; then
        echo "Getting state for #$i failed"
        echo "expected: \"$expect\""
        echo "got: \"$result\""
        exit 1
    fi
done
echo 'State tests succeeded!'


# Try checking the evaluate function.
echo 'Attempting to do AI tests...'
AI_TEST_BOARDS=(
    '8/8/8/8/8/8/8/8 w'
    '8/8/8/8/8/8/p7/8 w'
    '8/8/8/8/8/8/P7/8 w'
    '7p/8/PPPPPPP1/3P4/8/8/8/8 w'
    '7P/8/ppppppp1/3p4/8/8/8/8 w'
    '7p/8/ppppppp1/3p4/8/8/8/8 w'
    '7p/8/8/3p1P2/8/8/8/8 w'
    '7P/8/8/3P1p2/8/8/8/8 b'
    '7p/8/8/3p1P2/8/8/8/8 b'
    '7P/8/8/3P1p2/8/8/8/8 w'
    '7P/8/8/8/8/8/8/p7 w'
    '7P/8/8/8/8/8/8/p7 b'
)

AI_TEST_RESULTS=(
    'draw in 0'
    'black mates in 0'
    'white mates in 0'
    'white mates in 0'
    'black mates in 0'
    'black mates in 0'
    'white mates in 1'
    'black mates in 1'
    'white mates in 2'
    'black mates in 2'
    'white mates in 11'
    'black mates in 11'
)

AI_TEST_DEPTHS=(
    1
    1
    1
    1
    1
    1
    2
    2
    3
    3
    12
    12
)

for i in ${!AI_TEST_BOARDS[@]}
do
    board="${AI_TEST_BOARDS[i]}"
    expect="${AI_TEST_RESULTS[i]}"
    depth="${AI_TEST_DEPTHS[i]}"

    result=$($BINARY 5 "$board" $depth)
    printf "Testing '$board' with depth $depth -> '$expect'\e[100G- "

    if [ "$?" != 0 ]; then
        echo "Failed\n"
        echo "Getting evaluation for #$i failed:"
        echo $result
        exit 1
    fi

    if [ "$result" != "$expect" ]; then
        echo "Failed\n"
        echo "Getting evaluation for #$i failed"
        echo "expected: \"$expect\""
        echo "got: \"$result\""
        echo "board: $board"
        echo "depth: $depth"
        exit 1
    fi

    printf "Done\n"
done
echo 'AI tests succeeded!'
