#!/bin/bash

REDIS_PORT=6379
REDIS_CONFIG_FILE="/etc/redis/redis.conf"
REDIS_DATA_DIR="/var/lib/redis"
REDIS_LOG_FILE="/var/log/redis/redis-server.log"

mkdir -p $REDIS_DATA_DIR
chown redis:redis $REDIS_DATA_DIR

# create Redis config file
cat > $REDIS_CONFIG_FILE << EOF
port $REDIS_PORT
dir $REDIS_DATA_DIR
logfile $REDIS_LOG_FILE

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

chown redis:redis $REDIS_CONFIG_FILE
chmod 640 $REDIS_CONFIG_FILE

systemctl restart redis-server

echo "Redis production server setup complete"
echo "Please ensure to change the default password in $REDIS_CONFIG_FILE"