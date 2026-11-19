#pragma once
#include <string>
#include <functional>
#include "../model/DownloadProgress.h"
bool RunDownload(const std::wstring&,const std::wstring&,std::function<void(const DownloadProgress&)>,std::wstring&);
