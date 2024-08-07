pushd %~dp0
pyinstaller --onefile PacketGenerator.py
MOVE .\dist\PacketGenerator.exe .\GenPackets.exe
@RD /S /Q .\build
@RD /S /Q .\dist
@RD /S /F /Q .\PacketGenerator .spec
PAUSE