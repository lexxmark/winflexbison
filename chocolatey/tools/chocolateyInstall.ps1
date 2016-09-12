# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.4.6.zip" `
  -Checksum "5F55D7E3C72723A6162FE9B7613220127E3B79D4F267394B602E684B3B218306" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"