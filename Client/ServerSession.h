#pragma once

#include "pch.h"
#include "Client.h"
#include "Player.h"

class ServerSession : public PacketSession
{
public:
	ServerSession() {};
	~ServerSession() { cout << "~ServerSession" << endl; };

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

    std::unordered_map<int, ClientRef>& GetClients() { return clients; }
    ClientRef& GetClients(int _id) { return clients[_id]; }
    void SetClients(std::unordered_map<int, ClientRef>& newClients) { clients = newClients; }
	void AddClient(int id, const ClientRef& client) { clients[id] = client; }

    const PlayerRef& GetPlayer() const { return player; }
    void SetPlayer(PlayerRef& _player) { player = _player; }

	void AttackPkt(uint64 _id, uint64 _skill);
	void MovePkt(uint64 _direction, int64 _movetime);

public:
	string Login();
	
private:
	unordered_map<int, ClientRef> clients;
	PlayerRef player;
};