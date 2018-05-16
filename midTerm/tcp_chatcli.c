/**********tcp_chatcli.c************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1000
#define NAME_LEN 20

char *EXIT_STRING="exit";

//소켓 생성 및 서버연결, 생성된 소켓 리턴
int tcp_connect (int af,char *servip,unsigned short port,char* name);
void errquit(char*mesg) {perror(mesg); exit(1);}

int main(int argc, char *argv[]) {
	char bufall[MAXLINE+NAME_LEN],*bufmsg;	//이름+메시지를 위한 버퍼 , bufall에서 메시지부분의 포인터
	int maxfdp1,s,namelen;	//최대소켓디스크립터,소켓,이름의길이
	fd_set read_fds;
	//잘못된 프로그램 실행
	if(argc !=4) {
		printf("Usage : %s server_ip port name\n",argv[0]);
		exit(0);
	}
	//이름이 들어간 메시지형식 지정
	sprintf(bufall,"[%s] :",argv[3]);
	namelen = strlen(bufall);
	bufmsg = bufall+namelen;
	//tcp connect함수 호출
	s = tcp_connect(AF_INET,argv[1],atoi(argv[2]),argv[3]);
	if(s == -1) errquit("tcp_connect fail");
	puts("server Connected");
	//user information에 기록되기 위해 이름이 일차적으로 보내져야함.
	send(s,argv[3],strlen(argv[3]),0);	//send user name
	maxfdp1 = s+1;

	FD_ZERO(&read_fds);	
	while(1) {
		FD_SET(0,&read_fds);
		FD_SET(s,&read_fds);
		if(select(maxfdp1,&read_fds,NULL,NULL,NULL) <0) 
			errquit("select fail");
		if(FD_ISSET(s,&read_fds)) {	//서버와연결된 소켓에서 입력이 오면
			int nbyte;
			//메시지를 받고 출력
			if((nbyte=recv(s,bufmsg,MAXLINE,0)) >0) {
				bufmsg[nbyte] =0;
				printf("%s \n",bufmsg);
			}
		}
		if(FD_ISSET(0,&read_fds)) {	//사용자의 입력이 감지되면
			if(fgets(bufmsg,MAXLINE, stdin)) {
				//메시지 전송
				if(send(s,bufall,namelen+strlen(bufmsg),0)<0)
					puts("Error : Write error on socket.");
				//만약 종료문이면 종료.
				if(strstr(bufmsg,EXIT_STRING) !=NULL) {
					puts("Goodbye.");
					close(s);
					exit(0);
				}
			}
		}
	}
}

int tcp_connect(int af,char*servip,unsigned short port,char*name) {
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
