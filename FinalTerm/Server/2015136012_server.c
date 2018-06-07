#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h> 

#define MAXLINE 4096	//max text length
#define MAXCLI 5		//max client 
#define EXIT_STRING "exit"	

int tcp_listen(int);	//listen func
int acpt(int);			//accept func
void errquit(char *mesg) {perror(mesg); exit(1);}	
void doQuery(int accp_sock,int listen_sock,char** argv);
int sendDataport(int accp_sock,int mainport);
int dataport = 1025;

int main (int argc, char **argv)
{
	 int listen_sock, accp_sock ;
	 pid_t pid;
	 char buf[MAXLINE],bufmsg[MAXLINE];
	 struct sockaddr_in cliaddr, servaddr;
	 socklen_t addrlen = sizeof(cliaddr);
	 
	 //Server open listen socket for accept client
	 if (argc !=2) {
			printf(" Usage : %s port\n",argv[0]);
			exit(0);
		}
	 if ((listen_sock = socket (AF_INET, SOCK_STREAM, 0)) <0) {
			perror("socket fail");
			exit(1);
		}
	 servaddr.sin_family = AF_INET;
	 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	 servaddr.sin_port = htons(atoi(argv[1]));
	 bind (listen_sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
	 listen (listen_sock, MAXCLI);

	while(1) {
		puts("wait for client");
		//accept a connection
		accp_sock = accept (listen_sock, (struct sockaddr *) &cliaddr, &addrlen);

		puts("Client connected");

		//if its parents process > server exit command
		if((pid = fork ()) >0) {
			close(accp_sock);
			if (fgets(bufmsg,MAXLINE,stdin)){
				if(strstr(bufmsg,EXIT_STRING)!=NULL){	//server entered exit
					puts("GoodBye");
					close(listen_sock);		//close listen socket 
					exit(0);
				}
			}
			bufmsg[0] = '\0';
		}
		//if it’s 0, it’s child process > client command
		if (pid  == 0 ) {	
			close (listen_sock);
			doQuery(accp_sock,listen_sock,argv);
		}
	}//end of while
	 close(accp_sock);
}


void doQuery(int accp_sock,int listen_sock,char** argv){
	puts("Child process Created (for Client request)");
	char buf[MAXLINE];

	int isrecv;				//data port
	while ( (isrecv = recv(accp_sock, buf, MAXLINE,0)) > 0)  {
	   printf("String received : %s",buf);
	   char *token,*originaltext;			//for strt-token			
	   originaltext=buf;
	   token=strtok(originaltext," ");		//extract command only

	   //If its exit
	   if (strcmp(EXIT_STRING,buf)==0)  {
			printf("Client Exit\n");
	   }
	   //if its ls > show server directory ist
	   if (strcmp("ls\n",buf)==0)  {
		DIR *mydir;
		struct dirent *myfile;
		char cwd[MAXLINE];
		char port[MAXLINE];
		int datasock=sendDataport(accp_sock,atoi(argv[1]));
		if (getcwd(cwd, sizeof(cwd)) != NULL){	//get current directory
		    mydir = opendir(cwd);
		    puts(cwd);
		    while((myfile = readdir(mydir)) != NULL)	//get file list in cwd
		    {
			send(datasock, myfile->d_name, MAXLINE, 0);
			//printf(" %s\n", myfile->d_name);
		    }
			send(datasock,"F",2,0);	//send ls end
		    closedir(mydir);
		}
	   }

	   //put func
	   if (strcmp("put",token)==0)  {
			char port[MAXLINE],buffer[MAXLINE],lineNum_c[MAXLINE],lineNum2_c[MAXLINE],check[MAXLINE];
			int datasock,lineNum,lineNum2,i;
			FILE *fp;
			token=strtok(NULL," \n");
			printf("(PUT) File name : %s\n",token);
			datasock=sendDataport(accp_sock,atoi(argv[1]));		

			recv(accp_sock,check,MAXLINE,0);	//check if its connected
			if(strcmp("T",check)==0){
				if((fp=fopen(token,"w"))==NULL)
					printf("Create File Error\n");
				else
				{
					recv(accp_sock, lineNum_c, MAXLINE,0);		//get linenumber of file
					lineNum=atoi(lineNum_c);
					//recieve each line's data and write on file
					for(i= 0; i < lineNum; i++) { 
						recv(datasock, buffer, MAXLINE,0);
						fwrite(buffer,sizeof(char),MAXLINE,fp);
					}
					//is file is longer than MAXLINE
					recv(accp_sock, lineNum2_c, MAXLINE,0);
					lineNum2=atoi(lineNum2_c);
					if (lineNum2 > 0) { 
						recv(datasock, buffer, MAXLINE,0);
						fwrite(buffer,sizeof(char),lineNum2,fp);
					}
					puts("Download Success");
					fclose(fp);
				}
			}
	   }

	   //get func
	   if (strcmp("get",token)==0)  {
			char port[MAXLINE],buffer[MAXLINE],lineNum_c[MAXLINE],lineNum2_c[MAXLINE];
			int datasock,lSize,lineNum,lineNum2,i;
			FILE *fp;
			token=strtok(NULL," \n");
			printf("(GET) File name : %s\n",token);
			datasock=sendDataport(accp_sock,atoi(argv[1]));
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
			send(accp_sock, lineNum2_c, MAXLINE, 0);
			if (lineNum2 > 0) { 
				fread (buffer,sizeof(char),lineNum2,fp);
				send(datasock, buffer, MAXLINE, 0);
			}
			fclose(fp);
			printf("File uploaded\n");
		}
			else{
				send(accp_sock,"F",MAXLINE,0);
			}
	   }

	  }	//end of while
 if (isrecv < 0)
	   printf("Can't recive Query from Client\n");

 exit(0);
 }//end of for(;;)
 //close socket of the server

int sendDataport(int accp_sock,int mainport){
	char dataport_s[MAXLINE];
	int datasock;

	dataport=dataport+1;
	if(dataport==mainport){
		dataport=dataport+1;
	}
	sprintf(dataport_s,"%d",dataport);
	datasock=tcp_listen(dataport);				//creating socket for data connection
	send(accp_sock,dataport_s,MAXLINE,0);					//sending port no. to client
	datasock=acpt(datasock);
	return datasock;
}
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

int acpt(int sock)
{
	int dataconnfd;
	socklen_t dataaddrlen;
	struct sockaddr_in datacliaddr;

	dataaddrlen = sizeof(datacliaddr);
	  //accept a connection
	if ((dataconnfd = accept (sock, (struct sockaddr *) &datacliaddr, &dataaddrlen)) <0) {
		printf("Accept data Socket Fail\n");
		exit(2);
	}

	return(dataconnfd);
}
