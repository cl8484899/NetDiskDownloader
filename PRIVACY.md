# Privacy Policy

**Project:** NetDiskDownloader (网盘下载器)  
**Maintainer:** GitHub user `cl848489`  
**Backend API:** `https://www.fileora.cn/netdisk/getDownloadInfo`

## Information handled by the application

NetDiskDownloader does not run background telemetry or analytics. It does not send information merely because the application is opened.

When you enter a download code and explicitly start a download, the application sends the download code and pagination parameters (`pageNum` and `pageSize`) to the backend API over HTTPS. The API uses the request to obtain the file information and returns file metadata and download URLs. The application then downloads the requested files from those URLs and saves them in the folder you selected.

## Storage and logs

The maintainer states that the backend does not intentionally retain download codes, client IP addresses, or download-history records. The backend's web server, reverse proxy, CDN, security services, and hosting provider may nevertheless receive connection metadata, including the source IP address, as part of normal network communication. Their access, security, and diagnostic logs must be configured and reviewed separately; this policy should only be relied on while the actual infrastructure configuration matches it.

## Third-party download hosts

The files are downloaded from the URLs returned by the backend. Those destination servers may receive ordinary connection information, such as your IP address, when your device connects to them. Their own privacy practices are outside this project's control.

## User choice

You can avoid sending a download code by not starting a download. The application does not need an account sign-in for the described download request.

## Contact and updates

For questions or privacy concerns, please open an issue in the [GitHub repository](https://github.com/cl848489/NetDiskDownloader). This policy may be updated when the application's data handling or backend infrastructure changes.
