#include "HttpDownloader.h"
#include <windows.h>
#include <winhttp.h>
#include <cstdlib>

bool DownloadFile(const std::wstring& url, const std::wstring& tmp,
                  std::function<void(double, int)> progress, std::wstring& err) {
    URL_COMPONENTS u{};
    u.dwStructSize = sizeof(u);
    u.dwHostNameLength = (DWORD)-1;
    u.dwUrlPathLength = (DWORD)-1;
    u.dwExtraInfoLength = (DWORD)-1;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &u)) {
        err = L"下载 URL 无效";
        return false;
    }

    std::wstring host(u.lpszHostName, u.dwHostNameLength);
    std::wstring path(u.lpszUrlPath, u.dwUrlPathLength);
    if (u.dwExtraInfoLength) path.append(u.lpszExtraInfo, u.dwExtraInfoLength);

    HINTERNET session = WinHttpOpen(L"NetDiskDownloader/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!session) {
        err = L"WinHTTP 初始化失败";
        return false;
    }

    HINTERNET connection = WinHttpConnect(session, host.c_str(), u.nPort, 0);
    HINTERNET request = connection ? WinHttpOpenRequest(connection, L"GET", path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        u.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0) : nullptr;

    bool ok = false;
    HANDLE file = INVALID_HANDLE_VALUE;
    if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(request, nullptr)) {
        DWORD statusCode = 0, statusSize = sizeof(statusCode);
        if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX)
            && statusCode >= 200 && statusCode < 300) {

            // 尝试读取 Content-Length；服务器未提供时，单文件百分比显示为“计算中”。
            ULONGLONG contentLength = 0;
            wchar_t lengthText[64]{};
            DWORD lengthSize = sizeof(lengthText);
            if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH,
                WINHTTP_HEADER_NAME_BY_INDEX, lengthText, &lengthSize, WINHTTP_NO_HEADER_INDEX)) {
                contentLength = _wcstoui64(lengthText, nullptr, 10);
            }

            file = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file != INVALID_HANDLE_VALUE) {
                ok = true;
                char buffer[65536];
                DWORD available = 0, received = 0, written = 0;
                ULONGLONG totalBytes = 0, speedBytes = 0, lastTick = GetTickCount64();
                int lastPercent = -2;
                if (progress) progress(0, contentLength ? 0 : -1);

                for (;;) {
                    if (!WinHttpQueryDataAvailable(request, &available)) {
                        ok = false; err = L"读取网络数据失败"; break;
                    }
                    if (!available) break;
                    DWORD wanted = available > sizeof(buffer) ? sizeof(buffer) : available;
                    if (!WinHttpReadData(request, buffer, wanted, &received) || !received) {
                        ok = false; err = L"接收文件数据失败"; break;
                    }
                    if (!WriteFile(file, buffer, received, &written, nullptr) || written != received) {
                        ok = false; err = L"写入文件失败"; break;
                    }
                    totalBytes += received;
                    speedBytes += received;

                    int percent = contentLength
                        ? (int)((totalBytes * 100) / contentLength) : -1;
                    if (percent > 100) percent = 100;
                    ULONGLONG now = GetTickCount64();
                    if (progress && (percent != lastPercent || now - lastTick >= 1000)) {
                        double speed = (now > lastTick)
                            ? (double)speedBytes * 1000.0 / (double)(now - lastTick) : 0.0;
                        progress(speed, percent);
                        lastPercent = percent;
                        speedBytes = 0;
                        lastTick = now;
                    }
                }
                FlushFileBuffers(file);
                CloseHandle(file);
                file = INVALID_HANDLE_VALUE;
                if (ok && progress) progress(0, 100);
            } else {
                err = L"创建临时文件失败";
            }
        } else {
            err = L"下载 HTTP 状态码：" + std::to_wstring(statusCode);
        }
    } else {
        err = L"下载请求失败，错误码：" + std::to_wstring(GetLastError());
    }

    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    if (!ok) DeleteFileW(tmp.c_str());
    if (progress) progress(0, ok ? 100 : -1);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return ok;
}
