DROP DATABASE IF EXISTS :db_name;
DROP USER IF EXISTS :new_user;

CREATE USER :new_user WITH PASSWORD :'new_user_password';

DROP DATABASE IF EXISTS :db_name;
CREATE DATABASE :db_name;
GRANT ALL PRIVILEGES ON DATABASE :db_name TO :new_user;

\c :db_name

CREATE TABLE chats (
    id SERIAL PRIMARY KEY,
    user1_id INT NOT NULL,
    user2_id INT NOT NULL,
    deleted BOOLEAN DEFAULT FALSE,
    deleted_timestamp TIMESTAMP WITH TIME ZONE,
    created_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_message_timestamp TIMESTAMP WITH TIME ZONE,
    CONSTRAINT unique_chat UNIQUE (user1_id, user2_id)
);

CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    chat_id INT NOT NULL REFERENCES chats(id),
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    deleted BOOLEAN DEFAULT FALSE,
    created_timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    deleted_timestamp TIMESTAMP WITH TIME ZONE,
    last_edited_timestamp TIMESTAMP WITH TIME ZONE,

    CONSTRAINT valid_chat_participants CHECK (
        EXISTS (
            SELECT 1 FROM chats 
            WHERE chats.id = chat_id 
            AND (
                (chats.user1_id = sender_id AND chats.user2_id = receiver_id)
                OR 
                (chats.user1_id = receiver_id AND chats.user2_id = sender_id)
            )
        )
    )
);

CREATE INDEX idx_chat_users ON chats(user1_id, user2_id);
CREATE INDEX idx_chat_last_message ON chats(last_message_timestamp DESC);
CREATE INDEX idx_messages_chat ON messages(chat_id);
CREATE INDEX idx_messages_timestamp ON messages(created_timestamp);

GRANT ALL PRIVILEGES ON TABLE messages TO :new_user;
GRANT ALL PRIVILEGES ON TABLE chats TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE messages_id_seq TO :new_user;
GRANT USAGE, SELECT ON SEQUENCE chats_id_seq TO :new_user;

DROP ROLE IF EXISTS logi;
CREATE ROLE logi WITH LOGIN PASSWORD 'logi';