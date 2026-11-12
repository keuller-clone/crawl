#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The manual's screenshots (MANUAL.md): tools/manual.zen on the host simulator, into docs/img/manual, then each
# PNG packed (the simulator writes them unpacked). Built as a release is (ZN_HUD=0: no testing tools, the status
# card without its raw battery reading), then the host built back as the tests have it, as tools/site_gallery.sh.
set -e
cd "$(dirname "$0")/.."
CC="${CC:-cc} -DZN_HUD=0" sh host/build_host.sh >/dev/null
mkdir -p docs/img/manual
build/host/crawl_host tools/manual.zen docs/img/manual | grep -v "^  ok" | tail -8
rm -f docs/img/manual/midi_out.txt
python3 tools/png_pack.py docs/img/manual/*.png
sh host/build_host.sh >/dev/null
