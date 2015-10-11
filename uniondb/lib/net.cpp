#include "net.h"

namespace LeelooNet
{

NetServer::NetServer(void)
{
    Loki::Functor<void, TYPELIST_2(void*,bool&)> functor(this, &NetServer::ThreadProg);
    Loki::Functor< void, TYPELIST_1(bool&)> bindFunctor(BindFirst(functor, (void*)NULL));
    m_thread.SetTask(bindFunctor);
}
NetServer::~NetServer(void)
{
    std::string strIp = LEELOO_DEFAULT_IP_VAL_CONFIG_NET;
    Leeloo::ConfigSingleton::Instance().GetConfig(LEELOO_IP_KEY_CONFIG_NET, strIp);
    std::string strPort = LEELOO_DEFAULT_TCPPORT_VAL_CONFIG_NET;
    Leeloo::ConfigSingleton::Instance().GetConfig(LEELOO_TCPPORT_KEY_CONFIG_NET, strPort);
    unsigned short nPort = atoi(strPort.c_str());
    SOCKET sockClient=socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr(strIp.c_str());
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(nPort);
    connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
    closesocket(sockClient);
    //
    m_thread.StopTask();
}
HANDLE NetServer::GetCompletion(void)
{
    return m_hCompletion;
}
void NetServer::FreeSession(Leeloo::IdType sessionId)
{
    m_idManager.FreeFreeId(sessionId);
    m_session[sessionId].UnInit();
}
DWORD WINAPI ServerThread(LPVOID lpParam)
{       
    // 得到完成端口对象句柄
    HANDLE hCompletion = ((NetServer*)lpParam)->GetCompletion();
    DWORD dwTrans;
    PPER_HANDLE_DATA pPerHandle;
    PPER_IO_DATA pPerIO;
    //
    while(TRUE)
    {       
        
        // 在关联到此完成端口的所有套接字上等待I/O完成
#ifdef LEELOO_64_PLATFORM
        BOOL bOK = ::GetQueuedCompletionStatus(hCompletion, 
            &dwTrans, (PULONG_PTR)&pPerHandle, (LPOVERLAPPED*)&pPerIO, WSA_INFINITE);
#else
        BOOL bOK = ::GetQueuedCompletionStatus(hCompletion, 
            &dwTrans, (LPDWORD)&pPerHandle, (LPOVERLAPPED*)&pPerIO, WSA_INFINITE);
#endif

        if(!bOK)    // 在此套接字上有错误发生
        {
            ::closesocket(pPerHandle->m_s);
            ((NetServer*)lpParam)->FreeSession(pPerIO->m_sessionId);
            continue;
        }
        
        if(dwTrans == 0 &&                                // 套接字被对方关闭
            (pPerIO->m_nOperationType == LEELOO_READ_OP_NET || pPerIO->m_nOperationType == LEELOO_WRITE_OP_NET))     
        {
            ::closesocket(pPerHandle->m_s);
            ((NetServer*)lpParam)->FreeSession(pPerIO->m_sessionId);
            continue;
        }
        ((NetServer*)lpParam)->Handle(pPerHandle, pPerIO, dwTrans);
    }
    return 0;
}
Leeloo::Ret NetServer::Handle(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans)
{
    switch(pPerIO->m_nOperationType)  // 通过per-I/O数据中的nOperationType域查看什么I/O请求完成了
    {
    case LEELOO_READ_OP_NET: // 完成一个接收请求
        {
            WSABUF buf;
            buf.buf = pPerIO->m_buf;
            buf.len = dwTrans;
            pPerIO->m_nOperationType = LEELOO_WRITE_OP_NET;
            DWORD nFlags = 0;
            //????? judge WSASend's ret
            ::WSASend(pPerHandle->m_s, &buf, 1, &dwTrans, 0, &pPerIO->m_ol, NULL);
        }
        break;
    case LEELOO_WRITE_OP_NET:
        {
            // 继续投递接收I/O请求
            WSABUF buf;
            buf.buf = pPerIO->m_buf;
            buf.len = LEELOO_BUFFER_SIZE_NET;
            pPerIO->m_nOperationType = LEELOO_READ_OP_NET;
            DWORD nFlags = 0;
            ::WSARecv(pPerHandle->m_s, &buf, 1, &dwTrans, &nFlags, &pPerIO->m_ol, NULL);
        }
    //case LEELOO_ACCEPT_OP_NET:
    //    break;
    }
    return Leeloo::SUCCESS;
}
Leeloo::Ret NetServer::Accept(HANDLE hCompletion, SOCKET sNew, SOCKADDR_IN saRemote, int nRemoteLen)
{
    Leeloo::IdType idNew = m_idManager.GetFreeId();
    if(idNew == Leeloo::INVALID_VALID)
    {
        ::closesocket(sNew);
        LEELOO_WRITE_LOG("GetFreeId error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return Leeloo::FAIL;
    }
    if(idNew >= LEELOO_MAX_SESSION_NET)
    {
        ::closesocket(sNew);
        LEELOO_WRITE_LOG("idNew >= LEELOO_MAX_SESSION_NET error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return Leeloo::FAIL;
    }
    if(m_session[idNew].Init(Session::LINK_TCPRECVSEND) != Leeloo::SUCCESS)
    {
        m_idManager.FreeFreeId(idNew);
        ::closesocket(sNew);
        LEELOO_WRITE_LOG("session init error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return Leeloo::FAIL;
    }
    m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerIo->m_sessionId = idNew;
    m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerIo->m_linkId = Session::LINK_TCPRECVSEND;
    //
    m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerHandle->m_s = sNew;
    memcpy(&(m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerHandle->m_addr), &saRemote, nRemoteLen);
    ::CreateIoCompletionPort((HANDLE)sNew, hCompletion, (DWORD)(m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerHandle), 0);
    //
    m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerIo->m_nOperationType = LEELOO_READ_OP_NET;
    WSABUF buf;
    buf.buf = m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerIo->m_buf;
    buf.len = LEELOO_BUFFER_SIZE_NET;     
    DWORD dwRecv;
    DWORD dwFlags = 0;
    ::WSARecv(sNew, &buf, 1, &dwRecv, &dwFlags, &(m_session[idNew].m_link[Session::LINK_TCPRECVSEND].m_pPerIo->m_ol), NULL);
    return Leeloo::SUCCESS;
}
void NetServer::ThreadProg(void* pPara, bool& bStop)
{
    //void* pParaLocal = pPara;
    //while(!bStop && Leeloo::Global::IsRun())
    //{
    //    std::cout << "NetServer::ThreadProg" << std::endl;
    //    Leeloo::Time::Sleep(1000);
    //}
    WORD wVersionRequested;
    WSADATA wsaData;
    //????? judge version
    wVersionRequested = MAKEWORD( 2, 2 );
    int ret = WSAStartup( wVersionRequested, &wsaData );
    if(ret != 0)
    {
        LEELOO_WRITE_LOG("WSAStartup error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return;
    }
    //
    //创建完成端口对象，
    std::string strThreadNum = LEELOO_DEFAULT_THREADNUM_VAL_CONFIG_NET;
    Leeloo::ConfigSingleton::Instance().GetConfig(LEELOO_THREADNUM_KEY_CONFIG_NET, strThreadNum);
    int threadNumber = atoi(strThreadNum.c_str());
    m_hCompletion = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, threadNumber);
    for(Leeloo::Int i=0;i<threadNumber;i++)
        ::CreateThread(NULL, 0, ServerThread, (LPVOID)this, 0, 0);
    //
    std::string strIp = LEELOO_DEFAULT_IP_VAL_CONFIG_NET;
    Leeloo::ConfigSingleton::Instance().GetConfig(LEELOO_IP_KEY_CONFIG_NET, strIp);
    std::string strPort = LEELOO_DEFAULT_TCPPORT_VAL_CONFIG_NET;
    Leeloo::ConfigSingleton::Instance().GetConfig(LEELOO_TCPPORT_KEY_CONFIG_NET, strPort);
    unsigned short nPort = atoi(strPort.c_str());
    //
    std::cout << "ip:" << strIp << std::endl;
    std::cout << "port:" << strPort << std::endl;
    std::cout << "thread num:" << strThreadNum << std::endl;
    //
    //创建监听套接字，绑定到本地地址，开始监听
    SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN si;
    si.sin_family = AF_INET;
    si.sin_port = ::htons(nPort);
    si.sin_addr.S_un.S_addr = ::inet_addr(strIp.c_str());//INADDR_ANY;
    ret = ::bind(sListen, (sockaddr*)&si, sizeof(si));
    if (ret != 0)
    {
        LEELOO_WRITE_LOG("bind error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return;
    }
    ret = ::listen(sListen, 5);
    if (ret != 0)
    {
        LEELOO_WRITE_LOG("listen error", Leeloo::LEELOO_ERROR_LEVEL_LOG)
        return;
    }
    //循环处理到来的连接
    while (!bStop && Leeloo::Global::IsRun())
    {
        // 等待接受未决的连接请求
        SOCKADDR_IN saRemote;
        int nRemoteLen = sizeof(saRemote);
        //
        SOCKET sNew = ::accept(sListen, (sockaddr*)&saRemote, &nRemoteLen);
        //
        Accept(m_hCompletion, sNew, saRemote, nRemoteLen);
    }
    WSACleanup();
}

}
