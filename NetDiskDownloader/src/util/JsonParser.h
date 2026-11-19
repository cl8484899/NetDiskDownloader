#pragma once
#include <string>
#include <vector>
#include "../model/DownloadInfo.h"
bool ParseDownloadInfoJson(const std::string&, int&, std::wstring&, std::vector<DownloadInfo>&,
                           int& pageNum, int& pageSize, int& pages, long long& total, std::wstring&);
