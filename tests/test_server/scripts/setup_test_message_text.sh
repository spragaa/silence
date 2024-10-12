#!/bin/bash

# /chat_application/tests/test_server/scripts$ sudo ./setup_test_message_text.sh 
# Job for redis-server.service failed because the control process exited with error code.
# See "systemctl status redis-server.service" and "journalctl -xeu redis-server.service" for details.
# Redis test server setup complete
# Test Redis server is running on port 6380
# Please ensure to change the default password in /etc/redis/redis_test.conf

TEST_REDIS_PORT=6380
TEST_REDIS_CONFIG_FILE="/etc/redis/redis_test.conf"
TEST_REDIS_DATA_DIR="/var/lib/redis_test"
TEST_REDIS_LOG_FILE="/var/log/redis/redis-test-server.log"

# pkill -f "redis-server $TEST_REDIS_CONFIG_FILE"
# rm -rf $TEST_REDIS_DATA_DIR/*

mkdir -p $TEST_REDIS_DATA_DIR
chown redis:redis $TEST_REDIS_DATA_DIR

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

# spraga is super strong pass, 46.1 billion light-years to crack it, trust me
requirepass spraga

bind 127.0.0.1
EOF

chown redis:redis $TEST_REDIS_CONFIG_FILE
chmod 640 $TEST_REDIS_CONFIG_FILE

systemctl restart redis-server

echo "Redis test server setup complete"
echo "Test Redis server is running on port $TEST_REDIS_PORT"
echo "Please ensure to change the default password in $TEST_REDIS_CONFIG_FILE"