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
