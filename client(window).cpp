#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
// inet_ntoa가 deprecated가 되었는데.. 사용하려면 아래 설정을 해야 한다.
#pragma warning(disable:4996)
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
// 소켓을 사용하기 위한 라이브러리
#include <WinSock2.h>
#include<Windows.h>
#include<winsock.h>
#include <io.h>
// 수신 버퍼 사이즈
#define BUFFERSIZE 1024
#define MAXLINE 1024
#define PORT 3600
//#define IP "192.168.1.88"

using namespace std;
// 콘솔에 메시지를 출력하는 함수

struct Info1 {
    char ver[10];
    int size;
};

struct Info2 {
    char filePath[50];
    char sendType[20];
    char os[20];
    int port;
    char hostname[256];
};

struct sftp_Info {
    char username[MAXLINE];
    char password[MAXLINE];
};
void print(vector<char>* str)
{
    // 포인트 위치
    int p = 0;
    // 버퍼 설정. +1은 \0를 넣기 위한 크기
    char out[BUFFERSIZE + 1];
    // 콘솔 출력
    cout << "From server message : ";
    for (int n = 0; n < (str->size() / BUFFERSIZE) + 1; n++)
    {
        // 버퍼 사이즈 설정
        int size = str->size();
        // 수신 데이터가 버퍼 사이즈를 넘었을 경우.
        if (size > BUFFERSIZE) {
            if (str->size() < (n + 1) * BUFFERSIZE)
            {
                size = str->size() % BUFFERSIZE;
            }
            else
            {
                size = BUFFERSIZE;
            }
        }
        // echo 메시지와 콘솔 메시지를 작성한다.
        for (int i = 0; i < size; i++, p++)
        {
            out[i] = *(str->begin() + p);
        }
        // 콘솔 메시지 콘솔 출력.
        cout << out;
    }
}
// 실행 함수
int main(int argc, char** argv) {
    // 소켓 정보 데이터 설정
    //struct Info1 info1 = { "new",1000 };
    //struct Info2 info2 = { "C:\\Users\\yjw49\\Desktop\\pa th\\", "https", "Windows", 8080 };
    // 소켓 정보 데이터 설정
    struct Info1 info1;
    struct Info2 info2;
    info1.size = 1000;
    strcpy(info2.os, "Windows");
    if (argc < 2)
    {
        printf("enter the ip number!\n");
        return -1;
    }
    char IP[20] = "";
    strcpy(IP, argv[1]);
    printf("Connected to %s!\n", IP);

    if (argc == 2) {
        //ver 프로토콜방법 path port
        printf("다운 받고 싶은 버전을 입력하시오 : ");
        fgets(info1.ver, sizeof(info1.ver), stdin);
        info1.ver[strlen(info1.ver) - 1] = '\0';
        printf("프로토콜 방법을 입력하시오(https, websocket, sftp) : ");
        fgets(info2.sendType, sizeof(info2.sendType), stdin);
        info2.sendType[strlen(info2.sendType) - 1] = '\0';
        printf("저장받을 경로를 입력하시오 : ");
        fgets(info2.filePath, sizeof(info2.filePath), stdin);
        info2.filePath[strlen(info2.filePath) - 1] = '\0';
        printf("port 번호를 입력하시오 : ");
        scanf("%d", &info2.port);
    }
    else {
        if (argc < 6) {
            printf("입력인자가 부족합니다.\n");
            printf("<버전> <프로토콜 방법> <저장받을 경로> <port 번호>\n");
            return -1;
        }
        strcpy(info1.ver, argv[1]);
        strcpy(info2.sendType, argv[2]);
        strcpy(info2.filePath, argv[3]);
        info2.port = atoi(argv[4]);
    }

    int nPath = access(info2.filePath, 0);	//0이면 존재, -1이면 존재 X
    if (nPath == -1) {
        printf("입력하신 경로가 존재하지 않습니다.\n");
        return -1;
    }
    if (info2.filePath[strlen(info2.filePath) - 1] != '\\')
        strcat(info2.filePath, "\\");

    strcpy(info2.hostname, "\0");
    char HostName[512];
    char buf[MAXLINE] = { 0, };
    WSADATA wsaData;
    // 소켓 실행.
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }
    gethostname(HostName, sizeof(HostName));
    printf("로컬 호스트의 이름은 \"%s\"입니다.\n\n", HostName);
    strcpy(info2.hostname, HostName);
    // Internet의 Stream 방식으로 소켓 생성
    SOCKET server_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    // 소켓 주소 설정
    SOCKADDR_IN addr;
    // 구조체 초기화
    memset(&addr, 0, sizeof(addr));
    // 소켓은 Internet 타입
    addr.sin_family = AF_INET;
    // 127.0.0.1(localhost)로 접속하기
    addr.sin_addr.s_addr = inet_addr(IP);
    // 포트 3600으로 접속
    addr.sin_port = htons(PORT);
    // 접속
    if (connect(server_sockfd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        // 에러 콘솔 출력
        perror("connect error : ");
        return 1;
    }
    // telent은 한글자씩 데이터가 오기 때문에 글자를 모을 buffer가 필요하다.
    if (send(server_sockfd, (char*)&info1, sizeof(Info1), 0) == -1) //first struct(info1)
    {
        perror("send1 error");
        return 1;
    }

    if (send(server_sockfd, (char*)&info2, sizeof(info2), 0) == -1) //second struct(info2)
    {
        perror("send2 error");
        return 1;
    }

    if (!strncmp(info2.sendType, "sftp", 4))
    {
        struct sftp_Info sftp_info;
        memset(&sftp_info, 0, sizeof(struct sftp_Info)); //Initialize struct of sftp
        char username[MAXLINE];
        char password[MAXLINE];
        scanf("%s %s", username, password);
        printf("user : %s, pwd : %s\n", username, password);

        strcpy(sftp_info.username, username);
        strcpy(sftp_info.password, password);
        if (send(server_sockfd, (char*)&sftp_info, sizeof(sftp_info), 0) == -1) //sftp struct
        {
            perror("send3 error");
            return 1;
        }
    }
    memset(buf, 0x00, MAXLINE);
    if (recv(server_sockfd, buf, MAXLINE, 0) == SOCKET_ERROR) {
        perror("recv error");
        return 1;
    }

    if (!strncmp(buf, "error", 5))
    {
        printf("%s\n", buf);
        char ch[10];
        memset(ch, 0x00, 10);
        scanf("%s", ch);
        if (!strncmp(ch, "Y", 1) || !strncmp(ch, "y", 1))
        {
            if (send(server_sockfd, ch, 10, 0) == -1) 
            {
                perror("send choice error");
                return 1;
            }

        }
        else
        {
            // 서버 소켓 종료
            closesocket(server_sockfd);
            // 소켓 종료
            WSACleanup();
            return 0;
        }

    }
    else if (!strcmp(buf, "no file"))
    {
        printf("error - no file\n");
        // 서버 소켓 종료
        closesocket(server_sockfd);
        // 소켓 종료
        WSACleanup();
        return 0;
    }
    else
    {
        printf("cli cmd : %s\n", buf);
        WinExec(buf, SW_SHOW);
        system(buf);
        // 서버 소켓 종료
        closesocket(server_sockfd);
        // 소켓 종료
        WSACleanup();
        return 0;
    }
    memset(buf, 0x00, MAXLINE);
    if (recv(server_sockfd, buf, MAXLINE, 0) == SOCKET_ERROR) {
        perror("recv2 error");
        return 1;
    }
    printf("cli cmd : %s\n", buf);
    WinExec(buf, SW_SHOW);
    system(buf);
    // 서버 소켓 종료
    closesocket(server_sockfd);
    // 소켓 종료
    WSACleanup();
    return 0;
}