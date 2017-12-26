#include "unp.h"
char *username;// NULL;
//username = malloc(10*sizeof(char));
char *password;// malloc(10*sizeof(char));

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	char		buf[MAXLINE];
	char            sendBuf[MAXLINE];
	int		n;
	int flag=0;////
	stdineof = 0;
	FD_ZERO(&rset);

	char loginData[20];	
	strcpy(loginData,username);
	strcat(loginData,"/");
	strcat(loginData,password);
	//strcat(username,password);	
	Writen(sockfd,loginData,strlen(loginData));
	//bzero(buf,strlen(buf));
	//Writen(sockfd,password,strlen(password));
	//printf("%s",loginData);
	
	for ( ; ; ) {
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);

		

		if (FD_ISSET(sockfd, &rset)) {	/* socket is readable */
			if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1)
					return;		/* normal termination */
				else
					err_quit("str_cli: server terminated prematurely");
			}
			if(flag==0){
				printf("-----login infomation-----\n");
				if(strncmp(buf,"1",sizeof("1"))==0){
					printf("you have login success\n");
					flag=1;
					continue;
				}
				else{
					printf("%s\n",buf);
					err_quit("wrong name or password!");}

			}
			Write(fileno(stdout),buf,n);
		}

		if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */
			if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				Shutdown(sockfd, SHUT_WR);	/* send FIN */
				FD_CLR(fileno(fp), &rset);
				continue;
			}
			//if(flag==0){
			//	Writen(sockfd,username,sizeof(username));
				//Writen(sockfd,password,sizeof(password));
			//	continue;
			//}
			if(flag==1){
				strncpy(sendBuf,buf,n);
				strcat(sendBuf,"--from the most handsome one:");
				strcat(sendBuf,username);
				strcat(sendBuf,"\n");
				Writen(sockfd,sendBuf, strlen(sendBuf));
				bzero(sendBuf,strlen(sendBuf));
			}
		}
		
	}
}
int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	username=argv[2];
	password=argv[3];
	if (argc != 4)
		err_quit("usage: tcpcli <IPaddress> <username> <password>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	str_cli(stdin, sockfd);		/* do it all */
	
	//free(username);free(password);
	exit(0);
}
