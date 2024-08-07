#pragma once

#include "pch.h"
#include "Client.h"
#include "DummyManager.h"

class DummySession : public PacketSession
{
public:
	DummySession() { connect = false; };
	~DummySession() { cout << "~DummySession" << endl; };

	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

	ClientRef	GetClient() const { return client; }
	void		SetClient(const ClientRef& _client) { client = _client; }

	void		MovePkt(uint64 _direction, uint64 _moveTime);

public:
	string Login();
	bool	  connect;

private:
	ClientRef client;
};