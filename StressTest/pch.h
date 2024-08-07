#pragma once

#define WIN32_LEAN_AND_MEAN // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

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

using DummySessionRef = shared_ptr<class DummySession>;
using ClientRef = shared_ptr<class Client>;