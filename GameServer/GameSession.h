#pragma once
#include "Session.h"
#include "utils.h"
#include "Room.h"

class GameSession : public PacketSession
{
public:
	GameSession();
	~GameSession() { cout << "~GameSession" << endl; };

public:

	virtual void					OnConnected() override;
	virtual void					OnDisconnected() override;
	virtual void					OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void					OnSend(int32 len) override;

public:
	vector<int>						GetRandomDirectionIndices();

	void							RemoveViewPlayer(uint64 _id) { WRITE_LOCK_IDX(1); viewPlayer.erase(_id); };
	void							SetViewPlayer(const unordered_set<uint64_t>& _players) { WRITE_LOCK_IDX(1); viewPlayer = _players; }
	unordered_set<uint64_t>			GetViewPlayer() { READ_LOCK_IDX(1); return viewPlayer; }
	void							AddViewPlayer(const uint64 _id) { WRITE_LOCK_IDX(1); viewPlayer.insert(_id); }
	void							ResetViewPlayer() { WRITE_LOCK_IDX(1); viewPlayer.clear(); }


public:
	void							SetCurrentPlayer(const PlayerRef& _player) { WRITE_LOCK_IDX(0);  currentPlayer = _player; }
	PlayerRef						GetCurrentPlayer() { READ_LOCK_IDX(0); return currentPlayer; }

	void							SetRoom(const weak_ptr<Room>& _roomPtr) { room = _roomPtr; }
	weak_ptr<Room>					GetRoom() const { return room; }
	uint64							GetId() const { return myId; }
	void							SetId(uint64 _id) { myId = _id; }
	void							RoomReset() { room.reset(); };
public:
	bool							CanGo(POS _pos);
	void							ResetPath();
	void							SetPath(POS _dest, map<POS, POS>& _parent);
	bool							EmptyPath();
	uint32							GetPathIndex() { READ_LOCK_IDX(2); return pathIndex; };
	void							SetPathIndex(uint32 _path) { WRITE_LOCK_IDX(2); pathIndex = _path; };
	vector<POS>						GetPath() { READ_LOCK_IDX(2); return path; };
	uint32							GetPathCount() { READ_LOCK_IDX(2); return pathCount; };
	void							SetPathCount(uint32 _count) { WRITE_LOCK_IDX(2); pathCount = _count; };
	bool							SaveStateSnap();
	
public:
	// To Client Packet
	void							RemovePkt(uint64 _id);
	void							RespawnPkt(int32 _hp, POS _pos, int32 _exp);
	void							AddObjectPkt(Protocol::PlayerType _pt, uint64 _id, string _name, POS _pos);
	void							MovePkt(uint64 _id, POS _pos, int64 _time);
	void							DamagePkt(int32 _damage);
	void							ChatPkt(uint64 _id, string _mess);
	void							StatChangePkt(int32 _level, int32 _hp, int32 _maxhp, int32 _mp, int32 _maxmp, int32 _exp, int32 _maxexp);
	void							LoginPkt(bool _success, uint64 _id, Protocol::PlayerType _pt, string _name, POS _pos, STAT _stat);
	void							LoginPkt(bool _success, PlayerRef _player);
	void							LoginPkt(bool _success);
	void							LoadInventoryPkt();
	void							LoadEquipmentPkt();
	void							ConsumeItemPkt(bool _success, uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _newQuantity);
	void							DropPkt(bool _success, uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex, uint64 _dropQuantity);
	void							MoveInventoryIndex(bool _success, Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex, uint64 _moveItemId, uint64 _quantity);
	void							EquipPkt(bool _success, uint64 _itemId, Protocol::EquipmentSlot _slot);
	void							UnEquipPkt(bool _success, uint64 _itemId, Protocol::EquipmentSlot _slot, Protocol::InventoryTab _tab, uint64 _invToSlotIndex);
	void							GoldChangePkt(uint64 _newGold, int64 _delta);
	void							RankingPkt(const vector<RankingData>& _ranking);
	void							RemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tab, uint64 _slotIndex);
	void							SwapPkt(bool _success, Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex);

	JobQueueRef						GetJobQueue() { return jobQueue; }
	void							StartSaveTimer();
private:
	USE_MANY_LOCKS(3); // 0 : currentPlayer , 1 : viewPlayer, 2 : path
	PlayerRef currentPlayer;
	unordered_set<uint64> viewPlayer;
	weak_ptr<Room>	room;
	uint64			myId;


	vector<POS>		path;
	uint32			pathIndex = 1;
	uint32			pathCount = 0;

private:
	JobQueueRef jobQueue;
};

