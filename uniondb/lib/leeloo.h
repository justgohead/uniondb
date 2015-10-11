#ifndef _LEELOO_H_
#define _LEELOO_H_

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <assert.h>
#include <math.h>
#include <strstream>

#ifdef LEELOO_WINDOWS_OS
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <intrin.h>
#else
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#endif

//loki
#define LOKI_USE_REFERENCE
#include "../../depend/loki/TypeManip.h"
#include "../../depend/loki/Reference/functor.h"

namespace Leeloo
{

//base type
#ifdef LEELOO_WINDOWS_OS

#ifdef LEELOO_64_PLATFORM
typedef INT64  Int;
typedef UINT64 Uint;
typedef double Float;
#else
typedef INT32  Int;
typedef UINT32 Uint;
typedef float  Float;
#endif
typedef INT64  Int64;
typedef INT32  Int32;
typedef INT16  Int16;
typedef INT8   Int8;
typedef UINT64 Uint64;
typedef UINT32 Uint32;
typedef UINT16 Uint16;
typedef UINT8  Uint8;
typedef double Float64;
typedef float  Float32;

#else

#ifdef LEELOO_64_PLATFORM
typedef int64_t  Int;
typedef uint64_t Uint;
typedef double   Float;
#else
typedef int32_t  Int;
typedef uint32_t Uint;
typedef float    Float;
#endif
typedef int64_t  Int64;
typedef int32_t  Int32;
typedef int16_t  Int16;
typedef int8_t   Int8;
typedef uint64_t Uint64;
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t  Uint8;
typedef double   Float64;
typedef float    Float32;

#endif

//return val
typedef Int Ret;
const Ret SUCCESS = 0;
const Ret FAIL    = 1;//unknown error
//leeloo error code
const Ret PARAINVALID_ERRORCODE                    = 2;
const Ret NOMEMORY_ERRORCODE                       = 3;
const Ret STATEINVALID_ERRORCODE                   = 4;
const Ret REGTHREAD_FAIL_CASQUEUEMANAGER_ERRORCODE = 5;
const Ret RELEASE_FAIL_CASRELEASER_ERRORCODE       = 6;
const Ret STARTED_THREAD_ERRORCODE                 = 7;
const Ret NO_ID_ERRORCODE                          = 8;
const Ret DB_RECORD_EXISTED_ERRORCODE              = 9;

//app error code start
const Ret USER_ERRORCODE = 1024;

//cas
#ifdef LEELOO_WINDOWS_OS
#define LEELOO_CAS( destination, exchange, comparand ) ( InterlockedCompareExchangePointer( (PVOID volatile *)destination, (PVOID)exchange, (PVOID)comparand ) != comparand ? true : false )
#else
//#define CAS( destination, exchange, comparand ) ( __sync_val_compare_and_swap( destination, comparand, exchange ) != comparand ? true : false )
#define LEELOO_CAS( destination, exchange, comparand ) ( !__sync_bool_compare_and_swap( destination, comparand, exchange ) )
#endif

#ifdef LEELOO_WINDOWS_OS
#ifdef LEELOO_64_PLATFORM
#define LEELOO_INCREMENT(pInt) InterlockedIncrement64(pInt);
#define LEELOO_DECREMENT(pInt) InterlockedDecrement64(pInt);
#else
#define LEELOO_INCREMENT(pInt) InterlockedIncrement((volatile LONG*)pInt);
#define LEELOO_DECREMENT(pInt) InterlockedDecrement((volatile LONG*)pInt);
#endif
#else
#endif

class NoLockPolicy
{
public:
    NoLockPolicy(){}
    virtual ~NoLockPolicy(){}
    inline void Lock(void){}
    inline void Unlock(void){}
};
class LockPolicy
{
public:
    LockPolicy();
    virtual ~LockPolicy();
    inline void Lock(void);
    inline void Unlock(void);
private:
#ifdef LEELOO_WINDOWS_OS
    CRITICAL_SECTION m_mtx_;
#else
    pthread_mutex_t m_mtx_;
#endif
};
template <typename T = LockPolicy>
class GuardLock
{
public:
    inline GuardLock(T& lock):m_lock(lock)
    {
        m_lock.Lock();
    }
    inline ~GuardLock()
    {
        m_lock.Unlock();
    }
private:
    T& m_lock;
};
//Singleton
template < typename T, typename TlockPolicy = LockPolicy >
class Singleton
{
    typedef GuardLock<TlockPolicy> Lock;
public:
    inline static T& Instance(void)
    {
        if(m_pInstance == NULL)
        {
            Lock lock(m_lockPolicy);
            if(m_pInstance == NULL)
                m_pInstance = new T();
        }
        return *m_pInstance;
    }
    inline static void Deinstance(void)
    {
        if(m_pInstance != NULL)
        {
            Lock lock(m_lockPolicy);
            if(m_pInstance != NULL)
                delete m_pInstance;
            m_pInstance = NULL;
        }
    }
private:
    Singleton(){}
    ~Singleton(){}
    Singleton(Singleton& ref){}
    Singleton& operator=(Singleton& ref){return *this;}
private:
    static TlockPolicy m_lockPolicy;
    static T* m_pInstance;
};
template < typename T, typename TlockPolicy >
TlockPolicy Singleton< T, TlockPolicy >::m_lockPolicy;
template < typename T, typename TlockPolicy >
T* Singleton< T, TlockPolicy >::m_pInstance = NULL;

//global
const Int LEELOO_RUN_GLOBAL = 0;
const Int LEELOO_STOP_GLOBAL = 1;
class Global
{
public:
    static bool IsRun(void);
    static void SetRun(void);
    static void SetStop(void);
private:
    Global(){}
    ~Global(){}
    Global(Global& ref){}
    Global& operator=(Global& ref){return *this;}
private:
    volatile static Int m_runned;
};

//log level
const std::string LEELOO_DEBUG_LEVEL_LOG = "0";
const std::string LEELOO_WARN_LEVEL_LOG = "1";
const std::string LEELOO_ERROR_LEVEL_LOG = "2";
const std::string LEELOO_NOLOG_LEVEL_LOG = "3";
//config
const std::string LEELOO_LOGLEVEL_KEY_CONFIG = "/log/level";
const std::string LEELOO_DEFAULT_LOGLEVEL_VAL_CONFIG = LEELOO_NOLOG_LEVEL_LOG;
class Config
{
public:
    Config();
    virtual ~Config();
    Ret ReadConfig(std::string configName);
    Ret GetConfig(std::string key, std::string& val);
private:
    Config(Config& ref){}
    Config& operator=(Config& ref){return *this;}
private:
    std::map<std::string, std::string> m_keyValMap;
};
typedef Leeloo::Singleton<Config> ConfigSingleton;

//log
#define LEELOO_WRITE_LOG(strLog, strLevel) {\
std::strstream mystream;\
char cBuf[16];\
sprintf(cBuf, "%d", __LINE__);\
Leeloo::LogSingleton::Instance().WriteLog(strLog, strLevel, __FILE__, cBuf, __FUNCTION__);\
}
//mystream << __LINE__;\
//mystream << "\0";\
//mystream.str()
class Log : public LockPolicy
{
    typedef GuardLock<LockPolicy> Lock;
public:
    Log();
    virtual ~Log();
    Ret WriteLog(std::string strLog, std::string strLevel, std::string file = "", std::string line = "", std::string function = "");
private:
    Log(Log& ref){}
    Log& operator=(Log& ref){return *this;}
private:
    std::ofstream m_file;
};
typedef Leeloo::Singleton<Log> LogSingleton;

class Time
{
public:
    Time()
    {
#ifdef LEELOO_WINDOWS_OS
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        memcpy(((unsigned char*)&m_datetime) + 0, &(st.wYear), 2);
        memcpy(((unsigned char*)&m_datetime) + 2, &(st.wMonth), 1);
        memcpy(((unsigned char*)&m_datetime) + 3, &(st.wDay), 1);
        memcpy(((unsigned char*)&m_datetime) + 4, &(st.wHour), 1);
        memcpy(((unsigned char*)&m_datetime) + 5, &(st.wMinute), 1);
        memcpy(((unsigned char*)&m_datetime) + 6, &(st.wSecond), 1);
        st.wMilliseconds /= 4;
        memcpy(((unsigned char*)&m_datetime) + 7, &(st.wMilliseconds), 1);
#else
#endif
    }
    ~Time()
    {}
    //milliseconds,1/1000second
    inline static void Sleep(Int time)
    {
#ifdef LEELOO_WINDOWS_OS
        ::Sleep(time);
#else
        usleep(time*1000);
#endif
    }
    inline static Time GetTimeCur(void)
    {
        return Time();
    }
public:
    Time(Time& ref){m_datetime = ref.m_datetime;}
    Time& operator=(Time& ref)
    {
        if(this != &ref)
        {
            m_datetime = ref.m_datetime;
        }
        return *this;
    }
private:
    Uint64 m_datetime;
};

typedef struct _NoCountPolicy
{
    inline void Increase(void){}
    inline void Decrease(void){}
    inline Int GetCount(void){return 0;}
}NoCountPolicy;
typedef struct _SingleThreadReferenceCountPolicy
{
    inline void Increase(void){m_referenceCount++;}
    inline void Decrease(void){m_referenceCount--;}
    inline Int GetCount(void){return m_referenceCount;}
    volatile Int m_referenceCount;
}SingleThreadReferenceCountPolicy;
typedef struct _CasReferenceCountPolicy
{
    inline void Increase(void){LEELOO_INCREMENT(&m_referenceCount);}
    inline void Decrease(void){LEELOO_DECREMENT(&m_referenceCount);}
    inline Int GetCount(void){return m_referenceCount;}
    volatile Int m_referenceCount;
}MultyThreadReferenceCountPolicy;
template < typename TcountPolicy = MultyThreadReferenceCountPolicy >
class Base : public TcountPolicy
{
public:
    Base()
    {InitCountPolicy();}
    virtual ~Base()
    {}
    inline void Increase(void)
    {
        TcountPolicy::Increase();
    }
    inline void Decrease(void)
    {
        TcountPolicy::Decrease();
    }
    inline Int GetCount(void)
    {
        return TcountPolicy::GetCount();
    }
private:
    inline void InitCountPolicy(void)
	{
        InitCountPolicyIn(Loki::Type2Type< TcountPolicy >());
    }
    inline void InitCountPolicyIn(Loki::Type2Type< NoCountPolicy >)
	{
    }
    inline void InitCountPolicyIn(Loki::Type2Type< SingleThreadReferenceCountPolicy >)
	{
        SingleThreadReferenceCountPolicy::m_referenceCount = 0;
    }
	inline void InitCountPolicyIn(Loki::Type2Type< MultyThreadReferenceCountPolicy >)
	{
        MultyThreadReferenceCountPolicy::m_referenceCount = 0;
    }
private:
    Base(Base& ref){}
    Base& operator=(Base& ref){return *this;}
private:
};

template < typename T>
class SmartPtr
{
public:
    inline SmartPtr():m_ptr(NULL)
    {}
    inline explicit SmartPtr(T* ptr):m_ptr(NULL)
    {
        m_ptr = ptr;
        if(m_ptr != NULL)
            m_ptr->Increase();
    }
    inline explicit SmartPtr(std::auto_ptr< T > smartPtr):m_ptr(NULL)
    {
        m_ptr = smartPtr.release();
        if(m_ptr != NULL)
            m_ptr->Increase();
    }
    inline SmartPtr(SmartPtr& ref):m_ptr(NULL)
    {
        m_ptr = ref.m_ptr;
        if(m_ptr != NULL)
            m_ptr->Increase();
    }
    inline SmartPtr& operator=(SmartPtr& ref)
    {
        if(this != &ref && m_ptr != ref.m_ptr)
        {
            if(m_ptr != NULL)
            {
                m_ptr->Decrease();
                if(m_ptr->GetCount() == 0)
                    delete m_ptr;//m_ptr = NULL;
            }
            m_ptr = ref.m_ptr;
            if(m_ptr != NULL)
                m_ptr->Increase();
        }
        return *this;
    }
    virtual ~SmartPtr()
    {
        if(m_ptr != NULL)
        {
            m_ptr->Decrease();
            if(m_ptr->GetCount() == 0)
                delete m_ptr;//m_ptr = NULL;
        }
    }
    inline T* operator->() const
    {
        return m_ptr;
    }
    inline T& operator*() const
    {
        return *m_ptr;
    }
private:
    T* m_ptr;
};

#ifdef LEELOO_WINDOWS_OS
//#define LEELOO_HIGHEST_PRIORITY_THREAD THREAD_PRIORITY_TIME_CRITICAL
//#define LEELOO_NORMAL_PRIORITY_THREAD THREAD_PRIORITY_NORMAL
//#define LEELOO_LOWEST_PRIORITY_THREAD THREAD_PRIORITY_IDLE
#else
#endif
class Thread : public Base<>
{
#ifdef LEELOO_WINDOWS_OS
    static void ProcThread (void *arg)
    {
        Thread* pThread = (Thread*)arg;
        assert(pThread != NULL);
        pThread->Run();
    }
#else
    static void* ProgThread(void* arg)
    {
//#ifdef LEELOO_VAR_THREADLOCAL
//#else
        AllocatorSingleton::Instance().RegThread();
//#endif
        Thread* pThread = (Thread*)arg;
        assert(pThread != NULL);
        pThread->Run();
//#ifdef LEELOO_VAR_THREADLOCAL
//#else
        AllocatorSingleton::Instance().UnregThread();
//#endif
    }
#endif
typedef struct {
#ifdef LEELOO_WINDOWS_OS
    HANDLE m_h;
    DWORD m_id;
#else
    pthread_t m_h;
#endif
    }ThreadId;
public:
    Thread():m_bStop(false),m_bStarted(false)
    {
    }
    virtual ~Thread()
    {
        StopTask();
    }
private:
    Thread(Thread& ref){}
    Thread& operator=(Thread& ref){return *this;}
public:
    Ret SetTask(Loki::Functor<void,TYPELIST_1(bool&)> functor)
    {
        if(m_bStarted)
            return STARTED_THREAD_ERRORCODE;
        m_bStarted = true;
        m_functor = functor;
#ifdef LEELOO_WINDOWS_OS
        m_threadId.m_h = CreateThread(NULL,
            0,
            (LPTHREAD_START_ROUTINE)ProcThread,
            (VOID*)this,
            0,
            (LPDWORD)&m_threadId.m_id);
        //SetThreadPriority(m_threadId.m_h, ZTLIB_NORMAL_PRIORITY_THREAD);
        return SUCCESS;
#else
        pthread_create(&m_threadId.m_h, NULL, ProgThread, (void*)this);
        return SUCCESS;
#endif
    }
    void StopTask(void)
    {
        m_bStop = true;
#ifdef LEELOO_WINDOWS_OS
        if(m_bStarted)
        {
            ResumeThread();
            BreakDelay();
            //if(sync)
                WaitForSingleObject( m_threadId.m_h, INFINITE );
            CloseHandle( m_threadId.m_h );
        }
#else
        if(m_bStarted)
        {
            //::ResumeThread(m_threadId);
            BreakDelay();
            pthread_join(m_threadId.m_h, NULL);
            //pthread_destroy
        }
#endif
        m_bStop = false;
        m_bStarted = false;
#ifdef LEELOO_WINDOWS_OS
#else
#endif
    }
    //bool IsStarted(void)
    //{
    //    return m_bStarted;
    //}
    inline Ret SuspendThread(void)
    {
#ifdef LEELOO_WINDOWS_OS
        ::SuspendThread(m_threadId.m_h);
        return SUCCESS;
#else
#endif
    }
    inline Ret ResumeThread(void)
    {
#ifdef LEELOO_WINDOWS_OS
        ::ResumeThread(m_threadId.m_h);
        return SUCCESS;
#else
#endif
    }
private:
    virtual void BreakDelay(void)
    {}
    //virtual void Run(void)
    inline void Run(void)
    {
        //m_bStop == true, must return
        //std::cout << "Thread::Run" << std::endl;
        //
        //Loki::Functor< void, Loki::NullType> bindFunctor(BindFirst(m_functor, m_bStop));
        //try{
        //    bindFunctor();
        //}catch(...){
        //}
        //
        //Loki::Functor< void, Loki::NullType> bindFunctor(BindFirst(m_functor, m_bStop));
        try{
            m_functor(m_bStop);
        }catch(...){
        }
    }
protected:
    bool m_bStop;
    ThreadId m_threadId;
    bool m_bStarted;
    Loki::Functor<void,TYPELIST_1(bool&)> m_functor;
};

//queue, multi thread
//T must point to Base or Base's child
template <typename T>
class Queue : public LockPolicy
{
    typedef GuardLock<LockPolicy> Lock;
public:
    Queue(){}
    virtual ~Queue()
    {
        std::list<T*>::iterator iter;
        for(iter=m_list.begin();iter!=m_list.end();iter++)
        {
            delete *iter;
        }
    }
    inline void Push(std::auto_ptr<T> tSmart)
    {
        if(tSmart.get() == NULL)
            return;
        Lock lock(*this);
        m_list.push_back(tSmart.release());
    }
    inline std::auto_ptr<T> Pop(void)
    {
        std::auto_ptr<T> smart(NULL);
        Lock lock(*this);
        if(m_list.size() > 0)
        {
            smart = std::auto_ptr<T>(*m_list.begin());
            m_list.pop_front();
        }
        return smart;
    }
private:
    Queue(Queue& ref){}
    Queue& operator=(Queue& ref){return *this;}
private:
    std::list<T*> m_list;
};

//id manager
typedef Leeloo::Int IdType;
#ifdef LEELOO_64_PLATFORM
const IdType MAX_VALID = 0x7fffffffffffffff;
#else
const IdType MAX_VALID = 0x7fffffff;
#endif
const IdType INVALID_VALID = -1;
class IdManager : public Base<>
{
public:
    IdManager();
    virtual ~IdManager();
    IdType GetFreeId(void);
    void FreeFreeId(IdType id);
private:
    IdManager(IdManager& ref){}
    IdManager& operator=(IdManager& ref){return *this;}
private:
    //????? maybe have many freeid ,clear to m_topFreeId?
    Queue<IdType> m_freeIdQueue;
    IdType m_topFreeId;
};

//knowege framework
//data
template < typename T = char>
class Data
{
public:
    Data():m_pBuffer(NULL),m_bufLen(0),m_dataLen(0),m_pos(0)
    {
    }
    explicit Data(Int bufLen):m_pBuffer(NULL),m_bufLen(0),m_dataLen(0),m_pos(0)
    {
        if(bufLen > 0)
        {
            m_pBuffer = new T[bufLen+2];
            m_bufLen = bufLen;
        }
    }
    virtual ~Data()
    {
        if(m_pBuffer != NULL)
            delete[] m_pBuffer;
    }
    void SetBuffer(Int bufLen)
    {
        if(bufLen > 0)
        {
            if(m_pBuffer != NULL)
                delete[] m_pBuffer;
            m_pBuffer = new T[bufLen+2];
            m_bufLen = bufLen;
            m_dataLen = 0;
        }
    }
    Ret Append(const T* pData, Int dataLen)
    {
        if(pData == NULL || dataLen <= 0)
            return PARAINVALID_ERRORCODE;
        if(m_dataLen+dataLen > m_bufLen)
            return Leeloo::FAIL;
        memcpy(m_pBuffer + m_dataLen, pData, sizeof(T)*dataLen);
        m_dataLen += dataLen;
        m_pBuffer[m_dataLen] = '\0';
        return Leeloo::SUCCESS;
    }
    const T* GetData(Int& dataLen)
    {
        dataLen = m_dataLen;
        return m_pBuffer;
    }
    void Clear(void)
    {
        m_dataLen = 0;
    }
    Ret Seek(Int pos = 0)
    {
        if(pos < m_dataLen)
        {
            m_pos = pos;
            return SUCCESS;
        }
        return PARAINVALID_ERRORCODE;
    }
    Ret Read(T* pBuffer, Int number, Int& numReaded)
    {
        if(pBuffer == NULL || number <= 0)
            return PARAINVALID_ERRORCODE;
        numReaded = m_dataLen - m_pos;
        if(numReaded > number)
            numReaded = number;
        if(numReaded > 0)
        {
            memcpy(pBuffer, m_pBuffer+m_pos, numReaded);
            m_pos += numReaded;
            return SUCCESS;
        }
        return FAIL;
    }
private:
    Data(Data& ref){}
    Data& operator=(Data& ref){return *this;}
private:
    T* m_pBuffer;
    Int m_bufLen;
    Int m_dataLen;
    Int m_pos;
};
//info
typedef struct _InfoItem
{
    std::string m_key;
    //Int m_dataStart;
    //Int m_dataLen;
    std::string m_strVal;
    Data<char> m_dataExt;
}InfoItem;
template < typename Tkey = std::string, typename Titem = InfoItem >
class Info
{
public:
    Info()
    {
    }
    virtual ~Info()
    {
        std::vector<Titem*>::iterator iter;
        for(iter=m_itemVector.begin();iter!=m_itemVector.end();iter++)
            delete *iter;
    }
    Ret SetInfo(Tkey key, std::auto_ptr<Titem> itemSmart)
    {
        if(m_itemMap.find(key) != m_itemMap.end())
            return FAIL;
        m_itemVector.push_back(itemSmart.release());
        m_itemMap.insert(std::pair<Tkey, Int>(key, m_itemVector.size()-1));
        return SUCCESS;
    }
    const Titem* GetInfo(Tkey key)
    {
        std::map<Tkey, Int>::iterator iter = m_itemMap.find(key);
        if(m_itemMap.find(key) == m_itemMap.end())
            return NULL;
        return m_itemVector[iter->second];
    }
	void Clear(void)
	{
        m_itemVector.clear();
        m_itemMap.clear();
	}
private:
    Info(Info& ref){}
    Info& operator=(Info& ref){return *this;}
private:
    std::map<Tkey, Int> m_itemMap;
    std::vector<Titem*> m_itemVector;
};
//protocol
template < typename Tdata = Data<>, typename Tinfo = Info<> >
class Protocol
{
public:
    Protocol()
    {
    }
    virtual ~Protocol()
    {
    }
    Ret Decode(Tdata& in, Tinfo& inExt, Tinfo& out, Tinfo& outExt)
    {
        return SUCCESS;
    }
    Ret Encode(Tinfo& in, Tinfo& inExt, Tdata& out, Tinfo& outExt)
    {
        return SUCCESS;
    }
private:
    Protocol(Protocol& ref){}
    Protocol& operator=(Protocol& ref){return *this;}
private:

};
//dispose framework:nn,expert,prog
//disposer
template < typename Tdata = Data<>, typename Tinfo = Info<> >
class Disposer
{
public:
    Disposer()
    {
    }
    virtual ~Disposer()
    {
    }
    Ret Dispose(Tdata& data, Tinfo& in, Tinfo& inExt, Tinfo& out, Tinfo& outExt)
    {
        return SUCCESS;
    }
private:
    Disposer(Disposer& ref){}
    Disposer& operator=(Disposer& ref){return *this;}
private:
};
//knowledge
typedef struct _ReqresPolicy
{
}ReqresPolicy;
typedef struct _ReportPolicy
{
}ReportPolicy;
typedef struct _NotifyPolicy
{
}NotifyPolicy;
/*
typedef struct _PubresPolicy
{
}PubresPolicy;
*/
template < typename Tdata = Data<>, typename Tinfo = Info<>, typename Tprotocol = Protocol<>, typename Tdisposer = Disposer<>, typename Tpolicy = ReqresPolicy >
class KnowledgeFramework
{
public:
    KnowledgeFramework()
    {
    }
    virtual ~KnowledgeFramework()
    {
    }
    Ret Think(Tdata& in, Tinfo& inExt, Tdata& out, Tinfo& outExt)
    {
        return ThinkInside(in, inExt, out, outExt, Loki::Type2Type< Tpolicy >());
    }
protected:
    virtual Ret ObtainPubsRes(Tdata& out, Tinfo& outExt, Tdata& in, Tinfo& inExt){return FAIL;}
protected:
	Ret ThinkInside(Tdata& in, Tinfo& inExt, Tdata& out, Tinfo& outExt, Loki::Type2Type< ReqresPolicy >)
    {
        Tprotocol protocol;
        Tdisposer disposer;
        Tinfo inTmp, outTmp;
        Tinfo outExtTmp;
        if(protocol.Decode(in, inExt, inTmp, outExtTmp) != SUCCESS)
            return FAIL;
        if(disposer.Dispose(in, inTmp, inExt, outTmp, outExtTmp) != SUCCESS)
            return FAIL;
        if(protocol.Encode(outTmp, outExtTmp, out, outExt) != SUCCESS)
            return FAIL;
        return SUCCESS;
    }
    Ret ThinkInside(Tdata& in, Tinfo& inExt, Tdata& out, Tinfo& outExt, Loki::Type2Type< ReportPolicy >)
    {
        Tprotocol protocol;
        Tdisposer disposer;
        Tinfo inTmp, outTmp;
        Tinfo outExtTmp;
        if(protocol.Decode(in, inExt, inTmp, outExtTmp) != SUCCESS)
            return FAIL;
        if(disposer.Dispose(in, inTmp, inExt, outTmp, outExtTmp) != SUCCESS)
            return FAIL;
        return SUCCESS;
    }
    Ret ThinkInside(Tdata& in, Tinfo& inExt, Tdata& out, Tinfo& outExt, Loki::Type2Type< NotifyPolicy >)
    {
        Tprotocol protocol;
        Tdisposer disposer;
        Tinfo inTmp, outTmp;
        Tinfo outExtTmp;
        if(disposer.Dispose(in, inTmp, inExt, outTmp, outExtTmp) != SUCCESS)
            return FAIL;
        if(protocol.Encode(outTmp, outExtTmp, out, outExt) != SUCCESS)
            return FAIL;
        return SUCCESS;
    }
    /*
    Ret ThinkInside(Tdata& in, Tinfo& inExt, Tdata& out, Tinfo& outExt, Loki::Type2Type< PubresPolicy >)
    {
        Tprotocol protocol;
        Tdisposer disposer;
        Tinfo inTmp, outTmp;
        Tinfo outExtTmp;
        if(disposer.Dispose(in, inTmp, inExt, outTmp, outExtTmp) != SUCCESS)
            return FAIL;
        if(protocol.Encode(outTmp, outExtTmp, out, outExt) != SUCCESS)
            return FAIL;
        //if(ObtainRes(out, outExt, in, inTmp) != SUCCESS)
        //    return FAIL;
        //if(protocol.Decode(in, inTmp, inTmp, outExtTmp) != ZTLIB_SUCCESS)
        //    return ZTLIB_FAIL;
        //if(disposer.Dispose(in, inTmp, inExt, outTmp, outExtTmp) != ZTLIB_SUCCESS)
        //    return ZTLIB_FAIL;
        return SUCCESS;
    }
    */
private:
    KnowledgeFramework(KnowledgeFramework& ref){}
    KnowledgeFramework& operator=(KnowledgeFramework& ref){return *this;}
private:
};

}

#endif
