#pragma once
#include "Session.h"
#include "utils.h"

class DBGameSession : public PacketSession
{
public:
	DBGameSession() {};
	~DBGameSession() { cout << "~DBGameSession" << endl; };

public:

	virtual void					OnConnected() override;
	virtual void					OnDisconnected() override;
	virtual void					OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void					OnSend(int32 len) override;
};

