# 网盘下载器 C++ Win32 源码

## 功能

-   原生 Win32
    GUI：下载码输入、选择保存目录、开始下载、总进度、文件数、速度。
-   API：`GET https://www.fileora.cn/netdisk/getDownloadInfo?downloadCode={downloadCode}&pageNum={pageNum}&pageSize={pageSize}`。
-   只接受业务 `code == 200`；读取 `data[]` 的 `filePath` 和
    `downloadUrl`。
-   按页下载；当文件总数超过 50 时，每页最多使用 5 个工作线程，否则使用
    1 个线程。不支持断点续传。
-   文件写入 `.download_tmp`，成功后再替换正式文件。
-   32 位 Win32 工程，可在 32 位 Windows 运行，也可在 64 位 Windows 通过
    WOW64 运行。

## 编译

Visual Studio 2022 安装"使用 C++ 的桌面开发"，打开
`NetDiskDownloader.sln`，选择 `Release | Win32`，生成解决方案。Release
配置使用 `/MT` 静态运行库，链接 WinHTTP、Common
Controls、Shell32、Ole32。

## 兼容性

工程设置 `WINVER` 和 `_WIN32_WINNT` 为 `0x0601`。但项目默认工具集为
VS2022 v143；这不代表最终二进制已验证可在 Windows 7 上运行。若必须支持
Win7 SP1，需要使用兼容的工具集/SDK重新构建，并在真实 Win7 SP1
上测试。HTTPS 可用性还受 Win7 补丁、根证书和 TLS
设置影响；程序不关闭证书验证。

## 路径约束

API 的 `filePath` 必须是相对路径；拒绝绝对路径、`..` 和 Windows
非法文件名字符，防止写出用户选定的目录。下载失败会停止后续任务。当前总体进度按完成文件数计算（API
未提供文件大小）。

## 当前版本限制

未实现取消、断点续传、并发下载、日志和单文件字节级总进度。此源码包尚未在
Windows/MSVC 环境实际编译或运行验证；EXE 体积 1～5 MB 仍需编译后实测。

本地服务说明：如果当前版本没有使用本机 `localhost` 服务，请勿将
`localhost` 描述为客户端依赖；客户端的远程 API 地址为
`https://www.fileora.cn/netdisk/getDownloadInfo`。

## 界面说明

当接口业务 `code` 为 200
但没有可下载文件时，提示"下载码错误或已过期"；底部状态栏显示当前下载文件路径，文本过长时由
Windows 静态文本控件以省略号截断。

## 版本信息与代码签名

项目新增 `NetDiskDownloader.rc`，为 EXE
写入产品名称、文件说明、内部名称及版本号（当前为 1.0.0.0）。版本资源中的
`CompanyName` 是产品/项目标识，不代表数字证书中的签名主体身份。

代码签名准备说明见
[SIGNING.md](https://github.com/cl8484899/NetDiskDownloader/blob/main/SIGNING.md)。项目没有内置证书或私钥；签名需在满足资格审核的代码签名服务中完成。不要将
PFX、私钥、密码或签名令牌提交到仓库。

项目的签名范围、维护者角色和联网行为见 [Code signing
policy](https://github.com/cl8484899/NetDiskDownloader/blob/main/CODE_SIGNING_POLICY.md)。用户数据处理说明见
[隐私政策](https://github.com/cl8484899/NetDiskDownloader/blob/main/PRIVACY.md)。项目依赖独立运营的远程商业后端
API；该后端不包含在客户端源码仓库中，也不随客户端 EXE 分发。

如果实际下载架构是客户端直接访问 API 返回的 `downloadUrl`
下载文件，而后端不代理或中转文件内容，可保留并使用以下说明：文件内容由客户端根据
API 返回的 `downloadUrl` 直接下载，后端 API 不代理或中转文件内容。
