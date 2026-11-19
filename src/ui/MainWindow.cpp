#include "MainWindow.h"
#include "../service/DownloadService.h"
#include "../util/FormatUtil.h"
#include <commctrl.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <thread>
#include <memory>

#define ID_CODE 101
#define ID_START 102
#define ID_FOLDER 103
#define ID_FOLDER_PATH 104
#define ID_FILE_LIST 105
#define WM_PROG (WM_APP+1)
#define WM_DONE (WM_APP+2)

struct UProg { DownloadProgress p; };
struct UDone { bool ok; std::wstring msg; };

bool MainWindow::Create(HINSTANCE inst) {
 INITCOMMONCONTROLSEX cc{sizeof(cc), ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES};
 InitCommonControlsEx(&cc);
 WNDCLASSW wc{};
 wc.lpfnWndProc=Proc; wc.hInstance=inst; wc.lpszClassName=L"NDDownloader";
 wc.hCursor=LoadCursor(0,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
 RegisterClassW(&wc);
 h_=CreateWindowW(wc.lpszClassName,L"网盘下载器",
     WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
     CW_USEDEFAULT,CW_USEDEFAULT,820,650,0,0,inst,this);
 if(!h_) return false;
 ShowWindow(h_,SW_SHOW); UpdateWindow(h_); return true;
}

LRESULT CALLBACK MainWindow::Proc(HWND h,UINT m,WPARAM w,LPARAM l) {
 auto p=(MainWindow*)GetWindowLongPtrW(h,GWLP_USERDATA);
 if(m==WM_NCCREATE) {
  p=(MainWindow*)((CREATESTRUCTW*)l)->lpCreateParams;
  SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)p); p->h_=h;
 }
 return p?p->Msg(m,w,l):DefWindowProcW(h,m,w,l);
}

void MainWindow::SetDefaultDownloadFolder() {
 PWSTR path=nullptr;
 if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads,0,nullptr,&path)) && path) {
  folder_=path; CoTaskMemFree(path);
 } else {
  wchar_t home[MAX_PATH]{};
  DWORD n=GetEnvironmentVariableW(L"USERPROFILE",home,MAX_PATH);
  if(n>0 && n<MAX_PATH) folder_=std::wstring(home)+L"\\Downloads";
 }
 SetWindowTextW(folderEdit_,folder_.c_str());
}

void MainWindow::SelectFolder() {
 BROWSEINFOW b{}; b.hwndOwner=h_; b.lpszTitle=L"请选择保存目录";
 b.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
 PIDLIST_ABSOLUTE id=SHBrowseForFolderW(&b);
 if(id) {
  wchar_t p[MAX_PATH]{};
  if(SHGetPathFromIDListW(id,p)) { folder_=p; SetWindowTextW(folderEdit_,folder_.c_str()); }
  CoTaskMemFree(id);
 }
}

void MainWindow::UpdateFileList(const std::vector<DownloadFileStatus>& files) {
 int count=ListView_GetItemCount(list_);
 while(count<(int)files.size()) {
  LVITEMW item{}; item.mask=LVIF_TEXT; item.iItem=count; item.pszText=(LPWSTR)L"等待获取";
  ListView_InsertItem(list_,&item); ++count;
 }
 while(count>(int)files.size()) { ListView_DeleteItem(list_,count-1); --count; }
 for(int i=0;i<(int)files.size();++i) {
  const std::wstring name=files[i].filePath.empty()?L"等待获取文件信息":files[i].filePath;
  const std::wstring state=files[i].status.empty()?L"等待中":files[i].status;
  ListView_SetItemText(list_,i,0,(LPWSTR)name.c_str());
  ListView_SetItemText(list_,i,1,(LPWSTR)state.c_str());
 }
}

void MainWindow::Start() {
 if(busy_) return;
 wchar_t buf[128]{}; GetWindowTextW(edit_,buf,128); std::wstring code=buf;
 if(code.empty()||code.find_first_not_of(L"0123456789")!=code.npos) {
  MessageBoxW(h_,L"请输入纯数字下载码",L"提示",MB_OK|MB_ICONINFORMATION); return;
 }
 if(folder_.empty()) { SelectFolder(); if(folder_.empty()) return; }

 // 安全检查：如果保存目录下已存在与下载码同名的目录，禁止直接覆盖。
 // 该目录可能包含用户已有文件，自动覆盖存在数据丢失或恶意文件覆盖风险。
 std::wstring codeFolder = folder_;
 if (!codeFolder.empty() && codeFolder.back() != L'\\' && codeFolder.back() != L'/')
  codeFolder += L"\\";
 codeFolder += code;
 DWORD folderAttr = GetFileAttributesW(codeFolder.c_str());
 if (folderAttr != INVALID_FILE_ATTRIBUTES &&
     (folderAttr & FILE_ATTRIBUTE_DIRECTORY)) {
  std::wstring warning = L"检测到目标文件夹已存在，请删除该文件夹或更换保存目录后重试"
      L"\r\n\r\n重复的文件夹：\r\n" + codeFolder;
  MessageBoxW(h_, warning.c_str(), L"下载目录冲突", MB_OK | MB_ICONWARNING);
  return;
 }
 busy_=true; EnableWindow(edit_,FALSE); EnableWindow(start_,FALSE); EnableWindow(choose_,FALSE);
 ListView_DeleteAllItems(list_);
 SendMessageW(bar_,PBM_SETPOS,0,0);
 SetWindowTextW(count_,L"文件：0/0"); SetWindowTextW(speed_,L"速度：0 B/s");
 SetWindowTextW(status_,L"正在获取下载信息...");
 HWND target=h_; auto root=folder_;
 std::thread([target,code,root]() {
  std::wstring e;
  bool ok=RunDownload(code,root,[target](const DownloadProgress& p) {
   auto* q=new UProg{p}; if(!PostMessageW(target,WM_PROG,0,(LPARAM)q)) delete q;
  },e);
  auto* q=new UDone{ok,ok?L"全部下载完成":e};
  if(!PostMessageW(target,WM_DONE,0,(LPARAM)q)) delete q;
 }).detach();
}

LRESULT MainWindow::Msg(UINT m,WPARAM w,LPARAM l) {
 switch(m) {
 case WM_CREATE:
  CreateWindowW(L"STATIC",L"下载码：",WS_CHILD|WS_VISIBLE,20,20,65,24,h_,0,0,0);
  edit_=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,85,17,300,28,h_,(HMENU)ID_CODE,0,0);
  start_=CreateWindowW(L"BUTTON",L"开始下载",WS_CHILD|WS_VISIBLE,400,16,110,30,h_,(HMENU)ID_START,0,0);
  CreateWindowW(L"STATIC",L"保存目录：",WS_CHILD|WS_VISIBLE,20,62,85,27,h_,0,0,0);
  folderEdit_=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_READONLY,110,59,575,27,h_,(HMENU)ID_FOLDER_PATH,0,0);
  choose_=CreateWindowW(L"BUTTON",L"选择文件夹",WS_CHILD|WS_VISIBLE,695,58,105,29,h_,(HMENU)ID_FOLDER,0,0);
  bar_=CreateWindowW(PROGRESS_CLASSW,L"",WS_CHILD|WS_VISIBLE,20,105,780,22,h_,0,0,0);
  SendMessageW(bar_,PBM_SETRANGE,0,MAKELPARAM(0,100));
  count_=CreateWindowW(L"STATIC",L"文件：0/0",WS_CHILD|WS_VISIBLE,20,137,190,24,h_,0,0,0);
  speed_=CreateWindowW(L"STATIC",L"速度：0 B/s",WS_CHILD|WS_VISIBLE,240,137,230,24,h_,0,0,0);
  status_=CreateWindowW(L"STATIC",L"请输入下载码",WS_CHILD|WS_VISIBLE|SS_ENDELLIPSIS,20,165,780,24,h_,0,0,0);
  list_=CreateWindowExW(WS_EX_CLIENTEDGE,WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,20,195,780,400,h_,(HMENU)ID_FILE_LIST,GetModuleHandleW(nullptr),0);
  ListView_SetExtendedListViewStyle(list_,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
  { LVCOLUMNW c{}; c.mask=LVCF_TEXT|LVCF_WIDTH; c.pszText=(LPWSTR)L"文件路径"; c.cx=620; ListView_InsertColumn(list_,0,&c); c.pszText=(LPWSTR)L"下载状态"; c.cx=140; ListView_InsertColumn(list_,1,&c); }
  SetDefaultDownloadFolder();
  return 0;
 case WM_COMMAND:
  if(LOWORD(w)==ID_FOLDER) { SelectFolder(); return 0; }
  if(LOWORD(w)==ID_START) { Start(); return 0; }
  break;
 case WM_PROG: {
  std::unique_ptr<UProg> q((UProg*)l); auto& p=q->p;
  int pct=p.totalFiles?p.completedFiles*100/p.totalFiles:0;
  SendMessageW(bar_,PBM_SETPOS,pct,0);
  SetWindowTextW(count_,(L"文件："+std::to_wstring(p.completedFiles)+L"/"+std::to_wstring(p.totalFiles)).c_str());
  SetWindowTextW(speed_,(L"速度："+SpeedText(p.bytesPerSecond)).c_str());
  SetWindowTextW(status_,p.status.c_str());
  UpdateFileList(p.files);
  return 0;
 }
 case WM_DONE: {
  std::unique_ptr<UDone> q((UDone*)l); busy_=false;
  EnableWindow(edit_,TRUE); EnableWindow(start_,TRUE); EnableWindow(choose_,TRUE);
  SetWindowTextW(status_,q->msg.c_str());
  if(q->ok) SendMessageW(bar_,PBM_SETPOS,100,0);
  else MessageBoxW(h_,q->msg.c_str(),L"下载失败",MB_OK|MB_ICONERROR);
  return 0;
 }
 case WM_DESTROY: PostQuitMessage(0); return 0;
 }
 return DefWindowProcW(h_,m,w,l);
}
