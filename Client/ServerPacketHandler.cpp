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
			pl->SetStat(stats); // 있으면 세터 호출
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
				inv.slot_index = item.slot_index();
				inventoryList.push_back(inv);
			}
			pl->SetInventory(inventoryList);
		}

	
		if (pkt.equipment().equipment_size() > 0)
		{
			const auto& eq = pkt.equipment().equipment(0);
			if (eq.weapon() != 0)  pl->SetEquip(E_EQUIP::WEAPON, eq.weapon());
			if (eq.helmet() != 0)  pl->SetEquip(E_EQUIP::HELMET, eq.helmet());
			if (eq.top() != 0)     pl->SetEquip(E_EQUIP::TOP, eq.top());
			if (eq.bottom() != 0)  pl->SetEquip(E_EQUIP::BOTTOM, eq.bottom());
		}

		serverSession->SetPlayer(pl);

		g_left_x = ps.posx - SCREEN_WIDTH / 2;
		g_top_y = ps.posy - SCREEN_HEIGHT / 2;
	}

	return true;
}

bool Handle_S_LOAD_INVENTORY(PacketSessionRef& session, Protocol::S_LOAD_INVENTORY& pkt)
{
	return false;
}

bool Handle_S_LOAD_EQUIPMENT(PacketSessionRef& session, Protocol::S_LOAD_EQUIPMENT& pkt)
{
	return false;
}

bool Handle_S_CONSUME_RESULT(PacketSessionRef& session, Protocol::S_CONSUME_RESULT& pkt)
{
	return false;
}

bool Handle_S_DROP_RESULT(PacketSessionRef& session, Protocol::S_DROP_RESULT& pkt)
{
	return false;
}

bool Handle_S_MOVE_INVENTORY_RESULT(PacketSessionRef& session, Protocol::S_MOVE_INVENTORY_RESULT& pkt)
{
	return false;
}

bool Handle_S_EQUIP_RESULT(PacketSessionRef& session, Protocol::S_EQUIP_RESULT& pkt)
{
	return false;
}

bool Handle_S_UNEQUIP_RESULT(PacketSessionRef& session, Protocol::S_UNEQUIP_RESULT& pkt)
{
	return false;
}

bool Handle_S_GOLD_CHANGE(PacketSessionRef& session, Protocol::S_GOLD_CHANGE& pkt)
{
	return false;
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

		serverSession->GetPlayer()->SetStat(st);
	}

	return true;
}

bool Handle_S_DAMAGE(PacketSessionRef& session, Protocol::S_DAMAGE& pkt)
{
	return false;
}

bool Handle_S_RESPAWN(PacketSessionRef& session, Protocol::S_RESPAWN& pkt)
{
	ServerSessionRef serverSession = static_pointer_cast<ServerSession>(session);

	if (serverSession->GetPlayer()->Respawn(pkt.exp(), pkt.hp(), pkt.x(), pkt.y()))
		serverSession->MovePkt(-1, duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
		
	return true;
}
