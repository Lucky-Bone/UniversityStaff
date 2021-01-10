#include <stdio.h>  
#include <winsock.h>  
#pragma comment(lib,"Ws2_32.lib")  
  
USHORT checksum(USHORT* buff, int size)  
{  
    unsigned long cksum = 0;  
    while(size>1)  
    {  
        cksum += *buff++;  
        size -= sizeof(USHORT);  
    }  
    // ÊÇÆæÊý  
    if(size)  
    {  
        cksum += *(UCHAR*)buff;  
    }  
    // ½«32Î»µÄchsum¸ß16Î»ºÍµÍ16Î»Ïà¼Ó£¬È»ºóÈ¡·´  
    cksum = (cksum >> 16) + (cksum & 0xffff);  
    cksum += (cksum >> 16);    // ???   
    return (USHORT)(~cksum);  
}  
  
  
  
typedef struct icmp_hdr  
{  
    unsigned char   icmp_type;   // ÏûÏ¢ÀàÐÍ  
    unsigned char   icmp_code;   // ´úÂë  
    unsigned short icmp_checksum; // Ð£ÑéºÍ  
// ÏÂÃæÊÇ»ØÏÔÍ·  
    unsigned short icmp_id;   // ÓÃÀ´Î©Ò»±êÊ¶´ËÇëÇóµÄIDºÅ£¬Í¨³£ÉèÖÃÎª½ø³ÌID  
    unsigned short icmp_sequence; // ÐòÁÐºÅ  
    unsigned long   icmp_timestamp; // Ê±¼ä´Á  
} ICMP_HDR, *PICMP_HDR;  
  
  
  
int SetTimeout(SOCKET s, int nTime, BOOL bRecv)  
{  
int ret = ::setsockopt(s, SOL_SOCKET,   
   bRecv ? SO_RCVTIMEO : SO_SNDTIMEO, (char*)&nTime, sizeof(nTime));  
return ret != SOCKET_ERROR;  
}  
  
int Computer(char szDestIP[30])                   //É¨ÃèÖ÷»úÊÇ·ñ´æ»î  
{  
    WSADATA wsaData;  
    WORD wVersionRequested=MAKEWORD(1,1);  
    if (WSAStartup(wVersionRequested , &wsaData))  
    {  
        printf("Winsock Initialization failed.\n");  
        exit(1);  
    }  
    SOCKET sRaw=::socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);  
    SetTimeout(sRaw,1000,TRUE);  
    SOCKADDR_IN dest;  
    dest.sin_family=AF_INET;  
    dest.sin_port=htons(0);  
    dest.sin_addr.S_un.S_addr=inet_addr(szDestIP);  
  
    char buff[sizeof(ICMP_HDR)+32];  
    ICMP_HDR * pIcmp=(ICMP_HDR *)buff;  
  
    pIcmp->icmp_type=8;  
    pIcmp->icmp_code=0;  
    pIcmp->icmp_id=(USHORT)::GetCurrentProcessId();  
    pIcmp->icmp_checksum=0;  
    pIcmp->icmp_sequence=0;  
  
    memset(&buff[sizeof(ICMP_HDR)],'E',32);  
  
    USHORT nSeq=0;  
    char revBuf[1024];  
    SOCKADDR_IN from;  
    int nLen=sizeof(from);  
        static int nCount=0;  
        int nRet;  
/*      if (nCount++==4) 
        { 
            break; 
        }*/  
        pIcmp->icmp_checksum=0;  
        pIcmp->icmp_timestamp=::GetTickCount();  
        pIcmp->icmp_sequence=nSeq++;  
        pIcmp->icmp_checksum=checksum((USHORT *)buff,sizeof(ICMP_HDR)+32);  
        nRet=::sendto(sRaw,buff,sizeof(ICMP_HDR)+32,0,(SOCKADDR *)&dest,sizeof(dest));  
        if (nRet==SOCKET_ERROR)  
        {  
            printf("sendto() failed:%d\n",::WSAGetLastError());  
            return -1;  
        }  
        nRet=::recvfrom(sRaw,revBuf,1024,0,(sockaddr *)&from,&nLen);  
        if (nRet==SOCKET_ERROR)  
        {  
            printf("%s Ö÷»úÃ»ÓÐ´æ»î£¡\n",szDestIP);  
            return -1;  
        }  
        printf("%s Ö÷»ú´æ»î£¡\n",szDestIP);  
        closesocket(nRet);  
    WSACleanup();  
    return 0;  
}  
  
void Port(char adr[20])               //É¨Ãè´æ»îÖ÷»ú¶Ë¿Ú  
{  
    int mysocket,m,n;  
    int pcount = 0;   
    struct sockaddr_in my_addr;  
    WSADATA wsaData;  
    WORD wVersionRequested=MAKEWORD(1,1);  
    printf("ÇëÊäÈëÒªÉ¨ÃèµÄ¶Ë¿Ú·¶Î§£¨ÀýÈç1-1024£©£º");  
    scanf("%d-%d",&m,&n);  
    if (WSAStartup(wVersionRequested , &wsaData))  
    {  
        printf("Winsock Initialization failed.\n");  
        exit(1);  
    }  
    for(int i=m; i<n; i++)  
    {  
        if((mysocket = socket(AF_INET, SOCK_STREAM,0)) == INVALID_SOCKET)  
            exit(1);  
        my_addr.sin_family = AF_INET;  
        my_addr.sin_port = htons(i);  
        my_addr.sin_addr.s_addr = inet_addr(adr);  
        if(connect(mysocket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)  
        {  
            printf("Port %d - ¹Ø±Õ\n", i);  
            closesocket(mysocket);  
        }  
        else  
        {  
            pcount++;  
            printf("Port %d - ´ò¿ª\n", i);  
        }  
    }  
    printf("%d ports open on host - %s\n", pcount, adr);  
    closesocket(mysocket);  
    WSACleanup();  
}  
  
  
void change(int a,int b,int c,int d,char IP[20])          //IP×ª»»  
{  
    char IPPort[4][4]={'\0'};  
    char temp[2]={'.','\0'};  
    itoa(a,IPPort[0],10);   
    itoa(b,IPPort[1],10);   
    itoa(c,IPPort[2],10);   
    itoa(d,IPPort[3],10);   
    strcat(IP,IPPort[0]);  
    strcat(IP,temp);  
    strcat(IP,IPPort[1]);  
    strcat(IP,temp);  
    strcat(IP,IPPort[2]);  
    strcat(IP,temp);  
    strcat(IP,IPPort[3]);  
}  
  
void main()  
{  
    int a[4],b[4];  
loop1:  
    printf("ÇëÊäÈëÆðÊ¼IP£º");  
    scanf("%d.%d.%d.%d",&a[0],&a[1],&a[2],&a[3]);  
    if (a[0]>255||a[1]>255||a[2]>255||a[3]>255)  
    {  
        printf("ÊäÈëµÄÆðÊ¼µØÖ·ÓÐÎó£¡ÇëÖØÐÂÊäÈë£¡\n");  
        goto loop1;  
    }  
loop2:  
    printf("ÇëÊäÈë½áÊøIP£º");  
    scanf("%d.%d.%d.%d",&b[0],&b[1],&b[2],&b[3]);  
    if (b[0]>255||b[1]>255||b[2]>255||b[3]>255)  
    {  
        printf("ÊäÈëµÄ½áÊøÓÐÎó£¡ÇëÖØÐÂÊäÈë£¡\n");  
        goto loop2;  
    }  
    while(!(a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]&&a[3]==(b[3]+1)))  
    {  
        char IP[20]={'\0'};  
        change(a[0],a[1],a[2],a[3],IP);  
        if((Computer(IP))==0)  
        {  
            Port(IP);  
        }  
        a[3]++;  
        if (a[3]>=255)  
        {  
            a[3]=0;  
            a[2]++;  
        }  
        if (a[2]>=255)  
        {  
            a[2]=0;  
            a[1]++;  
        }  
        if (a[1]>=255)  
        {  
            a[1]=0;  
            a[0]++;  
        }  
        if (a[0]>=255)  
        {  
            printf("µØÖ·Òç³ö£¡\n");  
            break;  
        }  
    }  
  
      
}  
