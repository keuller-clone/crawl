# Decisions

Every design decision and every deviation from the brief (`CLAUDE.md`), with its date and reason. Newest last.

## 2026-11-10 M0

**Base and references.** Crawl's `main` is ZEN-1 1.2 (aceyprime/zen-1 `main`, commit `ec92952`) merged with its
full history under the brief's two commits. Remotes: `origin` (keuller-clone/crawl), `upstream-zen1`,
`upstream-grimm`, and `fomni` (the platform guard needs FoMni's commits). GR!MM v2.0.2 is the annotated tag
`refs/grimm/v2.0.2` (commit `b519acb`), a read-only reference: `git show refs/grimm/v2.0.2:firmware/...`. It is not
built and not in the working tree.

**The guard's FoMni commits are pinned under `refs/fomni/`.** The two commits `tools/guard_platform.sh` diffs
against (`b81eb6b`, the platform; `6c32623`, the installers) are no longer reachable from any FoMni branch or tag,
so `git fetch fomni` alone does not bring them. They are fetched by hash and kept as `refs/fomni/platform` and
`refs/fomni/installers` (BUILDING.md), so `git gc` cannot drop them. The guard script itself is unchanged.

**Package identity family: `FM-1_5xx`.** Dev builds are `FM-1_500`, release builds `FM-1_5XXYYZZ`
(`tools/build.py`, `firmware/src/main.c`, `tools/make_pages.py`). Reason: a Crawl build must not report itself as
a ZEN-1 build (ZEN-1 is `6xx`, FuMi-1 `7xx`, FoMni `8xx`, X0X and Felucca `9xx`; there is no public registry).
The installers treat every custom `FM-1_*` identity alike and the restore path keys on stock `FM-1_015`, so the
family does not change install or restore behavior. Steven's M0 flash is the test.

**What the rename touches.** User-visible strings (the splash, the home title, the welcome card, the crash and
safe-mode screens), the USB product name (`Crawl FM-1`, the same length as `ZEN-1 FM-1`, which is the one
platform line the guard allows to differ), the build outputs (`crawl.bin`, `crawl.fwsc`, `crawl-X.Y.fwsc`,
`crawl.wasm`, `build/host/crawl_host`), the emulator and bench pages' names, and the site tools' file names and
repository. Source file names (`zen_*.c/h`, `host/zen_host.c`, `web/zen_installer.html`), in-code identifiers and
comments that name ZEN-1 stay: renaming them is churn with no effect on the device, and the comments describe
where the code came from. The splash tagline (`Ambient scenes for the FM-1`) and the landing page, guide and
installer texts stay ZEN-1's until M3 and M5, since at M0 they are still true.

**The build runs in one container, from PowerShell.** `Dockerfile` makes one Linux x86-64 image
(`emscripten/emsdk:6.0.13` plus gcc, Python with Pillow, numpy, scipy and mido, the JieLi toolchain fetched by
`tools/get_toolchain.sh`, and the three AC79 SDK files checked by hash). Inside it the host is Linux, so
`build.sh` takes ZEN-1's native path and the JieLi tools run directly: no container inside the container.
`build.ps1`, `host/build_host.ps1`, `web/emu/build.ps1`, `tests/run_tests.ps1` and `tools/guard_platform.ps1`
only call `tools/docker_run.ps1`, which mounts the tree at `/work` and runs the same shell script. None of the
build logic is forked. The host simulator binary is a Linux program and runs through `tools/docker_run.ps1` too.
The three SDK files are downloaded into the image rather than cloned as the whole SDK: the package needs only
those (BUILDING.md), and `tools/build.py` already carries their hashes.

**Line endings.** `.gitattributes` forces LF in the working tree (`* text=auto eol=lf`, `*.ps1 text eol=crlf`):
the tree is mounted into Linux and the shell scripts would not run with CRLF.

**`tools/publish_pages.sh` commits as the configured git user.** ZEN-1's copy forced its author's name and
no-reply address on the gh-pages commit; publishing Crawl's site as AceyPrime would be wrong. Publishing itself is
M5.

## 2026-11-10 M1

**`scenario-ambient`'s `expect race 0` failure is accepted (Steven).** It is inherited from ZEN-1 and fails the
same way on an untouched ZEN-1 checkout in this container. `tests/run_tests.sh` is unchanged, so the run still
reports it; a milestone's "passing tests" means every test but this one until it is fixed upstream or here.

**The M0 flash is deferred (Steven).** Steven tests in the browser emulator first. M1 therefore has no hardware
figures of its own.

**M1's method.** Memory is measured from the M0 ELF (`tools/budget_map.py`). ZEN-1's engine CPU is measured on the
host (`tests/host/budget_bench.c`) and scaled to the FM-1 by the one hardware reading ZEN-1's source records
(8 koto voices with reverb: 38 %). GR!MM's parts were first sized by reading their source. Steven then allowed building GR!MM v2.0.2 in a scratch
checkout (`build/grimm-ref`, git-ignored) for measuring only: `tools/budget_grimm.py` times its sources and effects
in its own host mode, and its FM-1 ELF gives code sizes. It adds one printf to the scratch copy's host probe.
GR!MM stays out of Crawl's build. `docs/BUDGET.md` marks each figure measured, read or estimated.

**Mode B: no go for v1, provisionally**, on measured memory (`docs/BUDGET.md`). Mode A plans for 8 voices and a
145 KB pool budget for the effects until hardware says otherwise.

**`tools/docker_run.ps1` takes its arguments raw.** With a `param` block PowerShell bound the command's own flags
(`sh -c`) as its parameters, and with `$ErrorActionPreference = "Stop"` Windows PowerShell 5.1 turned docker's
progress on stderr into a fatal error when redirected.
