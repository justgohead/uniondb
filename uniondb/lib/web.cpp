#include "web.h"

namespace LeelooNet
{

Leeloo::Ret HttpProtocol::ParseHead(char* pHttpHead, Leeloo::Int len, Leeloo::Int& totalPacket)
{
	//?????
	//Int bufLen;
	//char* pBuf = (char*)in.GetData(&bufLen);
    //pBuf[bufLen] = '\0';
    //{
    //    char* pData = (char*)pBuf;
    //    *(pData + bufLen) = 0;
    //}
    //
	assert(pHttpHead != NULL && len > 0);
    totalPacket = len;
    char cLast = pHttpHead[len-1];
    //pHttpHead[len-1] = '\0';
	//????? 判断不全面,上传数据和POST数据完整性没有判断
	Leeloo::Int requestLen = len;
	char* pRequest = (char*)pHttpHead;
	char* pCrlf = strstr(pRequest, ZTLIB_CRLF_FLAG_PROTOCOLHTTP);
	if(pCrlf != NULL)
	{
		char* pUri = strstr(pRequest, ZTLIB_URI_FLAG_PROTOCOLHTTP);
		if(pUri != NULL && pUri < pCrlf)
		{
			char* pHttp = strstr(pUri, ZTLIB_HTTP_FLAG_PROTOCOLHTTP);
			if(pHttp != NULL && pHttp < pCrlf)
			{
				char* pPutMethod = strstr(pRequest, ZTLIB_POST_METHOD_PROTOCOLHTTP);
				if(pPutMethod != NULL && pPutMethod == pRequest)
				{
					//?????判断上传长度
                    char* pStart = strstr(pHttpHead, "Content-Length:");
                    if(pStart != NULL)
                    {
                        char* pEnd = strstr(pStart, "\r");
                        if(pEnd == NULL)
                        {
                            pEnd = strstr(pStart, "\n");
                        }
                        //if(pEnd - pStart < 8192)
                        if(pEnd != NULL)
                        {
                            int length = atoi(pStart+strlen("Content-Length:"));
                            //
                            pStart = strstr(pStart, "\r\n\r\n");
                            if(pStart != NULL)
                            {
                                pStart += 4;
                                //pStart = strstr(pStart, "Content-Type");
                                //if(pStart != NULL)
                                {
                                //    pStart = strstr(pStart, "\r\n\r\n");
                                //    if(pStart != NULL)
                                    {
                                //        pStart += 4;
                                        totalPacket = length + (pStart - pHttpHead);
                                        if(len - (pStart - pHttpHead) >= length)
                                        {
                                            return Leeloo::SUCCESS;
                                        }
                                    }
                                }
                            }
                        }
                    }
				}
				else
				{
                    pPutMethod = strstr(pRequest, ZTLIB_GET_METHOD_PROTOCOLHTTP);
				    if(pPutMethod != NULL && pPutMethod == pRequest)
                    {
					    return Leeloo::SUCCESS;
                    }
                    else
                    {
                        return Leeloo::SUCCESS;
                    }
				}
			}
			else
			{
				//没有HTTP，结束接收
				return Leeloo::SUCCESS;
			}
		}
		else
		{
			//没有URI，结束接收
			return Leeloo::SUCCESS;
		}
	}
	else
	{
		//没有换行但较长，结束接收
		//if(len > ZTLIB_ONCEIO_REQUEST_WEBSERVER)
		//{
			return Leeloo::SUCCESS;
		//}
	}
    return Leeloo::FAIL;
}
Leeloo::Ret HttpProtocol::Decode(HttpData& in, HttpInfo& inExt, HttpInfo& out, HttpInfo& outExt)
{
    out.Clear();
	Leeloo::Int requestLen;
	size_t posCrlf, posUri, posHttp, posTemp;
	//InfoItem val;
	//const char* pRequest = in.GetData(&requestLen);
    char* pRequest = (char*)in.GetData(requestLen);
    //pRequest[requestLen] = '\0';
	//
	if(pRequest != NULL && requestLen > 0)
	{
		std::string strRequest = std::string(pRequest, requestLen);
		posCrlf = strRequest.find(ZTLIB_CRLF_FLAG_PROTOCOLHTTP);
		posUri = strRequest.find(ZTLIB_URI_FLAG_PROTOCOLHTTP);
		if(posUri != std::string::npos)
		{
			posHttp = strRequest.find(ZTLIB_HTTP_FLAG_PROTOCOLHTTP);
		}
		if(posCrlf != std::string::npos && posUri != std::string::npos && posHttp != std::string::npos && posUri < posCrlf && posHttp < posCrlf)
		{
			//decode method
			posTemp = strRequest.find(ZTLIB_SPACE_FLAG_PROTOCOLHTTP);
			if(posTemp != std::string::npos)
			{
				std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
				itemSmart->m_strVal = strRequest.substr(0, posTemp);
				out.SetInfo(ZTLIB_METHOD_INFO_PROTOCOLHTTP, itemSmart);
			}
			//decode uri
			posTemp = strRequest.find(ZTLIB_SPACE_FLAG_PROTOCOLHTTP, posUri+1);
			if(posTemp != std::string::npos)
			{
				std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
				itemSmart->m_strVal = strRequest.substr(posUri+1, posTemp-(posUri+1));
				out.SetInfo(ZTLIB_URI_INFO_PROTOCOLHTTP, itemSmart);
			}
			//decode httpver
			posTemp = strRequest.find(ZTLIB_CRLF_FLAG_PROTOCOLHTTP, posHttp+1);
			if(posTemp != std::string::npos)
			{
				std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
				itemSmart->m_strVal = strRequest.substr(posHttp+1, posTemp-(posHttp+1));
				out.SetInfo(ZTLIB_HTTPVER_INFO_PROTOCOLHTTP, itemSmart);
			}
			//解码上传数据和post参数
			//out.GetInfo(ICEARK_METHOD_INFO_PROTOCOLHTTP, val);
			Leeloo::InfoItem* pItem = (Leeloo::InfoItem*)out.GetInfo(ZTLIB_METHOD_INFO_PROTOCOLHTTP);
			if(pItem != NULL && pItem->m_strVal.compare(ZTLIB_POST_METHOD_PROTOCOLHTTP) == 0)
			{
                //char* pBuf = (char*)strRequest.c_str();
                //char* pStart = (char*)strstr(pBuf, "Content-Length:");
                char* pStart = (char*)strstr(pRequest, "Content-Length:");
                if(pStart != NULL)
                {
                    char* pEnd = strstr(pStart, "\r");
                    if(pEnd == NULL)
                    {
                        pEnd = strstr(pStart, "\n");
                    }
                    if(pEnd - pStart < 8192)
                    {
                        int length = atoi(pStart+strlen("Content-Length:"));
                        //
                        pStart = strstr(pStart, "\r\n\r\n");
                        if(pStart != NULL)
                        {
                            pStart += 4;
                            pStart = strstr(pStart, "Content-Type");
                            if(pStart != NULL)
                            {
                                pStart = strstr(pStart, "\r\n\r\n");
                                if(pStart != NULL)
                                {
                                    pStart += 4;
                                    std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
                                    //?????length空间太大会出错
                                    if(length > 0 && length < ZTLIB_MAXLEN_REQUEST_WEBSERVER)
									    itemSmart->m_strVal = std::string(pStart, length);
									out.SetInfo(ZTLIB_POSTDATA_INFO_PROTOCOLHTTP, itemSmart);
                                }
                            }
                        }
                    }
                }
			}
		}
	}
    return Leeloo::SUCCESS;
}
Leeloo::Ret HttpProtocol::Encode(HttpInfo& in, HttpInfo& inExt, HttpData& out, HttpInfo& outExt)
{
    out.Clear();
	Leeloo::InfoItem* pItem = (Leeloo::InfoItem*)in.GetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP);
	Leeloo::Int elementNumner;
	char* pStr = (char*)pItem->m_dataExt.GetData(elementNumner);
    out.SetBuffer(elementNumner);
	out.Append(pStr, elementNumner);
    return Leeloo::SUCCESS;
}
const char* g_pResStartHtml = "HTTP/1.1 200 OK\r\nServer: uniondb.net/2.0\r\nX-Powered-By: uniondb.net\r\nDate: Sun, 06 Jul 2015 11:01:21 GMT\r\nContent-Type: text/html\r\nAccept-Ranges: bytes\r\nLast-Modified: Wed, 02 Jul 2010 01:01:26 GMT\r\nETag: \"0f71527dfdbc81:ade\"\r\nContent-Length: ";//46\r\n\r\n<html><head></head><body>adfasfa</body></html>";
const char* g_pResStartExe = "HTTP/1.1 200 OK\r\nServer: uniondb.net/2.0\r\nX-Powered-By: uniondb.net\r\nDate: Sun, 06 Jul 2015 11:01:21 GMT\r\nContent-Type: application/exe\r\nAccept-Ranges: bytes\r\nLast-Modified: Wed, 02 Jul 2010 01:01:26 GMT\r\nETag: \"0f71527dfdbc81:ade\"\r\nContent-Length: ";
const char* g_pResStartIamge = "HTTP/1.1 200 OK\r\nServer: uniondb.net/2.0\r\nX-Powered-By: uniondb.net\r\nDate: Sun, 06 Jul 2015 11:01:21 GMT\r\nContent-Type: Image/jpg\r\nAccept-Ranges: bytes\r\nLast-Modified: Wed, 02 Jul 2010 01:01:26 GMT\r\nETag: \"0f71527dfdbc81:ade\"\r\nContent-Length: ";
//const char* g_pResIndexHtml = "<html><head></head><body>首页</body></html>";
void ResponseByFile(std::string fileName, std::string& res, std::string type)
{
    char strFilelength[MAX_PATH];
    if(type == "txt")
        res = g_pResStartHtml;
    else
    if(type == "exe")
        res = g_pResStartExe;
    else
        res = g_pResStartIamge;
    //
    //struct _stat info;
    //_stat(fileName.c_str(), &info);
    //int size = info.st_size;
    //sprintf(strFilelength, "%d\r\n\r\n", size);
    //res += strFilelength;
    //
    //res += strResponse;
    size_t lenReaded;
    const int BUFLEN = 8192;
    char filebuf[BUFLEN+2];
    //
    FILE* fp = fopen(fileName.c_str(), "rb");
    if(fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        sprintf(strFilelength, "%d\r\n\r\n", size);
        res += strFilelength;
        fseek(fp, 0, SEEK_SET);
        while(true)
        {
            lenReaded = fread(filebuf, 1, BUFLEN, fp);
            if(lenReaded == 0)
                break;
            res.append(filebuf, lenReaded);
            if(lenReaded < BUFLEN)
            {
                break;
            }
        }
        fclose(fp);
    }
    else
    {
        int size = 0;
        sprintf(strFilelength, "%d\r\n\r\n", size);
        res += strFilelength;
    }
}
Leeloo::Ret HttpDisposer::Dispose(HttpData& data, HttpInfo& in, HttpInfo& inExt, HttpInfo& out, HttpInfo& outExt)
{
    out.Clear();
    //
    Leeloo::InfoItem* pItemMethod = (Leeloo::InfoItem*)in.GetInfo(ZTLIB_METHOD_INFO_PROTOCOLHTTP);
    Leeloo::InfoItem* pItemUri = (Leeloo::InfoItem*)in.GetInfo(ZTLIB_URI_INFO_PROTOCOLHTTP);
    std::auto_ptr<Leeloo::InfoItem> resVal(new Leeloo::InfoItem);
    resVal->m_strVal = "";
    if(pItemMethod != NULL && pItemUri != NULL)
    {
        if(pItemUri->m_strVal == "")
        {
            pItemUri->m_strVal = "/";
        }
        if(pItemUri->m_strVal == "/")
        {
	        pItemUri->m_strVal += "index.htm";
        }
        transform(pItemMethod->m_strVal.begin(), pItemMethod->m_strVal.end(), pItemMethod->m_strVal.begin(), ::toupper);
        if(pItemMethod->m_strVal.compare(ZTLIB_GET_METHOD_PROTOCOLHTTP) == 0)
        {
            if(pItemUri->m_strVal == "/index.htm")
            {
                if(pItemUri->m_strVal.at(0) == '/')
                    pItemUri->m_strVal = pItemUri->m_strVal.substr(1);
                ResponseByFile(pItemUri->m_strVal, resVal->m_strVal, "txt");
                std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
                itemSmart->m_dataExt.SetBuffer(resVal->m_strVal.length());
                itemSmart->m_dataExt.Append((char*)resVal->m_strVal.c_str(), resVal->m_strVal.length());
                out.SetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP, itemSmart);
			    return Leeloo::SUCCESS;
            }
            else
            if(pItemUri->m_strVal.find(".htm") != std::string::npos || pItemUri->m_strVal.find(".css") != std::string::npos || pItemUri->m_strVal.find(".xml") != std::string::npos)
            {//txt
                if(pItemUri->m_strVal.at(0) == '/')
                    pItemUri->m_strVal = pItemUri->m_strVal.substr(1);
                ResponseByFile(pItemUri->m_strVal, resVal->m_strVal, "txt");
                std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
                itemSmart->m_dataExt.SetBuffer(resVal->m_strVal.length());
                itemSmart->m_dataExt.Append((char*)resVal->m_strVal.c_str(), resVal->m_strVal.length());
                out.SetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP, itemSmart);
			    return Leeloo::SUCCESS;
            }
            else
            if(pItemUri->m_strVal.find(".zip") != std::string::npos || pItemUri->m_strVal.find(".exe") != std::string::npos || pItemUri->m_strVal.find(".apk") != std::string::npos)
			{//exe,apk
                if(pItemUri->m_strVal.at(0) == '/')
                    pItemUri->m_strVal = pItemUri->m_strVal.substr(1);
				ResponseByFile(pItemUri->m_strVal, resVal->m_strVal, "exe");
                std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
                itemSmart->m_dataExt.SetBuffer(resVal->m_strVal.length());
                itemSmart->m_dataExt.Append((char*)resVal->m_strVal.c_str(), resVal->m_strVal.length());
                out.SetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP, itemSmart);
			    return Leeloo::SUCCESS;
			}
            else
            {//下载其他图片文件
                if(pItemUri->m_strVal.at(0) == '/')
                    pItemUri->m_strVal = pItemUri->m_strVal.substr(1);
                ResponseByFile(pItemUri->m_strVal, resVal->m_strVal, "img");
                std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
                itemSmart->m_dataExt.SetBuffer(resVal->m_strVal.length());
                itemSmart->m_dataExt.Append((char*)resVal->m_strVal.c_str(), resVal->m_strVal.length());
                out.SetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP, itemSmart);
			    return Leeloo::SUCCESS;
            }
        }
        //else
        //if(pItemMethod->m_strVal.compare(ZTLIB_POST_METHOD_PROTOCOLHTTP) == 0)
        //{}
    }
    {
	    std::auto_ptr<Leeloo::InfoItem> itemSmart(new Leeloo::InfoItem);
	    const char* res = "HTTP/1.0 200 OK\r\n\r\n<html><body>解析请求失败，我们无法响应您的请求!<BR>Decode request fail, We can't response your request!</body></html>\r\n\r\n";
	    itemSmart->m_dataExt.Append((char*)res, strlen(res));
	    out.SetInfo(ZTLIB_RESPONSE_INFO_PROTOCOLHTTP, itemSmart);
    }
    return Leeloo::SUCCESS;
}

WebServer::WebServer(void)
{}
WebServer::~WebServer(void)
{}
Leeloo::Ret WebServer::Handle(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans)
{
    switch(pPerIO->m_nOperationType)  // 通过per-I/O数据中的nOperationType域查看什么I/O请求完成了
    {
    case LEELOO_READ_OP_NET: // 完成一个接收请求
        {
            HttpProtocol protocol;
            HttpData dataIn;//, dataOut;
            HttpInfo infoInExt, infoOutExt;
            Leeloo::Int totalPacket;
            pPerIO->m_buf[dwTrans] = '\0';
            if(protocol.ParseHead(pPerIO->m_buf, dwTrans, totalPacket) == Leeloo::SUCCESS)
            {
                dataIn.SetBuffer(dwTrans+2);
                dataIn.Append(pPerIO->m_buf, dwTrans);
                dataIn.Append("\0", 1);
                Leeloo::KnowledgeFramework< HttpData, HttpInfo, HttpProtocol, HttpDisposer, Leeloo::ReqresPolicy > knowledgeFramework;
                if(knowledgeFramework.Think(dataIn, infoInExt, pPerIO->m_dataSendTemp, infoOutExt) == Leeloo::SUCCESS)
                {
                    pPerIO->m_dataSendTemp.Seek();
                    Leeloo::Int numReaded;
                    pPerIO->m_dataSendTemp.Read(pPerIO->m_buf, LEELOO_BUFFER_SIZE_NET, numReaded);
                    WSABUF buf;
                    buf.buf = pPerIO->m_buf;
                    buf.len = numReaded;
                    pPerIO->m_nOperationType = LEELOO_WRITE_OP_NET;
                    DWORD nFlags = 0;
                    int ret = ::WSASend(pPerHandle->m_s, &buf, 1, &dwTrans, 0, &pPerIO->m_ol, NULL);
                    if (ret == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSA_IO_PENDING)
                        {
                            // 继续投递接收I/O请求
                            WSABUF buf;
                            buf.buf = pPerIO->m_buf;
                            buf.len = LEELOO_BUFFER_SIZE_NET;
                            pPerIO->m_nOperationType = LEELOO_READ_OP_NET;
                            DWORD nFlags = 0;
                            ::WSARecv(pPerHandle->m_s, &buf, 1, &dwTrans, &nFlags, &pPerIO->m_ol, NULL);
                            ::closesocket(pPerHandle->m_s);
                        }
                    }
                }
            }
            else
            {
                // 继续投递接收I/O请求
                WSABUF buf;
                buf.buf = pPerIO->m_buf;
                buf.len = LEELOO_BUFFER_SIZE_NET;
                pPerIO->m_nOperationType = LEELOO_READ_OP_NET;
                DWORD nFlags = 0;
                ::WSARecv(pPerHandle->m_s, &buf, 1, &dwTrans, &nFlags, &pPerIO->m_ol, NULL);
                ::closesocket(pPerHandle->m_s);
            }
        }
        break;
    case LEELOO_WRITE_OP_NET:
        {
            Leeloo::Int numReaded;
            if(pPerIO->m_dataSendTemp.Read(pPerIO->m_buf, LEELOO_BUFFER_SIZE_NET, numReaded) == Leeloo::SUCCESS)
            {
                WSABUF buf;
                buf.buf = pPerIO->m_buf;
                buf.len = numReaded;
                pPerIO->m_nOperationType = LEELOO_WRITE_OP_NET;
                DWORD nFlags = 0;
                int ret = ::WSASend(pPerHandle->m_s, &buf, 1, &dwTrans, 0, &pPerIO->m_ol, NULL);
                if (ret == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING)
                    {
                        // 继续投递接收I/O请求
                        WSABUF buf;
                        buf.buf = pPerIO->m_buf;
                        buf.len = LEELOO_BUFFER_SIZE_NET;
                        pPerIO->m_nOperationType = LEELOO_READ_OP_NET;
                        DWORD nFlags = 0;
                        ::WSARecv(pPerHandle->m_s, &buf, 1, &dwTrans, &nFlags, &pPerIO->m_ol, NULL);
                        ::closesocket(pPerHandle->m_s);
                    }
                }
            }
            else
            {
                // 继续投递接收I/O请求
                WSABUF buf;
                buf.buf = pPerIO->m_buf;
                buf.len = LEELOO_BUFFER_SIZE_NET;
                pPerIO->m_nOperationType = LEELOO_READ_OP_NET;
                DWORD nFlags = 0;
                ::WSARecv(pPerHandle->m_s, &buf, 1, &dwTrans, &nFlags, &pPerIO->m_ol, NULL);
            }
        }
    //case LEELOO_ACCEPT_OP_NET:
    //    break;
    }
    return Leeloo::SUCCESS;
}

}
