#include <algorithm>
#include "DownloadService.h"
#include "../client/ApiClient.h"
#include "../client/HttpDownloader.h"
#include "../util/FileUtil.h"
#include <windows.h>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

bool RunDownload(const std::wstring& code,const std::wstring& root,
                 std::function<void(const DownloadProgress&)> cb,std::wstring& err) {
 const int pageSize=50;
 int pages=0;
 long long total=0;
 std::atomic<int> completedTotal{0};
 int pageNum=1;
 bool hasFirstPage=false;
 DownloadProgress progress;
 progress.totalFiles=0;
 progress.completedFiles=0;
 progress.bytesPerSecond=0;
 progress.status=L"正在获取第 1 页下载列表";
 if(cb) cb(progress);

 while(true) {
  std::vector<DownloadInfo> list;
  int returnedPages=0;
  long long returnedTotal=0;
  if(!FetchInfoPage(code,pageNum,pageSize,list,returnedPages,returnedTotal,err)) return false;

  if(!hasFirstPage) {
   pages=returnedPages; total=returnedTotal; hasFirstPage=true;
   if(total<=0 || pages<=0 || list.empty()) { err=L"下载码错误或已过期"; return false; }
   if(total>100000) { err=L"文件数量异常，已停止下载"; return false; }
   progress.totalFiles=(int)total;
   progress.files.resize((size_t)total);
   for(auto& f:progress.files) f.status=L"等待获取";
  } else if(returnedPages!=pages || returnedTotal!=total) {
   err=L"分页期间接口总数发生变化，请重新获取下载列表"; return false;
  }

  const size_t offset=(size_t)(pageNum-1)*pageSize;
  if(offset+list.size()>progress.files.size()) { err=L"接口分页记录数量超出总数"; return false; }
  {
   for(size_t i=0;i<list.size();++i) {
    progress.files[offset+i].filePath=list[i].filePath;
    progress.files[offset+i].status=L"排队中";
   }
   progress.status=L"已获取第 "+std::to_wstring(pageNum)+L" 页，开始下载";
   if(cb) cb(progress);
  }

  // 总文件数超过 50 时，每页最多使用 5 个线程；否则单线程。
  const unsigned int threadCount=total>50?5:1;
  std::atomic<size_t> next{0};
  std::atomic<int> completedPage{0};
  std::atomic<bool> failed{false};
  std::mutex stateMutex,errorMutex;
  std::wstring firstError;

  auto worker=[&]() {
   for(;;) {
    size_t index=next.fetch_add(1);
    if(index>=list.size()) break;
    const auto& item=list[index];
    const size_t globalIndex=offset+index;
    std::wstring target,localErr;
    bool ok=false;
    {
     std::lock_guard<std::mutex> lock(stateMutex);
     progress.files[globalIndex].status=L"下载中 0%";
     progress.files[globalIndex].percent=0;
     progress.status=L"正在下载："+item.filePath;
     if(cb) cb(progress);
    }
    if(!MakeTargetPath(root,item.filePath,target,localErr) || !EnsureDirectories(target,localErr)) {
     ok=false;
    } else {
     std::wstring tmp=target+L".download_tmp";
     ok=DownloadFile(item.downloadUrl,tmp,[&](double v, int percent) {
      std::lock_guard<std::mutex> lock(stateMutex);
      progress.bytesPerSecond=v;
      if (percent >= 0) {
       progress.files[globalIndex].percent=percent;
       progress.files[globalIndex].status=L"下载中 "+std::to_wstring(percent)+L"%";
      } else {
       progress.files[globalIndex].status=L"下载中（进度未知）";
      }
      if(cb) cb(progress);
     },localErr);
     if(ok && !MoveFileExW(tmp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
      localErr=L"临时文件重命名失败";
      DeleteFileW(tmp.c_str());
      ok=false;
     }
    }
    if(!ok) {
     failed=true;
     std::lock_guard<std::mutex> e(errorMutex);
     if(firstError.empty()) firstError=item.filePath+L"："+localErr;
    }
    int donePage=++completedPage;
    int doneAll=++completedTotal;
    {
     std::lock_guard<std::mutex> lock(stateMutex);
     progress.files[globalIndex].status=ok?L"下载成功":L"下载失败";
     progress.files[globalIndex].percent=ok?100:progress.files[globalIndex].percent;
     progress.completedFiles=doneAll;
     progress.bytesPerSecond=0;
     progress.status=L"本页完成 "+std::to_wstring(donePage)+L"/"+
                     std::to_wstring(list.size())+L"，总进度 "+
                     std::to_wstring(doneAll)+L"/"+std::to_wstring(total);
     if(cb) cb(progress);
    }
   }
  };

  std::vector<std::thread> workers;
  unsigned int actualThreads=(unsigned int)std::min<size_t>(threadCount,list.size());
  for(unsigned int i=0;i<actualThreads;i++) workers.emplace_back(worker);
  for(auto& t:workers) t.join();

  if(failed) { err=firstError.empty()?L"本页部分文件下载失败：第 "+std::to_wstring(pageNum)+L" 页":firstError; return false; }
  // 当前页全部下载完成后，才请求下一页。
  if(pageNum>=pages) break;
  ++pageNum;
  {
   std::lock_guard<std::mutex> lock(stateMutex);
   progress.status=L"第 "+std::to_wstring(pageNum-1)+L" 页下载完成，正在获取第 "+std::to_wstring(pageNum)+L" 页";
   if(cb) cb(progress);
  }
 }

 if(completedTotal!=(int)total) {
  err=L"下载数量不完整，预期 "+std::to_wstring(total)+L" 个，实际完成 "+std::to_wstring((int)completedTotal)+L" 个";
  return false;
 }
 progress.completedFiles=(int)completedTotal;
 progress.bytesPerSecond=0;
 progress.status=L"全部下载完成";
 if(cb) cb(progress);
 return true;
}
