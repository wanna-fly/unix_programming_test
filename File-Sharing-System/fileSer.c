/**
 * @file fileSer.c
 * @Synopsis this is the server of file sharing system
 * @author yang baixiang
 * @version 1.0.0
 * @date 2017-12-25
 */

#include	"unp.h"
#include	<string.h>
#include	<unistd.h>
#include	<dirent.h>
#include	<errno.h>
#include	<fcntl.h>
struct my_stat{
	char file_name[10];
	int u_id;
};
int my_index[5] = {0};
int i_file=0;

struct my_stat st[5];
int get_uid(const char *filename){
	int k = 0;
	for(; k <= i_file; k++){
		if(strcmp(st[k].file_name, filename) == 0)
			return st[k].u_id;
	}
	if(k == i_file)
		return -1;
}

int find(const char *name){//find the file in the directory
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("./SerData")) == NULL)
		err_quit("can't open the directory to find file!\n");
	while((dirp = readdir(dp)) != NULL){
		if((strcmp(dirp->d_name,".") == 0) || (strcmp(dirp->d_name,"..") == 0))
			continue;
		if(strncmp(name, dirp->d_name, strlen(name)) == 0){
			return 1;
		}
	}
	if(closedir(dp) < 0)
		err_quit("can't close the dir!\n");
	return 0;
}
int file_size(const char *path){
	struct stat buf;
	stat(path,&buf);
	return buf.st_size;
}

void upload_handler(int sockfd, char *buf, int u_id){
	uint8_t name_size = 0;
	uint64_t file_size = 0;
	char name[20];
	Read(sockfd, &name_size, sizeof(name_size));
	//printf("name_size%d\n",name_size);
	Read(sockfd, &file_size, sizeof(file_size));
	Read(sockfd, name, name_size);
	name[name_size] = '\0';
	printf("ready to recv the file:%s\n",name);
	strcpy(st[i_file].file_name,name);
	st[i_file].u_id = u_id;
	i_file++;
	int fd; int n;
	char path[30] = "./SerData/";
	strcat(path, name);
	uint64_t read_size = 0;
	fd = open(path,O_RDWR|O_CREAT,S_IXUSR|S_IWUSR|S_IRUSR);//
	while(1){
		n = read(sockfd, buf, sizeof(buf));//read from socket diff from file, blocking when readsize < request
		Writen(fd, buf, n);
		read_size+=n;
		bzero(buf, MAXLINE);
		if(read_size == file_size)
			break;
	}// to read bigfile...
	if(n < 0)
		err_quit("read error!");
	printf("file has been recived in %s\n",path);
	close(fd);					
}

void list_handler(int sockfd, char *buf){
	DIR *dp;
	struct dirent *dirp;
	uint8_t name_size;
	if((dp = opendir("./SerData")) == NULL)
		err_quit("can't open the directory!\n");
	while((dirp = readdir(dp)) != NULL){
		if((strcmp(dirp->d_name,".") == 0) || (strcmp(dirp->d_name,"..") == 0))
			continue;
		name_size = strlen(dirp->d_name);
		Writen(sockfd, &name_size, sizeof(name_size));
		//strncpy(buf, dirp->d_name, strlen(dirp->d_name));
		Writen(sockfd, dirp->d_name, name_size);// how to recv the data in cli?
	}
	name_size = 0;
	Writen(sockfd, &name_size, sizeof(name_size));
	if(closedir(dp) < 0)
		err_quit("can't close the dir!\n");
}

void del_handler(int sockfd, char *buf, int u_id){//how to judge the access
	uint8_t name_size = 0;
	char name[20];
	uint8_t exist = 0;
	Read(sockfd, &name_size, sizeof(name_size));
	Read(sockfd, name, name_size);
	name[name_size] = '\0';
	char path[30] = "./SerData/";
	strcat(path, name);
	if(find(name) == 1){
		if(get_uid(name) != u_id)
			exist = 2;
		else{
			if(remove(path) == 0){
				exist = 1;
				printf("%s has been deleted!\n",name);
			}
			else
				exist = 3;
		}

	}
	Writen(sockfd, &exist, sizeof(exist));
}

void download_handler(int sockfd, char *buf){
	uint8_t exist = 0;
	uint8_t name_size = 0;
	char name[20];
	Read(sockfd, &name_size, sizeof(name_size));
	Read(sockfd, name, name_size);
	//name[name_size] = '\0';
	char path[30] = "./SerData/";
	//int n = find(name);
	strncat(path, name, strlen(name));
	printf("%s\n",path);
	if(find(name) == 0){
		Writen(sockfd, &exist, sizeof(exist));
	}
	else{
		exist = 1;	
		printf("ready to send file %s...\n",name);
		Writen(sockfd, &exist, sizeof(exist));
		uint64_t f_size = file_size(path);
		Writen(sockfd, &f_size, sizeof(f_size));
		int fd; int n;
		if((fd = open(path,O_RDONLY)) < 0){
			printf("%s\n",strerror(errno));
			err_quit("server can't open this file\n");
		}
		if((n = Read(fd, buf, f_size)) < 0)
			err_quit("error when reading this file\n");
		Writen(sockfd, buf, f_size);
		printf("sending file finished %s\n",path);
		close(fd);
	}
}

int
main(int argc, char **argv)
{
	int					i, j, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	//char                recvBuf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

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
		char *cmd1;char *cmd2;
		rset = allset;		/* structure assignment */
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			//printf("%u\n",clilen);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
			//printf("%d accepted\n",connfd);
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
					if(flag[i]==0)
					{	
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
						u=NULL;p=NULL;
						u=strtok(buf,"/");
						p=strtok(NULL,"/");
						//printf("%s\n",p);
						for( j=0; j < 4;j++){
							if( (strncmp(u,User[j],strlen(u)) == 0) && (strncmp(p,Password[j],strlen(p)) == 0 ) )
							{
								flag[i]=1;
								my_index[i] = j;
								printf("%s log in the system!\n",User[j]);
								Writen(sockfd,"1",sizeof("1"));
								bzero(buf, MAXLINE);
								break;
							}
							
							if(j==3)
							{
								Writen(sockfd,"0",sizeof("0"));
								Close(sockfd);
								FD_CLR(sockfd, &allset);
								client[i]=-1;
							}
						}
						}
					}
					else{
						uint8_t mode = 0;
					       	//if(Read(sockfd, &len, 1) < 0)//éè¦æ·»å éè¯¯å¤æ­æºå¶
						if ( (n = Read(sockfd, &mode, sizeof(mode))) == 0) {
							/*4connection closed by client */
							Close(sockfd);
							FD_CLR(sockfd, &allset);
							//FD_CLR(sockfd, &rset);
							client[i] = -1;
							flag[i] = 0;
							printf("one client quit\n");
						}
						else{//change the len into mode , recv the mode first and read,then switch,	
							switch(mode){
								case 1:
								{
									list_handler(sockfd, buf);
									break;
								}
								case 2:
								{
									upload_handler(sockfd, buf, my_index[i]);
									break;
								}
								case 3:
								{
									del_handler(sockfd, buf, my_index[i]);
									break;
								}
								case 4:
								{
									download_handler(sockfd, buf);
									break;
								}
							}
							bzero(buf,MAXLINE);
							//bzero(recvBuf,MAXLINE);// is recvBuf necessary?
						}
					}
				if (--nready <= 0)
					break;
				}				/* no more readable descriptors */
			}
		}
}
/* end fig02 */
