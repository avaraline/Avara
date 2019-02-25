Set-Alias bash "C:\tools\msys64\usr\bin\bash.exe"
bash -lc 'pacman --noconfirm --needed -Syuu zip mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-clang mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew make'
bash -lc 'PATH=$PATH:/mingw64/bin; echo $PATH; make winapp; make tests'
Exit $LASTEXITCODE