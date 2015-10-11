#ifndef _LEELOOFRAMEWORK_H_
#define _LEELOOFRAMEWORK_H_

#include "leeloo.h"

namespace LeelooFramework
{

//application framework,use in main thread
class AppFramework
{
public:
	AppFramework();
	virtual ~AppFramework();
    virtual Leeloo::Ret Start(void);
	virtual Leeloo::Ret Run(void);
	virtual Leeloo::Ret End(void);
private:
	AppFramework(AppFramework& ref){}
	AppFramework& operator=(AppFramework& ref){return *this;}
};

}

#endif