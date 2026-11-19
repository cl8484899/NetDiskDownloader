#include "FileUtil.h"
#include <windows.h>
static std::wstring parent(const std::wstring& p){auto n=p.find_last_of(L"\\/");return n==p.npos?L"":p.substr(0,n);}
bool MakeTargetPath(const std::wstring& root,const std::wstring& rel,std::wstring& out,std::wstring& err){
 if(rel.empty()||rel[0]==L'/'||rel[0]==L'\\'||(rel.size()>1&&rel[1]==L':')){err=L"接口文件路径必须是相对路径";return false;}
 std::wstring r=rel;for(auto& c:r)if(c==L'/')c=L'\\';
 size_t s=0;while(s<=r.size()){size_t e=r.find(L'\\',s);auto part=r.substr(s,e==r.npos?r.npos:e-s);if(part.empty()||part==L"."||part==L".."||part.find_first_of(L"<>:\"|?*")!=part.npos){err=L"接口文件路径包含不安全字符";return false;}if(e==r.npos)break;s=e+1;}
 out=root+(root.back()==L'\\'?L"":L"\\")+r;return true;
}
bool EnsureDirectories(const std::wstring& file,std::wstring& err){
 auto p=parent(file);if(p.empty())return true;
 std::wstring cur;
 if(p.size()>2&&p[1]==L':'){cur=p.substr(0,3);size_t i=3;while(i<=p.size()){size_t e=p.find(L'\\',i);auto part=p.substr(i,e==p.npos?p.npos:e-i);if(!part.empty()){if(cur.back()!=L'\\')cur+=L'\\';cur+=part;CreateDirectoryW(cur.c_str(),nullptr);}if(e==p.npos)break;i=e+1;}}
 else {cur=L"";size_t i=0;while(i<=p.size()){size_t e=p.find(L'\\',i);auto part=p.substr(i,e==p.npos?p.npos:e-i);if(!part.empty()){cur+=L"\\"+part;CreateDirectoryW(cur.c_str(),nullptr);}if(e==p.npos)break;i=e+1;}}
 DWORD a=GetFileAttributesW(p.c_str());if(a==INVALID_FILE_ATTRIBUTES){err=L"创建目录失败";return false;}return true;
}
