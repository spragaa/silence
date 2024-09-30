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
    text TEXT NOT NULL,
    deleted BOOLEAN DEFAULT FALSE,
    created_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    deleted_timestamp TIMESTAMP WITH TIME ZONE,
    last_edited_timestamp TIMESTAMP WITH TIME ZONE
);

GRANT ALL PRIVILEGES ON TABLE messages TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE messages_id_seq TO :new_user;

INSERT INTO messages (sender_id, receiver_id, receiver_nickname, text, deleted, created_timestamp) VALUES
(1, 2, 'Hello, Jane!', FALSE, CURRENT_TIMESTAMP),
(2, 1, 'Hi, John!', FALSE, CURRENT_TIMESTAMP - INTERVAL '1 hour');

DROP ROLE IF EXISTS logi;
CREATE ROLE logi WITH LOGIN PASSWORD 'logi';