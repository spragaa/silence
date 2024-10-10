#!/bin/bash

REDIS_CONFIG_FILE="/etc/redis/redis.conf"
REDIS_DATA_DIR="/var/lib/redis"

systemctl stop redis-server

# backup data for future, mb create second db with backup, do we even need this teardown script?
# BACKUP_DIR="/path/to/backup/directory"
# mkdir -p $BACKUP_DIR
# cp -R $REDIS_DATA_DIR $BACKUP_DIR/redis_data_backup_$(date +%Y%m%d_%H%M%S)

rm -f $REDIS_CONFIG_FILE
rm -rf $REDIS_DATA_DIR

echo "Redis production server teardown complete"
echo "Note: Redis has been stopped and configuration removed"
echo "If you want to completely remove Redis, run this command: 'sudo apt-get remove --purge -y redis-server'"