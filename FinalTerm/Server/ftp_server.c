#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

char* EXIT_STRING = "exit";
int recv_and_print(int sd);
int input_and_send(int sd);
#define MAXLINE 511

int main(int argc, char *argv[]) {
	struct sockaddr_in cliaddr, servaddr;
	int listen_sock,accp_sock;
	pid_t pid;
	if(argc !=2) {
		printf(" Usage : %s port\n",argv[0]);
		exit(0);
	}
	if((listen_sock=socket(PF_INET,SOCK_STREAM,0)) <0) {
		perror("socket fail");
		exit(0);
	}
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	
	if(bind(listen_sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) <0) {
		perror("bind fail");
		exit(0);
	}
	puts("Server is watting for Client");
	listen(listen_sock,1);
	//소켓설정 끝
	
	while(1){
	addrlen=sizeof(cliaddr);
	//accept connection
	if((accp_sock=accept(listen_sock,(struct sockaddr*)&cliaddr,&addrlen))<0){
		perror("accept fail");
		exit(0);
	}
	puts("Client connected");

	if((pid = fork())>0)
		input_and_send(accp_sock);
	else if(pid ==0)
		recv_and_print(accp_sock);
	//close(listen_sock);
	close(accp_sock);
	return 0;

	}//end of while
}

int input_and_send(int sd){
	char buf[MAXLINE+1];
	int nbyte;
	while(fgets(buf,sizeof(buf),stdin) !=NULL) {
		nbyte = strlen(buf);
		write(sd,buf,strlen(buf));
		if(strstr(buf,EXIT_STRING) !=NULL) {
		puts("Good bye.");
		close(sd);
		exit(0);
		}
	}
	return 0;
}

//if pid==0 (Child Node)
int recv_and_print(int sd){

printf("Child process Created (for Client request)\n");


//close listening socket
close (listen_sock);
int data_port=1024;						//for data connection
while ( (n = recv(accp_sock, buf, MAXLINE,0)) > 0)  {
   printf("String received : %s",buf);
   char *token,*dummy;
   dummy=buf;
   token=strtok(dummy," ");

   //exit String
   if (strcmp("quit\n",buf)==0)  {
	printf("
   	cout<<"The client has quit\n";
   }
   //show list string (ls)
   if (strcmp("ls\n",buf)==0)  {
	//FILE *in;
	char* ls;
	char temp[MAXLINE],port[MAXLINE];
	int datasock;
	data_port=data_port+1;
	if(data_port==atoi(argv[1])){
		data_port=data_port+1;
	}
	sprintf(port,"%d",data_port);
	datasock=create_socket(data_port);			//creating socket for data connection
	send(accp_sock, port,MAXLINE,0);				//sending data connection port no. to client
	datasock=accept_conn(datasock);	 			//accepting connection from client
	ls = system("ls");	
	send(datasock,ls,MAXLINE,0);
	/*if(!(in = popen("ls", "r"))){
		cout<<"error"<<endl;
	}
	while(fgets(temp, sizeof(temp), in)!=NULL){
		send(datasock,"1",MAXLINE,0);
		send(datasock, temp, MAXLINE, 0);
	
	}
	send(datasock,"0",MAXLINE,0);
	pclose(in);*/
   }

   if (strcmp("pwd\n",buf)==0)  {
   	char curr_dir[MAXLINE];

	GetCurrentDir(curr_dir,MAXLINE-1);
	send(accp_sock, curr_dir, MAXLINE, 0);
	//cout<<curr_dir<<endl;
   }

   if (strcmp("cd",token)==0)  {
	token=strtok(NULL," \n");
	cout<<"Path given is: "<<token<<endl;
	if(chdir(token)<0){
		send(accp_sock,"0",MAXLINE,0);
	}
	else{
		send(accp_sock,"1",MAXLINE,0);
	}
   }

   if (strcmp("put",token)==0)  {
	char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE],check[MAXLINE];
	int datasock,num_blks,num_last_blk,i;
	FILE *fp;
	token=strtok(NULL," \n");
	cout<<"Filename given is: "<<token<<endl;
	data_port=data_port+1;
	if(data_port==atoi(argv[1])){
		data_port=data_port+1;
	}
	sprintf(port,"%d",data_port);
	datasock=create_socket(data_port);				//creating socket for data connection
	send(accp_sock, port,MAXLINE,0);					//sending data connection port to client
	datasock=accept_conn(datasock);					//accepting connection
	recv(accp_sock,check,MAXLINE,0);
	cout<<check;
	if(strcmp("1",check)==0){
		if((fp=fopen(token,"w"))==NULL)
			cout<<"Error in creating file\n";
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
			cout<<"File download done.\n";
		}
	}
   }

   if (strcmp("get",token)==0)  {
	char port[MAXLINE],buffer[MAXLINE],char_num_blks[MAXLINE],char_num_last_blk[MAXLINE];
	int datasock,lSize,num_blks,num_last_blk,i;
	FILE *fp;
	token=strtok(NULL," \n");
	cout<<"Filename given is: "<<token<<endl;
	data_port=data_port+1;
	if(data_port==atoi(argv[1])){
		data_port=data_port+1;
	}
	sprintf(port,"%d",data_port);
	datasock=create_socket(data_port);				//creating socket for data connection
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
		cout<<"File upload done.\n";
	
	}
	else{
		send(accp_sock,"0",MAXLINE,0);
	}
   }

  }	//end of while

  if (n < 0)
   cout<<"Read error"<<endl;

  exit(0);
 }	//end of pid==0
}
