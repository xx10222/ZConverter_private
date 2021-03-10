#include <sys/socket.h>  
#include <arpa/inet.h>   
#include <sys/stat.h>
#include <stdio.h>     
#include <string.h>     
#include <unistd.h>     
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#define MAXLINE    1024
#define PORT 3600
#define IP "127.0.0.1"

struct Info1{
   char ver[10]; //request
   int size;
};

struct Info2{
   char filePath[50];
   char sendType[20];
   char os[20];
   int port;
   char hostname[256];
};

struct sftp_Info{

   char username[MAXLINE];
   char password[MAXLINE];
};

int main(int argc, char **argv)
{
    //struct -> argv[1], argv[2], ...
    struct Info1 info1 = {"5.6.1", 1000};
    struct Info2 info2 = {"/home/os2018202001/Desktop/test/client_hash/", "https", "Ubuntu", 8080};
    char host_name[256];
    if(gethostname(host_name, sizeof(host_name)) == 1)
    {
	printf("gethostname error\n");
	exit(1);
    }
    strcpy(info2.hostname, host_name); //save hostname at Info2
    printf("info2 hostname : %s\n", info2.hostname);

    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    char path[] = "/home/os2018202001/Desktop/test/client_hash/";




	char fileName[20]="";
	char filepwd[100]="";

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    serveraddr.sin_port = htons(PORT);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }
    
    if (send(server_sockfd, (struct Info1*)&info1, sizeof(info1), 0) == -1) //first struct
    {
       perror("send1 error");
        return 1;
    }
               
    if (send(server_sockfd, (struct Info2*)&info2, sizeof(info2), 0) == -1) //second struct
    {
       perror("send2 error");
        return 1;
    }

    //add information of client(sftp-username, passwd ...) -> scanf?
    if(!strncmp(info2.sendType, "https", 5)) //HTTPS
    {
	printf("no more information!\n");
    }
    else if(!strncmp(info2.sendType, "sftp", 4)) //SFTP
    {
	//uid_t user_id;
	//struct passwd *user_pw;
	struct sftp_Info sftp_info;
	memset(&sftp_info, 0, sizeof(struct sftp_Info)); //Initialize struct of sftp
	char username[MAXLINE];
	char password[MAXLINE];
	scanf("%s %s",username, password);
	printf("user : %s, pwd : %s\n",username, password);

	/*user_id = getuid();
	user_pw = getpwuid(user_id);
	strcpy(username, user_pw->pw_name);
	strcpy(password, user_pw->pw_passwd); //only can use root user!!!!! OMG
	printf("username : %s, password : %s\n",username, password);*/
	strcpy(sftp_info.username, username);
	strcpy(sftp_info.password, password);

	if(send(server_sockfd, (struct Info2*)&info2, sizeof(info2), 0) == -1) //sftp struct
	{
		perror("send3 error");
		return 1;
	}
	
    }
    else if(!strncmp(info2.sendType, "websocket", 9)) //Websocket
    {
	printf("no more information!\n");
    }

   printf("success all struct send\n");

   printf("wait command\n");
   memset(buf, 0x00, MAXLINE);
   read(server_sockfd,buf,MAXLINE); //receive cli_cmd from server(client.exe)

   if(!strncmp(buf, "error", 5)) //already use version
   {
	printf("%s\n",buf);
	char ch[10];
	memset(ch,0x00,10);
	scanf("%s",ch);
	if(!strncmp(ch,"Y",1) || !strncmp(ch,"y",1))
	{
		write(server_sockfd,ch,10);
	}
	else
	{
		close(server_sockfd);
		return 0;
	}
   }
   else if(!strcmp(buf, "no file"))
   {
	printf("error - no file\n");
	close(server_sockfd);
	return 0;
   }
   else
   {
	printf("command : %s\n", buf);
   	system(buf); //cli.exe running
	write(server_sockfd,"hash\n",strlen("hash\n"));

//check hashsum
	char fileName[20]="";
	char filepwd[100]="";
	read(server_sockfd, fileName, MAXLINE); //receive file name from server
	printf("@@@@@@@@@@file name : %s\n", fileName); 
	
	strcpy(filepwd, info2.filePath);
	strcat(filepwd, fileName);

	char hash_cmd[1024];
	strcpy(hash_cmd,"python hash.py ");
	strcat(hash_cmd, filepwd);
	system(hash_cmd); //execute python hash program - result at hash.txt

	char cli_md5[1024]="";
	char cli_sha1[1024]="";
	char cli_sha256[1024]="";

	//hash.txt open and store
	FILE* hfile = fopen("hash.txt", "r");
	if (hfile == NULL) {
		printf("fail to open file hash.txt!\n");
		return 1;
	}

	fgets(cli_md5, sizeof(cli_md5), hfile);
	//printf("%s", cli_md5);
	fgets(cli_sha1, sizeof(cli_sha1), hfile);
	//printf("%s", cli_sha1);
	fgets(cli_sha256, sizeof(cli_sha256), hfile);
	//printf("%s", cli_sha256);

	fclose(hfile);

	write(server_sockfd,cli_md5,MAXLINE); //md5
	write(server_sockfd,cli_sha1,MAXLINE); //sha1
	write(server_sockfd,cli_sha256,MAXLINE); //sha256


   	write(server_sockfd,"success\n",strlen("success\n"));
	close(server_sockfd);
   	return 0;
   }
}
