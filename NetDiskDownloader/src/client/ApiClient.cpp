#include "ApiClient.h"
#include "../util/JsonParser.h"
#include <windows.h>
#include <winhttp.h>
static bool request(const std::wstring& url,std::string& body,std::wstring& err){
 URL_COMPONENTS u{};u.dwStructSize=sizeof(u);u.dwHostNameLength=(DWORD)-1;u.dwUrlPathLength=(DWORD)-1;u.dwExtraInfoLength=(DWORD)-1;
 if(!WinHttpCrackUrl(url.c_str(),0,0,&u)){err=L"API URL 无效";return false;}
 std::wstring host(u.lpszHostName,u.dwHostNameLength),path(u.lpszUrlPath,u.dwUrlPathLength);if(u.dwExtraInfoLength)path.append(u.lpszExtraInfo,u.dwExtraInfoLength);
 HINTERNET s=WinHttpOpen(L"NetDiskDownloader/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,0,0,0);if(!s){err=L"WinHTTP 初始化失败";return false;}
 HINTERNET c=WinHttpConnect(s,host.c_str(),u.nPort,0),r=c?WinHttpOpenRequest(c,L"GET",path.c_str(),0,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,u.nScheme==INTERNET_SCHEME_HTTPS?WINHTTP_FLAG_SECURE:0):0;
 bool ok=false;if(r&&WinHttpSendRequest(r,0,0,0,0,0,0)&&WinHttpReceiveResponse(r,0)){DWORD st=0,n=sizeof(st);if(WinHttpQueryHeaders(r,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,0,&st,&n,0)&&st==200){char b[8192];DWORD avail=0,got=0;ok=true;while(WinHttpQueryDataAvailable(r,&avail)&&avail){DWORD want=avail>sizeof(b)?sizeof(b):avail;if(!WinHttpReadData(r,b,want,&got)||!got){ok=false;break;}body.append(b,got);}}else err=L"API HTTP 状态码："+std::to_wstring(st);}else err=L"API 请求失败，错误码："+std::to_wstring(GetLastError());
 if(r)WinHttpCloseHandle(r);if(c)WinHttpCloseHandle(c);WinHttpCloseHandle(s);return ok;
}
bool FetchInfoPage(const std::wstring& code, int pageNum, int pageSize,
                   std::vector<DownloadInfo>& items,
                   int& pages, long long& total,
                   std::wstring& err){
 std::wstring url=L"https://www.fileora.cn/netdisk/getDownloadInfo?downloadCode="+code+
      L"&pageNum="+std::to_wstring(pageNum)+L"&pageSize="+std::to_wstring(pageSize);
 std::string body;
 if(!request(url,body,err))return false;
 int status=-1,returnedPage=0,returnedSize=0,returnedPages=0;
 long long returnedTotal=0;
 std::wstring desc;
 if(!ParseDownloadInfoJson(body,status,desc,items,returnedPage,returnedSize,returnedPages,returnedTotal,err))return false;
 if(status!=200){err=desc.empty()?L"接口业务 code 不是 200":desc;return false;}
 if(returnedPage!=pageNum){err=L"接口返回页码与请求页码不一致";return false;}
 if(returnedSize<=0 || returnedPages<0 || returnedTotal<0){err=L"接口分页信息无效";return false;}
 pages=returnedPages;
 total=returnedTotal;
 if(items.empty() && total>0){err=L"分页接口返回空记录，页码："+std::to_wstring(pageNum);return false;}
 return true;
}
