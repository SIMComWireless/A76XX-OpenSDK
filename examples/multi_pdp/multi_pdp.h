#ifndef __MULTI_PDP_H__
#define __MULTI_PDP_H__

#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"

#define TCP_DEFAULT_SERVER        "117.131.85.140"
#define TCP_DEFAULT_SERVER_PORT   60061

#define TCP_DEFAULT_V6SERVER      "2408:4006:1220:6d00:5826:5b8e:1a42:48e7"
#define TCP_DEFAULT_V6SERVER_PORT  9902

#define UDP_DEFAULT_SERVER        "117.131.85.140"
#define UDP_DEFAULT_SERVER_PORT   60058

#define TCP_SOCKET_CID            3
#define UDP_SOCKET_CID            4

#define LOG_USE_ELOG

enum SCpdpindex
{
    SC_PDP1 = 1,
    SC_PDP2,
    SC_PDP3,
    SC_PDP4,
    SC_PDP_MAX
};
enum SCpdpiptype
{
    SC_PDPTYPE_IP,
    SC_PDPTYPE_IPV6,
    SC_PDPTYPE_IPV4V6,
};

typedef struct{
    /*the thread*/
    sTaskRef             tcp_client_task;
    void                 *tcp_client_thread_ptr;
    sMsgQRef             tcp_client_msgq;
    sMutexRef            tcp_handle_lock;

    /*for v4*/
    char                 server[128];
    uint32_t             serverport;
    char                 localport[32];
    int soc_fd;
    struct sockaddr addr;
    unsigned int addrLen;
    char                 recv_buff[1024*2];
    uint32_t             recv_cnt;

    //for v6
    char                 server_v6[128];
    uint32_t             serverport_v6;
    char                 localport_v6[32];
    int soc_fd_v6;
    struct sockaddr addr_v6;
    unsigned int addrLen_v6;
    char                 recv_buff_v6[1024*2];
    uint32_t             recv_cnt_v6;
}tcp_global_handle;

typedef struct{
    /*common config*/
    char                 server[128];
    uint32_t             serverport;
    char                 localport[32];

    int soc_fd;

    struct sockaddr addr;
    unsigned int addrLen;

    char                 recv_buff[1024*2];
    uint32_t             recv_cnt;
}udp_global_handle;

void system_printf(const char * format, ...);
void multipdp_task_create(void);
int tcp_client_init(uint8_t cid);
void trace_taskinfo(sTaskRef *tasklist);

#endif