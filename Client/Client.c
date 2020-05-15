#include <arpa/inet.h>
#include <curses.h>
//#include <json-c/json.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
//#include <cJSON.h>


///SET THESE ACCORDINGLY///
#define SERVER_PORT 4444
#define SERVER_IP "YOUR SERVER IP"


//do NOT touch these
#define DISCONNECT_COMMAND ":disconnect"
#define MAX_USER_MESSAGE 150   // Max lenght of the message a user can write
#define MAX_IDENTITY_PREFIX 50 // Max lenght of the identifier ("xxx.xxx.xxx.xxx:xxxx (User) :")
////////

WINDOW *chatWindow;         // The messages will appear here
WINDOW *textboxWindow;      // The message appears here as the user types
int bufferCursor_index = 0; // Specifies where to write the message (eg. He|llo)
struct winsize w;
void adjustWindowSizes();

int clientSocket;              // The socket which comunicates with the server
void disconnect(int clientSocket); // Disconnects from the server

pthread_t serverThread; // The tread that listens for incoming messages and prints them onto 'WINDOW chatWindow'
void *ServerThread_work(void *arguments);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

void insert_substring(char*, char*, int); // Inserts a string into a certain position into another string
char* substring(char*, int, int); 
void remCharAtIndex(char *str, int index);

void append(char *s, char c);

void sig_handler(int signo); // Catches SIGINT signal (CTRL+C)

/*struct json_object *parsed_json;
struct json_object *arr_userProfiles;
char *getUsernameByIndex(int index);
char *getPasswordByIndex(int index);
void reloadConfig();
bool authenticate(int usrIndex, char *inUsername, char *inPassword);*/

int main(int argc, char *argv[]) {
  /* Part that i started to work on initially (user profiles w/json) but then
    realized I didn't have enough time reloadConfig(); int usrIndex = 0;

    /////////////////////////////////
    // LOGGING SEQUENCE
    printf("Username: %s\n", getUsernameByIndex(usrIndex));

    printf("password: ");
    char *inputPassword;
    scanf("%s", inputPassword);

    if (!strcmp(inputPassword, getPasswordByIndex(usrIndex))) {
      printf("PASSWORD CORRETTA\n");
    } else {
      printf("PASSWORD SBAGLIATA");
    }

    do {
    } while (authenticate(usrIndex, getUsernameByIndex(usrIndex),
    inputPassword));

    /////////////////////////////////
    {   // User authenticated
      { // Connected to server
      }
    }*/
  initscr();
  //timeout(-1);
  noecho();
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_MAGENTA);

  // INITIALIZING GRAPHICS W/textboxWindow and chatWindow////////
  adjustWindowSizes();
  wbkgd(stdscr, COLOR_PAIR(1));
  wbkgd(chatWindow, COLOR_PAIR(1));
  wbkgd(textboxWindow, COLOR_PAIR(2));
  keypad(textboxWindow, TRUE);

  wrefresh(chatWindow);
  wrefresh(textboxWindow);
  wrefresh(stdscr);

  // CREATING SOCKET AND CONNECTING TO SERVER
  int ret;
  struct sockaddr_in serverAddr;
  char *buffer[MAX_USER_MESSAGE]; // Message buffer for client
  char *message[MAX_IDENTITY_PREFIX + MAX_USER_MESSAGE]; // Message that gets sent/received (identity part + Usr message + )

  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    wprintw(chatWindow, "[-]Error in connection.\n\r");
    wrefresh(chatWindow);
    endwin();
    exit(1);
  }
  wprintw(chatWindow, "\r[+] Client Socket is created.\n\r");
  wrefresh(chatWindow);

  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    wprintw(chatWindow, "[-] Error in connection.\n\r");
    wrefresh(chatWindow);
    endwin();
    exit(1);
  }
  wprintw(chatWindow, "[+] Connected to Server.\n\r");
  wrefresh(chatWindow);

  if (pthread_create(&serverThread, NULL, &ServerThread_work, NULL) == 0) {
    wprintw( chatWindow, "[+] Thread created succesfully\n\r______________________________\n\r");
    wrefresh(chatWindow);
  } else {
    wprintw(chatWindow, "[-] Error in creating thread\n\r");
    wrefresh(chatWindow);
  }

  //memset(buffer, '/0', sizeof(buffer));

  werase(textboxWindow);
  signal(SIGINT, sig_handler);//If the user causes SIGINT (CTRL+C)
  signal(SIGHUP, sig_handler);//If the user causes SIGINT (CTRL+C)
  char charTypedVector[2];
  charTypedVector[1] = '\0';
  
  while (1) {
    wmove(textboxWindow, 0, bufferCursor_index);//Moves the cursor in the right spot
    wrefresh(chatWindow);
    wrefresh(textboxWindow);
    
    int intTyped = wgetch(textboxWindow); //Waits and then returns input
    charTypedVector[0] = (char)intTyped; //Casts the input
    
    switch (intTyped) {

    case 10: // Enter
      if (!strlen(&buffer) == 0) { //If the message typed sin't null
        /* For some reason this bugs everything out. I don't have time to fix it :(
        if (strcmp(buffer, DISCONNECT_COMMAND) == true) {
          disconnect(clientSocket);
        } else {*/
          send(clientSocket, buffer, strlen(buffer) + 1, 0);
        //}

        memset(buffer, 0, sizeof(buffer));
        bufferCursor_index = 0; //Since the message is sent, the cursor goes back to the beginning
        werase(textboxWindow);
        refresh();
      }
      break;
    case 258: //Down arrow

    break;
    case 259: //Up arrow

    break;
    case 260: // Left arrow = Shifts cursor to the left
      if (bufferCursor_index != 0)
        bufferCursor_index--;
      break;
    case 261: // Right arrow = Shifts cursor to the right
      if (bufferCursor_index < strlen(buffer))
        bufferCursor_index++;
      break;
    case 263: // Backspace
      if (bufferCursor_index != 0) {
        remCharAtIndex(&buffer, bufferCursor_index - 1);
        wclear(textboxWindow);
        wprintw(textboxWindow, buffer);
        bufferCursor_index--;
      }
      break;
    case 330: // Delete
      if (bufferCursor_index < strlen(buffer)) {
        remCharAtIndex(&buffer, bufferCursor_index);
        wclear(textboxWindow);
        wprintw(textboxWindow, buffer);
      }
      break;
    case KEY_RESIZE: //When the window gets resized
      adjustWindowSizes();
      break;
    default: //If a normal char gets typed
      if (strlen(buffer) < MAX_USER_MESSAGE) {
        //insert_substring(&buffer, &charTyped, bufferCursor_index+1);		// modifica di guarducci
        insert_substring(&buffer, charTypedVector, bufferCursor_index+1);
        wmove(textboxWindow, 0, bufferCursor_index++); wrefresh(textboxWindow);
        if (strlen(buffer) != bufferCursor_index) {
          wclear(textboxWindow); wrefresh(textboxWindow);
          wprintw(textboxWindow, buffer); wrefresh(textboxWindow);
        } else {
          waddch(textboxWindow, charTypedVector[0]);wrefresh(textboxWindow);
          wrefresh(textboxWindow);wrefresh(textboxWindow);
        }
      }
      break;
    }
    wrefresh(textboxWindow);
  }
  endwin();
  return 0;
}
void sig_handler(int signo) {
  switch (signo){
    case SIGINT:
    case SIGHUP:
    case SIGQUIT:
      break;
  }
    disconnect(clientSocket);
  endwin();
  exit(1);
}

/* Part that i started to work on initially (user profiles w/json) but then
realized I didn't have enough time void reloadConfig() 

{ int *fp; char buffer[1024]; struct json_object *userProfile; size_t n_userProfiles;

  fp = fopen("userProfiles.json", "r");

  fread(&buffer, sizeof(buffer), 1, fp);
  fclose(fp);
  parsed_json = json_tokener_parse(buffer);
  json_object_object_get_ex(parsed_json, "userProfiles", &arr_userProfiles);
}
bool authenticate(int usrIndex, char *inUsername, char *inPassword) {
  if (!strcmp(inUsername, getUsernameByIndex(usrIndex)) &&
      !strcmp(inPassword, getPasswordByIndex(usrIndex)))
    return true;
  else
    return false;
}

char *getUsernameByIndex(int index) {
  struct json_object *userProfile;
  struct json_object *username;
  userProfile = json_object_array_get_idx(arr_userProfiles, index);
  json_object_object_get_ex(userProfile, "username", &username);
  return json_object_get_string(username);
}
char *getPasswordByIndex(int index) {
  struct json_object *userProfile;
  struct json_object *password;
  userProfile = json_object_array_get_idx(arr_userProfiles, index);
  json_object_object_get_ex(userProfile, "password", &password);
  return json_object_get_string(password);
}*/

void disconnect(int socket) { //Disconnects from the server
  send(socket, DISCONNECT_COMMAND, strlen(DISCONNECT_COMMAND) + 1, 0);
  wprintw(chatWindow, "\n[-] Disconnected from server.\n\r");
  wrefresh(chatWindow);
  pthread_cancel(serverThread);
  close(clientSocket);
  serverThread = NULL;
  clientSocket = NULL;
}
void append(char *s, char c) {
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}
void *ServerThread_work(void *arguments) { //Continuosly listens for messages
  char message[200];
  while (1) {
    wmove(textboxWindow, 0, bufferCursor_index);
    wrefresh(textboxWindow);
    memset(message, '\0', sizeof(message));
    if (recv(clientSocket, message, 200, 0) < 0) {
      wprintw(chatWindow, "[-] Error in receiving data.\n\r");
      wrefresh(chatWindow);
    } else {
      wprintw(chatWindow, "%s\n\r", message);
      wrefresh(chatWindow);
    }
  }
}
void insert_substring(char *a, char *b, int position)
{
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
 
char *substring(char *string, int position, int length) 
{
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
void remCharAtIndex(char *str, int index) {
  if (str[index] != '\0') {
    memmove(&str[index], &str[index + 1], strlen(str) - index);
  }
}
void refreshTermSize() { ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); }
void adjustWindowSizes() {
  refreshTermSize();
  wchar_t *text;

  if (textboxWindow != NULL)
    winchstr(textboxWindow, &text);
  textboxWindow = newwin(1, w.ws_col, w.ws_row - 1, 0);
  wprintw(textboxWindow, text);

  if (chatWindow != NULL)
    winchstr(chatWindow, &text);
  chatWindow = newwin(w.ws_row - 1, w.ws_col, 0, 0);
  wprintw(chatWindow, text);
}