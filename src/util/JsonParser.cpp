#include "JsonParser.h"
#include <windows.h>
#include <cstdlib>
#include <map>
struct J {
 enum T{N,S,NUM,O,A} t=N; std::wstring s; std::map<std::wstring,J> o; std::vector<J> a;
};
class P {
 const std::string& x; size_t i=0;
 void ws(){while(i<x.size() && (unsigned char)x[i]<=32)++i;}
 bool str(std::wstring& out){
  ws(); if(i>=x.size()||x[i++]!='"')return false;
  std::string b;
  while(i<x.size()){
   unsigned char c=x[i++];
   if(c=='"'){ if(!b.empty()){int n=MultiByteToWideChar(CP_UTF8,0,b.data(),(int)b.size(),nullptr,0); std::wstring w(n,0); MultiByteToWideChar(CP_UTF8,0,b.data(),(int)b.size(),&w[0],n);out+=w;} return true; }
   if(c=='\\'){ if(i>=x.size())return false; char e=x[i++]; if(e=='u'){ if(i+4>x.size())return false; wchar_t v=0; for(int k=0;k<4;k++){char h=x[i++];v=(wchar_t)(v*16+(h>='0'&&h<='9'?h-'0':h>='a'&&h<='f'?h-'a'+10:h>='A'&&h<='F'?h-'A'+10:0));} if(!b.empty()){int n=MultiByteToWideChar(CP_UTF8,0,b.data(),(int)b.size(),nullptr,0);std::wstring w(n,0);MultiByteToWideChar(CP_UTF8,0,b.data(),(int)b.size(),&w[0],n);out+=w;b.clear();} out+=v; } else { switch(e){case 'n':b+='\n';break;case 'r':b+='\r';break;case 't':b+='\t';break;default:b+=e;} } }
   else b+=(char)c;
  } return false;
 }
 bool val(J& v){
  ws(); if(i>=x.size())return false;
  if(x[i]=='"'){v.t=J::S;return str(v.s);}
  if(x[i]=='{'){++i;v.t=J::O;ws();if(i<x.size()&&x[i]=='}'){++i;return true;}do{std::wstring k;if(!str(k))return false;ws();if(i>=x.size()||x[i++]!=':')return false;J z;if(!val(z))return false;v.o[k]=std::move(z);ws();if(i<x.size()&&x[i]==','){++i;continue;}break;}while(true);ws();if(i>=x.size()||x[i++]!='}')return false;return true;}
  if(x[i]=='['){++i;v.t=J::A;ws();if(i<x.size()&&x[i]==']'){++i;return true;}do{J z;if(!val(z))return false;v.a.push_back(std::move(z));ws();if(i<x.size()&&x[i]==','){++i;continue;}break;}while(true);ws();if(i>=x.size()||x[i++]!=']')return false;return true;}
  size_t s=i;while(i<x.size()&&(isdigit((unsigned char)x[i])||x[i]=='-'))++i;
  if(i>s){v.t=J::NUM;v.s.assign(x.begin()+s,x.begin()+i);return true;}
  return false;
 }
public: P(const std::string& s):x(s){} bool parse(J& v){if(!val(v))return false;ws();return i==x.size();}
};
bool ParseDownloadInfoJson(const std::string& json,int& code,std::wstring& desc,
                           std::vector<DownloadInfo>& out,int& pageNum,int& pageSize,
                           int& pages,long long& total,std::wstring& err){
 J r;if(!P(json).parse(r)||r.t!=J::O){err=L"JSON 格式无效";return false;}
 auto c=r.o.find(L"code");if(c==r.o.end()||c->second.t!=J::NUM){err=L"响应缺少 code";return false;}
 code=wcstol(c->second.s.c_str(),nullptr,10);
 auto d=r.o.find(L"desc");if(d!=r.o.end()&&d->second.t==J::S)desc=d->second.s;
 auto data=r.o.find(L"data");if(data==r.o.end()||data->second.t!=J::O){err=L"响应 data 不是分页对象";return false;}
 auto number=[&](const wchar_t* key,long long& v)->bool{
   auto it=data->second.o.find(key);if(it==data->second.o.end()||it->second.t!=J::NUM)return false;
   v=_wtoi64(it->second.s.c_str());return true;
 };
 long long pn=0,ps=0,pg=0,tt=0;
 if(!number(L"pageNum",pn)||!number(L"pageSize",ps)||!number(L"pages",pg)||!number(L"total",tt)){
   err=L"分页信息缺失或格式错误";return false;
 }
 pageNum=(int)pn;pageSize=(int)ps;pages=(int)pg;total=tt;
 auto a=data->second.o.find(L"records");
 if(a==data->second.o.end()||a->second.t!=J::A){err=L"data.records 不是数组";return false;}
 for(auto& v:a->second.a){
   auto f=v.o.find(L"filePath"),u=v.o.find(L"downloadUrl");
   if(f!=v.o.end()&&u!=v.o.end()&&f->second.t==J::S&&u->second.t==J::S&&!f->second.s.empty()&&!u->second.s.empty())
     out.push_back({f->second.s,u->second.s});
 }
 return true;
}
