pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./DBProtocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=TestPacketHandler --recv=S_ --send=C_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

GenPackets.exe --path=./DBProtocol.proto --output=GameDBPacketHandler --recv=DS_ --send=SD_
GenPackets.exe --path=./DBProtocol.proto --output=DBPacketHandler --recv=SD_ --send=DS_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../../GameServer"
XCOPY /Y Enum.pb.cc "../../../GameServer"
XCOPY /Y Struct.pb.h "../../../GameServer"
XCOPY /Y Struct.pb.cc "../../../GameServer"
XCOPY /Y Protocol.pb.h "../../../GameServer"
XCOPY /Y Protocol.pb.cc "../../../GameServer"
XCOPY /Y Protocol.proto "../../../GameServer"
XCOPY /Y DBProtocol.pb.h "../../../GameServer"
XCOPY /Y DBProtocol.pb.cc "../../../GameServer"
XCOPY /Y DBProtocol.proto "../../../GameServer"
XCOPY /Y ClientPacketHandler.h "../../../GameServer"
XCOPY /Y GameDBPacketHandler.h "../../../GameServer"

XCOPY /Y Enum.pb.h "../../../Client"
XCOPY /Y Enum.pb.cc "../../../Client"
XCOPY /Y Struct.pb.h "../../../Client"
XCOPY /Y Struct.pb.cc "../../../Client"
XCOPY /Y Protocol.pb.h "../../../Client"
XCOPY /Y Protocol.pb.cc "../../../Client"
XCOPY /Y Protocol.proto "../../../Client"
XCOPY /Y ServerPacketHandler.h "../../../Client"

XCOPY /Y Enum.pb.h "../../../StressTest"
XCOPY /Y Enum.pb.cc "../../../StressTest"
XCOPY /Y Struct.pb.h "../../../StressTest"
XCOPY /Y Struct.pb.cc "../../../StressTest"
XCOPY /Y Protocol.pb.h "../../../StressTest"
XCOPY /Y Protocol.pb.cc "../../../StressTest"
XCOPY /Y Protocol.proto "../../../StressTest"
XCOPY /Y TestPacketHandler.h "../../../StressTest"

XCOPY /Y Enum.pb.h "../../../DBServer"
XCOPY /Y Enum.pb.cc "../../../DBServer"
XCOPY /Y Struct.pb.h "../../../DBServer"
XCOPY /Y Struct.pb.cc "../../../DBServer"
XCOPY /Y DBProtocol.pb.h "../../../DBServer"
XCOPY /Y DBProtocol.pb.cc "../../../DBServer"
XCOPY /Y DBProtocol.proto "../../../DBServer"
XCOPY /Y DBPacketHandler.h "../../../DBServer"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE