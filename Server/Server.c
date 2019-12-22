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

#define PORT 4444
#define MAX_USERS_CONNECTED 10

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine) (void *), void *arg);

struct ConnectedUser{
    int socket;
	struct sockaddr_in Addr;
	pthread_t thread;
    char name[10];
} ConnUsers[MAX_USERS_CONNECTED];
int ConnUsersCount=0;

void MessBroadcast(char * message);
int addUser(int newUserSocket, char newName[10], struct sockaddr_in newUserAddr);
char * findName(int searchSocket);

int main(){
    

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	//char buffer[100];
	//pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		
		//////////////////////

		char name[10]="";
		sprintf(name,"Client%d", ConnUsersCount+1);

		if(addUser(newSocket,name,newAddr)==1){
			printf("Connection accepted from %s:%d  '%s' socket(%d)\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), name, newSocket);
			printf("ConnUsersCount== %d\n", ConnUsersCount);
					
		}
	}
	close(newSocket);
	return 0;
}

void *perform_work(void *arguments){
    
	
	int USER_INDEX=0;

	bool success=false;
	for(USER_INDEX=0; ConnUsers[USER_INDEX].thread!=pthread_self() && USER_INDEX<ConnUsersCount;USER_INDEX++){

	}
	struct ConnectedUser *user= &ConnUsers[USER_INDEX];

	char * address = inet_ntoa(user->Addr.sin_addr);
	uint16_t port = ntohs(user->Addr.sin_port);
	char * name= user->name;

	char buffer[100]; //Where the received message will be stored
	char message[200]; //Where the final message that will be sent is stored
	while(1){
		
		recv(user->socket, buffer, 100, 0);
		if(strcmp(buffer, "")!=0){
			if(strcmp(buffer, ":exit") == 0){
				printf("Disconnected from %s:%d\n", address, port);
				break;
			}
			else{
				printf("Messaggio da [%s:%d (%s)] = '%s' - Faccio il broadcast...\n", address,port, name,buffer);
				
				snprintf(message,sizeof(message),"%s:%d (%s) : %s", address,port, name,buffer);

				MessBroadcast(message);
			}
		}
		memset(buffer,0, sizeof(buffer));
		memset(message,0, sizeof(message));
	}
}

void MessBroadcast(char * message){
	for(int n=0;n<=ConnUsersCount-1;n++){
		printf("Mando il messaggio a %s:%d '%s' socket(%d)\n", inet_ntoa(ConnUsers[n].Addr.sin_addr), ntohs(ConnUsers[n].Addr.sin_port), ConnUsers[n].name, ConnUsers[n].socket);
		send(ConnUsers[n].socket, message, strlen(message)+1, 0);
	}
}
int addUser(int newUserSocket, char newName[10], struct sockaddr_in newUserAddr){
	if( ConnUsersCount < MAX_USERS_CONNECTED){
		ConnUsers[ConnUsersCount].socket=newUserSocket;
		
		ConnUsers[ConnUsersCount].Addr=newUserAddr;
		strncpy(ConnUsers[ConnUsersCount].name,newName,10);
		if(pthread_create(&ConnUsers[ConnUsersCount++].thread, NULL, perform_work, NULL)==0){
			printf("[+] Thread created succesfully");
		}
		else{
			printf("[-] Error in creating thread");
		}
		return 1;
	}
	return 0;
}

/*char * findName(int searchSocket){
	//printf("Dentro findName\n");
	for(int n=0;n<=ConnUsersCount-1;n++){
		if(ConnUsers[n].socket == searchSocket){
			//printf("Trovato nome!\n");
			return ConnUsers[n].name;
		}
	}
	//printf("Nome non trovato!\n");
	return NULL;
}*/
