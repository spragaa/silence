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
('linus_torvalds', 'linus_torvalds_pass', '2024-09-28 10:00:00+00', '2024-09-28 10:00:00+00', TRUE),
('george_hotz', 'george_hotz_pass', '2024-09-27 10:00:00+00', '2024-09-28 09:00:00+00', FALSE),
('joh_gjengset', 'joh_gjengset_pass', '2024-09-23 10:00:00+00', '2024-09-28 02:00:00+00', TRUE),
('steve_macenski', 'steve_macenski_pass', '2024-09-25 10:00:00+00', '2024-09-28 01:00:00+00', FALSE);

DROP ROLE IF EXISTS logi;

CREATE ROLE logi WITH LOGIN PASSWORD 'logi';