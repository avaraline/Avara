'#define GIT_VERSION "{0}"' -f (git describe --always --dirty) | Out-File -FilePath  "$PSScriptRoot\..\src\util\GitVersion.h" -Encoding utf8
