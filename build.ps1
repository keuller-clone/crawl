# SPDX-License-Identifier: GPL-3.0-only
# build.sh from Windows: the same build, inside Crawl's build container (BUILDING.md).
#   .\build.ps1 [--release X.Y]     ->  build/crawl.fwsc (or build/crawl-X.Y.fwsc)
& "$PSScriptRoot\tools\docker_run.ps1" sh build.sh @args
exit $LASTEXITCODE
