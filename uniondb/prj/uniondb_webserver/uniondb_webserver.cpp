// uniondb_webserver.cpp : 定义控制台应用程序的入口点。
//

#ifdef LEELOO_WINDOWS_OS
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#endif
#include <stdio.h>
#include <iostream>

#include "../../lib/leeloo.h"
#include "../../lib/leeloo.cpp"
#include "../../lib/framework.h"
#include "../../lib/framework.cpp"
#include "../../lib/net.h"
#include "../../lib/net.cpp"
#include "../../lib/web.h"
#include "../../lib/web.cpp"

class MyAppFramework : public LeelooFramework::AppFramework
{
public:
	MyAppFramework(){}
	virtual ~MyAppFramework(){}
	virtual Leeloo::Ret Start(void)
	{
		LeelooFramework::AppFramework::Start();
        LeelooNet::WebServerSingleton::Instance();
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
        LeelooNet::WebServerSingleton::Deinstance();
		LeelooFramework::AppFramework::End();
		return Leeloo::SUCCESS;
	}
private:
	MyAppFramework(MyAppFramework& ref){}
	MyAppFramework& operator=(MyAppFramework& ref){return *this;}
};
typedef Leeloo::Singleton<MyAppFramework> MyAppFrameworkSingleton;

/*
#ifdef LEELOO_WINDOWS_OS
#pragma data_seg("Shared")
unsigned g_nInstanceCount = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:Shared,RWS")
#else
#endif
#ifdef LEELOO_WINDOWS_OS
    ::CreateMutex(NULL, FALSE, "leeloo_webserver");
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        std::cout << "already run" << std::endl;
        return -1;
    }
    if(g_nInstanceCount >= 1)
    {
        std::cout << "already run" << std::endl;
        return -1;
    }
    g_nInstanceCount ++;
#else
#endif
*/

#define UNIONDB_MINIDUMP_USED
#ifdef UNIONDB_MINIDUMP_USED
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")
#endif
#ifdef UNIONDB_MINIDUMP_USED
long WINAPI ExceptionFilter(EXCEPTION_POINTERS * lParam)
{
    //DateTime dateTime(GetCurDateTime());
    char strFileName[64] = "leeloo_webserver.dmp";
    //std::string stdstrDumpName = ConfigSingleton::Instance().GetItem(ICEARK_DUMPNAME_SYSTEM_CONFIG);
    //if(stdstrDumpName.compare(ICEARK_BLANK_SYSTEM_CONFIG) == 0)
    //    stdstrDumpName = "iceark";
    //std::string stdstrOutputpath = ConfigSingleton::Instance().GetItem(ICEARK_OUTPUTPATH_SYSTEM_CONFIG);
    //if(stdstrOutputpath.compare(ICEARK_BLANK_SYSTEM_CONFIG) == 0)
    //    sprintf(strFileName, "dump_%s_%04d_%02d_%02d_%02d_%02d_%02d.dmp", stdstrDumpName.c_str(), dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute(), dateTime.second());
    //else
    //    sprintf(strFileName, "%sdump_%s_%04d_%02d_%02d_%02d_%02d_%02d.dmp", stdstrOutputpath.c_str(), stdstrDumpName.c_str(), dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute(), dateTime.second());
    //strFileName = "ztcache.dmp";
    HANDLE hFile = ::CreateFile( strFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile != INVALID_HANDLE_VALUE)
    {
            MINIDUMP_EXCEPTION_INFORMATION einfo;
            einfo.ThreadId = ::GetCurrentThreadId();
            einfo.ExceptionPointers = lParam;
            einfo.ClientPointers = FALSE;
            MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, &einfo, NULL, NULL);
            CloseHandle(hFile);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
int main(int argc, char* argv[])
{
#ifdef UNIONDB_MINIDUMP_USED
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    SetUnhandledExceptionFilter(ExceptionFilter);
#endif
    //
    std::cout << "web:http://www.uniondb.net/" << std::endl;
    std::cout << "email:uniondb@aliyun.com" << std::endl;
    std::cout << "uniondb webserver(open source version) 1.1" << std::endl;
    //
    MyAppFrameworkSingleton::Instance().Start();
    MyAppFrameworkSingleton::Instance().Run();
    MyAppFrameworkSingleton::Instance().End();
    MyAppFrameworkSingleton::Deinstance();
	return 0;
}