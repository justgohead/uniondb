#ifndef _LEELOONET_H_
#define _LEELOONET_H_

#include "leeloo.h"
//#include "db.h"

#ifdef LEELOO_WINDOWS_OS
#pragma comment(lib, "Ws2_32")
#else
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#endif

namespace LeelooNet
{

const std::string LEELOO_IP_KEY_CONFIG_NET = "/net/ip";
const std::string LEELOO_DEFAULT_IP_VAL_CONFIG_NET = "127.0.0.1";
const std::string LEELOO_TCPPORT_KEY_CONFIG_NET = "/net/tcpport";
const std::string LEELOO_DEFAULT_TCPPORT_VAL_CONFIG_NET = "8181";
const std::string LEELOO_THREADNUM_KEY_CONFIG_NET = "/net/threadnum";
const std::string LEELOO_DEFAULT_THREADNUM_VAL_CONFIG_NET = "64";

const Leeloo::Int LEELOO_BUFFER_SIZE_NET = 8192;//1400;
const Leeloo::Int LEELOO_MAX_SESSION_NET = 1024*20;

const Leeloo::Int LEELOO_READ_OP_NET   = 1;
const Leeloo::Int LEELOO_WRITE_OP_NET  = 2;
const Leeloo::Int LEELOO_ACCEPT_OP_NET = 3;

//const Leeloo::Uint LEELOO_TCP_ATTRIB_NET   = 1;
//const Leeloo::Uint LEELOO_UDP_ATTRIB_NET  = 2;
//const Leeloo::Uint LEELOO_SEND_ATTRIB_NET   = 4;
//const Leeloo::Uint LEELOO_RECV_ATTRIB_NET  = 8;

typedef struct _PER_HANDLE_DATA        // per-handle数据
{
    SOCKET m_s;                        // 对应的套接字句柄
    sockaddr_in m_addr;                // 客户方地址               //
} PER_HANDLE_DATA, *PPER_HANDLE_DATA;
typedef struct _PER_IO_DATA            // per-I/O数据
{
    OVERLAPPED m_ol;                // 重叠结构
    char m_buf[LEELOO_BUFFER_SIZE_NET+2];        // 数据缓冲区
    Leeloo::Int m_nOperationType;            // 操作类型
    //
    Leeloo::Int m_linkId;
    Leeloo::Int m_sessionId;
    Leeloo::Data<char> m_dataSendTemp;
} PER_IO_DATA, *PPER_IO_DATA;

typedef struct{
    PPER_HANDLE_DATA m_pPerHandle;
    PPER_IO_DATA m_pPerIo;
}Link;

class Session : public Leeloo::Base<>
{
public:
    enum LinkType{LINK_TCPRECVSEND=0,LINK_TCPSEND,LINK_TCPRECV,LINK_UDPSEND,LINK_TOTAL};
    Session()
    {
        for(Leeloo::Int i=0;i<LINK_TOTAL;i++)
        {
            m_link[i].m_pPerHandle = NULL;
            m_link[i].m_pPerIo = NULL;
        }
        m_bInit = false;
    }
    virtual ~Session()
    {
        for(Leeloo::Int i=0;i<LINK_TOTAL;i++)
        {
            if(m_link[i].m_pPerHandle != NULL)
                ::GlobalFree(m_link[i].m_pPerHandle);
            if(m_link[i].m_pPerIo != NULL)
                ::GlobalFree(m_link[i].m_pPerIo);
        }
    }
    Leeloo::Ret Init(LinkType linkType)
    {
        m_bInit = false;
        if(m_link[linkType].m_pPerHandle == NULL)
        {
            m_link[linkType].m_pPerHandle = (PPER_HANDLE_DATA)::GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
            if(m_link[linkType].m_pPerHandle == NULL)
                return Leeloo::NOMEMORY_ERRORCODE;
        }
        if(m_link[linkType].m_pPerIo == NULL)
        {
            m_link[linkType].m_pPerIo = (PPER_IO_DATA)::GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
            if(m_link[linkType].m_pPerIo == NULL)
            {
                ::GlobalFree(m_link[linkType].m_pPerHandle);
                m_link[linkType].m_pPerHandle = NULL;
                return Leeloo::NOMEMORY_ERRORCODE;
            }
        }
        m_bInit = true;
        return Leeloo::SUCCESS;
    }
    void UnInit(void)
    {
        m_bInit = false;
    }
    Leeloo::Queue<Leeloo::Data<>> m_queueRecv;
    Leeloo::Queue<Leeloo::Data<>> m_queueSend;
    Link m_link[LINK_TOTAL];
    volatile bool m_bInit;
};

class NetServer : public Leeloo::Base<>
{
public:
    NetServer(void);
    virtual ~NetServer(void);
    HANDLE GetCompletion(void);
    void FreeSession(Leeloo::IdType sessionId);
public:
    virtual Leeloo::Ret Handle(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans);
protected:
    virtual Leeloo::Ret Accept(HANDLE hCompletion, SOCKET sNew, SOCKADDR_IN saRemote, int nRemoteLen);
protected:
    void ThreadProg(void* pPara, bool& bStop);
private:
    NetServer(NetServer& ref){}
    NetServer& operator=(NetServer& ref){return *this;}
protected:
    Leeloo::Thread m_thread;
    //????? Vector<Session>
    Session m_session[LEELOO_MAX_SESSION_NET];
    Leeloo::IdManager m_idManager;
    HANDLE m_hCompletion;
};
typedef Leeloo::Singleton<NetServer> NetServerSingleton;

}

#endif