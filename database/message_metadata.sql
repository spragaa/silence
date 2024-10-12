DROP DATABASE IF EXISTS :db_name;
DROP USER IF EXISTS :new_user;

CREATE USER :new_user WITH PASSWORD :'new_user_password';

DROP DATABASE IF EXISTS :db_name;
CREATE DATABASE :db_name;
GRANT ALL PRIVILEGES ON DATABASE :db_name TO :new_user;

\c :db_name

CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    deleted BOOLEAN DEFAULT FALSE,
    created_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    deleted_timestamp TIMESTAMP WITH TIME ZONE,
    last_edited_timestamp TIMESTAMP WITH TIME ZONE
);

GRANT ALL PRIVILEGES ON TABLE messages TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE messages_id_seq TO :new_user;

DROP ROLE IF EXISTS logi;
CREATE ROLE logi WITH LOGIN PASSWORD 'logi';