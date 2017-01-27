# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.4.7.zip" `
  -Checksum "7553A2D6738C799E101EC38A6AD073885EAD892826F87BC1A24E78BCD7AC2A8C" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"