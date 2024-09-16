# chat_applicaiton
```bash
bash build_both.sh --debug --clean
 --debug - adds debug messages
 --clean - clears build 

 applicable for `build_server.sh` and `build_client.sh` 

bash build_and_run_client.sh --debug --clean --nickname=user1
bash build_server.sh --debug && bash run_server.sh 
```

next steps:
1. implement request structure
2. add password and authorization (done)
3. user registration and authorization (nearly there, save user nickname on client side) 
4. sending messages to the server 
5. receiving messages by receiver user 
6. add password and authorization 

bugs? 
1. why users id are 1, 3, 5, 7, ... ?


improvements:
1. remove user_id, nickname is id
2. remembder nickname on the client side