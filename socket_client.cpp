
#include <string.h>
#include "stdafx.h"
#include "windows.h"
#include "winsock2.h"
#include<stdio.h>
#pragma comment(lib,"ws2_32.lib")\
struct sockaddr_in svraddr= {0};
int addrlen = sizeof(svraddr);
int icount = 0;
bool flag[10];
//向服务器发送信息
static struct _tagCCmdBuf{

	char cmd[4]; //info 或 over

	char fn[12]; // info 时文件名
	int  sec;//总共发送包数
	int  bagsize;//包的大小
}CCmdBuf;

//接受服务器信息
static struct _tagSCmdBuf{
	int  sec; //get 时取得sec值后，发送对应的包
}SCmdBuf;

static struct _tagFileInfo
{
	char filename[50]; //文件名
	int  bypes;//文件字节数
	int  ibag;//包数

}fileinfo;

static struct _tagNodeInfo{
	int sec; //包的id
	char info[1024];//包的内容
}fbuffer[13];//一次读取文件大小

void envinit()
{
	WSADATA wsa= {0}; //初始化
    WSAStartup(MAKEWORD(2,2),&wsa);
}
void envclean()
{
	WSACleanup ();
}
SOCKET initSocketClient(const char * ipaddr,short iport)
{
	SOCKET sockclient = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(INVALID_SOCKET==sockclient)
        return sockclient;
    else
	{

		svraddr.sin_family = AF_INET;
		svraddr.sin_port=htons(iport);
		svraddr.sin_addr.S_un.S_addr=inet_addr(ipaddr);
	}
    printf("clientsocket 初始完成\n");
	return sockclient;

}
static int sendtosrv(SOCKET sockclient,char *buffer ,int size)
{
	int flag=sendto(sockclient,buffer,size,0,(struct sockaddr *)&svraddr,sizeof(svraddr));
	if(SOCKET_ERROR==flag)
		printf("sent error is happed\n");
	return flag;
}
static void closeclisocket(SOCKET sockclient)
{
	closesocket(sockclient);
}

static  int sentcmd(SOCKET sockclient,char *buffer ,int size)
{
	sendtosrv(sockclient,buffer,size);
	return 0;
}

//取得文件长度
static int GetFileLen(FILE *fp)
{
    printf("我要你的长度！、\n");
	fseek(fp,0,SEEK_END);
	int len=ftell(fp);
	fseek(fp,0,SEEK_SET);
    printf("我给你长度\n");
	return len;
}

//取得文件名 SCmdBuf.se=-1;
static void GetFileName(char * sn, char * fn)
{
	int i,j;
	int n=strlen(sn);
	for(i=0;i<n;i++)
		if(sn[i]=='/')
		{
			i=i+1;
			break;
		}
	for(j=0;i<n;i++,j++)
		fn[j]=sn[i];
	fn[j]='\0';
}

//取得文件信息
static FILE * getfileinfo(const char * filepath,struct _tagFileInfo * pfileinfo)
{
	FILE *fp = NULL;
	if(NULL != pfileinfo)
		memset(pfileinfo,0,sizeof(struct _tagFileInfo));
	fp = fopen(filepath,"rb");
	if(NULL != fp )
	{
		GetFileName((char *)filepath,pfileinfo->filename);
		pfileinfo->bypes = GetFileLen(fp);
		printf("%d\n",pfileinfo->bypes);
		if(pfileinfo->bypes%1024)
			pfileinfo->ibag = pfileinfo->bypes/1024 + 1;
		else
			pfileinfo->ibag = pfileinfo->bypes/1024;
	}
	printf("传输内容\n");
	printf("内存大小（B）\t\t%d\n",pfileinfo->bypes);
	printf("包的数量\t\t%d\n",pfileinfo->ibag);
	printf("传输文件的名字\t\t%s",pfileinfo->filename);
	printf("\n");

	printf("/*--------------------------我是无耻的下划线----------------------------*/\n");
    return fp;
}
//向服务器发送文件
int sentfile(const char * ipaddr,const short iport,const char * filepath)
{

	FILE * fp;
	envinit();
	fp = getfileinfo(filepath , &fileinfo);
	SOCKET sockclient =  initSocketClient(ipaddr,iport);
	strcpy(CCmdBuf.cmd,"inf");
	strcpy(CCmdBuf.fn,fileinfo.filename);
	CCmdBuf.bagsize = sizeof(struct _tagNodeInfo);
	CCmdBuf.sec = fileinfo.ibag;
    sentcmd(sockclient,(char *)&CCmdBuf,sizeof(struct _tagCCmdBuf));
	recvfrom(sockclient,(char *)&SCmdBuf,sizeof(SCmdBuf),0,(struct sockaddr *)&svraddr,&addrlen);
    memset(fbuffer,'\0',sizeof(fbuffer));
    printf("清按任意键开始！");

	if(SCmdBuf.sec==-1)
	{
	    system("pause");
            printf("SCmdBuf.sec=%d\t我们开始传输！\n",SCmdBuf.sec);
		int count = 0;
		while(1){
		//	printf("%d\b",icount);
			if(icount == fileinfo.ibag){
                fbuffer[11].sec=-2;
			    sendtosrv(sockclient,(char *)&fbuffer[11] ,sizeof(fbuffer[0]));
			    recvfrom(sockclient,(char *)&SCmdBuf,sizeof(SCmdBuf),0,(struct sockaddr *)&svraddr,&addrlen);
			    if(SCmdBuf.sec!=-2) printf("这算什么返回值？\n");
                break;
			}
			fread(fbuffer[count].info,1,1024,fp);
            sendtosrv(sockclient,(char *)&fbuffer[count] ,sizeof(fbuffer[0]));
			count ++;icount ++;
			printf("%d\n",count);
			if(count == 10)
			{
			    fbuffer[11].sec=-1;
			    sendtosrv(sockclient,(char *)&fbuffer[11] ,sizeof(fbuffer[0]));
			    printf("10ge\n");
			    recvfrom(sockclient,(char *)&SCmdBuf,sizeof(SCmdBuf),0,(struct sockaddr *)&svraddr,&addrlen);
			    if( SCmdBuf.sec!=-1){
                    printf("1 something wrong!\n");
                    system("pause");
                }
			    printf("over\n\n");
                memset(fbuffer,'\0',sizeof(fbuffer));
                count =0;
			}
		}
		//printf("%d\n",icount);
	}
	else{
		printf("the server is busy!\n");
		return -1;
	}
	fclose(fp);
	envclean();
	return 0;
}
int main()
{
    puts("请输入 IP 端口 文件地址！用空格隔开O(∩_∩)O谢谢");
    char add[16],ipid[50];
	int P;
    scanf("%s %d %s",ipid,&P,add);
    sentfile(ipid,P,add);
	system("pause");
    return 0;
}
