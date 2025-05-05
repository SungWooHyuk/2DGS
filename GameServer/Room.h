#pragma once
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	void									Enter(PlayerRef _player);
	void									Leave(PlayerRef _player);
	void									Broadcast(SendBufferRef _sendBuffer);

	const unordered_map<uint64, PlayerRef>& GetPlayers() { READ_LOCK; return players; };
	void									SetPlayers(const unordered_map<uint64_t, PlayerRef>& _newPlayers) { WRITE_LOCK; players = _newPlayers; };
private:
	USE_LOCK;
	unordered_map<uint64, PlayerRef> players;
};
