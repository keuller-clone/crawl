# SPDX-License-Identifier: GPL-3.0-only
# tools/guard_platform.sh from Windows: the platform guard, inside Crawl's build container (git runs there
# against this tree; it needs the fomni remote fetched, see BUILDING.md). Run it before every commit.
& "$PSScriptRoot\docker_run.ps1" sh tools/guard_platform.sh @args
exit $LASTEXITCODE
