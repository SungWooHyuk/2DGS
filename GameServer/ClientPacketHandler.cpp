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

		ROOMMANAGER->EnterRoom(gamesession);
		gamesession->LoginPkt(true, player->GetId(), player->GetPT(), player->GetName(), pos, st);

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
	// 소비아이템 소모
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	
	uint64 itemId = pkt.item_id();
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 slotIndex = pkt.inv_slot_index();

	gamesession->GetCurrentPlayer()->ConsumeItem(itemId, tab, slotIndex);
	return true;
}

bool Handle_C_DROP_ITEM(PacketSessionRef& session, Protocol::C_DROP_ITEM& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 itemId = pkt.item_id();
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 slotIndex = pkt.inv_slot_index();
	uint64 quantity = pkt.quantity();

	gamesession->GetCurrentPlayer()->DropItem(itemId, tab, slotIndex, quantity);
	return true;
}

bool Handle_C_MOVE_INVENTORY_ITEM(PacketSessionRef& session, Protocol::C_MOVE_INVENTORY_ITEM& pkt)
{
	// 인벤 아이템 이동
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);

	Protocol::InventoryTab fromTab = pkt.from_tab();
	uint64 fromIndex = pkt.inv_from_index();
	Protocol::InventoryTab toTab = pkt.to_tab();
	uint64 toIndex = pkt.inv_to_index();

	gamesession->GetCurrentPlayer()->MoveInventoryItem(fromTab, fromIndex, toTab, toIndex);

	return true;
}

bool Handle_C_INVEN_SWAP_ITEM(PacketSessionRef& session, Protocol::C_INVEN_SWAP_ITEM& pkt)
{
	// 아이템 교환(이동)
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 fromItemId = pkt.from_item_id();
	Protocol::InventoryTab fromTab = pkt.from_tab();
	uint64 fromIndex = pkt.inv_from_index();
	uint64 toItemId = pkt.to_item_id();
	Protocol::InventoryTab toTab = pkt.to_tab();
	uint64 toIndex = pkt.inv_to_index();

	gamesession->GetCurrentPlayer()->InvenSwapItem(fromItemId, fromTab, fromIndex, toItemId, toTab, toIndex);
	return true;
}

bool Handle_C_ADD_ITEM(PacketSessionRef& session, Protocol::C_ADD_ITEM& pkt)
{
	// 아이템 추가
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);

	uint64 itemId = pkt.item_id();
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 slotIndex = pkt.inv_slot_index();
	uint64 quantity = pkt.quantity();

	gamesession->GetCurrentPlayer()->AddItem(itemId, tab, slotIndex, quantity);
	return true;
}

bool Handle_C_REMOVE_ITEM(PacketSessionRef& session, Protocol::C_REMOVE_ITEM& pkt)
{
	// 아이템 삭제
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);

	uint64 itemId = pkt.item_id();
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 slotIndex = pkt.inv_slot_index();

	gamesession->GetCurrentPlayer()->RemoveItem(itemId, tab, slotIndex);
	return true;
}

bool Handle_C_EQUIP(PacketSessionRef& session, Protocol::C_EQUIP& pkt)
{
	//장비 장착
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);

	uint64 itemId = pkt.item_id();
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 slotIndex = pkt.inv_slot_index();
	Protocol::EquipmentSlot slot = pkt.slot_type();

	gamesession->GetCurrentPlayer()->Equip(itemId, tab, slotIndex, slot);
	return true;
}

bool Handle_C_UNEQUIP(PacketSessionRef& session, Protocol::C_UNEQUIP& pkt)
{
	// 장비 해제
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 itemId = pkt.item_id();
	uint64 slotIndex = pkt.inv_slot_index();
	Protocol::EquipmentSlot slot = pkt.slot_type();
	Protocol::InventoryTab tab = pkt.tab_type();

	gamesession->GetCurrentPlayer()->UnEquip(itemId, slotIndex, slot, tab);
	return true;
}

bool Handle_C_SORT_INVENTORY(PacketSessionRef& session, Protocol::C_SORT_INVENTORY& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);

	Protocol::InventoryTab tab = pkt.tab_type();
	// 클라한테 넘겨서 해당 탭 순번 앞으로 다 땡긴 후 클라한테 S_LOAD_INVENTORY 패킷 쏴줘서 인벤토리 업데이트하기
	return true;
}

bool Handle_C_UPDATE_ITEM(PacketSessionRef& session, Protocol::C_UPDATE_ITEM& pkt)
{
	// 아이템 수량 업데이트
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	Protocol::InventoryTab tab = pkt.tab_type();
	uint64 itemId = pkt.item_id();
	uint64 slotIndex = pkt.inv_slot_index();
	uint64 quantity = pkt.quantity();

	gamesession->GetCurrentPlayer()->UpdateItem(tab, itemId, slotIndex, quantity);
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	uint64 direction = pkt.direction();
	int64 move_time = pkt.move_time();
	GAMESESSIONMANAGER->Move(gamesession, direction, move_time);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	string mess = pkt.msg();
	GAMESESSIONMANAGER->Chat(gamesession, mess);

	return true;
}

bool Handle_C_TELEPORT(PacketSessionRef& session, Protocol::C_TELEPORT& pkt)
{
	return true;
}

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	GAMESESSIONMANAGER->Attack(gamesession, pkt.id(), pkt.skill());

	return true;
}

bool Handle_C_LOGOUT(PacketSessionRef& session, Protocol::C_LOGOUT& pkt)
{
	// 로그아웃 처리
	GameSessionRef gamesession = static_pointer_cast<GameSession>(session);
	gamesession->OnDisconnected();
	return true;
}
