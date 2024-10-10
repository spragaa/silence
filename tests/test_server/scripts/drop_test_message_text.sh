#!/bin/bash

TEST_REDIS_PORT=6380
TEST_REDIS_CONFIG_FILE="/etc/redis/redis_test.conf"
TEST_REDIS_DATA_DIR="/var/lib/redis_test"

redis-cli -p $TEST_REDIS_PORT shutdown

rm -f $TEST_REDIS_CONFIG_FILE
rm -rf $TEST_REDIS_DATA_DIR

echo "Redis test server teardown complete"
echo "Test Redis server has been stopped and test data removed