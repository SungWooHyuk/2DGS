#include "pch.h"
#include "utils.h"
#include "ClientPacketHandler.h"
#include "MapData.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"
#include "GameDBPacketHandler.h"
#include "DBGameSessionManager.h"
#include "DBGameSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string pktName = pkt.name();
	
	if (pktName.substr(0, 5) == "dummy")
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator(seed);
		std::uniform_int_distribution<int> distribution(10, 1900);

		POS pos;
		STAT st;

		while (true) {
			int	x = distribution(generator);
			int	y = distribution(generator);

			if (MAPDATA.GetTile(y, x) != MAPDATA.e_OBSTACLE) {
				pos.posx = x;
				pos.posy = y;
				break;
			};
		}

		st.hp = 5000;
		st.maxHp = 5000;
		st.exp = 0;
		st.maxExp = 5000;
		st.mp = 5000;
		st.maxMp = 5000;
		st.level = 5000;
		
		PlayerRef player = MakeShared<Player>(pktName, st, pos, ST_INGAME, 9999, Protocol::PLAYER_TYPE_CLIENT);
		player->SetId(gamesession->GetId());
		player->SetOwnerSession(gamesession);
		gamesession->SetCurrentPlayer(player);

		ROOMMANAGER.EnterRoom(gamesession);
		gamesession->LoginPkt(true, player->GetId(), player->GetPT(), player->GetName(), pos, st);

		unordered_set<uint64> vl = ROOMMANAGER.ViewList(gamesession, false);

		gamesession->SetViewPlayer(vl); // first viewlist enroll

		for (const auto vp : vl)
		{
			if (GAMESESSIONMANAGER.GetSession(vp))
			{
				POS vpPos = { GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->POSX , GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->POSY };

				if (gamesession->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_DUMMY)
				{
					gamesession->AddObjectPkt(
						GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetPT(),
						GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetId(),
						GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetName(),
						vpPos);
				}

				if (GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetPT() == Protocol::PlayerType::PLAYER_TYPE_CLIENT)
				{

					POS plPos = { player->POSX, player->POSY };

					GAMESESSIONMANAGER.GetSession(vp)->AddObjectPkt(
						player->GetPT(),
						player->GetId(),
						player->GetName(),
						plPos
					);
				}
			}
		}
	}
	else
	{
		DBProtocol::SD_LOGIN pkt;
		pkt.set_name(pktName);
		pkt.set_user_id(gamesession->GetId());
		auto sendBuffer = GameDBPacketHandler::MakeSendBuffer(pkt);
		DBMANAGER.GetSession()->Send(sendBuffer);
	}

	return true;
}

bool Handle_C_CONSUME_ITEM(PacketSessionRef& session, Protocol::C_CONSUME_ITEM& pkt)
{
	return false;
}

bool Handle_C_DROP_ITEM(PacketSessionRef& session, Protocol::C_DROP_ITEM& pkt)
{
	return false;
}

bool Handle_C_MOVE_INVENTORY_ITEM(PacketSessionRef& session, Protocol::C_MOVE_INVENTORY_ITEM& pkt)
{
	return false;
}

bool Handle_C_INVEN_SWAP_ITEM(PacketSessionRef& session, Protocol::C_INVEN_SWAP_ITEM& pkt)
{
	return false;
}

bool Handle_C_ADD_ITEM(PacketSessionRef& session, Protocol::C_ADD_ITEM& pkt)
{
	return false;
}

bool Handle_C_REMOVE_ITEM(PacketSessionRef& session, Protocol::C_REMOVE_ITEM& pkt)
{
	return false;
}

bool Handle_C_EQUIP(PacketSessionRef& session, Protocol::C_EQUIP& pkt)
{
	return false;
}

bool Handle_C_UNEQUIP(PacketSessionRef& session, Protocol::C_UNEQUIP& pkt)
{
	return false;
}

bool Handle_C_SORT_INVENTORY(PacketSessionRef& session, Protocol::C_SORT_INVENTORY& pkt)
{
	return false;
}

bool Handle_C_UPDATE_ITEM(PacketSessionRef& session, Protocol::C_UPDATE_ITEM& pkt)
{
	return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 direction = pkt.direction();
	int64 move_time = pkt.move_time();
	GAMESESSIONMANAGER.Move(gamesession, direction, move_time);

	return false;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string mess = pkt.msg();
	GAMESESSIONMANAGER.Chat(gamesession, mess);

	return false;
}

bool Handle_C_TELEPORT(PacketSessionRef& session, Protocol::C_TELEPORT& pkt)
{
	// Stress test ��
	return false;
}

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	GAMESESSIONMANAGER.Attack(gamesession, pkt.id(), pkt.skill());

	return false;
}

bool Handle_C_LOGOUT(PacketSessionRef& session, Protocol::C_LOGOUT& pkt)
{
	return false;
}
