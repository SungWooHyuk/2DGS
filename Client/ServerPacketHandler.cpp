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

	if (pkt.success())
	{
		for (const auto& player : pkt.players())
		{
			STAT st;
			POS ps;
			TP tp;

			st.exp = player.exp();
			st.hp = player.hp();
			st.mp = player.mp();
			st.level = player.level();
			st.maxExp = player.maxexp();
			st.maxHp = player.maxhp();
			st.maxMp = player.maxmp();
			ps.posx = player.x();
			ps.posy = player.y();
			tp.attackEndTime = chrono::system_clock::now();
			tp.attackTime = chrono::system_clock::now();
			tp.moveTime = chrono::system_clock::now();

			PlayerRef pl = MakeShared<Player>(*SFSYSTEM.GetPlayerAttack(),
				*SFSYSTEM.GetPlayer(),
				0, 0, 65, 65, st, ps, tp,
				player.id(), player.name());

			g_left_x	= ps.posx - SCREEN_WIDTH / 2;
			g_top_y		= ps.posy - SCREEN_HEIGHT / 2;
			serverSession->SetPlayer(pl);
		}

		return true;
	}
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
