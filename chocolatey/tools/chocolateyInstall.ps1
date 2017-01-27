# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.5.7.zip" `
  -Checksum "79992E198547DAF71DEC9B4AC80EDC28F5AA9C23DD74246F97C9BDF46190CA33" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"