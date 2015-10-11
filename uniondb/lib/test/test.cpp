#ifdef LEELOO_WINDOWS_OS
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <iostream>

#include "../leeloo.h"
#include "../leeloo.cpp"
#include "../framework.h"
#include "../framework.cpp"
#include "../db.h"
#include "../net.h"
#include "../web.h"

class MyAppFramework : public LeelooFramework::AppFramework
{
public:
	MyAppFramework(){}
	virtual ~MyAppFramework(){}
	virtual Leeloo::Ret Start(void)
	{
		LeelooFramework::AppFramework::Start();
        LeelooDb::DbSingleton::Instance();
        ;
		return Leeloo::SUCCESS;
	}
	virtual Leeloo::Ret Run(void)
	{
		LeelooFramework::AppFramework::Run();
		return Leeloo::SUCCESS;
	}
	virtual Leeloo::Ret End(void)
	{
        Leeloo::Global::SetStop();
        ;
        LeelooDb::DbSingleton::Deinstance();
		LeelooFramework::AppFramework::End();
		return Leeloo::SUCCESS;
	}
private:
	MyAppFramework(MyAppFramework& ref){}
	MyAppFramework& operator=(MyAppFramework& ref){return *this;}
};
typedef Leeloo::Singleton<MyAppFramework> MyAppFrameworkSingleton;

class MyThread
{
public:
    MyThread()
    {
        Loki::Functor<void, TYPELIST_2(void*,bool&)> functor(this, &MyThread::ThreadProg);
        Loki::Functor< void, TYPELIST_1(bool&)> bindFunctor(BindFirst(functor, (void*)0));
        m_thread.SetTask(bindFunctor);
    }
    ~MyThread()
    {
        m_thread.StopTask();
    }
private:
    void ThreadProg(void* pPara, bool& bStop)
    {
        Leeloo::Time::Sleep(1000);
        Leeloo::Int para = (Leeloo::Int)pPara;
        std::cout << "ThreadObj::TestThread. pPara= " << para << std::endl;
        Leeloo::Int i = 0;
        while(!bStop)
        {
            std::cout << i << std::endl;
            i++;
            Leeloo::Time::Sleep(1000);
        }
    }
private:
    Leeloo::Thread m_thread;
};
class TestClass : public Leeloo::Base<>
{
public:
    ~TestClass(){std::cout << "~TestClass()" << std::endl;}
};

int main(int argc, char* argv[])
{
	MyAppFrameworkSingleton::Instance().Start();
    //
    {
        std::cout << "test SmartPtr start" << std::endl;
        {
        Leeloo::SmartPtr<TestClass> smart(new TestClass());
        Leeloo::SmartPtr<TestClass> smart1 = smart;
        }
        std::cout << "test SmartPtr end" << std::endl;
    }
    {
        std::cout << "test Thread start" << std::endl;
        {
        MyThread threadObj;
        Leeloo::Time::Sleep(3000);
        }
        std::cout << "test Thread end" << std::endl;
    }
    {
        std::cout << "test Db start" << std::endl;
        {
        std::auto_ptr<std::string> keySmart(new std::string("key"));
        std::auto_ptr<Leeloo::Data<>> valSmart(new Leeloo::Data<>(3));
        valSmart->Append("val", 3);
        LeelooDb::DbSingleton::Instance().Set(keySmart, valSmart);
        valSmart.reset(NULL);
        }
        {
        std::auto_ptr<std::string> keySmart(new std::string("key"));
        std::auto_ptr<Leeloo::Data<>> valSmart;
        LeelooDb::DbSingleton::Instance().Get(keySmart, valSmart);
        Leeloo::Int valLen;
        std::cout << valSmart->GetData(valLen) << std::endl;
        }
        {
        std::auto_ptr<std::string> keySmart(new std::string("key"));
        LeelooDb::DbSingleton::Instance().Del(keySmart);
        }
        {
        std::auto_ptr<std::string> keySmart(new std::string("key"));
        std::auto_ptr<Leeloo::Data<>> valSmart;
        LeelooDb::DbSingleton::Instance().Get(keySmart, valSmart);
        valSmart.reset(NULL);
        }
        std::cout << "test Db end" << std::endl;
    }
    /*
    LeelooNet::NetServerSingleton::Instance();//should moveto MyAppFramework::Start
    {
        Leeloo::Time::Sleep(5000);
        //
        std::string strIp = LeelooNet::LEELOO_DEFAULT_IP_VAL_CONFIG_NET;
        Leeloo::ConfigSingleton::Instance().GetConfig(LeelooNet::LEELOO_IP_KEY_CONFIG_NET, strIp);
        std::string strPort = LeelooNet::LEELOO_DEFAULT_TCPPORT_VAL_CONFIG_NET;
        Leeloo::ConfigSingleton::Instance().GetConfig(LeelooNet::LEELOO_TCPPORT_KEY_CONFIG_NET, strPort);
        unsigned short nPort = atoi(strPort.c_str());
        SOCKET sockClient=socket(AF_INET,SOCK_STREAM,0);
        SOCKADDR_IN addrSrv;
        addrSrv.sin_addr.S_un.S_addr=inet_addr(strIp.c_str());
        addrSrv.sin_family=AF_INET;
        addrSrv.sin_port=htons(nPort);
        int ret = connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
        //
        char cBuf[MAX_PATH] = "as";
        int sendNum = send(sockClient, cBuf, 2, 0);
        cBuf[0] = '\0';
        int recvNum = recv(sockClient, cBuf, MAX_PATH, 0);
        closesocket(sockClient);
        //
        Leeloo::Time::Sleep(3000*100000);
    }
    LeelooNet::NetServerSingleton::Deinstance();//should moveto MyAppFramework::End
    */
    /*
    LeelooNet::WebServerSingleton::Instance();//should moveto MyAppFramework::Start
    {
        Leeloo::Time::Sleep(3000*100000);
    }
    LeelooNet::WebServerSingleton::Deinstance();//should moveto MyAppFramework::End
    */
	MyAppFrameworkSingleton::Instance().Run();
	MyAppFrameworkSingleton::Instance().End();
    MyAppFrameworkSingleton::Deinstance();
	return 0;
}

/*
void TestThreadProg(void* pPara, bool& bStop)
{
    while(!bStop)
    {
        std::cout << "TestThreadProg" << std::endl;
    }
}
class ThreadObj
{
public:
    ThreadObj()
    {
        //Loki::Functor<void,TYPELIST_1(bool&)> functor(TestThreadProg);
        //Loki::Functor<void,TYPELIST_1(bool&)> functor(this, &ThreadObj::TestThread);
        //m_thread.SetTask(functor);
        //Loki::Functor<void,TYPELIST_1(bool&)> functor1(this, &ThreadObj::TestThread1);
        //m_thread1.SetTask(functor1);
        //Loki::Functor<void,TYPELIST_1(bool&)> functor2(this, &ThreadObj::TestThread2);
        //m_thread2.SetTask(functor2);
        //m_testSmartPtr = Ztlib::SmartPtr<Ctest>(new Ctest());
        Leeloo::Int iData = 0;
        m_iCasData.Set(iData);
        {
        Loki::Functor<void, TYPELIST_2(void*,bool&)> functor(this, &ThreadObj::TestThread);
        Loki::Functor< void, TYPELIST_1(bool&)> bindFunctor(BindFirst(functor, (void*)0));
        m_thread.SetTask(bindFunctor);
        }
        {
        Loki::Functor<void, TYPELIST_2(void*,bool&)> functor(this, &ThreadObj::TestThread1);
        Loki::Functor< void, TYPELIST_1(bool&)> bindFunctor(BindFirst(functor, (void*)1));
        m_thread1.SetTask(bindFunctor);
        }
    }
    ~ThreadObj()
    {
        m_thread.StopTask();
        m_thread1.StopTask();
    }
private:
    void TestThread(void* pPara, bool& bStop)
    {
        std::cout << "ThreadObj::TestThread. pPara= " << pPara << std::endl;
        Leeloo::Int i = 0;
        Leeloo::Int iData;
        while(!bStop && (i++)<10)
        {
            m_iCasData.Get(iData);
            std::cout << iData << std::endl;
            //Leeloo::Time::Sleep(10);
        }
    }
    void TestThread1(void* pPara, bool& bStop)
    {
        std::cout << "ThreadObj::TestThread1. pPara= " << pPara << std::endl;
        Leeloo::Int i = 0;
        //Ztlib::SmartPtr<Ctest> testSmartPtr = m_testSmartPtr;
        while(!bStop && (i++)<3)
        {
            i++;
            if(i > 0x7fffffff)
                i = 0;
            m_iCasData.Set(i);
            //Leeloo::Time::Sleep(10);
        }
    }
private:
    Leeloo::Thread m_thread;
    Leeloo::Thread m_thread1;
    Leeloo::DataWrap<Leeloo::Int, Leeloo::CasDataPolicy> m_iCasData;
};
*/