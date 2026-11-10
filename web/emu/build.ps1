# SPDX-License-Identifier: GPL-3.0-only
# web/emu/build.sh from Windows: the browser emulator (Emscripten), built inside Crawl's build container.
#   [$env:FM_VERSION = "0.1";] .\web\emu\build.ps1   ->  build/emu/{index.html, worklet.js, crawl.wasm}
& "$PSScriptRoot\..\..\tools\docker_run.ps1" sh web/emu/build.sh @args
exit $LASTEXITCODE
