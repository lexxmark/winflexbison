# download and unpack a zip file
Install-ChocolateyZipPackage `
  -PackageName "winflexbison3" `
  -Url "https://github.com/lexxmark/winflexbison/releases/download/v2.5.24/win_flex_bison-2.5.24.zip" `
  -Checksum "39C6086CE211D5415500ACC5ED2D8939861CA1696AEE48909C7F6DAF5122B505" `
  -ChecksumType "sha256" `
  -UnzipLocation "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
