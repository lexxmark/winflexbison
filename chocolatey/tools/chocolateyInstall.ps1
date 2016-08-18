# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.5.5.zip" `
  -Checksum "38E57B9216AC49024B21ADBC54FA662FB68B8E3ACEDEA8BA41E8EF297563D851" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"