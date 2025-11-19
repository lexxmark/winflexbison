# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "https://github.com/lexxmark/winflexbison/releases/download/v2.5.25/win_flex_bison-2.5.25.zip" `
  -Checksum "8D324B62BE33604B2C45AD1DD34AB93D722534448F55A16CA7292DE32B6AC135" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
