# SPDX-License-Identifier: GPL-3.0-only
# tests/run_tests.sh from Windows: the host tests, the simulator scenarios, the browser build and the
# tools' self-tests, inside Crawl's build container.
& "$PSScriptRoot\..\tools\docker_run.ps1" sh tests/run_tests.sh @args
exit $LASTEXITCODE
