# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.5.6.zip" `
  -Checksum "6083985039A45D450946802D032DBCA739A4BC2D26F2F072B61D53CA48110981" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"