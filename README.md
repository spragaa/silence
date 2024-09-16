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
2. user registration and authorization 
3. sending messages to the server 
4. receiving messages by receiver user 
5. add password and authorization 

improvements:
1. remove user_id, nickname is id
2.  