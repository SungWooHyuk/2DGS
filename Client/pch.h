#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#include "../GameServer/utils.h"
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include "CorePch.h"
#include "Enum.pb.h"
#include "nlohmann/json.hpp""

using namespace chrono;

struct GoldRanking {
    string name;
    int gold;
};

using json = nlohmann::json;
using ServerSessionRef = shared_ptr<class ServerSession>;
using ClientRef = shared_ptr<class Client>;
using PlayerRef = shared_ptr<class Player>;