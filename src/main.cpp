#include "ui/MainWindow.h"
int WINAPI wWinMain(HINSTANCE h,HINSTANCE,PWSTR,int){MainWindow w;if(!w.Create(h))return 1;MSG m{};while(GetMessageW(&m,0,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return (int)m.wParam;}
