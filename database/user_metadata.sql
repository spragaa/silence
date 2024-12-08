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

CREATE TABLE crypto_keys (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL UNIQUE,
    dsa_public_key TEXT NOT NULL,
    el_gamal_public_key TEXT NOT NULL,
    -- mb add a flag, is_aes_key_initialized?
    created_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX idx_crypto_keys_user_id ON crypto_keys(user_id);

GRANT ALL PRIVILEGES ON TABLE users TO :new_user;
GRANT ALL PRIVILEGES ON TABLE crypto_keys TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE users_id_seq TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE crypto_keys_id_seq TO :new_user;

DROP ROLE IF EXISTS logi;
CREATE ROLE logi WITH LOGIN PASSWORD 'logi';
