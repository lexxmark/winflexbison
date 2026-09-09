master: [![master build status](https://ci.appveyor.com/api/projects/status/j9s6854v3pvig28t/branch/master?svg=true)](https://ci.appveyor.com/project/lexxmark/winflexbison-2rnxh/branch/master) &nbsp; dev: [![dev build status](https://ci.appveyor.com/api/projects/status/j9s6854v3pvig28t/branch/dev?svg=true)](https://ci.appveyor.com/project/lexxmark/winflexbison-2rnxh/branch/dev)

# WinFlexBison - Flex and Bison for Windows

WinFlexBison is a Windows port of [Flex (the fast lexical analyser)](https://github.com/westes/flex/) and [GNU Bison (parser generator)](https://www.gnu.org/software/bison/).
Both win_flex and win_bison are based on upstream sources but depend on system libraries only.

**NOTE**:
* 2.4.x versions include GNU Bison version 2.7
* 2.5.x versions include GNU Bison version 3.x.x

## License

WinFlexBison bundles four upstream projects, and both shipped executables are combined works that
include several of them:

* [Flex](https://github.com/westes/flex/) — BSD license
  ([flex/src/COPYING](https://github.com/lexxmark/winflexbison/blob/master/flex/src/COPYING),
  shipped in the release package as `COPYING.flex`)
* [GNU Bison](https://www.gnu.org/software/bison/) — GPLv3+
  ([bison/src/COPYING](https://github.com/lexxmark/winflexbison/blob/master/bison/src/COPYING),
  shipped as `COPYING.bison`)
* [GNU M4](https://www.gnu.org/software/m4/) — GPLv3+. Both tools run m4 over their skeletons;
  this port runs it in-process instead of as a child process, so M4 is linked into **win_flex.exe
  as well as win_bison.exe**
* [gnulib](https://www.gnu.org/software/gnulib/) — GPLv3+, a few files LGPL. Supplies the POSIX
  functions MSVC does not have, including the regular expression engine win_flex uses

Because GNU M4 and gnulib are linked into both executables, **win_flex.exe and win_bison.exe are
distributed under GPLv3+**, even though flex's own sources are under a BSD license. Everything
WinFlexBison itself adds — the Windows portability code, the build scripts, the custom build
rules — is GPLv3+ too. See [COPYING](COPYING) for the license text.

The scanners and parsers you *generate* with these tools are not covered by that. Flex's BSD
license places no copyleft on generated scanners, and the Bison skeletons carry the special
exception that lets you distribute a generated parser under terms of your choice; that exception
is reproduced at the top of every parser win_bison writes.

All documentation, especially the images under custom_build_rules/docs, is distributed under the
GNU Free Documentation License (FDL 1.3+). See [COPYING.DOC](COPYING.DOC).


## Downloads

https://github.com/lexxmark/winflexbison/releases provides stable versions.
To test non-released development versions see the artifacts provided by CI.


## Changelog

The release page includes the full Changelog but you may also see the [changelog.md](changelog.md) file.

## Build requirements

* Visual Studio 2017 or newer
* CMake


## Testing

The build badges above cover the 8 build jobs: VS2022 and VS2019, each x64 and
Win32, Release and Debug. Every one of them runs the Windows CTest gate — the
adapted flex suite, the bison compile-run and golden-diff tests, and our own
port-specific tests. `runtests.bat` runs the same gate locally and needs nothing
beyond Visual Studio and CMake.

On top of that, a **separate, non-gating CI job** runs the full **GNU Bison
Autotest suite — all 776 groups** — against `win_bison.exe` through an MSYS2
shell. Latest status:

| | |
|---|---|
| groups | 776 |
| passing | 694 |
| expected failures (xfail) | 12, each with a documented cause |
| skipped | 61 (Java/D tiers, and cases needing filenames NTFS forbids) |
| unexpected failures | none |

It is deliberately non-gating: it reports on upstream bison behaviour, and its
xfail set can shift with the worker's locale and toolchain, so it is a signal to
read rather than a gate to trip. The job appears in the
[AppVeyor build](https://ci.appveyor.com/project/lexxmark/winflexbison-2rnxh) as
the `WFB_JOB=autotest` cell — AppVeyor badges are per project rather than per
job, so it has no badge of its own. See
[`tests/bison-autotest/README.md`](tests/bison-autotest/README.md) for how to run
it and what each xfail means.


## HowTo

You may use win_flex and win_bison directly on the command line or [use them via CustomBuildRules in VisualStudio](custom_build_rules/README.md).


## Example flex/bison files

See https://github.com/meyerd/flex-bison-example
