#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <curses.h>

#define PORT 4444
#define MAX_USER_MESSAGE 150
#define MAX_IDENTITY_PREFIX 50

void append(char* s, char c);

/*struct cursor{
   int x;
   int y;
}chatCursor,textboxCursor;*/
WINDOW * chatWindow;
WINDOW * textboxWindow;
int bufferCursor_index=0;
struct winsize w;

int clientSocket;

void *ServerThread_work(void *arguments);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);
pthread_t serverThread;
int lastInput();
char *substring(char *string, int position, int length);
void insert_substring(char *a, char *b, int position);

int main(){
	initscr();
	timeout(-1);
	
	start_color();
	init_pair(1,COLOR_WHITE,COLOR_BLACK);
	init_pair(2,COLOR_WHITE, COLOR_MAGENTA);

	




	/////INITIALIZING GRAPHICS W/texboxWIndow and chatWindow////////
    adjustWindowSizes();

	wbkgd(stdscr, COLOR_PAIR(1));
	wbkgd(chatWindow, COLOR_PAIR(1));
	wbkgd(textboxWindow, COLOR_PAIR(2));
	keypad(textboxWindow, TRUE);


	wrefresh(chatWindow);
	wrefresh(textboxWindow);
	wrefresh(stdscr);
	/*chatCursor.x=0;
    chatCursor.y=0;
    textboxCursor.x=0;
    textboxCursor.y=0;*/
	
	
	int ret;
	struct sockaddr_in serverAddr;
	char buffer[MAX_USER_MESSAGE];
	char message[MAX_USER_MESSAGE+MAX_IDENTITY_PREFIX];
	
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		wprintw(chatWindow,"[-]Error in connection.\n\r");
		wrefresh(chatWindow);
		endwin();
		exit(1);
	}
	wprintw(chatWindow,"[+] Client Socket is created.\n\r");
	wrefresh(chatWindow);
	
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		wprintw(chatWindow,"[-] Error in connection.\n\r");
		wrefresh(chatWindow);
		endwin();
		exit(1);
	}
	wprintw(chatWindow,"[+] Connected to Server.\n\r");
	wrefresh(chatWindow);

	if(pthread_create(&serverThread, NULL, &ServerThread_work, NULL)==0){
		wprintw(chatWindow,"[+] Thread created succesfully\n\r______________________________\n\r");
		wrefresh(chatWindow);
	}
	else{
		wprintw(chatWindow,"[-] Error in creating thread\n\r");
		wrefresh(chatWindow);
	}

	
	memset(buffer,0, sizeof(buffer));

	werase(textboxWindow);
	while (1){
		wmove(textboxWindow, 0, bufferCursor_index);
		wrefresh(textboxWindow);
		wrefresh(chatWindow);
		/*fd_set fds;
    	FD_ZERO(&fds);
    	FD_SET(0, &fds);
		select(1, &fds, NULL, NULL, NULL);
		*/

		int intTyped=wgetch(textboxWindow);
		
   		char charTyped=intTyped;

		switch (intTyped){
		case 10: //Enter
			if(!strlen(buffer)==0){
				send(clientSocket, buffer, strlen(buffer)+1, 0);
				if(strcmp(buffer, ":exit")==true){
					close(clientSocket);
					wprintw(chatWindow,"[-] Disconnected from server.\n\r");
					wrefresh(chatWindow);
					endwin();
					exit(1);
				}
				memset(buffer,0, sizeof(buffer));
				bufferCursor_index=0;
				werase(textboxWindow);
				refresh();
			}
			break;
		case 260: //Left arrow
			if(bufferCursor_index != 0)
				bufferCursor_index--;
			break;
		case 261: //Right arrow
			if(bufferCursor_index < strlen(buffer))
				bufferCursor_index++;
			break;
		case 263: //Backspace
			if(bufferCursor_index != 0){
				remCharAtIndex(&buffer,bufferCursor_index-1);
				wclear(textboxWindow);
				wprintw(textboxWindow, buffer);
				bufferCursor_index--;
			}
			break;
		case 330: //Delete
			if(bufferCursor_index<strlen(buffer)){
				remCharAtIndex(&buffer,bufferCursor_index);
				wclear(textboxWindow);
				wprintw(textboxWindow, buffer);
			}
		break;
		case KEY_RESIZE:
			//adjustWindowSizes();
		break;
		default:
			if(strlen(buffer) < MAX_USER_MESSAGE){
				insert_substring(&buffer,&charTyped,bufferCursor_index+1);
				wmove(textboxWindow,0,bufferCursor_index++);
				if (strlen(buffer)!= bufferCursor_index){
					wclear(textboxWindow);
					wprintw(textboxWindow,buffer);
				}
				else{
					waddch(textboxWindow, charTyped);
				    wrefresh(textboxWindow);
				}
				
			}
			break;
		}	
	}
	endwin();
	return 0;
}
void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}
int lastInput(){
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
    } else {
		return c;
    }
}
void *ServerThread_work(void *arguments){
	char message[200]; 
	while (1){
		wmove(textboxWindow, 0, bufferCursor_index);
		wrefresh(textboxWindow);
		memset(message,'\0', sizeof(message));
		if(recv(clientSocket, message, 200, 0) < 0){
			wprintw(chatWindow,"[-] Error in receiving data.\n\r");
	 		wrefresh(chatWindow);
		}else{				
			wprintw(chatWindow,"%s\n\r", message);
			wrefresh(chatWindow);
		}
	}
}

void insert_substring(char *a, char *b, int position){
   char *f, *e;
   int length;
   
   length = strlen(a);
   
   f = substring(a, 1, position - 1 );  
   e = substring(a, position, length-position+1);

   strcpy(a, "");
   strcat(a, f);
   free(f);
   strcat(a, b);
   strcat(a, e);
   free(e);
}
char *substring(char *string, int position, int length){
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
   
   if( pointer == NULL )
       exit(EXIT_FAILURE);
 
   for( c = 0 ; c < length ; c++ )
      *(pointer+c) = *((string+position-1)+c);      
       
   *(pointer+c) = '\0';
 
   return pointer;
}
void remCharAtIndex(char* str, int index){
	if(str[index] != '\0'){
		memmove(&str[index], &str[index + 1], strlen(str) - index);
	}
}
void refreshTermSize(){
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
}
void adjustWindowSizes(){
	refreshTermSize();
	wchar_t * text;

	if(textboxWindow!=NULL)
		winchstr(textboxWindow,&text);
	textboxWindow = newwin(1, w.ws_col, w.ws_row-1, 0);
	wprintw(textboxWindow, text);

	if(chatWindow!=NULL)
		winchstr(chatWindow,&text);
	chatWindow = newwin(w.ws_row-1, w.ws_col, 0, 0);
	wprintw(chatWindow, text);
}