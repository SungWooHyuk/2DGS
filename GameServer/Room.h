#pragma once
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	void									Enter(PlayerRef _player);
	void									Leave(PlayerRef _player);
	void									Broadcast(SendBufferRef _sendBuffer);

	const unordered_map<uint64, PlayerRef>& GetPlayers() const { return players; };
	void									SetPlayers(const unordered_map<uint64_t, PlayerRef>& _newPlayers) { players = _newPlayers; };
public:

private:
	unordered_map<uint64, PlayerRef> players;
};
