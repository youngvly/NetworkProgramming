#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 511
#define MAX_SOCK 1024

char *EXIT_STRING = "exit";
char *START_STRING = "Connected to chat_server \n";

int maxfdp1;
int num_chat=0;
int clisock_list[MAX_SOCK];
int listen_sock;


void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();
void removeClient(int s);
int tcp_listen(int host,int port, int backlog);
void errquit(char *mesg) {perror(mesg); exit(1);}

int main(int argc, char *argv[]) {
	
	//xml file parsing
	doc = xmlParseFile (DOC_NAME);
	if(doc ==NULL) {
		fprintf(stderr,"Document not parsed Successfully.\n");
		return 0;
	}
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	struct sockaddr_in cliaddr;
	char buf[MAXLINE+1];
	int i,j,nbyte,accp_sock,addrlen = sizeof(struct sockaddr_in);
	fd_set read_fds;
	if(argc !=2) {
		printf("Usage : %s port\n",argv[0]);
		exit(0);
	}
	
	listen_sock=tcp_listen(INADDR_ANY, atoi(argv[1]),5);
	while(1) {
		FD_ZERO(&read_fds);
		FD_SET(listen_sock,&read_fds);
		for(i=0; i<num_chat; i++) 
			FD_SET(clisock_list[1],&read_fds);
		maxfdp1 = getmax() +1;
		puts("wait for client");
		if(select(maxfdp1, &read_fds,NULL,NULL,NULL) <0) 
			errquit("select fail");
		if(FD_ISSET(listen_sock,&read_fds)) {
			accp_sock = accept(listen_sock,(struct sockaddr*) &cliaddr,&addrlen);
			if(accp_sock ==-1) errquit("accept fail");
			addClient(accp_sock,&cliaddr);
			send(accp_sock,START_STRING,strlen(START_STRING),0);
			printf("%dnd client added.\n", num_chat);
		}
		
		for(i=0; i<num_chat; i++) {
			if(FD_ISSET(clisock_list[i],&read_fds)) {
				nbyte = recv(clisock_list[i],buf,MAXLINE,0);
				if(nbyte<=0) {
					removeClient(i);
					continue;
				}
				buf[nbyte]=0;
				if(strstr(buf,EXIT_STRING)!=NULL) {
					removeClient(i);
					continue;
				}
				for(j=0; j<num_chat; j++)
					send(clisock_list[j],buf,nbyte,0);
				printf("%s\n",buf);
			}
		}
	}
	return 0;
}

void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	inet_ntop(AF_INET,&newcliaddr->sin_addr,buf,sizeof(buf));
	printf("new client : %s\n",buf);
	clisock_list[num_chat] = s;
	num_chat++;
}

void removeClient(int s) {


}

