# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.4.9.zip" `
  -Checksum "D2D047CEF727A857BE98F04114C8C0E002D4560131DAFB002A965E6779906774" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"