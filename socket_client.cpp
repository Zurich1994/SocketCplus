
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
//�������������Ϣ
static struct _tagCCmdBuf{

	char cmd[4]; //info �� over

	char fn[12]; // info ʱ�ļ���
	int  sec;//�ܹ����Ͱ���
	int  bagsize;//���Ĵ�С
}CCmdBuf;

//���ܷ�������Ϣ
static struct _tagSCmdBuf{
	int  sec; //get ʱȡ��secֵ�󣬷��Ͷ�Ӧ�İ�
}SCmdBuf;

static struct _tagFileInfo
{
	char filename[50]; //�ļ���
	int  bypes;//�ļ��ֽ���
	int  ibag;//����

}fileinfo;

static struct _tagNodeInfo{
	int sec; //����id
	char info[1024];//��������
}fbuffer[13];//һ�ζ�ȡ�ļ���С

void envinit()
{
	WSADATA wsa= {0}; //��ʼ��
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
    printf("clientsocket ��ʼ���\n");
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

//ȡ���ļ�����
static int GetFileLen(FILE *fp)
{
    printf("��Ҫ��ĳ��ȣ���\n");
	fseek(fp,0,SEEK_END);
	int len=ftell(fp);
	fseek(fp,0,SEEK_SET);
    printf("�Ҹ��㳤��\n");
	return len;
}

//ȡ���ļ��� SCmdBuf.se=-1;
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

//ȡ���ļ���Ϣ
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
	printf("��������\n");
	printf("�ڴ��С��B��\t\t%d\n",pfileinfo->bypes);
	printf("��������\t\t%d\n",pfileinfo->ibag);
	printf("�����ļ�������\t\t%s",pfileinfo->filename);
	printf("\n");

	printf("/*--------------------------�����޳ܵ��»���----------------------------*/\n");
    return fp;
}
//������������ļ�
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
    printf("�尴�������ʼ��");

	if(SCmdBuf.sec==-1)
	{
	    system("pause");
            printf("SCmdBuf.sec=%d\t���ǿ�ʼ���䣡\n",SCmdBuf.sec);
		int count = 0;
		while(1){
		//	printf("%d\b",icount);
			if(icount == fileinfo.ibag){
                fbuffer[11].sec=-2;
			    sendtosrv(sockclient,(char *)&fbuffer[11] ,sizeof(fbuffer[0]));
			    recvfrom(sockclient,(char *)&SCmdBuf,sizeof(SCmdBuf),0,(struct sockaddr *)&svraddr,&addrlen);
			    if(SCmdBuf.sec!=-2) printf("����ʲô����ֵ��\n");
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
    puts("������ IP �˿� �ļ���ַ���ÿո����O(��_��)Oлл");
    char add[16],ipid[50];
	int P;
    scanf("%s %d %s",ipid,&P,add);
    sentfile(ipid,P,add);
	system("pause");
    return 0;
}
