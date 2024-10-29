## help
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
