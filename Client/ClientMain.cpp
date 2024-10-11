#include "pch.h"
#include "MapData.h"
#include "SFSystem.h"
#include "Tile.h"
#include "Player.h"
#include "Client.h"

#include "ThreadManager.h"
#include "ServerPacketHandler.h"
#include "Session.h"
#include "ServerSession.h"

#include "Service.h"
#include "BufferReader.h"

uint32	g_left_x;
uint32	g_top_y;

void Update(ClientServiceRef& _service)
{
	ServerSessionRef session = static_pointer_cast<ServerSession>(_service->GetSession());
	TILE->TileDraw();
	if (session->GetPlayer() != nullptr) 
		SFSYSTEM->Update(session);
}

int main()
{

	MapData* mapData = MapData::GetInstance(); // Map data
	SFSystem* sfml = SFSystem::GetInstance(); // Sfml data
	Tile* tile = Tile::GetInstance(); // Tile data
	ServerPacketHandler::Init(); // packethandler init

	ClientServiceRef service = MakeShared<ClientService>( // connect
		NetAddress(L"127.0.0.1", PORT_NUM),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		1);

	int num_threads = std::thread::hardware_concurrency();
	for (int32 i = 0; i < num_threads; i++)
	{
		GThreadManager->Launch([&service]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	service->Start();

	sf::RenderWindow window(sf::VideoMode(1365, WINDOW_HEIGHT), "2D CLIENT");
	SFSYSTEM->SetWindow(&window);

	
	while (window.isOpen())
	{
		ServerSessionRef session = static_pointer_cast<ServerSession>(service->GetSession());
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int direction = NO_MOVE;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					if (PLAYERMOVETIME <= NOW)
					{
						direction = LEFT;
						PLAYERMOVETIME = NOW + chrono::milliseconds(500);
					}
					break;
				case sf::Keyboard::Right:
					if (PLAYERMOVETIME <= NOW)
					{
						direction = RIGHT;
						PLAYERMOVETIME = NOW + chrono::milliseconds(500);
					}
					break;
				case sf::Keyboard::Up:
					if (PLAYERMOVETIME <= NOW)
					{
						direction = UP;
						PLAYERMOVETIME = NOW + chrono::milliseconds(500);
					}
					break;
				case sf::Keyboard::Down:
					if (PLAYERMOVETIME <= NOW)
					{
						direction = DOWN;
						PLAYERMOVETIME = NOW + chrono::milliseconds(500);
					}
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Space:
					if (PLAYERMOVETIME <= NOW){
						session->GetPlayer()->SetAttackTime();
						session->AttackPkt(session->GetPlayer()->GetId(), SCRACH);
					}
					break;
				}
				if (NO_MOVE != direction) 
					session->MovePkt(direction, duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count());
			}
		}

		window.clear();
		Update(service);
		window.display();
	}


	GThreadManager->Join();
}