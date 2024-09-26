# chat_applicaiton
```bash
bash build_both.sh --debug --clean
 --debug - adds debug messages
 --clean - clears build 

 applicable for `build_server.sh` and `build_client.sh` 

bash build_and_run_client.sh --debug --clean --nickname=user1
bash build_server.sh --debug && bash run_server.sh 
```

bash setup_user_metadata.sh --user=spraga --user_password=spraga

## unify and put into a script: 
sudo chmod 755 /home/logi/myself/programming/cpp/chat_application/scripts
sudo chmod 755 /home/logi/myself/programming/cpp/chat_application/database
sudo chmod 644 /home/logi/myself/programming/cpp/chat_application/database/user_metadata.sql

## postgresql
1. sudo systemctl status postgresql.service (stop, start, restart are also options here)
2. pass: pass
3. sudo -i -u postgres - switch to postgres user
4. 
    \l - list all tables
    \c <db_name> - connect to db
    \dt - list db realations
    SELECT * FROM USERS;
    SELECT * FROM MESSAGES;

## what to do:
1. go through commit 8da7e24935c59c4887ac9254a91b7d2e13235537
2. fix all small inaccuracies
3. finish CRUD for user and message
4. rename what needs to be renamed
    
## next steps:
1. implement request structure
~~2. add password and authorization~~
~~3. user registration and authorization (nearly there, save user nickname on client side)~~ 
~~4. sending messages to the server (done)~~
5. receiving messages by receiver user 
~~6. add password and authorization~~
7. add user_metadata and message_metadata hadling on server side 
8. add messages db handling on server side 
9. go through all comments in source code 

## bugs? 
1. why users id are 1, 3, 5, 7, ... ?
2. prettier.sh is broken?

## improvements:
1. disable multiple users in one app
2. remembder nickname on the client side
3. currently there are at least two copies of User on the client and on the server side
4. move installing of libs, dbs into one script
5. unify the pathes in the scripts and in the code, so it doesn't matter from where you run the script/code
6. somehow share debug_message accros scripts
7. consistent approach in cmake 
8. refactor server 
9. read server <-> dbs connection params from jsons
10. logs levels
11.         nlohmann::json response = {
    {"status", "success"},
    {"message", "Authorization successful"}
};
boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));

12. add logs with socker info: get_socket_info 

## ideas:
1. another server with rest api with statistics
2. 

