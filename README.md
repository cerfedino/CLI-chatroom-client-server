# CLI-chatroom-client-server
Simple chat server &amp; client written in C. Allows multiple clients to connect to the server's chatroom and talk to each other.

This was a simple school project and is very buggy (and awful)

## Compile instructions
### Server.c
```bash
gcc -g Server.c -o Server -lpthread
```
### Client.c
```bash
gcc -g Client.c -o Client -lpthread -lncurses -lcurses -ljson-c
```

## Run
Run Server first!
