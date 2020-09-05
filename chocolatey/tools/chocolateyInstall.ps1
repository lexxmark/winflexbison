# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "https://github.com/lexxmark/winflexbison/releases/download/v2.5.23/win_flex_bison-2.5.23.zip" `
  -Checksum "6AA5C8EA662DA1550020A5804C28BE63FFAA53486DA9F6842E24C379EC422DFC" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
