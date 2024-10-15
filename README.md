# chat_application (IN PROGRESS)

## usage
```bash
bash setup_user_metadata.sh --user=spraga --user_password=spraga
./build_and_run_server.sh --clean --debug
./build_and_run_client.sh --debug --clean --nickname=spraga --password=spraga
```

find . -name '*.*pp' | xargs wc -l

## some postgres stuff: 
sudo chmod 755 /home/logi/myself/programming/cpp/chat_application/scripts
sudo chmod 755 /home/logi/myself/programming/cpp/chat_application/database
sudo chmod 644 /home/logi/myself/programming/cpp/chat_application/database/user_metadata.sql

1. sudo systemctl status postgresql.service (stop, start, restart are also options here)
2. pass: pass
3. sudo -i -u postgres - switch to postgres user
4. 
    \l - list all tables
    \c <db_name> - connect to db
    \dt - list db realations
    SELECT * FROM USERS;
    SELECT * FROM MESSAGES;

## some redis stuff
`redis-cli`
`redis-cli -a <password>` if set

or `redis-cli`
   `AUTH <username> <password>`

now it is `AUTH default spraga`

    
## known bugs / suggestions / issues
1. user exp is broken -> try to cover all possible cases
2. prettier.sh is broken?
3. does it even make sense to use --clean in cmake? 
4. is it possib le to simplify setup_db scripts? 
5. is test_message_database even creates? 
6. couldn't setup another intance of redis db for testing purposes -> test_message_text_repository - crap 

## short term goals:
~~0. split Message class into MessageMetadata and MessageText~~
~~1. find the best db for message storage~~ not sure if its THE best, but I've found one)
~~2. implement connetion to message db class~~
~~3. make the decision on message id hadling -> adjust the implementation and then continue with the next steps~~
~~4. implement CRUD operrations for message db class~~
~~5. implement unit tests for CRUD operrations for message text db class~~ but they are not running yet ;)
~~6. fix message metadata crud tests and user tests~~
6. handle message sending on server side (save to both dbs)
~~7. move registration and authorization on client side to separate functions~~
8. send requests in one thread and receive responses on another thread
9. message statuses (created, sent, undelivered, delivered, read, unread)

## goals:
~~1. add password and authorization~~
~~2. user registration and authorization (nearly there, save user nickname on client side)~~ 
~~3. sending messages to the server (done)~~
4. receiving messages by receiver user 
~~5. add password and authorization~~
~~6. add user_metadata and message_metadata hadling on server side~~
7. tests
8. auto docs
9. messenger interface in terminal
10. asymetric cryptosystem realization

## refactoring stuff
1. unify the way to compare Timestamps, parse them from/to string, int, json and pqxx::response
2. go through all comments in source code 
3. implement request structure

## to do:
1. disable multiple users in one app
~~2. remembder nickname on the client side~~
~~3. currently there are at least two copies of User on the client and on the server side~~
4. move installing of libs, dbs into one script
5. unify the file paths in the scripts and in the code, so it doesn't matter from where you run the script/code
6. somehow share debug_message accros scripts
7. consistent approach in cmake 
~~8. refactor server~~ 
9. read server <-> dbs connection params from jsons
~~10. logs levels~~
~~11. refactor declarations and definitions of nlohmann::json response and request
12. add logs with socker info: get_socket_info 
13. there must some kind of script/config that will create env variables to work with psql flawlessly (postgres user, user_pass, postgres_pass and so on)
it is annoying to type them every time 
14. .github/worflows/autotests.yaml, gl hf :)))
15. rewrite logs in scripts and .sql files
16. read status
17. online status
18. notifications
19. media
20. store some amount of chat data on client side
21. add handshake, message framing, and control frames (ping/pong)

## ideas?
1. another server with rest api with statistics
2. addd lb support?
3. consistent hashring/hashing

## name ideas
1. silence 
2. 