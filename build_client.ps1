$env:Path = "C:\Qt\Qt5.12.11\5.12.11\mingw73_32\bin;C:\Qt\Qt5.12.11\Tools\mingw730_32\bin;$env:Path"
Set-Location C:\wangpan\NetDisk\build-release
& "C:\Qt\Qt5.12.11\5.12.11\mingw73_32\bin\qmake.exe" "C:\wangpan\NetDisk\NetDisk.pro" -spec win32-g++ "CONFIG+=release"
& "C:\Qt\Qt5.12.11\Tools\mingw730_32\bin\mingw32-make.exe" 2>&1 | Write-Output