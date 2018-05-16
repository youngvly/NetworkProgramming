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


int maxfdp1;		//maximmun socketnumber +1
int num_chat=0;		//채팅 참가자 수
int clisock_list[MAX_SOCK]; //채팅 참가자 소켓번호 목록
//채팅 참가자 정보 (시간, IP, 이름) 저장될 배열
time_t clitime_list[MAX_SOCK];
char *cli_IP[MAX_SOCK];
char *cliname_list[MAX_SOCK];
int listen_sock;	//서버의 listen소켓

void showClient();	//show Client list 
void addClient(int s, struct sockaddr_in *newcliaddr);	//add new Client
int getmax();						//find maximmun socket number
void removeClient(int s);				//remove Client
int tcp_listen(int host,int port, int backlog);		//make socket and listen function
void errquit(char *mesg) {perror(mesg); exit(1);}	

int main(int argc, char *argv[]) {
	
	/********To use Python initialize process ******/
	Py_Initialize();	//to use readxml, writexml.py 
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path");
	PyRun_SimpleString("sys.path.append('/home/young/Desktop/network_Programming/NetworkProgramming/midTerm')");	//file directory
	//to make Empty xml file
	PyRun_SimpleString("import writexml");
	PyRun_SimpleString("writexml.makeRecordFile()");
	PySys_SetArgv(argc,argv);
	PyObject *name,*ptime,*msg,*pdic,*pmodule,*pfunc,*pargs,*funcname;
	/***************************/	

	time_t msgtime;
	struct sockaddr_in cliaddr;	
	char buf[MAXLINE+1],bufmsg[MAXLINE];
	int i,j,nbyte,accp_sock,addrlen = sizeof(struct sockaddr_in);
	fd_set read_fds;		//읽기를 감지할 fd_set 구조체
	if(argc !=2) {
		printf("Usage : %s port\n",argv[0]);
		exit(0);
	}
	
	listen_sock=tcp_listen(INADDR_ANY, atoi(argv[1]),5);

	while(1) {
		//FD_ZERO(&read_fds);	//서버관리자의 입력도 받기위해, 이걸 두면 segmentation error발생.
		FD_SET(0,&read_fds);
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
		
	//broadcast recieved message to all Client 
		for(i=0; i<num_chat; i++) {
			if(FD_ISSET(clisock_list[i],&read_fds)) {
				nbyte = recv(clisock_list[i],buf,MAXLINE,0);
				//if recv()  fail, removeClient
				if(nbyte<=0) {
					removeClient(i);
					continue;
				}
				buf[nbyte]=0;
				//client entered exit
				if(strstr(buf,EXIT_STRING)!=NULL) {
					removeClient(i);
					continue;
				}
			//save message 
				time(&msgtime);	//message record 에 담길 메시지발신시간
				//xml record파일에(output.xml) message 저장
				//c의 변수를 python변수로 바꿈
				name = PyString_FromString(cliname_list[i]);
				ptime = PyString_FromString(ctime(&msgtime));
				msg = PyString_FromString(buf);
				//python의 함수를 불러옴
				funcname = PyString_FromString("writexml");
				pmodule = PyImport_Import(funcname);
				pdic = PyModule_GetDict(pmodule);
				pfunc= PyDict_GetItem(pdic,funcname);
				//매개변수 3개 설정 def writexml(msguser,msgtime,msgdata)
				pargs = PyTuple_New(3);
				PyTuple_SetItem(pargs,0,name);
				PyTuple_SetItem(pargs,1,ptime);
				PyTuple_SetItem(pargs,2,msg);
				PyObject_CallObject(pfunc,pargs);
				//파이선 매개변수 반환
				Py_DECREF(pdic);
				Py_DECREF(name);
				Py_DECREF(ptime);
				Py_DECREF(msg);
				Py_DECREF(pargs);
				//BroadCast
				for(j=0; j<num_chat; j++)
					send(clisock_list[j],buf,nbyte,0);
				printf("%s\n",buf);
			}
		}

/******additional option (exit, show user, show chat)***********/
	if(FD_ISSET(0,&read_fds)){				//내 키보드 변경 감지
		if (fgets(bufmsg,MAXLINE,stdin)){
			if(strstr(bufmsg,EXIT_STRING)!=NULL){	//server entered exit
				puts("GoodBye");
				close(listen_sock);		//close listen socket 
				exit(0);
			}
			//-------SHOW USER MENU
			else if(strstr(bufmsg,SHOW_USER)!=NULL){
				//show user menu	
				showClient(); 

			}	
			//---------SHOW CHATTING MENU
			else if(strstr(bufmsg,SHOW_CHAT)!=NULL){
				//show chatting record
				//call python file (readxml.py)
				PyRun_SimpleString("import readxml");
				PyRun_SimpleString("print readxml.readxml()");
			}
			bufmsg[0] = '\0';
		}
	}
	}
	Py_Finalize();		//close Python
	return 0;
}
//Print Client list 
void showClient() {
	//총 접속자수, 접속시간, IP
	//clisock_list[] ,num_chat
	printf(" ------------------------------User------------------------------\n");
	printf("           <<<Total Chatting User : %d>>>\n",num_chat);
	printf(" %-20s|%-20s|%-26s\n","User Name","IP","Enter Time");
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
	time(&clitime_list[num_chat]); 	//write client entered time
	//현재 buf에 들어있는 값이 ip, cli_IP[]에 저장
	cli_IP[num_chat] = (char*)malloc(sizeof((char*)buf));
	strcpy(cli_IP[num_chat],buf);
	//user의 이름은 user가 connect될 때 바로 이름을 send하게 되어있음. 이를 받아 저장.
	cliname_list[num_chat]=(char*)malloc(sizeof((char*)buf));
	recv(clisock_list[num_chat],namebuf,MAXLINE,0);
	strcpy(cliname_list[num_chat],namebuf);
	
	num_chat++;
	puts("addClient() end");
}

void removeClient(int s) {
	//할당받은 자원들 반납.
	free(cliname_list[s]);
	free(cli_IP[s]);
	close(clisock_list[s]);
	if(s!=num_chat-1)	//삭제하는 클라이언트가 배열의 마지막이아니라면
				//마지막에 있는 클라이언트를 그자리에 넣음
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
	//servaddr 구조체의 내용 세팅
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if(bind(sd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		perror("bind fail");
		exit(1);
	}
	//클라이언트로부터 연결요청을 기다림
	listen(sd,backlog);
	return sd;
}


		
