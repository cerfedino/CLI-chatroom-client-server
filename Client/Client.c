#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 4444
struct termios orig_termios;

void set_conio_terminal_mode();
int getch();
void reset_terminal_mode();
int kbhit();
void append(char* s, char c);

int main(){
	set_conio_terminal_mode();
	pid_t childpid;

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[100];
	char message[200];
	
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n\r");
		exit(1);
	}
	printf("[+]Client Socket is created.\n\r");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n\r");
		exit(1);
	}
	printf("[+]Connected to Server.\n\r");

	childpid = fork();
	
	if(childpid==0){
		while (1){
			if(recv(clientSocket, message, 200, 0) < 0){
				printf("[-]Error in receiving data.\n\r");
		 	}else{
				printf("%s\n\r", message);
			}
		}
	}
	else{
		memset(buffer,0, sizeof(buffer));
		while (1){
			fd_set fds;
    		FD_ZERO(&fds);
    		FD_SET(0, &fds);
			select(1, &fds, NULL, NULL, NULL);
			
			int intTyped=getch();
   			char charTyped=intTyped; // consume the character
			
			if(intTyped==13){
				send(clientSocket, buffer, strlen(buffer)+1, 0);
				if(strcmp(buffer, ":exit")==true){
					close(clientSocket);
					printf("[-]Disconnected from server.\n\r");
					exit(1);
				}
				memset(buffer,0, sizeof(buffer));
				
			}
			else{
				append(buffer,charTyped);
			}
		}
	}
	

	return 0;
}

void reset_terminal_mode(){
    tcsetattr(0, TCSANOW, &orig_termios);
}
void set_conio_terminal_mode(){
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}
int getch(){
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
    } else {
		return c;
    }
}
void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}