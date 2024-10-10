#!/bin/bash

TEST_REDIS_PORT=6380
TEST_REDIS_CONFIG_FILE="/etc/redis/redis_test.conf"
TEST_REDIS_DATA_DIR="/var/lib/redis_test"
TEST_REDIS_LOG_FILE="/var/log/redis/redis-test-server.log"

mkdir -p $TEST_REDIS_DATA_DIR
chown redis:redis $TEST_REDIS_DATA_DIR

# create Redis test config file
cat > $TEST_REDIS_CONFIG_FILE << EOF
port $TEST_REDIS_PORT
dir $TEST_REDIS_DATA_DIR
logfile $TEST_REDIS_LOG_FILE

# RDB persistence
save 900 1
save 300 10
save 60 10000

# AOF persistence
appendonly yes
appendfsync everysec

requirepass spraga

# spraga is super strong pass, 46.1 billion light-years to crack it, trust me
bind 127.0.0.1
EOF

chown redis:redis $TEST_REDIS_CONFIG_FILE
chmod 640 $TEST_REDIS_CONFIG_FILE

redis-server $TEST_REDIS_CONFIG_FILE &

echo "Redis test server setup complete"
echo "Test Redis server is running on port $TEST_REDIS_PORT"