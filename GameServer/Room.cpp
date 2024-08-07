#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "GameSession.h"

void Room::Enter(PlayerRef _player)
{
	players[_player->GetId()] = _player;
}

void Room::Leave(PlayerRef _player)
{
	players.erase(_player->GetId());
}

void Room::Broadcast(SendBufferRef _sendBuffer)
{
	for (auto& p : players)
	{
		p.second->GetOwnerSession()->Send(_sendBuffer);
	}
}
