#pragma once
#include "pch.h"
#include "Client.h"

Client::Client(int _id, int _x, int _y, atomic_bool _connected, chrono::high_resolution_clock::time_point _last_move_time)
	: myId(_id), x(_x), y(_y), last_move_time(_last_move_time)
{
	connected.store(_connected);
}