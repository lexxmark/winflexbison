# download and unpack a zip file
Install-ChocolateyZipPackage "winflexbison" "http://downloads.sourceforge.net/project/winflexbison/win_flex_bison-2.4.1.zip" "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
