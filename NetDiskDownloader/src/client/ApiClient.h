#pragma once
#include <string>
#include <vector>
#include "../model/DownloadInfo.h"

// 获取指定页下载信息，同时返回分页元数据。
bool FetchInfoPage(const std::wstring& code, int pageNum, int pageSize,
                   std::vector<DownloadInfo>& items,
                   int& pages, long long& total,
                   std::wstring& err);
