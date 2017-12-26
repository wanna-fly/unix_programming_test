
#include	"unp.h"
#include        "string.h"
#include	<time.h>

/*struct loginData{
	char *username;
	char *password;
};
struct ackFlag{v
	int flag;
}*/


int
main(int argc, char **argv)
{
	int					i, j, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	char                            recvBuf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	time_t tag;

	char *User[4] = {"root1","root2","root3","root4"};
	char *Password[4] = {"12345","54321","11111","22222"};
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	int flag[10]={0};
	printf("server is ready...\n");
	//char *p=NULL;char *u=NULL;
/* end fig01 */
/*	struct loginData login;
	struct ackFlag flag;*/
/* include fig02 */
	for ( ; ; ) {
		char *u=NULL;
		char *p=NULL;
		//u = (char *)malloc(5 * sizeof(char));
		//p = (char *)malloc(5 * sizeof(char));

		//char u[10]={0};
		//char p[10]={0};
		rset = allset;		/* structure assignment */
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, 4, NULL),
					ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}



		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
						/*4connection closed by client */
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					//FD_CLR(sockfd, &rset);
					client[i] = -1;
					flag[i] = 0;
					printf("one client quit\n");
				} 
				else{
					if(flag[i]==0)
					{	
						u=NULL;p=NULL;
						u=strtok(buf,"/");
						p=strtok(NULL,"/");
					//	printf("step1");
						printf("%s\n",p);
						for( j=0; j < 4;j++)
							{
								//printf("%s\n",buf);
								//printf("%s\n",User[j]);
							
								//mySplit(buf , u , p , n);
								
								//if((strncmp(buf,Password[j],strlen(Password[j]))==0) && (strlen(buf)==strlen(Password[j])) )
								if( (strncmp(u,User[j],strlen(u)) == 0) && (strncmp(p,Password[j],strlen(p)) == 0 ) )
									{
										flag[i]=1;
									//	free(p);free(u);
										printf("%s join in\n",User[j]);
										Writen(sockfd,"1",sizeof("1"));
										bzero(buf, MAXLINE);
										break;
									}
							
								if(j==3)
								{
									//char *msg="wrong name&password";
									Writen(sockfd,"0",sizeof("0"));
									Close(sockfd);
									FD_CLR(sockfd, &allset);
									client[i]=-1;
								}
							}
						//p=NULL;u=NULL;
						//free(p);free(u);
					}
					else{
						strncpy(recvBuf, buf,n);
						strcat(recvBuf,ctime(&tag));
						for(int k=0; k <= maxi ; k++){
							if((client[k]) > 0 && flag[k]==1 ){//printf("%d\n",client[k]);
								
								Writen(client[k], recvBuf, strlen(recvBuf));											}
						}
						bzero(buf,MAXLINE);
						bzero(recvBuf,strlen(recvBuf));
					}
					
				}
				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */
