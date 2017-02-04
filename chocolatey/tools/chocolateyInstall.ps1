# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.4.8.zip" `
  -Checksum "CD990FB85522FD7B308818E0DF27C80ADA8B39B2C5140F7E542ECD764EA09BBB" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"