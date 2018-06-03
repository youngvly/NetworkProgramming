#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h> 

#define MAXLINE 4096
#define CLINUM 5

char* EXIT_STRING = "exit";
int doQuery(int accp_sock, int listen_sock,int mainport);
int accept_conn(int sock);
void errquit(char *mesg) {perror(mesg); exit(1);}	


int main(int argc, char *argv[]) {
	struct sockaddr_in cliaddr, servaddr;
	int listen_sock,accp_sock,addrlen=sizeof(cliaddr);
	pid_t pid;
	fd_set read_fds;
	char bufmsg[MAXLINE];

	if(argc !=2) {
		printf(" Usage : %s port\n",argv[0]);
		exit(0);
	}

	listen_sock = socket(AF_INET,SOCK_STREAM,0);
	if(listen_sock ==-1){
		perror("socket fail");
		exit(1);
	}
	//servaddr 구조체의 내용 세팅
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	if(bind(listen_sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		perror("bind fail");
		exit(1);
	}
	//클라이언트로부터 연결요청을 기다림
	listen(listen_sock,CLINUM);

	puts("wait for client");
	
	while(1){
	if((accp_sock=accept(listen_sock,(struct sockaddr*)&cliaddr,&addrlen))<0)
		errquit("accept fail");

	puts("Client connected");
	if((pid = fork())==0)	//자식프로세스에서 클라언트 쿼리관리
	{
	
		doQuery(accp_sock,listen_sock,atoi(argv[1]));
	/*else if(pid>0){		//부모프로세스에서 서버입력관리
		if (fgets(bufmsg,MAXLINE,stdin)){
			if(strstr(bufmsg,EXIT_STRING)!=NULL){	//server entered exit
				puts("GoodBye");
				close(listen_sock);		//close listen socket 
				exit(0);
			}
		}
		bufmsg[0] = '\0';
	 }*/
	
	close(accp_sock);
	}//end of while
 return 0;
 }

//make listen socket 
int tcp_listen(int port){/*
int listenfd;
struct sockaddr_in dataservaddr;


//Create a socket for the soclet
//If sockfd<0 there was an error in the creation of the socket
if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
perror("Problem in creating the data socket");
exit(2);
}


//preparation of the socket address
dataservaddr.sin_family = AF_INET;
dataservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
dataservaddr.sin_port = htons(port);

if ((bind (listenfd, (struct sockaddr *) &dataservaddr, sizeof(dataservaddr))) <0) {
perror("Problem in binding the data socket");
exit(2);
}

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, 1);

return(listenfd);*/

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


//if pid==0 (Child Node)
int doQuery(int accp_sock,int listen_sock,int mainport){
	close (listen_sock);
	puts("Child process Created (for Client request)");

	int data_port=1024, isrecv;
	char buf[MAXLINE];

	while ( (isrecv = recv(accp_sock, buf, MAXLINE,0)) > 0)  {
	   printf("String received : %s",buf);
	   char *token,*dummy;	//문자열 토큰분리에 사용될 변수
	   dummy=buf;
	   token=strtok(dummy," ");

	   //exit String
	   if (strstr(buf,EXIT_STRING)!=NULL)  {
		printf("Client Exit\n");
	   }
	   //show list string (ls)
	   else if (strcmp("ls\n",buf)==0)  {
		puts("ls function");
	    	FILE *in;
		char temp[MAXLINE],port[MAXLINE];
		int datasock;
		data_port=data_port+1;
		if(data_port==mainport){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=tcp_listen(data_port);			//creating socket for data connection
		send(accp_sock, port,MAXLINE,0);				//sending data connection port no. to client
		datasock=accept_conn(datasock);	 			//accepting connection from client
		if(!(in = popen("ls", "r"))){
			errquit("fileopen error");
		}
		while(fgets(temp, sizeof(temp), in)!=NULL){
			send(datasock,"1",MAXLINE,0);
			send(datasock, temp, MAXLINE, 0);
		
		}
		send(datasock,"0",MAXLINE,0);
		pclose(in);
		//cout<<"file closed\n";
	   }

	   //put String ( put file to ftp) = getfunction in client
	   else if (strstr(token,"put")!=NULL)  {
		char port[MAXLINE],buffer[MAXLINE],
		char_num_once[MAXLINE],char_num_last[MAXLINE],	//한번에 보내질 파일의 줄수, 나머지..
		check[MAXLINE];
		int datasock,num_blks,num_last_blk,i;
		FILE *fp;
		token=strtok(NULL," \n");
		printf("(PUT) File name : %s\n",token);
		data_port=data_port+1;
		if(data_port==mainport){		//포트번호 중복방지
			data_port=data_port+1;
		}
		datasock=tcp_listen(data_port);				//creating socket for data connection
		sprintf(port,"%d",data_port);

		send(accp_sock, port,MAXLINE,0);				//sending data connection port to client
		datasock=accept_conn(datasock);					//accepting connection
		recv(accp_sock,check,MAXLINE,0);
		if(strcmp("1",check)==0){
			if((fp=fopen(token,"w"))==NULL)				//create file with same name
				printf("Create File Error\n");
			else
			{
				//한번에 보낼 수 없는 
				recv(accp_sock, char_num_once, MAXLINE,0);	//몇줄인지 확인
				num_blks=atoi(char_num_once);
				for(i= 0; i < num_blks; i++) { 			//파일에 write
					recv(datasock, buffer, MAXLINE,0);
					fwrite(buffer,sizeof(char),MAXLINE,fp);
				}
				//한번에 파일을 보낼 수 있는 길이라면
				recv(accp_sock, char_num_last, MAXLINE,0);
				num_last_blk=atoi(char_num_last);
				if (num_last_blk > 0) { 
					recv(datasock, buffer, MAXLINE,0);
					fwrite(buffer,sizeof(char),num_last_blk,fp);
				}
				fclose(fp);
				printf("Downloaded\n");
			}
		}//else : recv 0 = client can't find file
	   }

		//Client's put function
	   else if (strstr(token,"get")!=NULL)  {
		char port[MAXLINE],buffer[MAXLINE],
			char_num_once[MAXLINE],char_num_last[MAXLINE];	//한번에 보내질 줄수 , 나머지 줄수
		int datasock,lSize,num_blks,num_last_blk,i;
		FILE *fp;
		token=strtok(NULL," \n");	//get file name form token
		printf("(GET) File name : %s\n",token);
		data_port=data_port+1;
		if(data_port==mainport){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=tcp_listen(data_port);		//creating socket for data connection
		send(accp_sock, port,MAXLINE,0);		//sending port no. to client
		datasock=accept_conn(datasock);			//accepting connnection by client
		if ((fp=fopen(token,"r"))!=NULL)
		{
			//size of file
			send(accp_sock,"1",MAXLINE,0);
			fseek (fp , 0 , SEEK_END);
			lSize = ftell (fp);
			rewind (fp);
		
			num_blks = lSize/MAXLINE;
			num_last_blk = lSize%MAXLINE; 
			//send file at once
			sprintf(char_num_once,"%d",num_blks);
			send(accp_sock, char_num_once, MAXLINE, 0);
			for(i= 0; i < num_blks; i++) { 
				fread (buffer,sizeof(char),MAXLINE,fp);
				send(datasock, buffer, MAXLINE, 0);
			}
			//send rest of file
			sprintf(char_num_last,"%d",num_last_blk);
			send(accp_sock, char_num_last, MAXLINE, 0);
			if (num_last_blk > 0) { 
				fread (buffer,sizeof(char),num_last_blk,fp);
				send(datasock, buffer, MAXLINE, 0);
			}
			fclose(fp);
			printf("File uploaded\n");
		}
		else{
			send(accp_sock,"0",MAXLINE,0);
		}
	   }	//end of get comment 

	  }//end of while

	  if (isrecv < 0)
	   printf("Can't recive Query from Client\n");
	  puts("Child Process END");
	  exit(0);
	}
}
