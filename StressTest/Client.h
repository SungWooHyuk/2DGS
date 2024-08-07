#pragma once
#include "pch.h"

class Client
{
public:
	Client(int _id, int _x, int _y, atomic_bool _connected, chrono::high_resolution_clock::time_point _last_move_time);
	~Client() {};

	int		GetId() const { return myId; }
	void	SetId(int _id) { myId = _id; }
	int		GetX()	const { return x; }
	void	SetX(int _x) { x = _x; }
	int		GetY()	const { return y; }
	void	SetY(int _y) { y = _y; }
	bool    GetConnected() { return connected.load(); }
	atomic<bool>& GetAtomicConnected() { return connected; }
	void	SetConnected(bool _connected) { connected.store(_connected); }
	std::chrono::high_resolution_clock::time_point GetLastMoveTime() const { return last_move_time; }
	void	SetLastMoveTime(std::chrono::high_resolution_clock::time_point _last_move_time) { last_move_time = _last_move_time; }

private:
	int myId;
	int x;
	int y;
	atomic_bool connected;
	chrono::high_resolution_clock::time_point last_move_time;

};