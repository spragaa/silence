#!/bin/bash

DEBUG_MODE=false
if [[ "\$1" == "--debug" ]]; then
    DEBUG_MODE=true
    shift
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

NEW_USER=""
NEW_USER_PASSWORD=""

for arg in "$@"; do
    case $arg in
        --user=*)
        NEW_USER="${arg#*=}"
        ;;
        --user_password=*)
        NEW_USER_PASSWORD="${arg#*=}"
        ;;
    esac
done

if [ -z "$NEW_USER" ] || [ -z "$NEW_USER_PASSWORD" ]; then
    echo "Error: Both --user and --user_password must be provided."
    exit 1
fi

DB_NAME="message_metadata"

read -s -p "Enter password for postgres user: " POSTGRES_PASSWORD
echo

export PGPASSWORD="$POSTGRES_PASSWORD"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

SQL_FILE="$SCRIPT_DIR/../database/message_metadata.sql"

psql -U $SUPERUSER -h $DB_HOST -p $DB_PORT -v db_name="$DB_NAME" -v new_user="$NEW_USER" -v new_user_password="'$NEW_USER_PASSWORD'" -f "$SQL_FILE"

unset PGPASSWORD

if [ $? -eq 0 ]; then
    debug_echo "message_metadata database and table created successfully!"
else
    debug_echo "Error: Failed to create database and table."
    exit 1
fi