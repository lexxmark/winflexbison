# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://downloads.sourceforge.net/project/winflexbison/win_flex_bison-2.4.1.zip" `
  -Checksum "1fd2690fba8b1f342cc500c7a5fd67fe" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
