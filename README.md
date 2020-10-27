# WinFlexBison - Flex and Bison for Microsoft Windows

WinFlexBison is a Windows port of [Flex (the fast lexical analyser)](https://github.com/westes/flex/) and [GNU Bison (parser generator)](https://www.gnu.org/software/bison/).
Both win_flex and win_bison are based on upstream sources but depend on system libraries only.

**NOTE**:
* 2.4.x versions include GNU Bison version 2.7
* 2.5.x versions include GNU Bison version 3.x.x

## Build status
Bison 3.x (master) [![Build status](https://ci.appveyor.com/api/projects/status/58lcjnr0mb9uc8c8/branch/master?svg=true)](https://ci.appveyor.com/project/lexxmark/winflexbison/branch/master) and, for compatibility reasons, Bison 2.7 (bison2.7) [![Build status](https://ci.appveyor.com/api/projects/status/58lcjnr0mb9uc8c8/branch/bison2.7?svg=true)](https://ci.appveyor.com/project/lexxmark/winflexbison/branch/bison2.7)

## Downloads
https://github.com/lexxmark/winflexbison/releases provides stable versions. To test non-released development versions see the artifacts provided by CI under "Build status".

## Changelog
The release page includes the full Changelog but you may also see the [changelog.md](changelog.md) file.

## Build requirements
* Visual Studio 2015 or newer
* optional: CMake (when building with CMake)

## HowTo
You may use win_flex and win_bison directly on the command line or [use them via CustomBuildRules in VisualStudio](custom_build_rules/README.md).

## Example flex/bison files
See https://github.com/meyerd/flex-bison-example
