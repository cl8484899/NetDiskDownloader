#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "../model/DownloadProgress.h"

class MainWindow {
 HWND h_=0,edit_=0,start_=0,choose_=0,folderEdit_=0,bar_=0,status_=0,count_=0,speed_=0,list_=0;
 std::wstring folder_;
 bool busy_=false;
 static LRESULT CALLBACK Proc(HWND,UINT,WPARAM,LPARAM);
 LRESULT Msg(UINT,WPARAM,LPARAM);
 void SelectFolder();
 void Start();
 void SetDefaultDownloadFolder();
 void UpdateFileList(const std::vector<DownloadFileStatus>& files);
public:
 bool Create(HINSTANCE);
};
