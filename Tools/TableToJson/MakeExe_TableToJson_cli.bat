pushd %~dp0
pyinstaller --onefile TableToJsonCli.py

MOVE .\dist\TableToJsonCli.exe .\GenItemJson_cli.exe
@RD /S /Q .\build
@RD /S /Q .\dist
@DEL /F /Q .\TableToJsonCli.spec
PAUSE