# Code signing policy

> Before applying to SignPath Foundation, replace `[YOUR_GITHUB_USERNAME]` below with the actual GitHub account that owns and maintains this repository. Do not leave placeholders in the public repository.

## Project and signing scope

- Project: NetDiskDownloader (网盘下载器)
- Source repository: `https://github.com/[YOUR_GITHUB_USERNAME]/NetDiskDownloader`
- Signed artifact: the project's own `NetDiskDownloader.exe`, built from this repository.
- Release source: the protected `main` branch and GitHub Releases created from reviewed source.
- The project will not submit third-party binaries or binaries built from unrelated source code for signing.

## Service disclosure

Free code signing provided by SignPath.io, certificate by SignPath Foundation.

If accepted, the publisher shown by the Authenticode certificate will be SignPath Foundation, not the individual maintainer. The signature links the published binary to the reviewed source repository and the verified build pipeline.

## Maintainer roles

- Author / maintainer / repository owner: `[YOUR_GITHUB_USERNAME]`
- Reviewer: `[YOUR_GITHUB_USERNAME]` for maintainer-authored changes; contributions from other people must be reviewed before merging.
- Signing approver: `[YOUR_GITHUB_USERNAME]`, responsible for confirming that a release is intended for publication and matches the reviewed source.

All team members must enable multi-factor authentication on GitHub and SignPath. If additional maintainers are added, update this section to identify their roles.

## User privacy

The application does not run background telemetry or analytics in the source reviewed for this policy. When a user enters a download code and starts a download, the application sends that code and pagination parameters over HTTPS to `https://www.fileora.cn/netdisk/getDownloadInfo`. It then downloads the files from the download URLs returned by that service, into the folder selected by the user. The application does not send this information to other networked systems unless the user initiates the download and the service provides the corresponding download URL.

## Release and signing controls

1. Release builds are produced by the GitHub Actions workflow in this repository using GitHub-hosted Windows runners.
2. The unsigned executable is uploaded as a GitHub Actions artifact before a signing request is submitted.
3. SignPath's trusted GitHub build integration must verify the source/build origin.
4. Every production release signing request is subject to the manual approval required by SignPath Foundation's OSS signing conditions.
5. The maintainer checks the signed artifact and publishes it with the corresponding source release.
6. Signing credentials and API tokens are stored only in GitHub Actions Secrets / Variables and are never committed to this repository.

## Reporting

Please report suspected malicious or incorrectly signed releases through the GitHub repository's Issues page. Do not include download codes, credentials, or personal information in public reports.
