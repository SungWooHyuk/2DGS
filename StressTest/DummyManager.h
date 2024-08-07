#pragma once
#include "pch.h"

class DummySession;
using DummySessionRef = shared_ptr<DummySession>;
using namespace chrono;

class DummyManager
{
public:
	DummyManager();
	~DummyManager() {};

	void			Add(DummySessionRef _dummy);
	void			Remove(DummySessionRef _dummy);

	void			GetPointCloud(int* size, float** points);

	bool			Adjust_Number_Of_Client();
	void			AllMove();
	void			ChangeConnect(int _id);
	void			SetService(DummyServiceRef _service) { service = _service; }

	void			Move(DummySessionRef _dummysession, uint64 _id, uint64 _x, uint64 _y, int64 _movetime);
	void			Login(DummySessionRef _dummysession, uint64 _id, uint64 _x, uint64 _y);
private:
	array<DummySessionRef, MAX_CLIENTS> clients;
	array<int, MAX_CLIENTS>				client_map;

private:
	atomic_int num_connections;
	atomic_int client_to_close;

	int delay_multiplier;
	int max_limit;
	bool increasing;

	float point_cloud[MAX_TEST * 2];
	DummyServiceRef service;

	high_resolution_clock::time_point last_connect_time;
};

extern shared_ptr<DummyManager> GDummyManager;