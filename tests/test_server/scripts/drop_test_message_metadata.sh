#!/bin/bash

DEBUG_MODE=false
if [[ "\$1" == "--debug" ]]; then
    DEBUG_MODE=true
fi

debug_echo() {
    if [ "$DEBUG_MODE" = true ]; then
        local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        echo "[DEBUG] [$timestamp] \$1"
    fi
}

SUPERUSER="postgres"
DB_HOST="localhost"
DB_PORT="5432"

NEW_USER="${DB_NEW_USER}"
NEW_USER_PASSWORD="${DB_NEW_USER_PASSWORD}"

if [ -z "$NEW_USER" ] || [ -z "$NEW_USER_PASSWORD" ] || [ -z "$POSTGRES_PASSWORD" ]; then
    echo "Error: Environment variables not set. Run 'source set_env_variables.sh' first."
    exit 1
fi

DB_NAME="test_message_metadata"

export PGPASSWORD="$POSTGRES_PASSWORD"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

SQL_FILE="$SCRIPT_DIR/../database/drop_test_message_metadata.sql"

psql -U $SUPERUSER -h $DB_HOST -p $DB_PORT -v db_name="$DB_NAME" -v new_user="$NEW_USER" -v new_user_password="'$NEW_USER_PASSWORD'" -f "$SQL_FILE"

unset PGPASSWORD

if [ $? -eq 0 ]; then
    debug_echo "dropped test_message_metadata successfully!"
else
    debug_echo "Error: Failed to drop database and table"
    exit 1
fi