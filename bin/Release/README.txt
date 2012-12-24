version 2.1
--------------
fixed crash when execute win_bison.exe under WindowsXP (argv[0] don't have full application path)

version 2.0
--------------
upgrade win_bison to version 2.7 and win_flex to version 2.5.37

version 1.2
--------------
fixed win_flex.exe #line directives (some #line directives in output file were with unescaped backslashes)

version 1.1
--------------
fixed win_flex.exe parallel invocations (now all temporary files are process specific)
added FLEX_TMP_DIR environment variable support to redirect temporary files folder
added '.exe' to program name in win_flex.exe --version output (CMake support)
fixed win_bison.exe to use /data subfolder related to executable path rather than current working directory
