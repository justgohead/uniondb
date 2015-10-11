#ifndef _LEELOOWEB_H_
#define _LEELOOWEB_H_

#include "net.h"

namespace LeelooNet
{

typedef Leeloo::Data<char> HttpData;
typedef Leeloo::Info<> HttpInfo;
#define ZTLIB_CRLF_FLAG_PROTOCOLHTTP "\r\n"
#define ZTLIB_DCRLF_FLAG_PROTOCOLHTTP "\r\n\r\n"
#define ZTLIB_URI_FLAG_PROTOCOLHTTP " /"
#define ZTLIB_HTTP_FLAG_PROTOCOLHTTP " HTTP"
#define ZTLIB_SPACE_FLAG_PROTOCOLHTTP " "
#define ZTLIB_QUESTION_FLAG_PROTOCOLHTTP "?"
#define ZTLIB_EQU_FLAG_PROTOCOLHTTP "="
#define ZTLIB_AND_FLAG_PROTOCOLHTTP "&"
//
#define ZTLIB_METHOD_INFO_PROTOCOLHTTP "/request/method"
#define ZTLIB_URI_INFO_PROTOCOLHTTP "/request/uri"
#define ZTLIB_HTTPVER_INFO_PROTOCOLHTTP "/request/httpver"
#define ZTLIB_POSTDATA_INFO_PROTOCOLHTTP "/request/postdata"
//#define ZTLIB_IP_EXTRA_INFO_PROTOCOLHTTP "/request/ext/ip"
#define ZTLIB_ALIVE_EXTRA_INFO_PROTOCOLHTTP "/request/ext/alive"
#define ZTLIB_RESPONSE_INFO_PROTOCOLHTTP "/response/response"
//
#define ZTLIB_GET_METHOD_PROTOCOLHTTP "GET"
#define ZTLIB_POST_METHOD_PROTOCOLHTTP "POST"
//
#define ZTLIB_START_RESPONSE_PROTOCOLHTTP "HTTP/1.0 200 OK\r\n\r\n"
#define ZTLIB_END_RESPONSE_PROTOCOLHTTP "\r\n\r\n"
//
#define ZTLIB_MAXLEN_REQUEST_WEBSERVER 1024*1024
//
class HttpProtocol : public Leeloo::Protocol<HttpData, HttpInfo>
{
public:
    HttpProtocol(){}
    virtual ~HttpProtocol(){}
	Leeloo::Ret ParseHead(char* pHttpHead, Leeloo::Int len, Leeloo::Int& totalPacket);
    Leeloo::Ret Decode(HttpData& in, HttpInfo& inExt, HttpInfo& out, HttpInfo& outExt);
    Leeloo::Ret Encode(HttpInfo& in, HttpInfo& inExt, HttpData& out, HttpInfo& outExt);

private:
    HttpProtocol(HttpProtocol& ref){}
    HttpProtocol& operator=(HttpProtocol& ref){return *this;}
private:
};
class HttpDisposer : public Leeloo::Disposer<HttpData, HttpInfo>
{
public:
    HttpDisposer(){}
    virtual ~HttpDisposer(){}
    Leeloo::Ret Dispose(HttpData& data, HttpInfo& in, HttpInfo& inExt, HttpInfo& out, HttpInfo& outExt);
private:
    HttpDisposer(HttpDisposer& ref){}
    HttpDisposer& operator=(HttpDisposer& ref){return *this;}
private:
};

class WebServer : public NetServer
{
public:
    WebServer(void);
    virtual ~WebServer(void);
    virtual Leeloo::Ret Handle(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans);
private:
    WebServer(WebServer& ref){}
    WebServer& operator=(WebServer& ref){return *this;}
protected:
    
};
typedef Leeloo::Singleton<WebServer> WebServerSingleton;

}

#endif