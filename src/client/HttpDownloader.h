#pragma once
#include <string>
#include <functional>
// 回调参数：实时速度（字节/秒）、单文件下载百分比（未知文件大小时为 -1）
bool DownloadFile(const std::wstring&, const std::wstring&,
                  std::function<void(double, int)>, std::wstring&);
