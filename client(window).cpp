#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
// inet_ntoa�� deprecated�� �Ǿ��µ�.. ����Ϸ��� �Ʒ� ������ �ؾ� �Ѵ�.
#pragma warning(disable:4996)
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
// ������ ����ϱ� ���� ���̺귯��
#include <WinSock2.h>
#include<Windows.h>
#include<winsock.h>
#include <io.h>
// ���� ���� ������
#define BUFFERSIZE 1024
#define MAXLINE 1024
#define PORT 3600
//#define IP "192.168.1.88"

using namespace std;
// �ֿܼ� �޽����� ����ϴ� �Լ�

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
    // ����Ʈ ��ġ
    int p = 0;
    // ���� ����. +1�� \0�� �ֱ� ���� ũ��
    char out[BUFFERSIZE + 1];
    // �ܼ� ���
    cout << "From server message : ";
    for (int n = 0; n < (str->size() / BUFFERSIZE) + 1; n++)
    {
        // ���� ������ ����
        int size = str->size();
        // ���� �����Ͱ� ���� ����� �Ѿ��� ���.
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
        // echo �޽����� �ܼ� �޽����� �ۼ��Ѵ�.
        for (int i = 0; i < size; i++, p++)
        {
            out[i] = *(str->begin() + p);
        }
        // �ܼ� �޽��� �ܼ� ���.
        cout << out;
    }
}
// ���� �Լ�
int main(int argc, char** argv) {
    // ���� ���� ������ ����
    //struct Info1 info1 = { "new",1000 };
    //struct Info2 info2 = { "C:\\Users\\yjw49\\Desktop\\pa th\\", "https", "Windows", 8080 };
    // ���� ���� ������ ����
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
        //ver �������ݹ�� path port
        printf("�ٿ� �ް� ���� ������ �Է��Ͻÿ� : ");
        fgets(info1.ver, sizeof(info1.ver), stdin);
        info1.ver[strlen(info1.ver) - 1] = '\0';
        printf("�������� ����� �Է��Ͻÿ�(https, websocket, sftp) : ");
        fgets(info2.sendType, sizeof(info2.sendType), stdin);
        info2.sendType[strlen(info2.sendType) - 1] = '\0';
        printf("������� ��θ� �Է��Ͻÿ� : ");
        fgets(info2.filePath, sizeof(info2.filePath), stdin);
        info2.filePath[strlen(info2.filePath) - 1] = '\0';
        printf("port ��ȣ�� �Է��Ͻÿ� : ");
        scanf("%d", &info2.port);
    }
    else {
        if (argc < 6) {
            printf("�Է����ڰ� �����մϴ�.\n");
            printf("<����> <�������� ���> <������� ���> <port ��ȣ>\n");
            return -1;
        }
        strcpy(info1.ver, argv[1]);
        strcpy(info2.sendType, argv[2]);
        strcpy(info2.filePath, argv[3]);
        info2.port = atoi(argv[4]);
    }

    int nPath = access(info2.filePath, 0);	//0�̸� ����, -1�̸� ���� X
    if (nPath == -1) {
        printf("�Է��Ͻ� ��ΰ� �������� �ʽ��ϴ�.\n");
        return -1;
    }
    if (info2.filePath[strlen(info2.filePath) - 1] != '\\')
        strcat(info2.filePath, "\\");

    strcpy(info2.hostname, "\0");
    char HostName[512];
    char buf[MAXLINE] = { 0, };
    WSADATA wsaData;
    // ���� ����.
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }
    gethostname(HostName, sizeof(HostName));
    printf("���� ȣ��Ʈ�� �̸��� \"%s\"�Դϴ�.\n\n", HostName);
    strcpy(info2.hostname, HostName);
    // Internet�� Stream ������� ���� ����
    SOCKET server_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    // ���� �ּ� ����
    SOCKADDR_IN addr;
    // ����ü �ʱ�ȭ
    memset(&addr, 0, sizeof(addr));
    // ������ Internet Ÿ��
    addr.sin_family = AF_INET;
    // 127.0.0.1(localhost)�� �����ϱ�
    addr.sin_addr.s_addr = inet_addr(IP);
    // ��Ʈ 3600���� ����
    addr.sin_port = htons(PORT);
    // ����
    if (connect(server_sockfd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        // ���� �ܼ� ���
        perror("connect error : ");
        return 1;
    }
    // telent�� �ѱ��ھ� �����Ͱ� ���� ������ ���ڸ� ���� buffer�� �ʿ��ϴ�.
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
            // ���� ���� ����
            closesocket(server_sockfd);
            // ���� ����
            WSACleanup();
            return 0;
        }

    }
    else if (!strcmp(buf, "no file"))
    {
        printf("error - no file\n");
        // ���� ���� ����
        closesocket(server_sockfd);
        // ���� ����
        WSACleanup();
        return 0;
    }
    else
    {
        printf("cli cmd : %s\n", buf);
        WinExec(buf, SW_SHOW);
        system(buf);
        // ���� ���� ����
        closesocket(server_sockfd);
        // ���� ����
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
    // ���� ���� ����
    closesocket(server_sockfd);
    // ���� ����
    WSACleanup();
    return 0;
}