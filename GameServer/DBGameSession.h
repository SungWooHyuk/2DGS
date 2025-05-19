#pragma once
#include "Session.h"
#include "utils.h"
#include "GameSession.h"

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

	void							SendInventoryPkt(uint64 _id, const string& _name, const InventoryMap& _inventory);
	void							SendEquipmentPkt(uint64 _id, const string& _name, const EquipmentMap& _equipment);
	void							SendPlayerStatePkt(uint64 _id, const string& _name, const POS& _pos, const STAT& _stats);
	void							SendUpdateGoldPkt(uint64 _id, const string& _name, int _gold);
	void							SendSavePkt(uint64 _id, const InventoryMap& _inven, const EquipmentMap& _equip, const USER_INFO& _player);
};

