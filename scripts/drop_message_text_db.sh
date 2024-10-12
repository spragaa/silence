#!/bin/bash

REDIS_PORT=6379
REDIS_CONFIG_FILE="/etc/redis/redis.conf"
REDIS_DATA_DIR="/var/lib/redis"
REDIS_PASSWORD="spraga"

redis-cli -p $REDIS_PORT -a $REDIS_PASSWORD FLUSHALL
redis-cli -p $REDIS_PORT -a $REDIS_PASSWORD shutdown

rm -f $REDIS_CONFIG_FILE
rm -rf $REDIS_DATA_DIR

rm -f /var/log/redis/redis-server.log

echo "Redis production server teardown complete"
echo "Note: Redis has been stopped and configuration removed"
echo "If you want to completely remove Redis, run this command: 'sudo apt-get remove --purge -y redis-server'"