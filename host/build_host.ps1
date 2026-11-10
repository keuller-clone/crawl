# SPDX-License-Identifier: GPL-3.0-only
# host/build_host.sh from Windows: the host simulator, built inside Crawl's build container. The binary
# (build/host/crawl_host) is a Linux program: run it there too, e.g.
#   tools\docker_run.ps1 build/host/crawl_host tests/scenarios/first.zen build/scenarios/first
& "$PSScriptRoot\..\tools\docker_run.ps1" sh host/build_host.sh @args
exit $LASTEXITCODE
