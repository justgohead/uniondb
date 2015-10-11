#ifndef _LEELOODB_H_
#define _LEELOODB_H_

#include "leeloo.h"

namespace LeelooDb
{

typedef struct _NoDbPolicy
{
}NoDbPolicy;
template <typename TdbPolicy = NoDbPolicy>
class Db : public Leeloo::Base<>, public NoDbPolicy, public Leeloo::LockPolicy
{
    typedef Leeloo::GuardLock<Leeloo::LockPolicy> Lock;
public:
    Db(){}
    virtual ~Db()
    {
        std::map<std::string, Leeloo::Data<>*>::iterator iter;
        for(iter=m_map.begin();iter!=m_map.end();iter++)
        {
            delete iter->second;
        }
    }
public:
    Leeloo::Ret Get(std::auto_ptr<std::string> keySmart, std::auto_ptr<Leeloo::Data<>>& valSmart)
    {
        if(!(keySmart.get() != NULL))
            return Leeloo::PARAINVALID_ERRORCODE;
        valSmart.reset(NULL);
        Lock lock(*this);
        std::map<std::string, Leeloo::Data<>*>::iterator iter = m_map.find(*keySmart);
        if(iter == m_map.end())
            return Leeloo::FAIL;
        Leeloo::Data<>* pDataOri = iter->second;
        Leeloo::Int dataLen;
        const char* pData = pDataOri->GetData(dataLen);
        valSmart = std::auto_ptr<Leeloo::Data<>>(new Leeloo::Data<>);
        valSmart.reset(new Leeloo::Data<>(dataLen));
        valSmart->Append(pData, dataLen);
        return Leeloo::SUCCESS;
    }
    Leeloo::Ret Set(std::auto_ptr<std::string> keySmart, std::auto_ptr<Leeloo::Data<>> valSmart)
    {
        if(!(keySmart.get() != NULL && valSmart.get() != NULL))
            return Leeloo::PARAINVALID_ERRORCODE;
        Lock lock(*this);
        std::map<std::string, Leeloo::Data<>*>::iterator iter = m_map.find(*keySmart);
        if(iter != m_map.end())
        {
            delete iter->second;
            m_map.erase(iter);
        }
        m_map.insert(std::pair<std::string, Leeloo::Data<>*>(*keySmart, valSmart.release()));
        return Leeloo::SUCCESS;
    }
    Leeloo::Ret Del(std::auto_ptr<std::string> keySmart)
    {
        if(!(keySmart.get() != NULL))
            return Leeloo::PARAINVALID_ERRORCODE;
        Lock lock(*this);
        std::map<std::string, Leeloo::Data<>*>::iterator iter = m_map.find(*keySmart);
        if(iter == m_map.end())
            return Leeloo::FAIL;
        delete iter->second;
        m_map.erase(iter);
        return Leeloo::SUCCESS;
    }
private:
    Db(Db& ref){}
    Db& operator=(Db& ref){return *this;}
private:
    std::map<std::string, Leeloo::Data<>*> m_map;
};
typedef Leeloo::Singleton<Db<>> DbSingleton;

}

#endif