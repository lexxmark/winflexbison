# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.5.10.zip" `
  -Checksum "565967D6F6D021B617144698DEDD055DF9B91825CB61A7E1F29BFD22219F550C" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"