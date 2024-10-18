# chat_application (IN PROGRESS)
    
## known bugs / suggestions / issues
- user exp is broken -> try to cover all possible cases
- prettier.sh is broken?
- does it even make sense to use --clean in cmake? 
- is it possib le to simplify setup_db scripts? 
- is test_message_database even created? 
- couldn't setup another intance of redis db for testing purposes -> test_message_text_repository.* - crap 

## general goals
- end to end encryption (probably el hamal system)
- docs (mb auto docs)
- good test coverage
- not sure about UI tho, I'd like to implement all of the ui in terminal, but I think that default app is better?
- ??? another server with rest api with statistics
- addd lb support?
- consistent hashring/hashing
- hash passwords and mb more/all info

## to do

### short term goals:
- fix message ids
- merge message_metadata and user_metadata dbs into one with different tables?

### to do:
- is int enough for id? -> probably not 
- message statuses (created, sent, undelivered, delivered, read, unread)
- disable multiple users in one app
- move installing of libs, dbs into one script
- unify the file paths in the scripts and in the code, so it doesn't matter from where you run the script/code
- somehow share debug_message accros scripts
- consistent approach in cmake 
- read server <-> dbs connection params from jsons
- there must some kind of script/config that will create env variables to work with psql flawlessly (postgres user, user_pass, postgres_pass and so on) it is annoying to type them every time - what?!    :)
- add handshake, message framing, and control frames (ping/pong)
- messenger interface in terminal
- most of the runtime errors (db interactions, request/response hadling) should not prevent server/client from running

### refactoring 
- unify the way to compare Timestamps, parse them from/to string, int, json and pqxx::response
- go through all comments in source code 
- implement request structure
- refactor server
- refactor client 


## done
0. split Message class into MessageMetadata and MessageText
1. find the best db for message storage not sure if its THE best, but I've found one)
2. implement connetion to message db class
3. make the decision on message id hadling -> adjust the implementation and then continue with the next steps
4. implement CRUD operrations for message db class
5. implement unit tests for CRUD operrations for message text db class, but they are not running yet ;)
6. fix message metadata crud tests and user tests
7. move registration and authorization on client side to separate functions
8. send requests in one thread and receive responses on another thread
10. logs levels
11. refactor declarations and definitions of nlohmann::json response and request
12. add logs with socker info: get_socket_info 
2. remembder nickname on the client side
3. currently there are at least two copies of User on the client and on the server side
1. add password and authorization
2. user registration and authorization (nearly there, save user nickname on client side) 
3. sending messages to the server (done)
5. add password and authorization
6. add user_metadata and message_metadata hadling on server side
- receiving messages by receiver user 
- handle message sending on server side (save to both dbs)
