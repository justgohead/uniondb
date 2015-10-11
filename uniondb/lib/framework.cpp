#include "framework.h"
#include "db.h"

namespace LeelooFramework
{

AppFramework::AppFramework()
{
}
AppFramework::~AppFramework()
{
}
Leeloo::Ret AppFramework::Start(void)
{
    Leeloo::Global::SetRun();
    Leeloo::ConfigSingleton::Instance().ReadConfig("config.txt");
    Leeloo::LogSingleton::Instance();
    ;
	return Leeloo::SUCCESS;
}
Leeloo::Ret AppFramework::Run(void)
{
	std::string strInput;
	std::cout << "input q, exit program" << std::endl;
    while(Leeloo::Global::IsRun())
	{
		std::cin >> strInput;
		if(strInput.compare("q") == 0)
			break;
	}
	return Leeloo::SUCCESS;
}
Leeloo::Ret AppFramework::End(void)
{
    Leeloo::Global::SetStop();
	;
    Leeloo::LogSingleton::Deinstance();
    Leeloo::ConfigSingleton::Deinstance();
	return Leeloo::SUCCESS;
}

}
