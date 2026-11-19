# Windows EXE 代码签名说明

## 项目构建信息

- 项目：NetDiskDownloader
- 输出类型：Windows GUI EXE
- 平台：Win32（x86）
- 配置：Release
- 工具集：Visual Studio C++ v143
- C++ 标准：C++17
- Release 运行库：/MT（静态链接 MSVC 运行库）
- 版本资源：`NetDiskDownloader.rc`

## 本地构建

1. 使用 Visual Studio 打开 `NetDiskDownloader.sln`。
2. 选择 `Release` 和 `Win32`。
3. 执行“生成解决方案”。
4. 在项目输出目录找到 `NetDiskDownloader.exe`。

本项目当前未附带签名证书或私钥。不要把 `.pfx`、私钥、密码或访问令牌提交到 Git 仓库。

## 本地验证签名

在安装 Windows SDK（包含 SignTool）的 Windows 开发者命令提示符中运行：

```bat
signtool verify /pa /v NetDiskDownloader.exe
```

未签名时验证失败是预期结果。签名后还应检查证书链、时间戳和文件哈希。

## SignPath Foundation 申请准备

- 将符合其资格要求的完整源码放到公开 GitHub 仓库。
- 仓库保留本项目的 MIT LICENSE，并确认所有第三方依赖/资源的许可证兼容。
- 提交项目申请并等待 SignPath Foundation 审核。
- 获批后，按 SignPath 当前文档创建项目、签名策略和 GitHub 构建集成。
- 只允许可信构建流程提交待签名产物；不要在普通开发机上保存或导出签名私钥。

申请资格、免费服务范围、构建集成步骤及可用证书类型以 SignPath 官方当前条款和文档为准。
