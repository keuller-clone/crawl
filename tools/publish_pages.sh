#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Build the GitHub Pages site for release VERSION (tools/make_pages.py: landing page, emulator, web
# installer, firmware download) and push it to the gh-pages branch of the repository, replacing what
# was there.
#   tools/publish_pages.sh 0.0.60          (after ./build.sh --release 0.0.60)
#   tools/publish_pages.sh --emu LABEL     the emulator alone, for testing by ear before there is a release
# The remote is $PAGES_REMOTE (default: origin), the repository named by REPO in tools/make_pages.py.
# From Windows: build the emulator in the container first (web\emu\build.ps1), then run this from Git Bash;
# without emcc here it publishes build/emu as it is. Python is $PYTHON (default: python3).
set -e
cd "$(dirname "$0")/.."
EMU=0
if [ "$1" = "--emu" ]; then
    EMU=1
    shift
fi
V="${1:?usage: tools/publish_pages.sh [--emu] VERSION}"
PKG="build/crawl-$V.fwsc"
if [ "$EMU" = 1 ]; then
    PKG="-"
else
    [ -f "$PKG" ] || { echo "no $PKG: run ./build.sh --release $V first"; exit 1; }
fi
if grep -q '^REPO = .*OWNER' tools/make_pages.py; then
    echo "set REPO in tools/make_pages.py to the public repository first"; exit 1
fi
if command -v emcc >/dev/null 2>&1; then
    FM_VERSION="$V" sh web/emu/build.sh
else
    [ -f build/emu/crawl.wasm ] || { echo "no emcc here and no build/emu: run web/emu/build.ps1 first"; exit 1; }
    echo "no emcc here: publishing build/emu as built"
fi
SITE_SHA="$(git rev-parse --short public 2>/dev/null || true)" "${PYTHON:-python3}" tools/make_pages.py "$PKG" "$V" build/pages
REMOTE="$(git remote get-url "${PAGES_REMOTE:-origin}")"
SHA="$(git rev-parse --short public 2>/dev/null || git rev-parse --short HEAD)"
cd build/pages
rm -rf .git
git init -q -b gh-pages
git add -A
git commit -q -m "Site for Crawl $V (from $SHA)"
git push -q -f "$REMOTE" gh-pages
rm -rf .git
# ask GitHub Pages to build it now: a push alone has left a first deploy stuck on "building"
REPO="$(printf %s "$REMOTE" | sed -E 's#(git@github.com:|https://github.com/)##; s#\.git$##')"
gh api -X POST "repos/$REPO/pages/builds" >/dev/null 2>&1 || echo "(could not ask GitHub Pages to build: check the repository's Pages settings)"
echo "pushed the site for Crawl $V to gh-pages"
