#include "pch.h"
#include "ServerPacketHandler.h"
#include "ServerSession.h"
#include "SFSystem.h"
#include "Client.h"
#include "Player.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	if (!pkt.success())
		return false;

	for (const auto& player : pkt.players())
	{
		POS ps;
		TP tp;

		ps.posx = player.x();
		ps.posy = player.y();

		tp.attackEndTime = chrono::system_clock::now();
		tp.attackTime = chrono::system_clock::now();
		tp.moveTime = chrono::system_clock::now();

		PlayerRef pl = MakeShared<Player>(*SFSYSTEM.GetPlayerAttack(),
			*SFSYSTEM.GetPlayer(),
			0, 0, 65, 65, ps, tp,
			player.id(), player.name(), player.gold());

		// 골드 정보를 SFSystem에도 설정
		SFSYSTEM.SetGold(player.gold());

		if (pkt.stats_size() > 0)
		{
			const auto& s = pkt.stats(0);
			STAT stats;
			stats.exp = s.exp();
			stats.hp = s.hp();
			stats.mp = s.mp();
			stats.level = s.level();
			stats.maxExp = s.maxexp();
			stats.maxHp = s.maxhp();
			stats.maxMp = s.maxmp();
			stats.attackPower = s.attackpower();
			stats.defencePower = s.defensepower();
			stats.magicPower = s.magicpower();
			stats.strength = s.strenth();

			pl->SetStat(stats); 
		}


		vector<INVEN> inventoryList;
		if (pkt.has_inventory())
		{
			for (const auto& item : pkt.inventory().inventory())
			{
				INVEN inv;
				inv.itemId = item.item_id();
				inv.quantity = item.quantity();
				inv.tab_type = item.tab_type();
				inv.slot_index = item.inv_slot_index();
				inventoryList.push_back(inv);
			}
			pl->SetInventory(inventoryList);
		}

	
		if (pkt.equipment().equipment_size() > 0)
		{
			const auto& equipment_list = pkt.equipment().equipment();
			for (const auto& eq : equipment_list)
			{
				switch (eq.eq_slot())
				{
				case Protocol::WEAPON:
					if (eq.item_id() != 0)
						pl->SetEquip(Protocol::WEAPON, eq.item_id());
					break;
				case Protocol::HELMET:
					if (eq.item_id() != 0)
						pl->SetEquip(Protocol::HELMET, eq.item_id());
					break;
				case Protocol::TOP:
					if (eq.item_id() != 0)
						pl->SetEquip(Protocol::TOP, eq.item_id());
					break;
				case Protocol::BOTTOM:
					if (eq.item_id() != 0)
						pl->SetEquip(Protocol::BOTTOM, eq.item_id());
					break;
				default:
					break;
				}
			}
		}
		
		serverSession->SetPlayer(pl);
		SFSYSTEM.SetUser(pl);
		pl->SetOwnerSession(serverSession);
		g_left_x = ps.posx - SCREEN_WIDTH / 2;
		g_top_y = ps.posy - SCREEN_HEIGHT / 2;
	}

	return true;
}

bool Handle_S_LOAD_INVENTORY(PacketSessionRef& session, Protocol::S_LOAD_INVENTORY& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	vector<INVEN> inventoryList;

	for (const auto& slot : pkt.inventory())
	{
		INVEN inv;
		inv.itemId = slot.item_id();
		inv.quantity = slot.quantity();
		inv.tab_type = slot.tab_type();
		inv.slot_index = slot.inv_slot_index();

		inventoryList.push_back(inv);
	}

	if (!inventoryList.empty())
	{
		serverSession->GetPlayer()->SetInventory(inventoryList);
		return true;
	}
	return false;
}

bool Handle_S_LOAD_EQUIPMENT(PacketSessionRef& session, Protocol::S_LOAD_EQUIPMENT& pkt)
{	
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.equipment_size() > 0)
	{
		const Protocol::EquipmentItem& equip = pkt.equipment(0); // 1개

		// 서버로 부터 장비창 불러오기
		return true;
	}

	return false;
}

bool Handle_S_CONSUME_RESULT(PacketSessionRef& session, Protocol::S_CONSUME_RESULT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.success())
	{
		serverSession->GetPlayer()->UpdateItemQuantity(pkt.tab_type(), pkt.slot_index(), pkt.new_quantity());
		return true;
	}	
	else // 실패는 수량이 0보다 작을때 실패 줄껀데, 서버에서 처리해서 그냥 다른걸로 넘겨줘도 될듯함. 일단은 이렇게 잡고.
	{
		if (pkt.new_quantity() <= 0)
		{
			serverSession->GetPlayer()->RemoveItem(pkt.tab_type(), pkt.slot_index());
			SFSYSTEM.SetQuickSlotReset(pkt.slot_index());
		}
	}
	return false;
}

bool Handle_S_DROP_RESULT(PacketSessionRef& session, Protocol::S_DROP_RESULT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.success())
	{
		serverSession->GetPlayer()->RemoveItem(pkt.tab_type(), pkt.inv_slot_index());
		return true;
	}
	return false;
}

bool Handle_S_MOVE_INVENTORY_RESULT(PacketSessionRef& session, Protocol::S_MOVE_INVENTORY_RESULT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.success())
	{
		serverSession->GetPlayer()->MoveItem(pkt.from_tab(), pkt.inv_from_index(), pkt.to_tab(), pkt.inv_to_index(), pkt.quantity());
		return true;
	}
	return false;
}

bool Handle_S_EQUIP_RESULT(PacketSessionRef& session, Protocol::S_EQUIP_RESULT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.success())
	{
		serverSession->GetPlayer()->SetEquip(pkt.slot_type(), pkt.item_id());
		return true;
	}
	return false;
}

bool Handle_S_UNEQUIP_RESULT(PacketSessionRef& session, Protocol::S_UNEQUIP_RESULT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	if (pkt.success())
	{
		INVEN inv;
		inv.itemId = pkt.item_id();
		inv.quantity = 1;
		inv.slot_index = pkt.inv_to_slot_index();
		inv.tab_type = pkt.to_tab_type();
		serverSession->GetPlayer()->SetEquipToInventory(pkt.inv_to_slot_index(), inv); // 추가
		serverSession->GetPlayer()->UnEquip(pkt.slot_type(), pkt.item_id()); // 삭제
		return true;
	}
	return false;
}

bool Handle_S_GOLD_CHANGE(PacketSessionRef& session, Protocol::S_GOLD_CHANGE& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	
	if (pkt.new_gold() > 0)
	{
		serverSession->GetPlayer()->SetGold(pkt.new_gold());
		SFSYSTEM.SetGold(serverSession->GetPlayer()->GetGold());
		return true;
	}
	
	return true;
}

bool Handle_S_ADD_OBJECT(PacketSessionRef& session, Protocol::S_ADD_OBJECT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	for (const auto& cli : pkt.players())
	{
		POS ps;

		ps.posx = cli.x();
		ps.posy = cli.y();

		ClientRef client = MakeShared<Client>(cli.playertype(0), ps, cli.id(), cli.name());
		serverSession->AddClient(cli.id(), client);
	}

	return true;
}

bool Handle_S_REMOVE_OBJECT(PacketSessionRef& session, Protocol::S_REMOVE_OBJECT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	if (serverSession->GetPlayer()->GetId() == pkt.id())
	{
		return false;
	}
	else {
		serverSession->GetClients().erase(pkt.id());
	}

	return true;
}

bool Handle_S_MOVE_OBJECT(PacketSessionRef& session, Protocol::S_MOVE_OBJECT& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	for (const auto& cli : pkt.moves())
	{
		if (serverSession->GetPlayer()->GetId() == cli.id())
		{
			serverSession->GetPlayer()->Move(cli.x(), cli.y());
			g_left_x = cli.x() - SCREEN_WIDTH / 2;
			g_top_y = cli.y() - SCREEN_HEIGHT / 2;
		}
		else
		{
			if (serverSession->GetClients(cli.id()) != nullptr)
			{
				serverSession->GetClients(cli.id())->Move(cli.x(), cli.y());
			}
		}
	}

	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	for (const auto& msg : pkt.messages())
	{
		SFSYSTEM.SetChat(msg.mess());
	}

	return true;
}

bool Handle_S_STAT_CHANGE(PacketSessionRef& session, Protocol::S_STAT_CHANGE& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	for (const auto& cli : pkt.stats())
	{
		STAT st;
		st.hp = cli.hp();
		st.maxHp = cli.maxhp();
		st.mp = cli.mp();
		st.maxMp = cli.maxmp();
		st.exp = cli.exp();
		st.maxExp = cli.maxexp();
		st.level = cli.level();

		st.attackPower = cli.attackpower();
		st.defencePower = cli.defensepower();
		st.magicPower = cli.magicpower();
		st.strength = cli.strenth();

		serverSession->GetPlayer()->SetStat(st);
		return true;
	}

	return false;
}

bool Handle_S_DAMAGE(PacketSessionRef& session, Protocol::S_DAMAGE& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);
	serverSession->GetPlayer()->DamageHp(pkt.damage());
	return true;
}

bool Handle_S_RESPAWN(PacketSessionRef& session, Protocol::S_RESPAWN& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	if (serverSession->GetPlayer()->Respawn(pkt.exp(), pkt.hp(), pkt.x(), pkt.y()))
		serverSession->MovePkt(-1, duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
		
	return true;
}

bool Handle_S_RANKING(PacketSessionRef& session, Protocol::S_RANKING& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	vector<GoldRanking> ranking;
	
	for (const auto& rank : pkt.ranking())
	{
		GoldRanking gr;
		gr.name = rank.name();
		gr.gold = rank.gold();
		ranking.push_back(gr);
	}

	if (!ranking.empty())
	{
		SFSYSTEM.UpdateGoldRanking(ranking);
		return true;
	}

	return false;
}