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
#include <time.h>
#include <python2.7/Python.h>

#define MAXLINE 511
#define MAX_SOCK 1024

char *EXIT_STRING = "exit";
char *START_STRING = "Connected to chat_server \n";
char *SHOW_USER = "show user";
char *SHOW_CHAT = "show chat";


int maxfdp1;
int num_chat=0;
int clisock_list[MAX_SOCK];
time_t clitime_list[MAX_SOCK];
char *cli_IP[MAX_SOCK];
char *cliname_list[MAX_SOCK];
int listen_sock;

void showClient();
void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();
void removeClient(int s);
int tcp_listen(int host,int port, int backlog);
void errquit(char *mesg) {perror(mesg); exit(1);}

int main(int argc, char *argv[]) {
	
	Py_Initialize();	//to use readxml, writexml.py 
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path");
	PyRun_SimpleString("sys.path.append('/home/young/Desktop/network_Programming/NetworkProgramming/midTerm')");	//file directory
	
	struct sockaddr_in cliaddr;
	char buf[MAXLINE+1],*bufmsg;
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

	//connect and add	
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
				//client exit
				if(strstr(buf,EXIT_STRING)!=NULL) {
					removeClient(i);
					continue;
				}
				for(j=0; j<num_chat; j++)
					send(clisock_list[j],buf,nbyte,0);
				printf("%s\n",buf);
			}
		}
	//additional option (exit, show user, show chat)

		if (fgets(bufmsg,MAXLINE,stdin)){
			if(strstr(bufmsg,EXIT_STRING)!=NULL){
				puts("GoodBye");
				close(listen_sock);
				exit(0);
			}
			else if(strstr(bufmsg,SHOW_USER)!=NULL){
				//show user menu	
				showClient(); 

			}	
			else if(strstr(bufmsg,SHOW_CHAT)!=NULL){
				//show chatting record
				PyRun_SimpleString("import readxml");
				PyRun_SimpleString("print readxml.readxml()");
			}
			bufmsg[0] = '\0';
		}
	}
	Py_Finalize();
	return 0;
}
void showClient() {
	//총 접속자수, 접속시간, IP
	//clisock_list[] ,num_chat
	printf("------------------------------User------------------------------\n");
	printf("          <<<Total Chatting User : %d>>>\n",num_chat);
	printf("%-20s|%-20s|%-26s","User Name","IP","Enter Time");
	if(num_chat == 0 ) {
		printf("%-66s","no one");
		return ;
	}
	for(int i=0; i<num_chat; i++){
		printf("%-20s|%-20s|%-26s"
			,cliname_list[i],cli_IP[i],ctime(&clitime_list[i]));

	} 
}

void addClient(int s, struct sockaddr_in *newcliaddr) {
	char buf[20];
	char namebuf[20];
	inet_ntop(AF_INET,&newcliaddr->sin_addr,buf,sizeof(buf));
	printf("new client : %s\n",buf);

	//save client info
	clisock_list[num_chat] = s;	//sock num
	time(&clitime_list[num_chat]); 	//enter time

	//stop here??!?!?!
	cli_IP[num_chat] = (char*)malloc(sizeof((char*)buf));
	cliname_list[num_chat]=(char*)malloc(sizeof((char*)buf));
	strcpy(cli_IP[num_chat],buf);
//	sprintf(cli_IP[num_chat],"%s",buf); ;	//cli IP
	recv(clisock_list[num_chat],namebuf,MAXLINE,0);
	strcpy(cliname_list[num_chat],namebuf);
//	sprintf(cliname_list[num_chat],"%s",buf);	//cli name
	num_chat++;
	puts("addClient() end");
}

void removeClient(int s) {
	free(cli_IP[num_chat]);
	close(clisock_list[s]);
	if(s!=num_chat-1)
		clisock_list[s] = clisock_list[num_chat-1];
	num_chat--;
	printf("one person out. now =%dperson\n",num_chat);
}

//find maximum socket number
int getmax() {
	int max = listen_sock;
	int i;
	for(i=0; i<num_chat; i++)
		if(clisock_list[i]>max) max=clisock_list[i];
	return max;
}

//make listen socket 
int tcp_listen(int host, int port, int backlog){
	int sd;
	struct sockaddr_in servaddr;
	
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd ==-1){
		perror("socket fail");
		exit(1);
	}
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if(bind(sd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		perror("bind fail");
		exit(1);
	}
	listen(sd,backlog);
	return sd;
}


		
