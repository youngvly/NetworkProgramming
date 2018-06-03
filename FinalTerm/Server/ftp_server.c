#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
//#include <files.h>
#include <unistd.h>
#include <dirent.h> 

#define MAXLINE 4096 /*max text line length*/
#define LISTENQ 8 /*maximum number of client connections*/

#define EXIT_STRING "exit"
int create_socket(int);
int accept_conn(int);
void errquit(char *mesg) {perror(mesg); exit(1);}	


int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;

 if (argc !=2) {						//validating the input
		printf(" Usage : %s port\n",argv[0]);
		exit(0);
 }
 

 //Create a socket for the soclet
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
		perror("socket fail");
		exit(1);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(atoi(argv[1]));

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

	puts("wait for client");

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

	puts("Client connected");;

  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process

	puts("Child process Created (for Client request)");

  //close listening socket
  close (listenfd);
  int data_port=1024;						//for data connection
  while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
	   printf("String received : %s",buf);
	   char *token,*dummy;
	   dummy=buf;
	   token=strtok(dummy," ");

	   if (strcmp(EXIT_STRING,buf)==0)  {
		printf("Client Exit\n");
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
		datasock=create_socket(data_port);			//creating socket for data connection
		send(connfd, port,MAXLINE,0);				//sending data connection port no. to client
		datasock=accept_conn(datasock);	 			//accepting connection from client
		if(!(in = popen("ls", "r"))){
			perror("fileopen Error");
		}
		while(fgets(temp, sizeof(temp), in)!=NULL){
			send(datasock,"1",MAXLINE,0);
			send(datasock, temp, MAXLINE, 0);
		
		}
		send(datasock,"0",MAXLINE,0);
		pclose(in);

		//cout<<"file closed\n";
		/*int datasock;
		data_port=data_port+1;
		char temp[MAXLINE],port[MAXLINE];
		data_port=data_port+1;
		if(data_port==atoi(argv[1])){
			data_port=data_port+1;
		}
		sprintf(port,"%d",data_port);
		datasock=create_socket(data_port);			//creating socket for data connection
		send(connfd, port,MAXLINE,0);				//sending data connection port no. to client
		datasock=accept_conn(datasock);	 			//accepting connection from client

		char *curr_dir = NULL; 
		DIR *dp = NULL; 
		struct dirent *dptr = NULL; 
		unsigned int count = 0; 

		// Get the value of environment variable PWD 
		curr_dir = getenv("PWD"); 
		if(NULL == curr_dir) 
		{ 
		printf("\n ERROR : Could not get the working directory\n"); 
		return -1; 
		} 

		// Open the current directory 
		dp = opendir((const char*)curr_dir); 
		if(NULL == dp) 
		{ 
		printf("\n ERROR : Could not open the working directory\n"); 
		return -1; 
		} 

		printf("\n"); 
		// Go through and display all the names (files or folders) 
		// Contained in the directory. 
		for(count = 0; NULL != (dptr = readdir(dp)); count++) 
		{ 
			send(datasock,"1",MAXLINE,0);
			sprintf(temp,"%s\n",dptr->d_name);
			send(datasock, temp, MAXLINE, 0);
		printf("%s  ",dptr->d_name); 
		} 
		send(datasock,"0",MAXLINE,0);
		printf("\n %u", count); */
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
		datasock=create_socket(data_port);				//creating socket for data connection
		send(connfd, port,MAXLINE,0);					//sending data connection port to client
		datasock=accept_conn(datasock);					//accepting connection
		recv(connfd,check,MAXLINE,0);
		if(strcmp("1",check)==0){
			if((fp=fopen(token,"w"))==NULL)
				printf("Create File Error\n");
			else
			{
				recv(connfd, char_num_blks, MAXLINE,0);
				num_blks=atoi(char_num_blks);
				for(i= 0; i < num_blks; i++) { 
					recv(datasock, buffer, MAXLINE,0);
					fwrite(buffer,sizeof(char),MAXLINE,fp);
					//cout<<buffer<<endl;
				}
				recv(connfd, char_num_last_blk, MAXLINE,0);
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
		datasock=create_socket(data_port);				//creating socket for data connection
		send(connfd, port,MAXLINE,0);					//sending port no. to client
		datasock=accept_conn(datasock);					//accepting connnection by client
		if ((fp=fopen(token,"r"))!=NULL)
		{
			//size of file
			send(connfd,"1",MAXLINE,0);
			fseek (fp , 0 , SEEK_END);
			lSize = ftell (fp);
			rewind (fp);
			num_blks = lSize/MAXLINE;
			num_last_blk = lSize%MAXLINE; 
			sprintf(char_num_blks,"%d",num_blks);
			send(connfd, char_num_blks, MAXLINE, 0);
			//cout<<num_blks<<"	"<<num_last_blk<<endl;

			for(i= 0; i < num_blks; i++) { 
				fread (buffer,sizeof(char),MAXLINE,fp);
				send(datasock, buffer, MAXLINE, 0);
				//cout<<buffer<<"	"<<i<<endl;
			}
			sprintf(char_num_last_blk,"%d",num_last_blk);
			send(connfd, char_num_last_blk, MAXLINE, 0);
			if (num_last_blk > 0) { 
				fread (buffer,sizeof(char),num_last_blk,fp);
				send(datasock, buffer, MAXLINE, 0);
				//cout<<buffer<<endl;
			}
			fclose(fp);
			printf("File uploaded\n");
		
		}
		else{
			send(connfd,"0",MAXLINE,0);
		}
	   }

	  }	//end of while
 if (n < 0)
	   printf("Can't recive Query from Client\n");

 exit(0);
 }//end of for(;;)
 //close socket of the server
 close(connfd);
}
}

int create_socket(int port)
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
