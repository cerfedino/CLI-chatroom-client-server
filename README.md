# IRC-client-server
Simple Linux IRC server &amp; client written in C. Allows multiple clients to connect to the server's chatroom and talk to each other
## Compile instructions
### Server.c
  gcc -g Server.c -o Server -lpthread
### Client.c
  gcc -g Client.c -o Client -lpthread -lncurses -lcurses -ljson-c
