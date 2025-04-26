#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#pragma comment(lib, "../Libraries\\Lua\\lua54.lib")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "hiredis\\Debug\\hiredisd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib, "hiredis\\Release\\hiredis.lib")

#endif


#include "CorePch.h"
#include "Enum.pb.h"
#include "nlohmann/json.hpp""

using namespace std;
using json = nlohmann::json;
using GameSessionRef = shared_ptr<class GameSession>;
using DBGameSessionRef = shared_ptr<class DBGameSession>;
using PlayerRef = shared_ptr<class Player>;
using RoomRef = shared_ptr<class Room>;