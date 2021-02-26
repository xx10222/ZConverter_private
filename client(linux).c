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
//#define IP "192.168.1.88"

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

int main(int argc, char **argv) // ./client ver -> scenario1, ./client 5.6.2 -> scenario2
{
    struct Info1 info1;
    struct Info2 info2;
    info1.size=1000;
    strcpy(info2.os,"Ubuntu");
    if(argc<2)
    {	
	printf("enter the ip number!\n");
	return -1;
    }
    char IP[20]="";
    strcpy(IP,argv[1]);
    printf("Connected to %s!\n", IP);

    if(argc==2){
        //ver 프로토콜방법 path port
	printf("다운 받고 싶은 버전을 입력하시오 : ");
	fgets(info1.ver,sizeof(info1.ver),stdin);
	info1.ver[strlen(info1.ver)-1]='\0';
	printf("프로토콜 방법을 입력하시오(https, websocket, sftp) : ");
	fgets(info2.sendType,sizeof(info2.sendType),stdin);
	info2.sendType[strlen(info2.sendType)-1]='\0';
	printf("저장받을 경로를 입력하시오 : ");
	fgets(info2.filePath,sizeof(info2.filePath),stdin);
	info2.filePath[strlen(info2.filePath)-1]='\0';
	printf("port 번호를 입력하시오 : ");
	scanf("%d", &info2.port);
    }
    else{
        if(argc<6){
		printf("입력인자가 부족합니다.\n");
		printf("<버전> <프로토콜 방법> <저장받을 경로> <port 번호>\n");
		return -1;
	}
	strcpy(info1.ver,argv[1]);
	strcpy(info2.sendType,argv[2]);
	strcpy(info2.filePath,argv[3]);
	info2.port=atoi(argv[4]);
    }
    int nPath=access(info2.filePath,0); //0이면 존재, -1이면 존재X
    if(nPath==-1){
	printf("입력하신 경로가 존재하지 않습니다.\n");
	return -1;
    }
    if(info2.filePath[strlen(info2.filePath)-1]!='/')
	strcat(info2.filePath,"/");
    
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
   	write(server_sockfd,"success\n",strlen("success\n"));
	close(server_sockfd);
   	return 0;
    }
    memset(buf, 0x00, MAXLINE);
    read(server_sockfd,buf,MAXLINE); //receive cli_cmd from server(client.exe)
    printf("command : %s\n", buf);
    system(buf); //cli.exe running
    write(server_sockfd,"success\n",strlen("success\n"));

    close(server_sockfd);
    return 0;
}
