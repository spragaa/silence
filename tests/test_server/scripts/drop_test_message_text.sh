#!/bin/bash

# crap

TEST_REDIS_PORT=6380
TEST_REDIS_CONFIG_FILE="/etc/redis/redis.conf"
TEST_REDIS_DATA_DIR="/var/lib/redis"
REDIS_PASSWORD="spraga"

redis-cli -p $TEST_REDIS_PORT -a $REDIS_PASSWORD FLUSHALL
redis-cli -p $TEST_REDIS_PORT -a $REDIS_PASSWORD shutdown

rm -f $TEST_REDIS_CONFIG_FILE
rm -rf $TEST_REDIS_DATA_DIR

rm -f /var/log/redis/redis-server.log

echo "Redis production server teardown complete"
echo "Note: Redis has been stopped and configuration removed"
echo "If you want to completely remove Redis, run this command: 'sudo apt-get remove --purge -y redis-server'"