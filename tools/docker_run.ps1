# SPDX-License-Identifier: GPL-3.0-only
# Run a command in Crawl's build container (Dockerfile) with this tree mounted at /work. The build
# scripts are ZEN-1's, unchanged; this only launches the container from Windows (BUILDING.md).
#   tools\docker_run.ps1 sh build.sh [--release X.Y]
#   tools\docker_run.ps1 build/host/crawl_host tests/scenarios/first.zen build/scenarios/first
#   tools\docker_run.ps1 -Rebuild            rebuild the image (it is also built on first use)
# Passed through when set: FM_VERSION, OM_CDC, ZN_HUD, OM_JOBS, CC, PYTHON, TESTING_PKG, ZEN_CJK_FONT, SITE_SHA.
# CRAWL_IMAGE names the image (default crawl-build).
[CmdletBinding()]
param(
    [switch]$Rebuild,
    [Parameter(ValueFromRemainingArguments = $true)][string[]]$Command
)
# Not "Stop": Windows PowerShell 5.1 turns a native command's stderr (docker's progress) into an error
# record when the caller redirects it; exit codes are checked instead.
$ErrorActionPreference = "Continue"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$image = "crawl-build"
if ($env:CRAWL_IMAGE) { $image = $env:CRAWL_IMAGE }

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Host "docker_run: docker is not on PATH: install Docker Desktop (https://www.docker.com/products/docker-desktop/)"
    exit 1
}
docker info *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host "docker_run: Docker is not running: start Docker Desktop"
    exit 1
}
docker image inspect $image *> $null
$have = ($LASTEXITCODE -eq 0)
if ($Rebuild -or -not $have) {
    Write-Host "docker_run: building the image $image from $root\Dockerfile (the first time takes a few minutes)"
    docker build -t $image $root
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
if (-not $Command) { exit 0 }

$dockerArgs = @("run", "--rm", "-v", "${root}:/work", "-w", "/work")
foreach ($name in @("FM_VERSION", "OM_CDC", "ZN_HUD", "OM_JOBS", "CC", "PYTHON", "TESTING_PKG", "ZEN_CJK_FONT", "SITE_SHA")) {
    $value = [Environment]::GetEnvironmentVariable($name)
    if ($value) { $dockerArgs += @("-e", "$name=$value") }
}
$dockerArgs += $image
$dockerArgs += $Command
& docker @dockerArgs
exit $LASTEXITCODE
