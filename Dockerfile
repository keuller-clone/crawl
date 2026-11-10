# SPDX-License-Identifier: GPL-3.0-only
# Crawl's build image: one Linux x86-64 container with everything ZEN-1's build scripts expect
# (BUILDING.md, docs/dev-environment.md), so that build.sh, host/build_host.sh, web/emu/build.sh and
# tests/run_tests.sh run unchanged inside it. The *.ps1 wrappers launch it from Windows
# (tools/docker_run.ps1 builds the image on first use: `docker build -t crawl-build .`).
#
# Inside the container the host is Linux x86-64, so build.sh takes ZEN-1's native path: the JieLi
# toolchain runs directly (no container inside the container).
FROM emscripten/emsdk:6.0.13

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential git curl ca-certificates xz-utils diffutils file python3 python3-pip \
        libraqm0 libfribidi0 ffmpeg \
    && rm -rf /var/lib/apt/lists/*

# Pillow's wheels bundle libraqm but load the system's fribidi (libfribidi0 above; gen_font.py needs raqm's
# OpenType features), numpy and scipy for the tools, mido for the
# Python tools' self-tests. python-rtmidi is left out: USB stays on Windows (CLAUDE.md).
RUN pip3 install --no-cache-dir --break-system-packages "Pillow>=10" numpy scipy mido \
    || pip3 install --no-cache-dir "Pillow>=10" numpy scipy mido

# The JieLi Linux toolchain, by the project's own script, to /opt/jieli/toolchain.
COPY tools/get_toolchain.sh /tmp/get_toolchain.sh
RUN sh /tmp/get_toolchain.sh /opt/jieli && rm /tmp/get_toolchain.sh

# The three AC79 SDK files every package needs (BUILDING.md), from the tested SDK tag, checked
# against the hashes in tools/build.py (SDK_SHA256).
ARG SDK_TAG=AC79NN_SDK_V1.2.1_2023-12-13
RUN set -e; d=/opt/ac79sdk/cpu/wl82/tools; mkdir -p "$d/cfg"; \
    for f in uboot.boot cfg_tool.bin cfg/eq_cfg_hw.bin; do \
        curl -fL --retry 3 -o "$d/$f" "https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/raw/$SDK_TAG/cpu/wl82/tools/$f"; \
    done; \
    cd "$d" && printf '%s\n' \
        "4e3b4c220dc96641cb5a723f41e68ce41d5261ae9434bb33fbd7f2c59976ded4  uboot.boot" \
        "276579954f076886a6a7694f65dc71c034a63a2c204b76749065c0ac7b010d1b  cfg_tool.bin" \
        "41167491bffed4651750719c973d2758adeb9021a5670d02d6a53c85ed80ea7d  cfg/eq_cfg_hw.bin" \
        | sha256sum -c -

# The source tree is a Windows folder mounted at /work: git must accept its owner, and file modes
# are not meaningful there (tools/guard_platform.sh runs git diff).
RUN git config --system --add safe.directory '*' && git config --system core.fileMode false

ENV JIELI_TOOLCHAIN=/opt/jieli/toolchain \
    AC79_SDK=/opt/ac79sdk \
    PYTHONDONTWRITEBYTECODE=1
WORKDIR /work
