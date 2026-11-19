# Code signing policy

## Project and signing scope

- Project: NetDiskDownloader (网盘下载器)
- Source repository: https://github.com/cl848489/NetDiskDownloader
- Project owner and maintainer: GitHub user `cl848489`
- Signed artifact: only `NetDiskDownloader.exe`, built from this public repository.
- Release source: reviewed source committed to the `main` branch and corresponding GitHub Releases.
- Third-party binaries and binaries built from unrelated or undisclosed source code will not be submitted for signing.

## Service disclosure

Free code signing provided by SignPath.io, certificate by SignPath Foundation (only if the project is accepted into the program).

The client communicates with a separately operated backend service at `https://www.fileora.cn/netdisk/getDownloadInfo` over HTTPS. The backend service is not included in this repository and its server-side source code is not bundled with or distributed as part of the Windows executable. The client sends a download code and pagination parameters only when the user initiates a download request. This remote-service dependency is disclosed here for eligibility review; it is not represented as open-source backend software.

## Maintainer roles

- Author / maintainer / repository owner: `cl848489`
- Reviewer: `cl848489` for maintainer-authored changes; contributions from others must be reviewed before merging.
- Signing approver: `cl848489`, responsible for confirming that a release is intended for publication and corresponds to reviewed source.

All team members must enable multi-factor authentication on GitHub and SignPath. If additional maintainers are added, this policy will be updated to identify their roles.

## User privacy

The client does not include background telemetry or analytics. When a user enters a download code and starts a download, the client sends that code and pagination parameters (`pageNum` and `pageSize`) over HTTPS to `https://www.fileora.cn/netdisk/getDownloadInfo`. The service processes the request and returns file metadata and download URLs. The client then downloads files from the returned URLs into the folder selected by the user.

The project maintainer states that the backend does not intentionally retain download codes, client IP addresses, or download-history records. The backend and hosting configuration must be maintained consistently with this statement. As with any network service, the server and its infrastructure necessarily receive connection metadata (including the source IP address) while handling HTTPS requests; access logs, reverse-proxy logs, CDN logs, security logs, and hosting-provider logs must be checked separately to ensure they are not retained or used for tracking contrary to this policy. See [PRIVACY.md](PRIVACY.md) for the user-facing disclosure.

## Release and signing controls

1. Release builds are produced by the GitHub Actions workflow in this repository using GitHub-hosted Windows runners.
2. The unsigned executable is uploaded as a GitHub Actions artifact before a signing request is submitted.
3. If accepted, SignPath's trusted build integration will be configured to verify the source/build origin.
4. Signing requests will follow SignPath Foundation's required approval process.
5. The maintainer checks the signed artifact and publishes it with the corresponding source release.
6. Signing credentials and API tokens are stored only in GitHub Actions Secrets / Variables and are never committed to this repository.

## Reporting

Please report suspected malicious or incorrectly signed releases through the GitHub repository's Issues page. Do not include download codes, credentials, or personal information in public reports.
