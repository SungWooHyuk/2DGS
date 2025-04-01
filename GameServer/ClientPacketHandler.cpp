#include "pch.h"
#include "utils.h"
#include "ClientPacketHandler.h"
#include "DataBase.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	// Validation check
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string pktName = pkt.name();
	
	if (DB->CheckDB(pktName, gamesession))
	{
		PlayerRef player = gamesession->GetCurrentPlayer();
		player->SetId(gamesession->GetId());
		player->SetOwnerSession(gamesession);

		ROOMMANAGER->EnterRoom(gamesession);
		POS pos = { player->POSX, player->POSY };
		STAT stat = { player->GetStat().level, player->GetStat().hp, player->GetStat().mp, player->GetStat().exp, player->GetStat().maxHp, player->GetStat().maxMp, player->GetStat().maxExp };
		gamesession->LoginPkt(true, player->GetId(), player->GetPT(), player->GetName(), pos, stat);

		unordered_set<uint64> vl = ROOMMANAGER->ViewList(gamesession, false);

		gamesession->SetViewPlayer(vl); // first viewlist enroll
		
		for (const auto vp : vl)
		{
			if (GAMESESSIONMANAGER->GetSession(vp))
			{
				POS vpPos = { GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->POSX , GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->POSY };

				if (gamesession->GetCurrentPlayer()->GetPT() != Protocol::PLAYER_TYPE_DUMMY)
				{
					gamesession->AddObjectPkt(
						GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->GetPT(),
						GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->GetId(),
						GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->GetName(),
						vpPos);
				}
				
				if (GAMESESSIONMANAGER->GetSession(vp)->GetCurrentPlayer()->GetPT() == Protocol::PlayerType::PLAYER_TYPE_CLIENT)
				{

					POS plPos = { player->POSX, player->POSY };

					GAMESESSIONMANAGER->GetSession(vp)->AddObjectPkt(
						player->GetPT(),
						player->GetId(),
						player->GetName(),
						plPos
					);
				}
			}
			else
			{
				// session�� �ҷ��µ� nullptr ? �������ִ�. �ֳĸ� ViewList�� �ִµ�, �Ŵ������� ���ٴ°Ŵϱ� ��ȣ����ġ.
			}
		}

		return true;
		// �ٸ� Ŭ���̾�Ʈ RoomManager üŷ�ϱ�.
	}

	gamesession->LoginPkt(false);

	return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 direction = pkt.direction();
	int64 move_time = pkt.move_time();
	GAMESESSIONMANAGER->Move(gamesession, direction, move_time);

	return false;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string mess = pkt.msg();
	GAMESESSIONMANAGER->Chat(gamesession, mess);

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
	GAMESESSIONMANAGER->Attack(gamesession, pkt.id(), pkt.skill());

	return false;
}

bool Handle_C_LOGOUT(PacketSessionRef& session, Protocol::C_LOGOUT& pkt)
{
	return false;
}
