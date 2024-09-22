-- File: source_dir/database/user_metadata.sql

DROP DATABASE IF EXISTS :db_name;
DROP USER IF EXISTS :new_user;

CREATE USER :new_user WITH PASSWORD :'new_user_password';

DROP DATABASE IF EXISTS :db_name;
CREATE DATABASE :db_name;
GRANT ALL PRIVILEGES ON DATABASE :db_name TO :new_user;

\c :db_name

CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    nickname VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    registered_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_online_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    is_online BOOLEAN DEFAULT FALSE
);

GRANT ALL PRIVILEGES ON TABLE users TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE users_id_seq TO :new_user;

INSERT INTO users (nickname, password, registered_timestamp, last_online_timestamp, is_online) VALUES
('johndoe', 'hashed_password_here', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, TRUE),
('janedoe', 'another_hashed_password', CURRENT_TIMESTAMP - INTERVAL '1 day', CURRENT_TIMESTAMP - INTERVAL '1 hour', FALSE);

DROP ROLE IF EXISTS logi;

CREATE ROLE logi WITH LOGIN PASSWORD 'logi';