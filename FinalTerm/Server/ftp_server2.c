#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h> 

#define MAXLINE 4096 /*max text line length*/
#define MAXCLI 5

#define EXIT_STRING "exit"
int tcp_listen(int);
int accept_conn(int);
void errquit(char *mesg) {perror(mesg); exit(1);}	
void doQuery(int accp_sock,int listen_sock,char** argv,int s);
int clisock_list[MAXCLI]; //채팅 참가자 소켓번호 목록
int num_cli=0;
int getmax(){
	int max = 3;
	int i;
	for(i=0; i<num_cli; i++)
		if(clisock_list[i]>max) max=clisock_list[i];
	return max;
}


int main (int argc, char **argv)
{
 int listen_sock, accp_sock, n,maxfdp1,i;
 pid_t pid;
 socklen_t clilen=sizeof(struct sockaddr_in);
 char buf[MAXLINE],bufmsg[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
 fd_set read_fds;

 if (argc !=2) {						//validating the input
		printf(" Usage : %s port\n",argv[0]);
		exit(0);
 }
 

 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
 if ((listen_sock = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("socket fail");
		exit(1);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(atoi(argv[1]));

 //bind the socket
 bind (listen_sock, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listen_sock, MAXCLI);
 while(1) {
		FD_ZERO(&read_fds);
		FD_SET(0,&read_fds);
		FD_SET(listen_sock,&read_fds);
		for(i=0; i<num_cli; i++) 
			FD_SET(clisock_list[i],&read_fds);
		maxfdp1 = getmax() +1;
		puts("wait for client");

	//connect and add	
		if(select(maxfdp1, &read_fds,NULL,NULL,NULL) <0) 
			errquit("select fail");
		if(FD_ISSET(listen_sock,&read_fds)) {
			accp_sock = accept (listen_sock, (struct sockaddr *) &cliaddr, &clilen);
			if(accp_sock ==-1) errquit("accept fail");
			clisock_list[num_cli++] = accp_sock;	//sock num
			//send(accp_sock,START_STRING,strlen(START_STRING),0);
			printf("%dnd client added.\n", num_cli);
		}
	for(i=0; i<num_cli; i++) {
		if(FD_ISSET(clisock_list[i],&read_fds)) {
			doQuery(clisock_list[i],listen_sock,argv,i);
		}
	}
		if(FD_ISSET(0,&read_fds)){
			if (fgets(bufmsg,MAXLINE,stdin)){
				if(strstr(bufmsg,EXIT_STRING)!=NULL){	//server entered exit
					puts("GoodBye");
					close(listen_sock);		//close listen socket 
					exit(0);
				}
			}
			bufmsg[0] = '\0';
		}
  

 }//end of while
 //close(accp_sock);
}

void doQuery(int accp_sock,int listen_sock,char** argv,int s){
  puts("doQuery function");
  char buf[MAXLINE];

  int data_port=1024,n;						//for data connection
  if( (n = recv(accp_sock, buf, MAXLINE,0)) > 0)  {
	   printf("String received : %s",buf);
	   char *token,*dummy;
	   dummy=buf;
	   token=strtok(dummy," ");

	   if (strcmp(EXIT_STRING,buf)==0)  {
		close(clisock_list[s]);
		if(s!=num_cli-1)	//삭제하는 클라이언트가 배열의 마지막이아니라면
					//마지막에 있는 클라이언트를 그자리에 넣음
		clisock_list[s] = clisock_list[--num_cli];
		printf("one person out. now =%dperson\n",num_cli);
	   }

	   if (strcmp("ls\n",buf)==0)  {
		FILE *in;
		char temp[MAXLINE],port[MAXLINE];
		int datasock;
		data_port=data_port+1;
		if(data_port==atoi(argv[1])){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=tcp_listen(data_port);			//creating socket for data connection
		send(accp_sock, port,MAXLINE,0);				//sending data connection port no. to client
		datasock=accept_conn(datasock);	 			//accepting connection from client
		if(!(in = popen("ls", "r"))){
			perror("fileopen Error");
		}
		while(fgets(temp, sizeof(temp), in)!=NULL){
			//send(datasock,"1",MAXLINE,0);
			send(datasock, temp, MAXLINE, 0);
		}
		send(datasock,"0",MAXLINE,0);
		pclose(in);
	   }


	   if (strcmp("put",token)==0)  {
		char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE],check[MAXLINE];
		int datasock,num_blks,num_last_blk,i;
		FILE *fp;
		token=strtok(NULL," \n");
		printf("(PUT) File name : %s\n",token);
		data_port=data_port+1;
		if(data_port==atoi(argv[1])){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=tcp_listen(data_port);		//creating socket for data connection
		send(accp_sock, port,MAXLINE,0);	//sending data connection port to client
		datasock=accept_conn(datasock);		//accepting connection
		recv(accp_sock,check,MAXLINE,0);
		if(strcmp("1",check)==0){
			if((fp=fopen(token,"w"))==NULL)
				printf("Create File Error\n");
			else
			{
				recv(accp_sock, char_num_blks, MAXLINE,0);
				num_blks=atoi(char_num_blks);
				for(i= 0; i < num_blks; i++) { 
					recv(datasock, buffer, MAXLINE,0);
					fwrite(buffer,sizeof(char),MAXLINE,fp);
					//cout<<buffer<<endl;
				}
				recv(accp_sock, char_num_last_blk, MAXLINE,0);
				num_last_blk=atoi(char_num_last_blk);
				if (num_last_blk > 0) { 
					recv(datasock, buffer, MAXLINE,0);
					fwrite(buffer,sizeof(char),num_last_blk,fp);
					//cout<<buffer<<endl;
				}
				fclose(fp);
				printf("Downloaded\n");
			}
		}
	   }

	   if (strcmp("get",token)==0)  {
		char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE];
		int datasock,lSize,num_blks,num_last_blk,i;
		FILE *fp;
		token=strtok(NULL," \n");
		printf("(GET) File name : %s\n",token);
		data_port=data_port+1;
		if(data_port==atoi(argv[1])){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=tcp_listen(data_port);				//creating socket for data connection
		send(accp_sock, port,MAXLINE,0);					//sending port no. to client
		datasock=accept_conn(datasock);					//accepting connnection by client
		if ((fp=fopen(token,"r"))!=NULL)
		{
			//size of file
			send(accp_sock,"1",MAXLINE,0);
			fseek (fp , 0 , SEEK_END);
			lSize = ftell (fp);
			rewind (fp);
			num_blks = lSize/MAXLINE;
			num_last_blk = lSize%MAXLINE; 
			sprintf(char_num_blks,"%d",num_blks);
			send(accp_sock, char_num_blks, MAXLINE, 0);
			//cout<<num_blks<<"	"<<num_last_blk<<endl;

			for(i= 0; i < num_blks; i++) { 
				fread (buffer,sizeof(char),MAXLINE,fp);
				send(datasock, buffer, MAXLINE, 0);
				//cout<<buffer<<"	"<<i<<endl;
			}
			sprintf(char_num_last_blk,"%d",num_last_blk);
			send(accp_sock, char_num_last_blk, MAXLINE, 0);
			if (num_last_blk > 0) { 
				fread (buffer,sizeof(char),num_last_blk,fp);
				send(datasock, buffer, MAXLINE, 0);
				//cout<<buffer<<endl;
			}
			fclose(fp);
			printf("File uploaded\n");
		
		}
		else{
			send(accp_sock,"0",MAXLINE,0);
		}
	   }

	  }	//end of while
 else if (n < 0)
	   printf("Can't recive Query from Client\n");

 //exit(0);
 }//end of for(;;)
 //close socket of the server


int tcp_listen(int port)
{
	int sd;
	struct sockaddr_in servaddr;
	
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd ==-1){
		perror("socket fail");
		exit(1);
	}
	//servaddr 구조체의 내용 세팅
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if(bind(sd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		perror("bind fail");
		exit(1);
	}
	//클라이언트로부터 연결요청을 기다림
	listen(sd,1);
	return sd;
}

int accept_conn(int sock)
{
	int dataconnfd;
	socklen_t dataclilen;
	struct sockaddr_in datacliaddr;

	dataclilen = sizeof(datacliaddr);
	  //accept a connection
	if ((dataconnfd = accept (sock, (struct sockaddr *) &datacliaddr, &dataclilen)) <0) {
		printf("Accept data Socket Fail\n");
		exit(2);
	}

	return(dataconnfd);
}
