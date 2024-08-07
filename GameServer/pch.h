#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#pragma comment(lib, "../Libraries\\Lua\\lua54.lib")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif


#include "CorePch.h"
#include "Enum.pb.h"

using GameSessionRef = shared_ptr<class GameSession>;
using PlayerRef = shared_ptr<class Player>;
using RoomRef = shared_ptr<class Room>;