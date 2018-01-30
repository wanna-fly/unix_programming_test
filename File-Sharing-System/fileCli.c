/**
 * @file fileCli.c
 * @Synopsis this is a sample file sharing system with socket
 * @author yang baixiang
 * @version 1.0.0
 * @date 2017-12-25
 */
#include "unp.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>

char *username;
char *password;

void show_help(){
	printf("#Welcome to the ybx's File Sharing System!#\n");
        printf("#*****************************************#\n");
        printf("# Input your command as follows:          #\n");
        printf("# upload <filename>                       #\n");
        printf("# list                                    #\n");
        printf("# del <filename>                          #\n");
        printf("# download <filename>                     #\n");
	printf("# exit                                    #\n");
	printf("# help                                    #\n");
        printf("#*****************************************#\n");
}

int file_size(const char *path){
	struct stat buf;
	stat(path,&buf);
	return buf.st_size;
}//count the file size

int count_cmd(char *buf){
	int count;
	while( strtok(buf," ") != NULL){
		count++;
		buf = NULL;
	}
	return count;
}//count the cmd argc

void list(int sockfd, char *buf){
	uint8_t mode = 1;
	uint8_t name_size;
	int num = 0;
	Writen(sockfd, &mode, sizeof(mode));
	printf("================\n");
	while(1){
		Read(sockfd, &name_size, sizeof(name_size));
		if(name_size == 0 )
			break;
		Read(sockfd, buf, name_size);num++;
		Write(fileno(stdout), buf, name_size);
		printf("\n");
	}
	printf("================\n");
	printf("%d files in total\n",num);
}

void upload(char *path, int sockfd, char *buf){
	int fd; 
	uint64_t f_size = file_size(path);
	uint8_t mode = 2;
	uint8_t name_size = strlen(path);
	int n;
	//printf("%d\n",name_size);
	if((fd = open(path,O_RDONLY)) < 0)
		printf("%s\n",strerror(errno));
	else{
		Writen(sockfd, &mode, sizeof(mode));
		Writen(sockfd, &name_size, sizeof(name_size));
		Writen(sockfd, &f_size, sizeof(f_size)); 
		Writen(sockfd, path, name_size);//send mode,name_size and file_size
		while((n = read(fd, buf, sizeof(buf))) > 0){
			Writen(sockfd, buf, n);
			bzero(buf,MAXLINE);// how to read the big file?
		}
		if(n < 0)
			err_quit("read error!");
		printf("file uploading finished!\n");
	}
	close(fd);
}

void download(char *path, int sockfd, char *buf){
	uint8_t mode = 4;
	uint8_t exist = 0;
	//int fd, n;
	uint8_t name_size = strlen(path);
	Writen(sockfd, &mode, sizeof(mode));
	Writen(sockfd, &name_size, sizeof(name_size));
	Writen(sockfd, path, name_size);
	Read(sockfd, &exist, sizeof(exist));
	if(exist == 1){
		int fd,n;
		uint64_t f_size = 0;
		if((fd = open(path, O_WRONLY|O_CREAT, S_IWUSR|S_IRUSR)) < 0){
			err_quit("open error to download");
		}
		Read(sockfd, &f_size, sizeof(f_size));
		n = Read(sockfd, buf, f_size);
		Writen(fd, buf, n);
		close(fd);
		printf("download ... finished!\n");
	}
	else{
		printf("error:no such file found!\n");
	}	
}

void del(char *path, int sockfd, char *buf){
	uint8_t mode = 3;
	uint8_t exist = 0;
	//int fd, n;
	uint8_t name_size = strlen(path);
	Writen(sockfd, &mode, sizeof(mode));
	Writen(sockfd, &name_size, sizeof(name_size));
	Writen(sockfd, path, name_size);
	Read(sockfd, &exist, sizeof(exist));
	if(exist == 1){
		printf("file del successfluly!\n");
	}
	else if(exist == 2){
		printf("del error:NO ACCESS; because it's not your file!\n");
	}
	else{
		printf("no such file found!\n");
	}	
}

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, stdineof;
	fd_set		rset;
	uint8_t	name_size = 0; 
	char		buf[MAXLINE];
	char        copyBuf[30];//copy buf because strtok will destory buf
	int		n;
	int flag=0;//the user login or not
	//uint8_t mode = 0;//different command
	stdineof = 0;
	FD_ZERO(&rset);

	char loginData[20];	
	strcpy(loginData,username);
	strcat(loginData,"/");
	strcat(loginData,password);
	//strcat(username,password);	
	Writen(sockfd,loginData,strlen(loginData));
	//bzero(filebuf,strlen(buf));
	//Writen(sockfd,password,strlen(password));
	
	for ( ; ; ) {
		char *cmd1 = NULL;
		char *cmd2 = NULL;
		if (stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);//使客户端程序阻塞在select实现对服务器端状态的监听

		if (FD_ISSET(sockfd, &rset)) {	/* socket is readable */
			if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1)
					return;		/* normal termination */
				else
					err_quit("str_cli: server terminated prematurely");
			}
			if(flag==0){
				printf("************login infomation***************\n");
				if(strncmp(buf,"1",sizeof("1"))==0){			
					show_help();
					flag=1;
					bzero(buf,MAXLINE);
					continue;
				}
				else{
					err_quit("wrong name or password!\n");
					//close(sockfd);
					//goto L;
				}

			}
			//Write(fileno(stdout),buf,n);
		}

		if (FD_ISSET(fileno(fp), &rset)) {  /* input is readable */  
		
			if(flag==1){
				//strncpy(, buf, n);
				if ( (n = Read(fileno(fp), buf, MAXLINE)) == 0) {
					stdineof = 1;
					Shutdown(sockfd, SHUT_WR);	/* send FIN */
					FD_CLR(fileno(fp), &rset);
					continue;
				}
				strncpy(copyBuf, buf, strlen(buf));
				int num = count_cmd(buf);
				if(num == 1){
					if(strncmp(buf,"list",strlen("list")) == 0 )
						list(sockfd, buf);
					else if(strncmp(buf,"help",strlen("help")) == 0 )
						show_help();
					else if(strncmp(buf,"exit",strlen("exit")) == 0 ){
						printf("goodbye!\n");
						exit(0);
					}
					else
						printf("invalid cmd!\n");
				}
				else if(num == 2){
					cmd1 = NULL;cmd2 = NULL;
					cmd1 = strtok(copyBuf," ");
					cmd2 = strtok(NULL,"\n");
					if(!((strcmp(cmd1,"upload")==0)||(strcmp(cmd1,"list")==0) || (strcmp(cmd1,"del") == 0) || strcmp(cmd1,"download") == 0))
						err_quit("invaild command! ");
					//name_size = strlen(cmd2);// +1???
					//printf("%d\n",name_size);
					//bzero(buf,MAXLINE);//why set to zero influence the cmd2??
					if((strncmp(cmd1,"upload",strlen(cmd1))) == 0){
						upload(cmd2, sockfd, buf);	
					}
					else if(strncmp(cmd1, "del", strlen(cmd1)) == 0){
						del(cmd2, sockfd, buf);
					}
					else if(strncmp(cmd1, "download", strlen(cmd1)) == 0){
						download(cmd2, sockfd, buf);
					}
					else{
						err_quit("wrong command! type: 1\n");
					}
				}
				else{
					err_quit("wrong command! type: 2\n");
				}
			}
			bzero(buf,MAXLINE);
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

