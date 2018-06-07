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
	fd_set read_fds;

	//basic check of the arguments
	//additional checks can be inserted
	if(argc !=3) {
		printf("Usage : %s server_ip port \n",argv[0]);
		exit(0);
	}
	//tcp connect함수 호출
	sock = tcp_connect(AF_INET,argv[1],atoi(argv[2]));
	if(sock == -1) errquit("tcp_connect fail");
	int maxfdp1 = sock+1;
	printf("server Connected\n");

	FD_ZERO(&read_fds);	
	while(1) {
		printf("FTP QUERY >> ");	
		FD_SET(0,&read_fds);
		FD_SET(sock,&read_fds);
		if(select(maxfdp1,&read_fds,NULL,NULL,NULL) <0) 
			errquit("select fail");
		if(FD_ISSET(0,&read_fds)) {	//사용자의 입력이 감지되면
			if(fgets(bufmsg,MAXLINE, stdin)) {
				send(sock, bufmsg, MAXLINE, 0);
				sendQuery(bufmsg,sock,argv[1]);
			}
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
		char buf[MAXLINE]="T",check[MAXLINE]="T",port[MAXLINE];
		int datasock;
		recv(sock, port, MAXLINE,0);				//reciening data connection port
		datasock=tcp_connect (AF_INET,servip,atoi(port));
		puts("\n-------------Server File List-----------");	
		while(1){ 			//to indicate that more blocks are coming		
			//data print
			recv(datasock, buf, MAXLINE,0);
			if(strcmp("F",buf)==0)			//end of data
				break;
			puts(buf);	
		}
		puts("----------------------------------------");
	}
	//PUT Comment = Server's GET Function
	else if (strstr(token,"put")!=NULL)  {
		char port[MAXLINE], buffer[MAXLINE],lineNum_c[MAXLINE],lineNum2_c[MAXLINE];
		int datasock,lSize,lineNum,lineNum2,i;
		FILE *fp;
		recv(sock, port, MAXLINE,0);				//receiving the data port
	
		datasock=tcp_connect (AF_INET,servip,atoi(port));
		token=strtok(NULL," \n");
		if ((fp=fopen(token,"r"))!=NULL)
		{
		
			send(accp_sock, "T", MAXLINE, 0);
			fseek(fp, 0, SEEK_END);				//go to file's last point
			lSize = ftell(fp);						//tell point location = length of file
			rewind(fp);							//go to file's first point
			lineNum = lSize / MAXLINE;				//calculate line number
			lineNum2 = lSize % MAXLINE;				//if it's longer than MAXLINE , last linenumber ->lineNum2
			sprintf(lineNum_c, "%d", lineNum);
			send(accp_sock, lineNum_c, MAXLINE, 0);		//send line number (= time of send,recv)

			for(i= 0; i < lineNum; i++) { 
				fread (buffer,sizeof(char),MAXLINE,fp);
				send(datasock, buffer, MAXLINE, 0);
			}
			//send rest of file
			sprintf(lineNum2_c,"%d",lineNum2);
			send(sock, lineNum2_c, MAXLINE, 0);
			if (lineNum2 > 0) { 
				fread (buffer,sizeof(char),lineNum2,fp);
				send(datasock, buffer, MAXLINE, 0);
			}
			fclose(fp);
			printf("File uploaded\n");
		}
		else{
			send(sock,"F",MAXLINE,0);
			printf("%s File open Error",token);
			puts("Usage : put <filename>");
		}
	}
   
	//GET Comment = Server's Put function
	else if (strstr(token,"get")!=NULL)  {
		char port[MAXLINE], buffer[MAXLINE],lineNum_c[MAXLINE],lineNum2_c[MAXLINE],check[MAXLINE];
		int datasock,lSize,lineNum,lineNum2,i;
		FILE *fp;
		recv(sock, port, MAXLINE,0);
		datasock=tcp_connect (AF_INET,servip,atoi(port));
		token=strtok(NULL," \n");
		recv(sock,check,MAXLINE,0);
		if (strcmp("T", check) == 0) {
			if ((fp = fopen(token, "w")) == NULL)
				printf("Create File Error\n");
			else
			{
				recv(accp_sock, lineNum_c, MAXLINE, 0);		//get linenumber of file
				lineNum = atoi(lineNum_c);
				//recieve each line's data and write on file
				for (i = 0; i < lineNum; i++) {
					recv(datasock, buffer, MAXLINE, 0);
					fwrite(buffer, sizeof(char), MAXLINE, fp);
				}
				//is file is longer than MAXLINE
				recv(accp_sock, lineNum2_c, MAXLINE, 0);
				lineNum2 = atoi(lineNum2_c);
				if (lineNum2 > 0) {
					recv(datasock, buffer, MAXLINE, 0);
					fwrite(buffer, sizeof(char), lineNum2, fp);
				}
				puts("Download Success");
				fclose(fp);
			}
		}
		else{
			printf("File open Error\n");
			printf("Usage : put <filenames>\n");		
		}
	}//end of getfunc
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
