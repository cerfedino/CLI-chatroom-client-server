#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

///SET THESE ACCORDINGLY///
#define SERVER_PORT 4444
#define SERVER_IP "YOUR SERVER IP"


//do NOT touch these
#define DISCONNECT_COMMAND ":disconnect"
#define MAX_USER_MESSAGE 150   // Max lenght of the message a user can write
#define MAX_IDENTITY_PREFIX 50 // Max lenght of the identifier ("xxx.xxx.xxx.xxx:xxxx (User) :")
////////

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);

struct user{ //Connected users are saved into a linked list and removed when disconnected
    int socket; //The socket comunicating with that user
	struct sockaddr_in Addr;
	pthread_t thread; //Thread handling that user
	char name[10]; //Name of the user
};
struct node { //Linked list containing all the users
	struct user *user;
	int key;
   	struct node *next;
};
   
struct node *head; //First element of the list

void MessBroadcast(char * message); //Sends a message to all the connected users
int addUser(int newUserSocket, char newName[10], struct sockaddr_in newUserAddr); //Adds a newly connected user to the linked list, and creates a thread to listen to its messages
char * findName(int searchSocket); //Returns the name of the player which has that specific socket

int userListLength();
int main(){
	

    int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("[-] Error in connection.\n");
		exit(1);
	}
	printf("[+] Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(4444);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-] Error in binding.\n");
		exit(1);
	}

	if(listen(sockfd, 10) == 0){
		printf("[+] Server Running:\n%s:%d\nListening....\n",SERVER_IP,SERVER_PORT);
	}else{
		printf("[-] Error in binding.\n");
	}

	while(1){ //This loop continuosly accepts incoming requests
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		
		char name[10]="";
		sprintf(name,"Client%d", userListLength()); //A name gets generated

		if(addUser(newSocket,name,newAddr)==1){//The new user gets added to the linked list and a thread gets created to receive its messages
			printf("Connection accepted from %s:%d  '%s' socket(%d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), name, newSocket);
			printf("ConnUsersCount== %d\n", userListLength());		
		}
	}
	return 0;
}

void *perform_work(void *arguments){
	struct node *ptr = head;

	while(ptr != NULL) { //The thread looks for himself in the linked list
		if (ptr->user->thread != pthread_self()){
			ptr = ptr->next;
		}else{
			break;
		}
   	}
	
	struct user *user= ptr->user;

	char * address = inet_ntoa(user->Addr.sin_addr);
	uint16_t port = ntohs(user->Addr.sin_port);
	char * name= user->name;
	
	char buffer[MAX_USER_MESSAGE]; // Message buffer for client
    char message[MAX_IDENTITY_PREFIX + MAX_USER_MESSAGE]; // Message that gets sent/received (identity part + Usr message + )
	
	while(1){ //Continuosly listens for incoming messages
		recv(user->socket, buffer, 100, 0);
		if(strcmp(buffer, "")!=0){
			if(strcmp(buffer, DISCONNECT_COMMAND) == 0){ //If the user asks for it, the server disconnects him
				disconnect(user->socket);
				break;
			}
			else{ //Otherwise broadcasts the message to the rest of th users
				printf("Message from [%s:%d (%s)] = '%s' - Brodcasting...\n",address,port, name,buffer);
				snprintf(message,sizeof(message),"%s:%d (%s) : %s", address,port, name,buffer);
				MessBroadcast(message);
			}
		}
		memset(buffer,0, sizeof(buffer));
		memset(message,0, sizeof(message));
	}
}
void disconnect(int socket){//Closes a specific user's socket and removes him from the linked list 
	struct node *user = head;
	while(user != NULL) {
		if (user->user->socket == socket){
			break;
		}else{
			user = user->next;
		} 	
   	}
	if (user != NULL){
		if(user == head){
			head = head->next;
		}
		else{
			struct node *prev= head;
			while(prev->next != user){
				prev = prev->next;
			}
			prev->next = user->next;
			
	   		
		}
		printf("Disconnected from %s:%d\n", inet_ntoa(user->user->Addr.sin_addr), ntohs(user->user->Addr.sin_port));
		close(user->user->socket);
		free(user);
	}
	else{
		return;
	}
}
void MessBroadcast(char * message){//Sends a string to all the connected users
	struct node *current = head;
	while(current != NULL){
		printf("Sending message to %s:%d '%s' socket(%d)\n", inet_ntoa(current->user->Addr.sin_addr), ntohs(current->user->Addr.sin_port), current->user->name, current->user->socket);
		send(current->user->socket, message, strlen(message)+1, 0);

		current = current->next;
	}
}
int addUser(int newUserSocket, char newName[10], struct sockaddr_in newUserAddr){ //Adds a newly connected user to the linked list, and creates a thread which listens to its messages
		int key=0;
		struct node *newUser = head;
		while(newUser != NULL) {//Goes in the back of the linked list
      		newUser = newUser->next;
			key++;
   		}
		newUser = (struct node*) malloc(sizeof(struct node));
		newUser->key = key;
		newUser->user = (struct user*) malloc(sizeof(struct user));
		strncpy(newUser->user->name, newName,10);
		newUser->user->socket=newUserSocket;
		newUser->user->Addr=newUserAddr;
		newUser->next=head;
		head=newUser;
		
		if(pthread_create(&newUser->user->thread, NULL, perform_work, NULL)==0){
			printf("[+] Thread created succesfully\n");
			return 1;
		}
		else{
			printf("[-] Error in creating thread");
			return 1;
		}
}
int userListLength() {//Returns the lenght of the linked list
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}