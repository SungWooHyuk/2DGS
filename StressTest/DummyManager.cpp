#include "pch.h"
#include "DummyManager.h"
#include "DummySession.h"
#include "Service.h"
#include "Client.h"

using namespace chrono;

int global_delay; // ms단위, 1000이 넘으면 클라이언트 증가 종료
atomic_int active_clients;

DummyManager::DummyManager()
{
	/*for (auto& cl : clients) {
		cl->GetClient()->SetConnected(false);
		cl->GetClient()->SetId(INVALID_ID);
	}*/

	for (auto& cl : client_map) cl = -1;
	num_connections = 0;
	client_to_close = 0;
	active_clients = 0;

	delay_multiplier = 1;
	max_limit = MAXINT;
	increasing = true;
	last_connect_time = chrono::high_resolution_clock::now();
}

void DummyManager::Add(DummySessionRef _dummy)
{
	_dummy->connect = true;
	clients[num_connections] = _dummy;
	client_map[num_connections] = num_connections;

}

void DummyManager::Remove(DummySessionRef _dummy)
{
	int id = _dummy->GetClient()->GetId();
	clients[id].reset();
}

void DummyManager::GetPointCloud(int* size, float** points)
{
	{
		int index = 0;
		for (int i = 0; i < num_connections; ++i)
			if(clients[i])
				if (clients[i]->connect)
				{
					if (true == clients[i]->GetClient()->GetConnected()) {
						point_cloud[index * 2] = static_cast<float>(clients[i]->GetClient()->GetX());
						point_cloud[index * 2 + 1] = static_cast<float>(clients[i]->GetClient()->GetY());
						index++;
					}
				}
		*size = index;
		*points = point_cloud;
	}
}

void DummyManager::ChangeConnect(int _id)
{
	bool expected = true;
	bool desired = false;

	if (clients[_id]->GetClient()->GetAtomicConnected().compare_exchange_strong(expected, desired))
		active_clients--;

}

void DummyManager::Move(DummySessionRef _dummysession, uint64 _id, uint64 _x, uint64 _y, int64 _movetime)
{
	if (_id < MAX_CLIENTS) {
		int my_id = client_map[_id];
		if (-1 != my_id) {
			clients[my_id]->GetClient()->SetX(_x);
			clients[my_id]->GetClient()->SetY(_y);
		}
		if (_dummysession->GetClient()->GetId() == my_id) {
			if (0 != _movetime) {
				auto d_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() - _movetime;
				if (global_delay < d_ms) global_delay++;
				else if (global_delay > d_ms) global_delay--;
			}
		}
	}
}
void DummyManager::Login(DummySessionRef _dummysession, uint64 _id, uint64 _x, uint64 _y)
{
	DummySessionRef dummysession = _dummysession;
	ClientRef client = MakeShared<Client>(_id, _x, _y, true, chrono::high_resolution_clock::now());
	dummysession->SetClient(client);
	clients[num_connections] = dummysession;
	active_clients++;
}
bool DummyManager::Adjust_Number_Of_Client()
{

	if (active_clients >= MAX_TEST) return false;
	if (num_connections >= MAX_CLIENTS) return false;

	auto duration = high_resolution_clock::now() - last_connect_time;
	if (ACCEPT_DELY * delay_multiplier > duration_cast<milliseconds>(duration).count()) return false;

	int t_delay = global_delay;
	if (DELAY_LIMIT2 < t_delay) {
		if (true == increasing) {
			max_limit = active_clients;
			increasing = false;
		}
		if (100 > active_clients) return false;
		if (ACCEPT_DELY * 10 > duration_cast<milliseconds>(duration).count()) return false;
		last_connect_time = high_resolution_clock::now();
		ChangeConnect(client_to_close);
		clients[client_to_close]->OnDisconnected();
		client_to_close++;
		return false;
	}
	else
		if (DELAY_LIMIT < t_delay) {
			delay_multiplier = 10;
			return false;
		}
	if (max_limit - (max_limit / 20) < active_clients) return false;


	increasing = true;
	last_connect_time = high_resolution_clock::now();
	num_connections++;
	
	ASSERT_CRASH(service->Start());

	return true;
}

void DummyManager::AllMove()
{
	for (int i = 1; i < num_connections; ++i) {
		if (clients[i] != nullptr)
			if (clients[i]->connect)
			{
				if (false == clients[i]->GetClient()->GetConnected()) continue;
				if (clients[i]->GetClient()->GetLastMoveTime() + 1s > high_resolution_clock::now()) continue;
				clients[i]->GetClient()->SetLastMoveTime(high_resolution_clock::now());
				uint64 direction;
				switch (rand() % 4) {
				case 0: direction = 0; break;
				case 1: direction = 1; break;
				case 2: direction = 2; break;
				case 3: direction = 3; break;
				}
				clients[i]->MovePkt(direction, static_cast<unsigned>(duration_cast<milliseconds>(clients[i]->GetClient()->GetLastMoveTime().time_since_epoch()).count()));
			}
	}
}
