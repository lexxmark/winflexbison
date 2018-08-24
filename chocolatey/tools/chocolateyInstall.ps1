# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "http://sourceforge.net/projects/winflexbison/files/old_versions/win_flex_bison-2.5.15.zip" `
  -Checksum "F46C9B483525689EFA58FFE0495EB8017F5979618E9FF7D914F531C78DAD28D6" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"