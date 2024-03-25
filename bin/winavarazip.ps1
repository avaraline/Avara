mkdir WinAvara
cp .\Avara.msvc\x64\Release\*.exe WinAvara
cp .\Avara.msvc\x64\Release\*.dll WinAvara
cp -r .\levels WinAvara
cp -r .\rsrc WinAvara
Compress-Archive -Path .\WinAvara -DestinationPath .\WinAvara.zip