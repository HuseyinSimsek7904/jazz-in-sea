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
