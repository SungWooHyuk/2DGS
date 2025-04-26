#include "pch.h"
#include "utils.h"
#include "GameDBPacketHandler.h"
#include "GameSession.h"
#include "DBGameSession.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "Player.h"
#include "RoomManager.h"
#include "Item.h"

PacketHandlerFunc GDBPacketHandler[UINT16_MAX];

bool Handle_DS_LOGIN(PacketSessionRef& session, DBProtocol::DS_LOGIN& pkt)
{
	auto gamesession = GAMESESSIONMANAGER.GetSession(pkt.user_id());

	if (!pkt.success()) // 로그인 실패
	{
		gamesession->LoginPkt(false); // 클라이언트에 false 전송
		return false;
	}

	// 플레이어 생성 및 기본 정보 설정
	PlayerRef player = MakeShared<Player>();
	player->SetId(gamesession->GetId());
	player->SetOwnerSession(gamesession);
	gamesession->SetCurrentPlayer(player);

	// DB에서 받은 플레이어 정보 설정
	if (pkt.player_size() > 0)
	{
		const auto& playerInfo = pkt.player(0);
		POS pos{ playerInfo.x(), playerInfo.y() };
		player->SetName(playerInfo.name());
		player->SetStatLevel(playerInfo.level());
		player->SetStatHp(playerInfo.hp());
		player->SetStatMaxHp(playerInfo.maxhp());
		player->SetStatMp(playerInfo.mp());
		player->SetStatMaxMp(playerInfo.maxmp());
		player->SetStatExp(playerInfo.exp());
		player->SetStatMaxExp(playerInfo.maxexp());
		player->SetPos(pos);
		player->SetGold(playerInfo.gold());
	}

	// 인벤토리 설정
	for (const auto& invenSlot : pkt.inventory())
	{
		INVEN itemInfo;
		itemInfo.itemId = invenSlot.item_id();
		itemInfo.quantity = invenSlot.quantity();
		itemInfo.slot_index = invenSlot.slot_index();
		itemInfo.tab_type = invenSlot.tab_type();

		player->AddInventoryItem(itemInfo);
	}

	// 장비 설정
	for (const auto& equipItem : pkt.equipment())
	{
		ITEM_INFO itemInfo;

		if (equipItem.weapon() > 0)
		{
			auto info = ITEM.GetItem(equipItem.weapon());
			if(info)
				player->EquipItem(E_EQUIP::WEAPON, *info);
		}
		if (equipItem.helmet() > 0)
		{
			auto info = ITEM.GetItem(equipItem.helmet());
			if(info)
				player->EquipItem(E_EQUIP::HELMET, *info);
		}
		if (equipItem.top() > 0)
		{
			auto info = ITEM.GetItem(equipItem.top());
			if (info)
				player->EquipItem(E_EQUIP::TOP, *info);
		}
		if (equipItem.bottom() > 0)
		{
			auto info = ITEM.GetItem(equipItem.bottom());
			if (info)
				player->EquipItem(E_EQUIP::BOTTOM, *info);
		}
	}

	ROOMMANAGER.EnterRoom(gamesession);

	gamesession->LoginPkt(true, player);

	unordered_set<uint64> vl = ROOMMANAGER.ViewList(gamesession, false);
	gamesession->SetViewPlayer(vl); // first viewlist enroll
	
	for (const auto vp : vl)
	{
		if (GAMESESSIONMANAGER.GetSession(vp))
		{
			POS vpPos = { GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetPos().posx, GAMESESSIONMANAGER.GetSession(vp)->GetCurrentPlayer()->GetPos().posy };

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
				POS plPos = { player->GetPos().posx, player->GetPos().posy };
				GAMESESSIONMANAGER.GetSession(vp)->AddObjectPkt(
					player->GetPT(),
					player->GetId(),
					player->GetName(),
					plPos
				);
			}
		}
	}

	return true;
}

bool Handle_DS_REGISTER(PacketSessionRef& session, DBProtocol::DS_REGISTER& pkt)
{
	return false;
}

bool Handle_DS_SAVE_RESULT(PacketSessionRef& session, DBProtocol::DS_SAVE_RESULT& pkt)
{
	return false;
}

bool Handle_DS_USER_INFORMATION(PacketSessionRef& session, DBProtocol::DS_USER_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_INVENTORY_INFORMATION(PacketSessionRef& session, DBProtocol::DS_INVENTORY_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_EQUIP_INFORMATION(PacketSessionRef& session, DBProtocol::DS_EQUIP_INFORMATION& pkt)
{
	return false;
}

bool Handle_DS_EQUIP_ITEM(PacketSessionRef& session, DBProtocol::DS_EQUIP_ITEM& pkt)
{
	return false;
}

bool Handle_DS_FARMING_RESULT(PacketSessionRef& session, DBProtocol::DS_FARMING_RESULT& pkt)
{
	return false;
}

bool Handle_DS_CONSUME_ITEM(PacketSessionRef& session, DBProtocol::DS_CONSUME_ITEM& pkt)
{
	return false;
}

bool Handle_DS_MOVE_RESULT(PacketSessionRef& session, DBProtocol::DS_MOVE_RESULT& pkt)
{
	return false;
}

bool Handle_DS_UPDATE_GOLD(PacketSessionRef& session, DBProtocol::DS_UPDATE_GOLD& pkt)
{
	return true;
}