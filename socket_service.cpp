#include <iostream>
#include "stdafx.h"
#include "windows.h"
#include "winsock2.h"
#include <string.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
struct sockaddr_in svraddr;
bool flag[10];
int totalsec;
int last;
int icount;
int addrlen =sizeof(svraddr);
//发送服务器信息
static struct _tagSCmdBuf
{
    int  sec; //get 时取得sec值后，发送对应的包
} SCmdBuf;
//接受服务器发送信息
static struct _tagCCmdBuf
{

    char cmd[4]; //info 或 over
    char fn[12]; // info 时文件名
    int  sec;//总共发送包数
    int  bagsize;//包的大小
} CCmdBuf;
//接受文件的内容
static struct _tagNodeInfo
{
    int sec; //包的id
    char info[1024];//包的内容
} fbuffer[12]; //一次读取文件大小
void envinit()
{
    WSADATA wsa= {0}; //初始化
    WSAStartup(MAKEWORD(2,2),&wsa);
}
void envclean()
{
    WSACleanup ();
}
//socket 初始化 IP 端口 绑定！端口固定 8521 ip 为本地的
SOCKET initSocketClient()
{
    SOCKET sockclient = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(INVALID_SOCKET==sockclient)
        return sockclient;
    else
    {

        svraddr.sin_family = AF_INET;
        svraddr.sin_port=htons(8521);
        svraddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    }
    printf("serversocket 初始完成\n");
    return sockclient;
}
/*---------------------------------我是下划线--------------------------------*/
static FILE * getfileinfo()
{
    FILE *fp = NULL;
    char kk[20];
    strcpy(kk,"d:/");
    strcat(kk,CCmdBuf.fn);
    fp=fopen(kk,"wb");
    if(NULL==fp)
    {
        printf("fp get wrong\n");
        system("pause");
    }
    totalsec=CCmdBuf.sec;
    last=totalsec%10;
    printf("/*------------------------------我收到了文件信息可以传递了偶！----------------------------*/\n");
    return fp;
}
/*---------------------------------我是下划线--------------------------------*/
static int sendtosrv(SOCKET sockclient,char *buffer ,int size)
{
    int flag=sendto(sockclient,buffer,size,0,(struct sockaddr *)&svraddr,sizeof(svraddr));
    if(SOCKET_ERROR==flag)
        printf("sent error is happed\n");
    return flag;
}
int sentfile()
{
    envinit();
    SOCKET sockclient = initSocketClient();
    bind(sockclient,(LPSOCKADDR)&svraddr,sizeof(svraddr));///绑定
    FILE * fp;
    recvfrom(sockclient,(char *)&CCmdBuf,sizeof(struct _tagCCmdBuf),0,(struct sockaddr *)&svraddr,&addrlen);
    fp=getfileinfo();
    SCmdBuf.sec=-1;
    sendtosrv(sockclient,(char *)&SCmdBuf,sizeof(struct _tagSCmdBuf));
    int count=0;
    int ffflag=0;
    int fffflag=0;
    memset(fbuffer,'\0',sizeof(fbuffer));
    while(1)
    {
here:
        recvfrom(sockclient,(char *)&fbuffer[count],sizeof(_tagNodeInfo),0,(struct sockaddr *)&svraddr,&addrlen);
            printf("%d,%d\n",count,fbuffer[count].sec);
        if(fbuffer[count].sec==-1||ffflag==1)
        {
            ffflag=1;
            if(fbuffer[count].sec!=-1&&count!=10){
                fwrite(fbuffer[count].info,1,1024,fp);
                count++;
                icount++;
                //printf("%d\b",icount);
            }
            if(count==10)
            {
                count=0;
                SCmdBuf.sec=-1;
                sendtosrv(sockclient,(char *)&SCmdBuf,sizeof(struct _tagSCmdBuf));
                ffflag=0;
                memset(fbuffer,'\0',sizeof(fbuffer));
            }
            goto here;
        }
        else
        if(fbuffer[count].sec==-2||fffflag==1){
            fffflag=1;
            if(fbuffer[count].sec!=-2){
                fwrite(fbuffer[count].info,1,1024,fp);
                count++;
                icount++;
                //printf("%d\b",icount);
            }
            if(count==last){
                    SCmdBuf.sec=-2;
                    sendtosrv(sockclient,(char *)&SCmdBuf,sizeof(struct _tagSCmdBuf));
                    break;
                }
            goto here;
        }
        fwrite(fbuffer[count].info,1,1024,fp);
        count ++;
        icount++;
        //printf("%d\b",icount);
    }
    printf("%d==%d\n",icount,totalsec);
    fclose(fp);
    envclean();
    return 0;
}
int main()
{
    sentfile();
    system("pause");
    return 0;
}
