#pragma once
#include <string>
#include <vector>

struct DownloadFileStatus {
    std::wstring filePath;
    std::wstring status;
    int percent = 0;
};

struct DownloadProgress {
    int completedFiles = 0;
    int totalFiles = 0;
    double bytesPerSecond = 0;
    std::wstring status;
    std::vector<DownloadFileStatus> files;
};
