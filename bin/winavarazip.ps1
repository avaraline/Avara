mkdir WinAvara
cp .\build\*.exe WinAvara
cp .\build\*.dll WinAvara
cp -r .\levels WinAvara
cp -r .\rsrc WinAvara
ls WinAvara
Compress-Archive -Path .\WinAvara -DestinationPath .\WinAvara.zip