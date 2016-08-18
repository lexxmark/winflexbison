# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.4.5.zip" `
  -Checksum "DD36C3152E9F6728A5A08BCF93453D68BC8C8AC1074A43B51D50DDD018217414" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"