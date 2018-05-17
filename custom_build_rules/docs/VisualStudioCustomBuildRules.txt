These steps help you setup custom build rules for Visual Studio 2010 and up.
---------------
* First of all you should have 3 files:
    * **win_flex_bison_custom_build.props**
    * **win_flex_bison_custom_build.targets**
    * **win_flex_bison_custom_build.xml**


They are located in **custom_build_rules** sub-folder of the win_flex_bison archive. Or you can grab them [here](http://sourceforge.net/projects/winflexbison/files/win_flex_bison_custom_build_rules.zip/download)

Launch Visual Studio and open an VC++ project. Open context menu for project item in Solution Explorer panel and select "**Build Customizations...**" menu item.
[[img src=1.png alt=1.png]]

----

In popup dialog "Visual C++ Build Customization Files" press "**Find Existing...**" button.
[[img src=2.png alt=2.png]]

----

In Open File dialog select "**win_flex_bison_custom_build.targets**" file and press "Open".

----

You will see "Add Search Path?" message box, press "Yes"
[[img src=3.png alt=3.png]]

----

In "Visual C++ Build Customization Files" dialog check just added item **win_flex_bison_custom_build** and press "OK"
[[img src=4.png alt=4.png]]

----

Now you can add flex and bison files to the project and build.
[[img src=5.png alt=5.png]]
In build output you should see something like this:

~~~~

1>------ Rebuild All started: Project: ConsoleApplication1, Configuration: Debug Win32 ------
1>  Process sample bison file
1>  Process sample flex file
1>  stdafx.cpp
1>  ConsoleApplication1.cpp
1>  Generating Code...
1>  ConsoleApplication1.vcxproj -> C:\Users\ConsoleApplication1\Debug\ConsoleApplication1.exe
========== Rebuild All: 1 succeeded, 0 failed, 0 skipped ==========

~~~~

-----

For **sample.y** bison file there are two output files: **sample.tab.h** and **sample.tab.cpp**. For **sample.l** flex file you'll got **sample.flex.cpp**. Now you can add them to the project and build. (*Don't forget to exclude cpp files from using precompiled headers*)
[[img src=6.png alt=6.png]]

~~~~

1>------ Build started: Project: ConsoleApplication1, Configuration: Debug Win32 ------
1>  Process sample bison file
1>  Process sample flex file
1>  sample.tab.cpp
1>  sample.flex.cpp
1>  Generating Code...
1>  ConsoleApplication1.vcxproj -> C:\Users\ConsoleApplication1\Debug\ConsoleApplication1.exe
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

~~~~

----

If your flex/bison file is incorrect and you've got an error. But you don't see actual error message, something like this:

~~~~

1>------ Build started: Project: ConsoleApplication2, Configuration: Debug Win32 ------
1>  Process "grammar.y" bison file
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: The command "
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: start /B /WAIT /D "C:...\ConsoleApplication2\ConsoleApplication2\" win_bison.exe --output="grammar.tab.cpp" --defines="grammar.tab.h" --graph="1.dot" "grammar.y"
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: exit /b %errorlevel%" exited with code 1.
========== Build: 0 succeeded, 1 failed, 0 up-to-date, 0 skipped ==========

~~~~

You can change Build Output Verbosity from "Minimal" to "Normal" in "Options" dialog
[[img src=Verbosity.png alt=Verbosity.png]]
Then you will see more detailed output:

~~~~

1>BisonTarget:
1>  Process "grammar.y" bison file
1>  grammar.y:51.1-4: error: invalid directive: '%sdw'
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: The command "
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: start /B /WAIT /D "C:...\ConsoleApplication2\" win_bison.exe --output="grammar.tab.cpp" --defines="grammar.tab.h" --graph="1.dot" "grammar.y"
1>C:...\custom_build_rules\win_flex_bison_custom_build.targets(55,5): error MSB3721: exit /b %errorlevel%" exited with code 1.
1>
1>Build FAILED.
1>
1>Time Elapsed 00:00:01.21
========== Build: 0 succeeded, 1 failed, 0 up-to-date, 0 skipped ==========

~~~~

----

Also you can tune some flex/bison options in files properties dialog:
[[img src=Properties.png alt=Properties.png]]
[[img src=FlexProperties.png alt=FlexProperties.png]]
[[img src=BisonProperties.png alt=BisonProperties.png]]

----


To debug your scanner or parser you can set break points right into **sample.y** or **sample.l** code
[[img src=Flex_debuging.png alt=Flex_debuging.png]]

----

Enjoy!
--------