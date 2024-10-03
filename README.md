# chat_application (IN PROGRESS)

## today
1. implement user and message unit tests 
2. remove usage of id_counter in message and user class 

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

## known bugs + bugs suggestions
1. user exp is broken -> try to cover all possible cases
2. prettier.sh is broken?
3. does it even make sense to use --clean in cmake? 
4. is it possib le to simplify setup_db scripts? 
5. is test_message_database even creates? 

## next steps:
~~1. add password and authorization~~
~~2. user registration and authorization (nearly there, save user nickname on client side)~~ 
~~3. sending messages to the server (done)~~
4. receiving messages by receiver user 
~~5. add password and authorization~~
6. add user_metadata and message_metadata hadling on server side 
7. tests
8. auto docs
9. messenger interface in terminal

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
~~11.         nlohmann::json response = {~~
    ~~{"status", "success"},~~
    ~~{"message", "Authorization successful"}~~
};~~
~~boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));~~
12. add logs with socker info: get_socket_info 
13. there must some kind of script/config that will create env variables to work with psql flawlessly (postgres user, user_pass, postgres_pass and so on)
it is annoying to type them every time 
14. .github/worflows/autotests.yaml, gl hf :)))


## ideas?
1. another server with rest api with statistics
2. lb?
