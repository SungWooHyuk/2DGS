#pragma once
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	void									Enter(PlayerRef _player);
	void									Leave(PlayerRef _player);
	void									Broadcast(SendBufferRef _sendBuffer);

	const unordered_map<uint64, PlayerRef>& GetPlayers() {return players; };
	void									SetPlayers(const unordered_map<uint64_t, PlayerRef>& _newPlayers) {players = _newPlayers; };
private:
	
	unordered_map<uint64, PlayerRef> players;
};
