# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison" `
  -Url "https://github.com/lexxmark/winflexbison/releases/download/v2.4.12/win_flex_bison-2.4.12.zip" `
  -Checksum "5D93FF7A69379FCF521601B2163FCD35D54E018CE42671C718B5CD9B134626FF" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
