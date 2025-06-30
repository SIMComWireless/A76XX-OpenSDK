/**
  ******************************************************************************
  * @file    multi_pdp.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of multi pdp demo
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
#include "userspaceConfig.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "stdlib.h"
#include "string.h"
#include "simcom_debug.h"
#include "stdio.h"
#include "stdarg.h"
#ifdef HAS_UART
#include "simcom_uart.h"
#endif
#ifdef HAS_USB
#include "simcom_usb_vcom.h"
#endif
#include "simcom_network.h"
#include "simcom_simcard.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "multi_pdp.h"

#ifdef LOG_USE_ELOG
#include "elog.h"
#else
#include "bsp_uart.h"
#endif

#ifdef LOG_USE_ELOG
#define SYSINFO(fmt, ...) do { \
    log_i("[%s] %d <--> "fmt, __func__, __LINE__, ##__VA_ARGS__);\
} while (0);
#else
static char printf_buf[1024]={0};
#define MAX_PRINT_ONE_LEN 180
#define SYSINFO(fmt, ...) do { \
    system_printf("[%s,%d] "fmt, __func__, __LINE__, ##__VA_ARGS__);\
} while (0);
#endif

const char *iptype_to_string[] = {"IPV4V6","IPV4","IPV6","IP INVALID"};

typedef struct
{
    SCApnParm pdp_contents;
    struct sockaddr *v4;
    struct sockaddr *v6;
} SC_PdpConfig;

static SC_PdpConfig PdpContentGroup[] =
{
    {{SC_PDP1,    SC_PDPTYPE_IPV4V6 , "3gnet"},   NULL ,NULL},
    {{SC_PDP2,    SC_PDPTYPE_IPV4V6 , "APN2"},    NULL ,NULL},
    {{SC_PDP3,    SC_PDPTYPE_IPV4V6 , "APN3"},    NULL ,NULL},
    {{SC_PDP4,    SC_PDPTYPE_IPV4V6 , "APN4"},    NULL ,NULL},
};

sMsgQRef    g_mpdp_msgq = NULL;
sTaskRef    g_mpdp_task = 0;
void        *g_mpdp_thread_ptr = NULL;
sTimerRef   tcptimeRef = NULL;

tcp_global_handle tcp_client_handle;
udp_global_handle udp_client_handle;

int get_pdp_ip_contents(uint8_t cid, SC_PdpConfig *pdp_info);
int get_domain_ip_with_pdp(char *host, uint8_t cid);
int tcp_socket_connect(int *socketfd, int cid,char *host,unsigned short port);
int tcp_client_init(uint8_t cid);
int udp_client_init(uint8_t cid);
int udp_client_send(int fd,char *data,int len,int flag,const struct sockaddr *destcAddr);

#ifndef LOG_USE_ELOG
void system_printf(const char * format, ...)
{
    va_list ap;
    int i = 0;
    int len = 0;
    char tmp[200];
    va_start(ap, format);

    memset(printf_buf,0x00,sizeof(printf_buf));

    vsnprintf(printf_buf, sizeof(printf_buf), format, ap);

    len = strlen(printf_buf);

    do
    {
        if((len-i) > MAX_PRINT_ONE_LEN)
        {
            memset(tmp,0x00,sizeof(tmp));
            memcpy(tmp,printf_buf+i,MAX_PRINT_ONE_LEN);
            //PrintfResp(tmp);
            uart_write(uart_get_fd_by_filename("/dev/uart1"),tmp,strlen(tmp));
            i+=MAX_PRINT_ONE_LEN;
        }
        else
        {
            memset(tmp,0x00,sizeof(tmp));
            memcpy(tmp,printf_buf+i,len-i);
            strcat(tmp,"\r\n");
            //PrintfResp(tmp);
            uart_write(uart_get_fd_by_filename("/dev/uart1"),tmp,strlen(tmp));
            i = len;
        }
    }while(i<len);

    va_end(ap);
}
#endif

void printSockAddr(const struct sockaddr *addr)
{
    char ipstr[128]={0};

    if(addr == NULL){
        SYSINFO("input addr is NULL");
        return;
    }

    if(addr->sa_family == AF_INET)
    {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
        inet_ntop(AF_INET , &(addr_in->sin_addr) , ipstr , sizeof(ipstr));
        SYSINFO("IPV4 addr:%s port:%d" , ipstr , ntohs(addr_in->sin_port));
    }
    else if(addr->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in = (struct sockaddr_in6 *)addr;
        inet_ntop(AF_INET6 , &(addr_in->sin6_addr) , ipstr , sizeof(ipstr));
        SYSINFO("IPV6 addr:%s port:%d" , ipstr , ntohs(addr_in->sin6_port));
    }
    else
    {
        SYSINFO("Invalid sock addr");
    }
}

int get_PdpContentGroup_index(uint8_t cid)
{
    int i;

    if(cid < 1 || cid > sizeof(PdpContentGroup)/sizeof(SC_PdpConfig))
        return -1;

    for(i=0;i<sizeof(PdpContentGroup)/sizeof(SC_PdpConfig);i++)
    {
        if(cid == PdpContentGroup[i].pdp_contents.cid)
        {
            return i;
        }
    }

    return -1;
}

int get_pdp_ip_contents(uint8_t cid, SC_PdpConfig *pdp_info)
{
    struct sockaddr_in ipv4sa;
    struct sockaddr_in6 ipv6sa;
    int32_t ret = -1;
    struct SCipInfo *localip_info_p = (struct SCipInfo *)sAPI_Malloc(sizeof(struct SCipInfo));

    memset(localip_info_p , 0x00 , sizeof(struct SCipInfo));

    ret = sAPI_TcpipGetSocketPdpAddr(cid, 1, localip_info_p);

    if(ret == SC_TCPIP_SUCCESS)
    {
        SYSINFO("PDP%d %s" , cid , iptype_to_string[localip_info_p->type]);

        if(localip_info_p->type == TCPIP_PDP_IPV4)
        {
            memset(&ipv4sa, 0, sizeof(ipv4sa));
            ipv4sa.sin_family = AF_INET;
            memcpy(&ipv4sa.sin_addr, &localip_info_p->ip4, sizeof(ipv4sa.sin_addr));
            pdp_info->v4 = sAPI_Malloc(sizeof(struct sockaddr));
            memcpy(pdp_info->v4, &ipv4sa, sizeof(ipv4sa));
            printSockAddr(pdp_info->v4);

        }
        else if (localip_info_p->type == TCPIP_PDP_IPV6)
        {
            memset(&ipv6sa, 0, sizeof(ipv6sa));
            ipv6sa.sin6_family = AF_INET6;
            memcpy(&ipv6sa.sin6_addr, &localip_info_p->ip6, sizeof(ipv6sa.sin6_addr));
            pdp_info->v6 = sAPI_Malloc(sizeof(struct sockaddr));
            memcpy(pdp_info->v6, &ipv6sa, sizeof(ipv6sa));
            printSockAddr(pdp_info->v6);
        }
        else if(localip_info_p->type == TCPIP_PDP_IPV4V6)
        {
            //IPV4
            memset(&ipv4sa, 0, sizeof(ipv4sa));
            pdp_info->v4 = sAPI_Malloc(sizeof(struct sockaddr));
            ipv4sa.sin_family = AF_INET;
            memcpy(&ipv4sa.sin_addr, &localip_info_p->ip4, sizeof(ipv4sa.sin_addr));
            memcpy(pdp_info->v4, &ipv4sa, sizeof(ipv4sa));
            printSockAddr(pdp_info->v4);

            //IPV6
            memset(&ipv6sa, 0, sizeof(ipv6sa));
            pdp_info->v6 = sAPI_Malloc(sizeof(struct sockaddr));
            ipv6sa.sin6_family = AF_INET6;
            memcpy(&ipv6sa.sin6_addr, &localip_info_p->ip6, sizeof(ipv6sa.sin6_addr));
            memcpy(pdp_info->v6, &ipv6sa, sizeof(ipv6sa));
            printSockAddr(pdp_info->v6);
        }
    }

    sAPI_Free(localip_info_p);

    return ret;
}

int get_domain_ip_with_pdp(char *host, uint8_t cid)
{
    int ret = -1;

    if(host == NULL)
        return -1;
    
    if(cid < 1 || (cid ==8))
        return -1;

    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    struct sockaddr_in *addr;
    struct sockaddr_in6 *addrv6;

    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;
    char str[128];

    if (getaddrinfo_with_pcid(host, NULL, &hints, &addr_list,cid) != 0)
    {
        sAPI_Debug("getaddrinfo error");
        freeaddrinfo(addr_list);
        return -1;
    }
    SYSINFO("Parse domain \"%s\"" , host);

    for (rp = addr_list; rp != NULL; rp = rp->ai_next)
    {
        if(rp->ai_family == AF_INET)
        {
            addr = (struct sockaddr_in *)rp->ai_addr;
            memset(str, 0x00, sizeof(str));
            inet_ntop((*addr).sin_family, &(*addr).sin_addr, str, sizeof(str));
            SYSINFO("domain ipv4 address->%s" , str);
        }
        else if(rp->ai_family == AF_INET6)
        {
            addrv6 = (struct sockaddr_in6 *)rp->ai_addr;
            memset(str, 0x00, sizeof(str));
            inet_ntop((*addrv6).sin6_family, &(*addrv6).sin6_addr, str, sizeof(str));
            SYSINFO("domain ipv6 address->%s" , str);
        }
    }

    freeaddrinfo(addr_list);
    return ret;
}

void tcp_poll(tcp_global_handle *tcp_handle,udp_global_handle *udp_handle,struct timeval tv)
{
    int ret , socket_errno = 0;
    fd_set readfds;

    /*select init*/
    FD_ZERO(&readfds);
    FD_SET(tcp_handle->soc_fd, &readfds);
    FD_SET(udp_handle->soc_fd, &readfds); //add for UDP
    FD_SET(tcp_handle->soc_fd_v6, &readfds); //add for TCP V6

    ret = select(FD_SETSIZE,&readfds,NULL,NULL,&tv);
    if(ret < 0)
    {
        SYSINFO("sock select error %d",ret);
        return;
    }
    else if(ret > 0)
    {
        if(tcp_handle !=NULL)
        {
            if (FD_ISSET(tcp_handle->soc_fd, &readfds))
            {
                SYSINFO("tcp v4 recvfrom fd %d",tcp_handle->soc_fd);
                memset(&tcp_handle->recv_buff,0,sizeof(tcp_handle->recv_buff));
                tcp_handle->addrLen = sizeof(struct sockaddr);
                memset(&tcp_handle->addr, 0, sizeof(tcp_handle->addr));
                tcp_handle->recv_cnt = recvfrom(tcp_handle->soc_fd,tcp_handle->recv_buff,sizeof(tcp_handle->recv_buff),0,(struct sockaddr* )&tcp_handle->addr,&tcp_handle->addrLen);

                if(tcp_handle->recv_cnt > 0)
                {
                    char s[128];
                    uint16_t port = 0;
                    if(AF_INET == tcp_handle->addr.sa_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&tcp_handle->addr;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, sizeof(s));
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == tcp_handle->addr.sa_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&tcp_handle->addr;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, sizeof(s));
                        port = saddr->sin6_port;
                    }
                    SYSINFO("tcp v4 %zd bytes recv from [%s]:%hu data:%s",tcp_handle->recv_cnt,s,ntohs(port) , tcp_handle->recv_buff);

                    if(strncmp("This is V4 server",tcp_handle->recv_buff , strlen("This is V4 server")) == 0)
                    {
                        sendto(tcp_handle->soc_fd,"This is V4 client",strlen("This is V4 client"),0,(struct sockaddr* )&tcp_handle->addr,tcp_handle->addrLen);
                    }
                    else
                    {
                        ret = sendto(tcp_handle->soc_fd,tcp_handle->recv_buff,tcp_handle->recv_cnt,0,(struct sockaddr* )&tcp_handle->addr,tcp_handle->addrLen);
                        if(ret == tcp_handle->recv_cnt)
                            SYSINFO("tcp v4 sendback %d bytes", ret);
                    }
                }
                else
                {
                    socket_errno = lwip_getsockerrno(tcp_handle->soc_fd);
                    if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
                    {
                        SYSINFO("recv fail errno[%d]", socket_errno);
                        close(tcp_handle->soc_fd);
                        tcp_handle->soc_fd = -1;
                    }
                }
            }

            if (FD_ISSET(tcp_handle->soc_fd_v6, &readfds))
            {
                SYSINFO("tcp v6 recvfrom fd %d",tcp_handle->soc_fd_v6);
                memset(&tcp_handle->recv_buff_v6,0,sizeof(tcp_handle->recv_buff_v6));
                tcp_handle->addrLen_v6 = sizeof(struct sockaddr);
                memset(&tcp_handle->addr_v6, 0, sizeof(tcp_handle->addr_v6));
                tcp_handle->recv_cnt_v6 = recvfrom(tcp_handle->soc_fd_v6,tcp_handle->recv_buff_v6,sizeof(tcp_handle->recv_buff_v6),0,(struct sockaddr* )&tcp_handle->addr_v6,&tcp_handle->addrLen_v6);

                if(tcp_handle->recv_cnt_v6 > 0)
                {
                    char s[128];
                    uint16_t port = 0;
                    if(AF_INET == tcp_handle->addr_v6.sa_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&tcp_handle->addr_v6;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, sizeof(s));
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == tcp_handle->addr_v6.sa_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&tcp_handle->addr_v6;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, sizeof(s));
                        port = saddr->sin6_port;
                    }
                    SYSINFO("tcp v6 %zd bytes recv from [%s]:%hu data:%s",tcp_handle->recv_cnt_v6,s,ntohs(port) , tcp_handle->recv_buff_v6);

                    if(strncmp("This is V6 server",tcp_handle->recv_buff_v6 , strlen("This is V6 server")) == 0)
                        sendto(tcp_handle->soc_fd_v6,"This is V6 client",strlen("This is V6 client"),0,(struct sockaddr* )&tcp_handle->addr_v6,tcp_handle->addrLen_v6);

                    else
                    {
                        ret = sendto(tcp_handle->soc_fd_v6,tcp_handle->recv_buff_v6,tcp_handle->recv_cnt_v6,0,(struct sockaddr* )&tcp_handle->addr_v6,tcp_handle->addrLen_v6);
                        if(ret == tcp_handle->recv_cnt_v6)
                            SYSINFO("tcp v6 sendback %d bytes", ret);
                    }
                }
                else
                {
                    socket_errno = lwip_getsockerrno(tcp_handle->soc_fd_v6);
                    if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
                    {
                        SYSINFO("recv v6 fail errno[%d]", socket_errno);
                        close(tcp_handle->soc_fd_v6);
                        tcp_handle->soc_fd_v6 = -1;
                    }
                }
            }
        }

        if(udp_handle != NULL)
        {
            if(FD_ISSET(udp_handle->soc_fd, &readfds))
            {
                SYSINFO("udp recvfrom fd %d",udp_handle->soc_fd);
                memset(&udp_handle->recv_buff,0,sizeof(udp_handle->recv_buff));
                udp_handle->addrLen = sizeof(struct sockaddr);
                memset(&udp_handle->addr, 0, sizeof(udp_handle->addr));
                udp_handle->recv_cnt = recvfrom(udp_handle->soc_fd,udp_handle->recv_buff,sizeof(udp_handle->recv_buff),0,(struct sockaddr* )&udp_handle->addr,&udp_handle->addrLen);

                if(udp_handle->recv_cnt > 0)
                {
                    char s[128];
                    uint16_t port = 0;
                    if(AF_INET == udp_handle->addr.sa_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&udp_handle->addr;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, sizeof(s));
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == udp_handle->addr.sa_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&udp_handle->addr;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, sizeof(s));
                        port = saddr->sin6_port;
                    }
                    SYSINFO("udp %zd bytes recv from [%s]:%hu data:%s",udp_handle->recv_cnt,s,ntohs(port) , udp_handle->recv_buff);

                    ret = udp_client_send(udp_handle->soc_fd,udp_handle->recv_buff,udp_handle->recv_cnt,0,(struct sockaddr* )&udp_handle->addr);
                    if(ret == udp_handle->recv_cnt)
                        SYSINFO("udp sendback %d bytes", ret);
                }
                else
                {
                    socket_errno = lwip_getsockerrno(udp_handle->soc_fd);
                    if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
                    {
                        SYSINFO("recv fail errno[%d]", socket_errno);
                        close(udp_handle->soc_fd);
                        udp_handle->soc_fd = -1;
                    }
                }
            }
        }
    }
}

void sockrecv_handle_thread(void *param)
{
    while(1)
    {
        struct timeval tv;
        /*set the timeout*/
        tv.tv_sec  = 5;
        tv.tv_usec = 0;
        //for v4
        if(tcp_client_handle.soc_fd < 0)
        {
            if(tcp_socket_connect(&tcp_client_handle.soc_fd , TCP_SOCKET_CID , tcp_client_handle.server , tcp_client_handle.serverport) < 0)
            {
                SYSINFO("tcp_socket_connect v4 failed");
            }
        }
        //for v6
        if(tcp_client_handle.soc_fd_v6 < 0)
        {
            if(tcp_socket_connect(&tcp_client_handle.soc_fd_v6 , TCP_SOCKET_CID , tcp_client_handle.server_v6 , tcp_client_handle.serverport_v6) < 0)
            {
                SYSINFO("tcp_socket_connect v6 failed");
            }
        }

        if(tcp_client_handle.tcp_handle_lock != NULL)
        {
            //fix me
            sAPI_MutexLock(tcp_client_handle.tcp_handle_lock, SC_SUSPEND);
        }

        tcp_poll(&tcp_client_handle,&udp_client_handle,tv);

        if(tcp_client_handle.tcp_handle_lock != NULL)
        {
            //fix me
            sAPI_MutexUnLock(tcp_client_handle.tcp_handle_lock);
        }
    }

}

int tcp_socket_connect(int *socketfd, int cid,char *host,unsigned short port)
{
    int ret = -1;
    int fd = -1;
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    char portstr[8]={0};
    //for v4
    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;

    if (*socketfd > 0)
    {
        SYSINFO("tcp socket is busy sockfd");
        return -1;
    }

    snprintf(portstr, sizeof(portstr), "%d", port);

    if (getaddrinfo_with_pcid(host, portstr, &hints, &addr_list,cid) != 0)
    {
        SYSINFO("getaddrinfo error");
        freeaddrinfo(addr_list);
        return -1;
    }

    for (rp = addr_list; rp != NULL; rp = rp->ai_next)
    {
        if(rp->ai_family == AF_INET)
        {
            SYSINFO("tcp host ipv4");
            printSockAddr(rp->ai_addr);

            if ((fd = socket(rp->ai_family, rp->ai_socktype, 0)) < 0)
            {
                continue;
            }

            if((ret = bind(fd ,PdpContentGroup[get_PdpContentGroup_index(cid)].v4 , sizeof(struct sockaddr))) != 0)
            {
                SYSINFO("tcp bind v4 fail:%d" , ret);
                close(fd);
                *socketfd = -1;
                continue;
            }

            if ((ret = connect(fd, rp->ai_addr, rp->ai_addrlen)) == 0)
            {
                *socketfd = fd;
                SYSINFO("tcp connect v4 server sucess");
                sendto(fd,"This is V4 client",strlen("This is V4 client"),0,rp->ai_addr,rp->ai_addrlen);
                break;
            }
            else
            {
                SYSINFO("tcp connect v4 server failed %d" , ret);
                close(fd);
                 *socketfd = -1;
                continue;
            }
            
        }
        else if(rp->ai_family == AF_INET6)
        {
            SYSINFO("tcp host ipv6");
            printSockAddr(rp->ai_addr);

            if ((fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
            {
                continue;
            }
            if((ret = bind(fd ,PdpContentGroup[get_PdpContentGroup_index(cid)].v6 , sizeof(struct sockaddr))) != 0)
            {
                SYSINFO("tcp bind v6 fail:%d" , ret);
                close(fd);
                *socketfd = -1;
                continue;
            }
            if ((ret = connect(fd, rp->ai_addr, rp->ai_addrlen)) == 0)
            {
                *socketfd = fd;
                SYSINFO("tcp connect v6 server sucess");
                sendto(fd,"This is V6 client",strlen("This is V6 client"),0,rp->ai_addr,rp->ai_addrlen);
                break;
            }
            else
            {
                SYSINFO("tcp connect v6 server failed %d" , ret);
                close(fd);
                continue;
            }
            
        }
    }

    freeaddrinfo(addr_list);
    return ret;

}

int tcp_client_init(uint8_t cid)
{
    int status;
    //for tcp
    memset(&tcp_client_handle,0,sizeof(tcp_global_handle));

    memcpy(tcp_client_handle.server,TCP_DEFAULT_SERVER,sizeof(TCP_DEFAULT_SERVER));
    tcp_client_handle.serverport = TCP_DEFAULT_SERVER_PORT;

    memcpy(tcp_client_handle.server_v6,TCP_DEFAULT_V6SERVER,sizeof(TCP_DEFAULT_V6SERVER));
    tcp_client_handle.serverport_v6 = TCP_DEFAULT_V6SERVER_PORT;

    if(tcptimeRef == NULL)
        sAPI_TimerCreate(&tcptimeRef);

    status = sAPI_MutexCreate(&tcp_client_handle.tcp_handle_lock, SC_FIFO);
    if(status != SC_SUCCESS)
    {
        SYSINFO("create tcp mutex failed");
        return -1;
    }

    if(tcp_socket_connect(&tcp_client_handle.soc_fd , cid , tcp_client_handle.server , tcp_client_handle.serverport) < 0)
    {
        SYSINFO("tcp_socket_connect v4 failed");
    }

    if(tcp_socket_connect(&tcp_client_handle.soc_fd_v6 , cid , tcp_client_handle.server_v6 , tcp_client_handle.serverport_v6) < 0)
    {
        SYSINFO("tcp_socket_connect v6 failed");
    }

    tcp_client_handle.tcp_client_thread_ptr = sAPI_Malloc(1024*8);
    if(tcp_client_handle.tcp_client_thread_ptr == NULL)
    {
        SYSINFO("malloc stack for tcp thread failed");
        return -1;
    }

    status = sAPI_TaskCreate(&tcp_client_handle.tcp_client_task,
                                tcp_client_handle.tcp_client_thread_ptr,
                                1024*8,
                                150, (char *)"sockrecv_handle_thread",
                                sockrecv_handle_thread,
                                NULL);
    if(status != SC_SUCCESS)
    {
        SYSINFO("create tcp thread fail");
        sAPI_Free(tcp_client_handle.tcp_client_thread_ptr);
        memset(&tcp_client_handle,0,sizeof(tcp_global_handle));
        return -1;
    }

    return 0;
}

int udp_client_init(uint8_t cid)
{
    int ret = -1;
    int fd;
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    char portstr[8] = {0};
     //for udp
    memset(&udp_client_handle,0,sizeof(udp_global_handle));
    memcpy(udp_client_handle.server,UDP_DEFAULT_SERVER,sizeof(UDP_DEFAULT_SERVER));
    udp_client_handle.serverport = UDP_DEFAULT_SERVER_PORT;

    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family   = AF_INET/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 1;/*AI_PASSIVE;*/

    if (udp_client_handle.soc_fd > 0)
    {
        SYSINFO("udp socket is busy sockfd");
        freeaddrinfo(addr_list);
        return -1;
    }

    snprintf(portstr, sizeof(portstr), "%d", (int)udp_client_handle.serverport);

    if (getaddrinfo_with_pcid(udp_client_handle.server, portstr, &hints, &addr_list,cid) != 0)
    {
        SYSINFO("getaddrinfo error");
        freeaddrinfo(addr_list);
        return -1;
    }

    for (rp = addr_list; rp != NULL; rp = rp->ai_next)
    {
        if(rp->ai_family == AF_INET)
        {
            SYSINFO("udp host ipv4");
            printSockAddr(rp->ai_addr);

            if ((fd = socket(rp->ai_family, rp->ai_socktype, 0)) < 0)
            {
                continue;
            }

            udp_client_handle.soc_fd = fd;
            printSockAddr(PdpContentGroup[get_PdpContentGroup_index(cid)].v4);
            if((ret = bind(fd , PdpContentGroup[get_PdpContentGroup_index(cid)].v4 , sizeof(struct sockaddr))) != 0)
            {
                SYSINFO("udp bind fail:%d" , ret);
                close(fd);
                udp_client_handle.soc_fd = -1;
                break;
            }

            memcpy(&udp_client_handle.addr , rp->ai_addr , rp->ai_addrlen);
            ret = udp_client_send(fd,"This is UDP client",strlen("This is UDP client"),0,&udp_client_handle.addr);
            if(ret == 5)
            {
                SYSINFO("udp_client_send ret %d" , ret);
                break;
            }
        }
        else if(rp->ai_family == AF_INET6)
        {
            SYSINFO("udp host ipv6");
            printSockAddr(rp->ai_addr);
        }
    }

    freeaddrinfo(addr_list);
    return ret;
}

int udp_client_send(int fd,char *data,int len,int flag,const struct sockaddr *destcAddr)
{
    int bytes = 0;
    int index = 0;

    if(fd < 0)
        return -1;

    while (len)
    {
        if (destcAddr == NULL)
        {
            bytes = send(fd, data + index, len, flag);
        }
        else
        {
            bytes = sendto(fd, data + index, len, flag, destcAddr, sizeof(struct sockaddr));
        }
        if (bytes < 0)
        {
            return -1;
        }
        else
        {
            len = len - bytes;
            index = index + bytes;
        }
    }

    return index;
}

void socket_timer_callback(UINT32 argv)
{
    SIM_MSG_T send_data = {SIM_MSG_INIT, 0, -1, NULL};
    send_data.msg_id = SRV_SELF;
    sAPI_MsgQSend(g_mpdp_msgq,&send_data);
}

void multipdp_thread(void *ptr)
{
    uint8_t i,j;
    int32_t ret;
    char pdp_iptype[8];
    SCcpsiParm cpsi = { 0 };
    int state_cgreg;
    SCAPNact SCact[8] = {0};
    uint8_t cgact_all_state = 1;
    uint8_t NumOfPDP = 0;

    /*Create flag for network,the initialization operation must be used*/
    sAPI_NetworkInit();

    NumOfPDP = sizeof(PdpContentGroup)/sizeof(SC_PdpConfig);

    for(i = 0;i<NumOfPDP;i++)
    {
        memset(&pdp_iptype, 0x00, sizeof(pdp_iptype));

        if(PdpContentGroup[i].pdp_contents.iptype == SC_PDPTYPE_IP)
            memcpy(pdp_iptype, "IP", strlen("IP"));
        else if(PdpContentGroup[i].pdp_contents.iptype == SC_PDPTYPE_IPV6)
            memcpy(pdp_iptype, "IPV6", strlen("IPV6"));
        else if(PdpContentGroup[i].pdp_contents.iptype == SC_PDPTYPE_IPV4V6)
            memcpy(pdp_iptype, "IPV4V6", strlen("IPV4V6"));

        ret = sAPI_NetworkSetCgdcont(PdpContentGroup[i].pdp_contents.cid, pdp_iptype, PdpContentGroup[i].pdp_contents.ApnStr);
        if (ret == SC_NET_SUCCESS)
        {
            sAPI_Debug("Set cgdcont[&d] success.!", i);
        }
    }

     do {
        sAPI_TaskSleep(200);
        sAPI_NetworkGetCpsi(&cpsi);
        sAPI_NetworkGetCgreg(&state_cgreg);
        sAPI_Debug("NetworkMode->%s,cgreg->%d", cpsi.networkmode , state_cgreg);
        SYSINFO("NetworkMode->%s,cgreg->%d", cpsi.networkmode , state_cgreg);
    } while (strcmp(cpsi.networkmode, "LTE,Online"));

    while(1)
    {
        cgact_all_state = 1;
        ret = sAPI_NetworkGetCgact(SCact);
        if (ret == SC_NET_SUCCESS)
        {
            for(i = 0;i<NumOfPDP;i++)
            {
                for(j = 0;j<sizeof(SCact)/sizeof(SCAPNact);j++)
                {
                    if(SCact[j].cid == PdpContentGroup[i].pdp_contents.cid)
                    {
                        cgact_all_state &= SCact[j].isActived;
                        if(SCact[j].isActived == 0)
                        {
                            sAPI_NetworkSetCgact(1 , PdpContentGroup[i].pdp_contents.cid);
                            sAPI_TaskSleep(100);
                        }
                            
                    }
                }
            }
        }

        if(cgact_all_state == 1)
            break;
        sAPI_TaskSleep(100);
    }

    char *DomainName[]={"www.baidu.com", "112.74.93.163", "www.jd.com" ,"www.tmall.com"};

    for(i=0;i<NumOfPDP;i++)
    {
        get_pdp_ip_contents(PdpContentGroup[i].pdp_contents.cid , &PdpContentGroup[i]);
        get_domain_ip_with_pdp(DomainName[i] , PdpContentGroup[i].pdp_contents.cid);
    }

    tcp_client_init(TCP_SOCKET_CID);
    udp_client_init(UDP_SOCKET_CID);

    if(tcptimeRef != NULL)
        sAPI_TimerStart(tcptimeRef, 200*5, 200*5, socket_timer_callback, 0);

    while(1)
    {
        SIM_MSG_T recv_evt;
        if((ret = sAPI_MsgQRecv(g_mpdp_msgq,&recv_evt,SC_SUSPEND)) == SC_SUCCESS)
        {
            trace_taskinfo(tcp_client_handle.tcp_client_task);
            trace_taskinfo(g_mpdp_task);
        }
    }
}

void multipdp_task_create(void)
{
    SC_STATUS status  = 0;

    SYSINFO("MpdpInit");
    status = sAPI_MsgQCreate(&g_mpdp_msgq, "mpdp_msgq",  sizeof(SIM_MSG_T),10, SC_FIFO);

    g_mpdp_thread_ptr = sAPI_Malloc(1024 * 4);
    status = sAPI_TaskCreate(&g_mpdp_task,  g_mpdp_thread_ptr, 1024 * 4, 160, (char *)"mpdp_client", multipdp_thread,NULL);
    SYSINFO("multipdp_task_create status = %d",status);
}

void trace_taskinfo(sTaskRef *tasklist)
{
    unsigned long stackSize, stackInuse, stackPeak;
    char *taskName;
    sAPI_GetTaskStackInfo(tasklist, &taskName, &stackSize, &stackInuse, &stackPeak);
    SYSINFO("%s_stack(S:%ld U:%ld P:%ld)", taskName, stackSize, stackInuse, stackPeak);
}