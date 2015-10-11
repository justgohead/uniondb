#include "leeloo.h"

namespace Leeloo
{

LockPolicy::LockPolicy()
{
#ifdef LEELOO_WINDOWS_OS
    ::InitializeCriticalSection(&m_mtx_);
#else
    pthread_mutex_init(&m_mtx_, NULL);
#endif
}
LockPolicy::~LockPolicy()
{
#ifdef LEELOO_WINDOWS_OS
    ::DeleteCriticalSection(&m_mtx_);
#else
    pthread_mutex_destroy(&m_mtx_);
#endif
}
void LockPolicy::Lock(void)
{
#ifdef LEELOO_WINDOWS_OS
    ::EnterCriticalSection(&m_mtx_);
#else
    pthread_mutex_lock(&m_mtx_);
#endif
}
void LockPolicy::Unlock(void)
{
#ifdef LEELOO_WINDOWS_OS
    ::LeaveCriticalSection(&m_mtx_);
#else
    pthread_mutex_unlock(&m_mtx_);
#endif
}

volatile Int Global::m_runned = LEELOO_RUN_GLOBAL;
bool Global::IsRun(void)
{
    if(m_runned == LEELOO_RUN_GLOBAL)
        return true;
    return false;
}
void Global::SetRun(void)
{
    m_runned = LEELOO_RUN_GLOBAL;
}
void Global::SetStop(void)
{
    m_runned = LEELOO_STOP_GLOBAL;
}

Config::Config()
{}
Config::~Config()
{}
Ret Config::ReadConfig(std::string configName)
{
    std::ifstream file;
    file.open(configName.c_str(), std::ios_base::in);
    if(!file.is_open())
    {
        std::cout << "Config::ReadConfig(), fopen " << configName << " error, set default config" << std::endl;
        m_keyValMap.insert(std::pair<std::string, std::string>(LEELOO_LOGLEVEL_KEY_CONFIG, LEELOO_DEFAULT_LOGLEVEL_VAL_CONFIG));
    }
    else
    {
        std::string stdstrLine;
        bool bKey = false;
        std::string stdstrKey;
        while(true)
        {
            file >> stdstrLine;
            if(file.fail())
                break;
            if(stdstrLine.length() > 0 && stdstrLine.at(0) == '#')
                continue;
            if(bKey)
                bKey = false;
            else
                bKey = true;
            if(bKey)
                stdstrKey = stdstrLine;
            else
                m_keyValMap.insert(std::pair<std::string, std::string>(stdstrKey, stdstrLine));
        }
        file.close();
    }
    return SUCCESS;
}
Ret Config::GetConfig(std::string key, std::string& val)
{
    std::map<std::string, std::string>::iterator iter;
    iter = m_keyValMap.find(key);
    if(iter != m_keyValMap.end())
    {
        val = iter->second;
        return SUCCESS;
    }
    return FAIL;
}

Log::Log()
{
    m_file.open("log.txt", std::ios_base::out);
}
Log::~Log()
{
    if(m_file.is_open())
        m_file.close();
}
Ret Log::WriteLog(std::string strLog, std::string strLevel, std::string file, std::string line, std::string function)
{
    std::string strLogLevel = LEELOO_DEFAULT_LOGLEVEL_VAL_CONFIG;
    ConfigSingleton::Instance().GetConfig(LEELOO_LOGLEVEL_KEY_CONFIG, strLogLevel);
    strLogLevel = LEELOO_NOLOG_LEVEL_LOG;
    if(atoi(strLevel.c_str()) < atoi(strLogLevel.c_str()))
        return SUCCESS;
    switch(atoi(strLevel.c_str()))
    {
    case 0:
        strLevel = "debug";
        break;
    case 1:
        strLevel = "warn";
        break;
    case 2:
        strLevel = "error";
        break;
    case 3:
        strLevel = "nolog";
        break;
    default:
        strLevel = "nolog";
        break;
    }
    Lock lock(*this);
    if(m_file.is_open())
    {
        m_file << strLevel << "   " << file << "   " << line << "   " << function << " : " << strLog << std::endl;
        //fflush
    }
    return SUCCESS;
}

IdManager::IdManager():m_topFreeId(0)
{}
IdManager::~IdManager()
{}
IdType IdManager::GetFreeId(void)
{
    IdType id = INVALID_VALID;
    std::auto_ptr<IdType> idSmart = m_freeIdQueue.Pop();
    if(idSmart.get() != NULL)
    {
        return *idSmart;
    }
    if(m_topFreeId == MAX_VALID)
        return id;
    //id = m_topFreeId;
    //m_topFreeId ++;
    id = LEELOO_INCREMENT(&m_topFreeId)
    return id;
}
void IdManager::FreeFreeId(IdType id)
{
    std::auto_ptr<IdType> idSmart(new Leeloo::IdType());
    if(idSmart.get() != NULL)
    {
        *idSmart = id;
        m_freeIdQueue.Push(idSmart);
    }
}

}
