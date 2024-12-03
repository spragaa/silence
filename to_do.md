## known bugs / suggestions / issues
- user exp is broken -> try to cover all possible cases
- prettier.sh is broken?
- is test_message_database even created?
- couldn't setup another intance of redis db for testing purposes -> test_message_text_repository.* - crap
- if file already exists on the file_server side, it pushes new data to the end -> in such cases generates new name
- if file size % chunk_size == 0 => there are n+1 chunks and last one is not marked as the last one

## general goals
- end to end encryption (probably el hamal system for initial key exchange for aes + aes itself)
- docs (mb auto docs)
- good test coverage
- not sure about UI tho, I'd like to implement all of the ui in terminal, but I think that default app is better?
- add lb support?
- consistent hashring/hashing
- hash passwords and mb more/all info
- mb my own json wrapper
- mb my own 'pistache'

## to do

### short term goals:
- fix redis db connection
- add tests for file_server, file_server_client and their interaction
- handle the filenames on server side (message_metadata, responses for client, status feedback)
- integrate file server client into server workflow

### tasks:
- sha256 tests that will ensure randomness 
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
- fix policy warnings while building
- research pkcs# 5 standart for safe password storage
- add another msg type, smt like json/req/res (not sure if needed)
- github community standarts
- what is the correct naming for unit tests? camel case or snake? (and overall tests naming: in class Test or Tests?, is_valid_filename_test or is_valid_filename)
- should file_server and file_server_client be named server and client?
- can I move all constexpr into .hpp?
- https://coveralls.io
- --debug option for script is counter intuitive -> always print debug messages in scripts or remove them complitely

#### crypto:
- use precomputed safe primes
- add new table [user_id, el_gamal_public_key, dsa_public_key]

### refactoring
- unify the way to compare Timestamps, parse them from/to string, int, json and pqxx::response
- go through all comments in source code
- implement request structure
- refactor client
- refactor aes class into aes256 and aes256_state and maybe extract padding into another class too
- base class for request, and derived classes for each possible request type

## questions?
- is it possible to simplify setup_db scripts?
- merge message_metadata and user_metadata dbs into one with different tables?
- does it even make sense to use --clean in cmake?
- ??? another server with rest api with statistics

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
- fix message ids
- add _member or m_member everywhere???
- design and implement file server :)
- implement rest api client for file server
- test interaction of file_server and file_server_client with all possible requests, make sure that files are sent
- implement hashing for filenames
- solve this: client -> server -> file server, maybe there is way to skip server and pass data directly to file server?
- fix file sending
- refactor server
- implement some kind of db manager, that will manage all 4 dbs
- use std::filesystem::filepath instead of strings
- add namespaces
- https://stackoverflow.com/questions/45710667/defining-global-constants-in-c17
