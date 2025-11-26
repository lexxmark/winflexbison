[![Build status](https://ci.appveyor.com/api/projects/status/58lcjnr0mb9uc8c8/branch/master?svg=true)](https://ci.appveyor.com/project/lexxmark/winflexbison/branch/master)

# WinFlexBison - Flex and Bison for Windows

WinFlexBison is a Windows port of [Flex (the fast lexical analyser)](https://github.com/westes/flex/) and [GNU Bison (parser generator)](https://www.gnu.org/software/bison/).
Both win_flex and win_bison are based on upstream sources but depend on system libraries only.

**NOTE**:
* 2.4.x versions include GNU Bison version 2.7
* 2.5.x versions include GNU Bison version 3.x.x

## License

Flex uses a [BSD license](flex/src/COPYING), GNU Bison is [licensed under the GNU General Public License (GPLv3+)](bison/src/COPYING).  
All build scripts in WinFlexBison are distributed under GPLv3+. See [COPYING](COPYING) for details.

All documentation, especially those under custom_build_rules/doc, is distributed under the GNU Free Documentation License (FDL 1.3+).


## Downloads

https://github.com/lexxmark/winflexbison/releases provides stable versions.
To test non-released development versions see the artifacts provided by CI.


## Changelog

The release page includes the full Changelog but you may also see the [changelog.md](changelog.md) file.

## Build requirements

* Visual Studio 2017 or newer
* CMake

* Clang-cl or Visual Studio 2017 and newer
* CMake
* Ninja (optional, recommended)

## Build instructions Visual Studio 15 2017

mkdir CMakeBuildVS2017
cd CMakeBuildVS2017
cmake .. -G "Visual Studio 15 2017"
cmake --build . --config "Release" --target package

## Build instructions clang-cl

mkdir CMakeBuildClangCl
cd CMakeBuildClangCl
cmake .. -G"Ninja Multi-Config" -DCMAKE_C_COMPILER=clang-cl.exe
cmake --build . --config "Release" --target package

## HowTo

You may use win_flex and win_bison directly on the command line or [use them via CustomBuildRules in VisualStudio](custom_build_rules/README.md).


## Example flex/bison files

See https://github.com/meyerd/flex-bison-example
