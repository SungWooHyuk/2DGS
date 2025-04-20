pushd %~dp0
pyinstaller --onefile TableToJson.py

MOVE .\dist\TableToJson.exe .\GenItemJson.exe
@RD /S /Q .\build
@RD /S /Q .\dist
@DEL /F /Q .\TableToJson .spec
PAUSE