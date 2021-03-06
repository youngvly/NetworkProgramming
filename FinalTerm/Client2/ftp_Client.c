#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 4096 /*max text line length*/

int tcp_connect (int af,char *servip,unsigned short port);
void sendQuery(char bufmsg[MAXLINE],int sock,char* servip);
char *EXIT_STRING="exit";
void errquit(char *mesg) {perror(mesg); exit(1);}

int main(int argc, char **argv){
 int sock;
 struct sockaddr_in servaddr;
 char bufmsg[MAXLINE], recvline[MAXLINE];

 //basic check of the arguments
 //additional checks can be inserted
 if(argc !=3) {
	printf("Usage : %s server_ip port \n",argv[0]);
	exit(0);
 }
 //tcp connect함수 호출
  sock = tcp_connect(AF_INET,argv[1],atoi(argv[2]));
  if(sock == -1) errquit("tcp_connect fail");
  printf("server Connected\n");

  while(1) {
	printf("FTP QUERY >> ");	
	if(fgets(bufmsg,MAXLINE, stdin)) {
		send(sock, bufmsg, MAXLINE, 0);
		sendQuery(bufmsg,sock,argv[1]);
	}
	
  } 

 exit(0);
}

void sendQuery(char bufmsg[MAXLINE],int sock,char* servip){
	
  char *token,*dummy;
  dummy=bufmsg;
  token=strtok(dummy," ");
   
   if (strstr(bufmsg,EXIT_STRING)!=NULL)  {
   	//close(sockfd);
	exit(0);
   }
   else if (strstr(bufmsg,"myls")!=NULL){
	puts("\n------------Client File List-----------");
	system("ls");
	puts("----------------------------------------");
   }
   else if (strstr(bufmsg,"ls")!=NULL)  {
   	char buf[MAXLINE]="1",check[MAXLINE]="1",port[MAXLINE];
	int datasock;
	recv(sock, port, MAXLINE,0);				//reciening data connection port
	datasock=tcp_connect (AF_INET,servip,atoi(port));
	puts("\n-------------Server File List-----------");	
	while(1){ 			//to indicate that more blocks are coming		
		
		//data print
		recv(datasock, buf, MAXLINE,0);
		if(strcmp("0",buf)==0)			//end of data
			break;
		puts(buf);	
	}
	puts("----------------------------------------");
	
   }
   //PUT Comment = Server's GET Function
   else if (strstr(token,"put")!=NULL)  {
   	char port[MAXLINE], buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE];
	int datasock,lSize,num_blks,num_last_blk,i;
	FILE *fp;
	recv(sock, port, MAXLINE,0);				//receiving the data port
	
	datasock=tcp_connect (AF_INET,servip,atoi(port));
	token=strtok(NULL," \n");
	if ((fp=fopen(token,"r"))!=NULL)
	{
		
		send(sock,"1",MAXLINE,0);
		//size of file
		fseek (fp , 0 , SEEK_END);
		lSize = ftell (fp);
		rewind (fp);			//move pointer to start of file
		num_blks = lSize/MAXLINE;	//can write once
		num_last_blk = lSize%MAXLINE; 	//write after once
		
		//send file (can send at once)
		sprintf(char_num_blks,"%d",num_blks);
		send(sock, char_num_blks, MAXLINE, 0);	//send line num
		for(i= 0; i < num_blks; i++) { 
			fread (buffer,sizeof(char),MAXLINE,fp);
			send(datasock, buffer, MAXLINE, 0);
		}
		//send rest of file
		sprintf(char_num_last_blk,"%d",num_last_blk);
		send(sock, char_num_last_blk, MAXLINE, 0);
		if (num_last_blk > 0) { 
			fread (buffer,sizeof(char),num_last_blk,fp);
			send(datasock, buffer, MAXLINE, 0);
		}
		fclose(fp);
		printf("File uploaded\n");
	}
	else{
		send(sock,"0",MAXLINE,0);
		printf("%s File open Error",token);
		puts("Usage : put <filename>");
	}
   }
   
   //GET Comment = Server's Put function
   else if (strstr(token,"get")!=NULL)  {
   	char port[MAXLINE], buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE],message[MAXLINE];
	int datasock,lSize,num_blks,num_last_blk,i;
	FILE *fp;
	recv(sock, port, MAXLINE,0);
	datasock=tcp_connect (AF_INET,servip,atoi(port));
	token=strtok(NULL," \n");
	recv(sock,message,MAXLINE,0);
	if(strcmp("1",message)==0){
		if((fp=fopen(token,"w"))==NULL) printf("Create File Error\n");
		else
		{
			recv(sock, char_num_blks, MAXLINE,0);
			num_blks=atoi(char_num_blks);
			for(i= 0; i < num_blks; i++) { 
				recv(datasock, buffer, MAXLINE,0);
				fwrite(buffer,sizeof(char),MAXLINE,fp);
			}
			recv(sock, char_num_last_blk, MAXLINE,0);
			num_last_blk=atoi(char_num_last_blk);
			if (num_last_blk > 0) { 
				recv(datasock, buffer, MAXLINE,0);
				fwrite(buffer,sizeof(char),num_last_blk,fp);
			}
			fclose(fp);
			puts("Download Done.");
		}
	}
	else{
		printf("File open Error\n");
		printf("Usage : put <filenames>\n");		
	}
   }
   else{
	printf("Commend Error / \nCommand : 'Get <filename>'/ 'put <filename>' /'ls (show directory list)' / myls(Show my directory list) / exit\n");
   }
   printf("FTP QUERY >> ");
}


int tcp_connect(int af,char*servip,unsigned short port) {
	struct sockaddr_in servaddr;
	int s;
	//소켓 생성
	if((s = socket(af,SOCK_STREAM,0)) <0) 
		return -1;
	//채팅서버의 소켓 주소 구조체 servaddr 초기화
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET,servip,&servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	//연결요청
	if(connect(s,(struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		return -1;
	}
	return s;
}
