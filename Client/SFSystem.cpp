#pragma once
#include "pch.h"
#include "SFSystem.h"
#include "Client.h"
#include "Player.h"
#include "Item.h"
SFSystem::SFSystem()
{
	text.resize(static_cast<int>(SystemText::CHAT) + 1);
	box.resize(static_cast<int>(SystemBox::EXPINNERBOX) + 1);

	InitText();
	InitBox();

	board = new sf::Texture;
	player = new sf::Texture;
	monster = new sf::Texture;
	player_attack = new sf::Texture;

	ASSERT_CRASH(board->loadFromFile("./Image/maps.bmp"));
	ASSERT_CRASH(player->loadFromFile("./Image/Player.png"));
	ASSERT_CRASH(monster->loadFromFile("./Image/Monster.png"));
	ASSERT_CRASH(player_attack->loadFromFile("./Image/player_Attack1.png"));
	ASSERT_CRASH(font.loadFromFile("cour.ttf"));

}

SFSystem::~SFSystem()
{
	delete			board;
	delete			player;
	delete			monster;
	delete			player_attack;
}

void SFSystem::InitText()
{
	for (int i = 0; i < text.size(); ++i)
	{
		text[i].setFont(font);
		text[i].setFillColor(sf::Color(WHITE));
		text[i].setStyle(sf::Text::Bold);
	}


	text[SystemText::HP].setPosition(1050, 450);
	text[SystemText::MP].setPosition(1050, 550);
	text[SystemText::MAXHP].setPosition(1200, 450);
	text[SystemText::MAXMP].setPosition(1200, 550);
	text[SystemText::EXP].setPosition(1050, 650);
	text[SystemText::MAXEXP].setPosition(1200, 650);
	text[SystemText::LEVEL].setPosition(1050, 350);
	text[SystemText::NAME].setPosition(0, 0);
	text[SystemText::PLAYERPOS].setPosition(0, 0);

}

void SFSystem::InitBox()
{
	for (int i = 0; i < 3; ++i)
	{
		box[i].setSize(sf::Vector2f(BOX));
		box[i].setFillColor(sf::Color(WHITE));
		box[i].setPosition(sf::Vector2f(INFORMATION_X, INFORMATION_HP_Y + i * 100));
		box[i].setOutlineThickness(2);
		box[i].setOutlineColor(sf::Color(255, 215, 0));
	}

	for (int i = 3; i < 6; ++i)
		box[i].setPosition(sf::Vector2f(INFORMATION_X, INFORMATION_HP_Y + (i - 3) * 100));

	box[SystemBox::HPINNERBOX].setFillColor(sf::Color(RED));
	box[SystemBox::MPINNERBOX].setFillColor(sf::Color(BLUE));
	box[SystemBox::EXPINNERBOX].setFillColor(sf::Color(MAGENTA));
}

void SFSystem::SetText(SystemText _st, const char _str[])
{
	text[_st].setString(_str);
}

void SFSystem::SetChat(const string& _mess)
{
	AddSystemMessage(_mess);
}

void SFSystem::SetInnerBoxSize(SystemBox _sb, int _inner, int _maxinner)
{
	box[_sb].setSize(sf::Vector2f(sf::Vector2f(((100.f * _inner / _maxinner) * 3), 30)));
}

void SFSystem::Update(ServerSessionRef& _session)
{
	ServerSessionRef session = _session;
	UpdatePlayerStats(session->GetPlayer());

	session->GetPlayer()->Draw();
	for (auto& pl : session->GetClients())
		pl.second->Draw();

	Draw();
}

void SFSystem::UpdatePlayerStats(const PlayerRef& _player)
{
	// HP 업데이트
	SetInnerBoxSize(SystemBox::HPINNERBOX, _player->GetStat().hp, _player->GetStat().maxHp);
	string hp = to_string(_player->GetStat().hp);
	string _hp = "HP : ";
	_hp += hp;
	SetText(SystemText::HP, _hp.c_str());

	string maxhp = to_string(_player->GetStat().maxHp);
	string _maxhp = " / ";
	_maxhp += maxhp;
	SetText(SystemText::MAXHP, _maxhp.c_str());

	// MP 업데이트
	SetInnerBoxSize(SystemBox::MPINNERBOX, _player->GetStat().mp, _player->GetStat().maxMp);
	string mp = to_string(_player->GetStat().mp);
	string _mp = "MP : ";
	_mp += mp;
	SetText(SystemText::MP, _mp.c_str());

	string maxmp = to_string(_player->GetStat().maxMp);
	string _maxmp = " / ";
	_maxmp += maxmp;
	SetText(SystemText::MAXMP, _maxmp.c_str());

	// 레벨 업데이트
	string lv = to_string(_player->GetStat().level);
	string _lv = "LV : ";
	_lv += lv;
	SetText(SystemText::LEVEL, _lv.c_str());

	// 경험치 업데이트
	SetInnerBoxSize(SystemBox::EXPINNERBOX, _player->GetStat().exp, _player->GetStat().maxExp);
	string exp = to_string(_player->GetStat().exp);
	string _exp = "EXP : ";
	_exp += exp;
	SetText(SystemText::EXP, _exp.c_str());

	string maxexp = to_string(_player->GetStat().maxExp);
	string _maxexp = " / ";
	_maxexp += maxexp;
	SetText(SystemText::MAXEXP, _maxexp.c_str());
}

void SFSystem::Draw()
{
	DrawGoldSlot();
	DrawGoldRanking();

	for (int i = 0; i < text.size(); ++i)
		window->draw(text[i]);

	for (int i = 0; i < box.size(); ++i)
		window->draw(box[i]);

	DrawQuickSlots();
	DrawSystemMessages();
}

void SFSystem::InitializeUI()
{
	isInventoryOpen = false;
	isEquipmentOpen = false;
	currentInventoryTab = Protocol::EQUIP;  // 기본값을 장비 탭으로 설정

	// 마우스 드래그 관련 변수 초기화
	isDraggingInventory = false;
	isDraggingEquipment = false;
	dragOffset = sf::Vector2f(0, 0);

	// 골드 초기화
	myGold = 0;
	goldText.setFont(font);
	goldText.setCharacterSize(20);
	goldText.setFillColor(sf::Color::Yellow);

	LoadUITextures();
	goldConsumeRects.clear();
	int w = goldConsumeTexture.getSize().x / 2;
	int h = goldConsumeTexture.getSize().y / 2;

	goldConsumeRects.push_back(sf::IntRect(0, 0, w, h));
	goldConsumeRects.push_back(sf::IntRect(w, 0, w, h));
	goldConsumeRects.push_back(sf::IntRect(0, h, w, h));
	goldConsumeRects.push_back(sf::IntRect(w, h, w, h));

	// 툴팁 초기화
	tooltipBox.setFillColor(sf::Color(50, 50, 50, 200));
	tooltipBox.setOutlineColor(sf::Color::White);
	tooltipBox.setOutlineThickness(1);

	tooltipText.setFont(font);
	tooltipText.setCharacterSize(14);
	tooltipText.setFillColor(sf::Color::White);

	showTooltip = false;
	hoveredItemId = -1;
	hoveredSlotType = Protocol::INVENTORY_TAB_NONE;

	// 퀵슬롯 초기화
	InitializeQuickSlots();
	// 골드 슬롯 초기화
	InitializeGoldSlot();

	// 골드 랭킹 초기화
	goldRankings.clear();
	goldRankingBox.setSize(sf::Vector2f(200, 150));
	goldRankingBox.setFillColor(sf::Color(40, 40, 40, 220));
	goldRankingBox.setOutlineColor(sf::Color(255, 215, 0));
	goldRankingBox.setOutlineThickness(2);
	goldRankingBox.setPosition(window->getSize().x - 315, window->getSize().y - 260);

	goldRankingTitle.setFont(font);
	goldRankingTitle.setCharacterSize(20);
	goldRankingTitle.setFillColor(sf::Color(255, 215, 0));
	goldRankingTitle.setStyle(sf::Text::Bold);
	goldRankingTitle.setString("Gold Ranking");
	goldRankingTitle.setPosition(goldRankingBox.getPosition().x + 10, goldRankingBox.getPosition().y + 10);

	for (int i = 0; i < 5; ++i)
	{
		goldRankingTexts[i].setFont(font);
		goldRankingTexts[i].setCharacterSize(16);
		goldRankingTexts[i].setFillColor(sf::Color::White);
		goldRankingTexts[i].setPosition(goldRankingBox.getPosition().x + 10,
			goldRankingBox.getPosition().y + 40 + i * 20);
		goldRankingTexts[i].setString("---");
	}
}

void SFSystem::LoadUITextures()
{
	if (!equipmentTexture.loadFromFile("Image/equipment.png"))
		std::cout << "Failed to load equipment.png" << std::endl;
	if (!goldConsumeTexture.loadFromFile("Image/gold_consume.png"))
		std::cout << "Failed to load gold_consume.png" << std::endl;
	if (!inventoryTexture.loadFromFile("Image/inventory.png"))
		std::cout << "Failed to load inventory.png" << std::endl;

	equipmentSprite.setTexture(equipmentTexture);
	goldConsumeSprite.setTexture(goldConsumeTexture);
	inventorySprite.setTexture(inventoryTexture);

	// 크기
	equipmentSprite.setScale(0.3f, 0.3f);
	goldConsumeSprite.setScale(0.4f, 0.4f);
	inventorySprite.setScale(0.3f, 0.3f);

	// 위치 (Y좌표 30만큼 아래)
	float windowWidth = window->getSize().x;
	float windowHeight = window->getSize().y;
	equipmentSprite.setPosition(windowWidth / 2 - (equipmentTexture.getSize().x * 0.3f) / 2, windowHeight / 2 - (equipmentTexture.getSize().y * 0.3f) / 2 + 30);
	inventorySprite.setPosition(windowWidth / 2 - (inventoryTexture.getSize().x * 0.3f) / 2, windowHeight / 2 - (inventoryTexture.getSize().y * 0.3f) / 2 + 30);
	goldConsumeSprite.setPosition(windowWidth / 2 - (goldConsumeTexture.getSize().x * 0.3f) / 2, windowHeight / 2 - (goldConsumeTexture.getSize().y * 0.3f) / 2 + 30);

}

void SFSystem::ToggleInventory()
{
	isInventoryOpen = !isInventoryOpen;
}

void SFSystem::ToggleEquipment()
{
	isEquipmentOpen = !isEquipmentOpen;
}

void SFSystem::UpdateInventoryTab(Protocol::InventoryTab _tab)
{
	currentInventoryTab = _tab;
}

void SFSystem::SetGold(uint64 _gold)
{
	myGold = _gold;
	goldText.setString(std::to_string(myGold));
}

void SFSystem::DrawInventorySlots()
{
	if (!isInventoryOpen || user == nullptr) return;

	auto itemIconMap = ITEM.GetIconMap();
	const auto& inventory = user->GetInventory();

	float invX = inventorySprite.getPosition().x;
	float invY = inventorySprite.getPosition().y;
	float invW = inventoryTexture.getSize().x * 0.3f;
	float invH = inventoryTexture.getSize().y * 0.3f;

	int slotRows = 5;
	int slotCols = 5;
	float tabHeight = 30;
	float slotH = (invH - tabHeight) / slotRows;
	float slotW = invW / slotCols;

	float startY = invY + tabHeight;

	// 슬롯 테두리 그리기
	for (int i = 0; i < 25; ++i) {
		int row = i / slotCols;
		int col = i % slotCols;
		float centerX = invX + baseX[col];
		float centerY = invY + baseY[row];

		// 해당 슬롯에 들어갈 아이템 찾기
		auto it = find_if(
			inventory.begin(), inventory.end(),
			[&](const INVEN& item) {
				return item.tab_type == currentInventoryTab && item.slot_index == i;
			}
		);

		if (it != inventory.end())
		{
			auto iconIt = itemIconMap.find(it->itemId);
			if (iconIt != itemIconMap.end())
			{
				sf::Sprite itemSprite;
				itemSprite.setTexture(*iconIt->second.first);
				itemSprite.setTextureRect(iconIt->second.second);

				float scaleX = (slotW * 0.5f) / itemSprite.getTextureRect().width;
				float scaleY = (slotH * 0.5f) / itemSprite.getTextureRect().height;
				itemSprite.setScale(scaleX, scaleY);

				float iconW = itemSprite.getTextureRect().width * scaleX;
				float iconH = itemSprite.getTextureRect().height * scaleY;
				itemSprite.setPosition(centerX - iconW * 0.5f, centerY - iconH * 0.5f);

				window->draw(itemSprite);

				if (it->tab_type == Protocol::CONSUME || it->tab_type == Protocol::MISC)
				{
					sf::Text quantityText;
					quantityText.setFont(font);
					quantityText.setString(std::to_string(it->quantity));
					quantityText.setStyle(sf::Text::Bold);
					quantityText.setCharacterSize(12);  // 작은 크기로 설정
					quantityText.setFillColor(sf::Color::Red);

					// 수량 텍스트를 아이템 스프라이트의 우측 하단에 위치
					float textX = centerX + (slotW * 0.1f);  // 우측으로 약간 이동
					float textY = centerY + (slotH * 0.1f);  // 하단으로 약간 이동
					quantityText.setPosition(textX, textY);

					window->draw(quantityText);
				}
			}
		}
	}
}
void SFSystem::DrawEquipmentSlots()
{
	if (!isEquipmentOpen || user == nullptr) return;

	// 장비창 4칸 위치
	auto itemIconMap = ITEM.GetIconMap();
	sf::Vector2f equipBase = equipmentSprite.getPosition();
	// 장비 타입 텍스트 설정
	sf::Text equipTypeText;
	equipTypeText.setFont(font);
	equipTypeText.setStyle(sf::Text::Bold);
	equipTypeText.setCharacterSize(12);
	equipTypeText.setFillColor(sf::Color::White);

	for (int i = 1; i < 5; ++i) {
		Protocol::EquipmentSlot slot = static_cast<Protocol::EquipmentSlot>(i);
		uint32_t itemId = user->GetEquip(slot);


		equipTypeText.setString(equipStr[i]);
		equipTypeText.setPosition(
			equipBase.x + equipPos[i].x - equipTypeText.getGlobalBounds().width / 2,
			equipBase.y + equipPos[i].y - 30.f  // 슬롯 위에 표시
		);
		window->draw(equipTypeText);

		if (itemId == 0) continue; // 장착 아이템 없음

		auto iconIt = itemIconMap.find(itemId);
		if (iconIt != itemIconMap.end()) {
			sf::Sprite icon;
			icon.setTexture(*iconIt->second.first);
			icon.setTextureRect(iconIt->second.second);

			float scale = 40.0f / std::max(icon.getTextureRect().width, icon.getTextureRect().height);
			icon.setScale(scale, scale);

			float iconW = icon.getTextureRect().width * scale;
			float iconH = icon.getTextureRect().height * scale;
			icon.setPosition(equipBase.x + equipPos[i].x - 20, equipBase.y + equipPos[i].y - 15);

			window->draw(icon);
		}
	}
}
void SFSystem::DrawUI()
{
	if (isInventoryOpen)
	{
		window->draw(inventorySprite);
		// 글씨 아래로
		float tabWidth = inventoryTexture.getSize().x * 0.3f / 3;
		float tabHeight = 30;
		float tabY = inventorySprite.getPosition().y + 10;
		float tabX = inventorySprite.getPosition().x;
		sf::RectangleShape tabRect(sf::Vector2f(tabWidth, tabHeight));
		tabRect.setFillColor(sf::Color(255, 80, 0, 80));
		tabRect.setPosition(tabX + (static_cast<int>(currentInventoryTab) - 1) * tabWidth, tabY);
		window->draw(tabRect);

		DrawInventorySlots();
	}
	if (isEquipmentOpen)
	{
		window->draw(equipmentSprite);
		DrawEquipmentSlots();
		DrawEquipmentStat();
	}

	// 드래그 중인 아이템 이미지 그리기
	if (isDraggingItem)
	{
		window->draw(draggedItemSprite);
	}

	// 툴팁 그리기
	DrawTooltip();
}

void SFSystem::HandleUIInput(const sf::Event& event)
{
	sf::Vector2i mousePos = sf::Mouse::getPosition(*window);

	switch (event.type)
	{
	case sf::Event::KeyPressed:
		if (event.key.code == sf::Keyboard::I) ToggleInventory();
		else if (event.key.code == sf::Keyboard::E) ToggleEquipment();
		break;

	case sf::Event::MouseButtonPressed:
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			HandleMousePressed(mousePos);
		}
		break;

	case sf::Event::MouseButtonReleased:
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			HandleMouseReleased(mousePos);
		}
		break;

	case sf::Event::MouseMoved:
		HandleMouseMoved(mousePos);
		break;

	default:
		break;
	}

	HandleQuickSlotInput(event);
}

void SFSystem::HandleMousePressed(const sf::Vector2i& _mousePos)
{
	// 인벤토리 아이템 드래그 시도
	if (isInventoryOpen && inventorySprite.getGlobalBounds().contains(_mousePos.x, _mousePos.y))
	{
		// 탭 클릭 체크
		if (TryHandleInventoryTabClick(_mousePos)) return;

		// 아이템 드래그 시도
		if (!TryStartDraggingItem(_mousePos))
		{
			// 아이템 드래그가 아닌 경우 창 이동 시작
			dragOffset = sf::Vector2f(_mousePos.x, _mousePos.y) - inventorySprite.getPosition();
			isDraggingInventory = true;
		}
	}

	// 장비 아이템 드래그 시도
	if (isEquipmentOpen && equipmentSprite.getGlobalBounds().contains(_mousePos.x, _mousePos.y))
	{
		if (!TryStartDraggingEquip(_mousePos)) {
			StartDraggingWindow(equipmentSprite, isDraggingEquipment);
		}
	}
}
void SFSystem::HandleMouseReleased(const sf::Vector2i& _mousePos)
{
	if (isDraggingItem)
	{
		// 퀵슬롯에 드롭 시도
		for (int i = 0; i < 2; ++i)
		{
			if (TryDropToQuickSlot(_mousePos, i))
			{
				isDraggingItem = false;
				dragStartSlot = -1;
				dragStartTab = -1;
				return;
			}
		}

		TryDropItem(_mousePos);
	}

	isDraggingInventory = false;
	isDraggingEquipment = false;
	isDraggingItem = false;
	dragStartSlot = -1;
	dragStartTab = -1;
}
void SFSystem::HandleMouseMoved(const sf::Vector2i& _mousePos)
{
	if (isDraggingInventory)
	{
		sf::Vector2f newPos = sf::Vector2f(_mousePos.x, _mousePos.y) - dragOffset;
		inventorySprite.setPosition(newPos);
	}

	if (isDraggingEquipment)
		equipmentSprite.setPosition(_mousePos.x + dragOffset.x, _mousePos.y + dragOffset.y);

	// 드래그 중인 아이템 이미지 업데이트
	if (isDraggingItem)
	{
		auto itemIconMap = ITEM.GetIconMap();
		auto iconIt = itemIconMap.find(draggedItem.itemId);
		if (iconIt != itemIconMap.end())
		{
			draggedItemSprite.setTexture(*iconIt->second.first);
			draggedItemSprite.setTextureRect(iconIt->second.second);

			// 인벤토리 아이템인 경우
			if (static_cast<int>(draggedItem.tab_type) >= 0)
			{
				float invW = inventoryTexture.getSize().x * 0.3f;
				float invH = inventoryTexture.getSize().y * 0.3f;
				float slotW = invW / 5;  // 5x5 그리드
				float slotH = (invH - 30) / 5;  // 탭 높이 제외
				float scaleX = (slotW * 0.5f) / draggedItemSprite.getTextureRect().width;
				float scaleY = (slotH * 0.5f) / draggedItemSprite.getTextureRect().height;
				draggedItemSprite.setScale(scaleX, scaleY);
			}
			// 장비창 아이템인 경우
			else
			{
				float scale = 40.0f / std::max(draggedItemSprite.getTextureRect().width, draggedItemSprite.getTextureRect().height);
				draggedItemSprite.setScale(0.5f, 0.5f);
			}

			draggedItemSprite.setPosition(_mousePos.x - draggedItemSprite.getGlobalBounds().width / 2,
				_mousePos.y - draggedItemSprite.getGlobalBounds().height / 2);
		}
	}

	// 툴팁 업데이트
	UpdateTooltip(_mousePos);
}
void SFSystem::StartDraggingWindow(sf::Sprite& _sprite, bool& _isDragging)
{
	dragOffset = _sprite.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*window));
	_isDragging = true;
}
bool SFSystem::TryHandleInventoryTabClick(const sf::Vector2i& _mousPos)
{
	float tabWidth = inventoryTexture.getSize().x * 0.3f / 3;
	float tabHeight = 30.f;
	float invX = inventorySprite.getPosition().x;
	float invY = inventorySprite.getPosition().y;

	if (_mousPos.x >= invX && _mousPos.x <= invX + tabWidth * 3 &&
		_mousPos.y >= invY && _mousPos.y <= invY + tabHeight)
	{
		int tabIndex = (_mousPos.x - invX) / tabWidth;
		if (tabIndex >= 0 && tabIndex < 3)
		{
			UpdateInventoryTab(static_cast<Protocol::InventoryTab>(tabIndex + 1));
			return true;
		}
	}
	return false;
}
pair<bool, int> SFSystem::FindInventorySlot(const sf::Vector2i& _mousePos)
{

	if (!isInventoryOpen) return { false, -1 };

	float invX = inventorySprite.getPosition().x;
	float invY = inventorySprite.getPosition().y;
	float invW = inventoryTexture.getSize().x * 0.3f;
	float invH = inventoryTexture.getSize().y * 0.3f;

	int slotRows = 5;
	int slotCols = 5;
	float tabHeight = 30.f;
	float slotW = invW / slotCols;
	float slotH = (invH - tabHeight) / slotRows;


	for (int i = 0; i < 25; ++i)
	{
		int row = i / slotCols;
		int col = i % slotCols;
		float centerX = invX + baseX[col];
		float centerY = invY + baseY[row];

		float scaleX = (slotW * 0.7f) / 65.0f;
		float scaleY = (slotH * 0.7f) / 65.0f;
		float iconW = 65.0f * scaleX;
		float iconH = 65.0f * scaleY;

		sf::FloatRect slotRect(centerX - iconW * 0.5f, centerY - iconH * 0.5f, iconW, iconH);
		if (slotRect.contains(_mousePos.x, _mousePos.y))
		{
			return { true, i };
		}
	}
}

bool SFSystem::TryStartDraggingItem(const sf::Vector2i& _mousePos)
{
	if (!isInventoryOpen) return false;

	pair<bool, int> pi = FindInventorySlot(_mousePos);
	if (pi.first)
	{
		auto it = std::find_if(user->GetInventory().begin(), user->GetInventory().end(),
			[&](const INVEN& item) {
				return item.tab_type == currentInventoryTab && item.slot_index == pi.second;
			});

		if (it != user->GetInventory().end())
		{
			isDraggingItem = true;
			dragStartSlot = pi.second;
			dragStartTab = static_cast<int>(currentInventoryTab);
			draggedItem = *it;

			auto iconInfo = Item::GetInstance().GetIconMap().at(it->itemId);
			draggedItemSprite.setTexture(*iconInfo.first);
			draggedItemSprite.setTextureRect(iconInfo.second);

			float invW = inventoryTexture.getSize().x * 0.3f;
			float invH = inventoryTexture.getSize().y * 0.3f;
			float slotW = invW / 5;
			float slotH = (invH - 30) / 5;
			float scaleX = (slotW * 0.5f) / iconInfo.second.width;
			float scaleY = (slotH * 0.5f) / iconInfo.second.height;
			draggedItemSprite.setScale(scaleX, scaleY);

			draggedItemSprite.setPosition(_mousePos.x - (iconInfo.second.width * scaleX * 0.5f),
				_mousePos.y - (iconInfo.second.height * scaleY * 0.5f));
		}
		return true;
	}
	return false;
}
bool SFSystem::TryStartDraggingEquip(const sf::Vector2i& _mousePos)
{
	sf::Vector2f equipBase = equipmentSprite.getPosition();

	for (int i = 1; i < 5; ++i)
	{
		sf::FloatRect slotRect(equipBase.x + equipPos[i].x - 20.f, equipBase.y + equipPos[i].y - 20.f, 40.f, 40.f);
		if (slotRect.contains(_mousePos.x, _mousePos.y))
		{
			Protocol::EquipmentSlot slotType = static_cast<Protocol::EquipmentSlot>(i);
			uint32_t itemId = user->GetEquip(slotType);
			if (itemId != 0)
			{
				isDraggingItem = true;
				dragStartSlot = i;
				dragStartTab = -1;
				draggedItem.itemId = itemId;
				draggedItem.tab_type = Protocol::INVENTORY_TAB_NONE;
				draggedItem.slot_index = i;
				return true;
			}
			break;
		}
	}
	return false;
}
void SFSystem::TryDropItem(const sf::Vector2i& _mousePos)
{
	if (TryDropToInventory(_mousePos)) return;
	TryDropToEquipment(_mousePos);
}
bool SFSystem::TryDropToInventory(const sf::Vector2i& _mousePos)
{
	if (!isInventoryOpen) return false;
	pair<bool, int> pi = FindInventorySlot(_mousePos);
	if (pi.first)
	{
		// 장비창에서 인벤토리로 이동
		if (draggedItem.tab_type == Protocol::INVENTORY_TAB_NONE)
		{
			// 목적지 슬롯에 있는 아이템 찾기
			const INVEN* destItem = user->GetItem(currentInventoryTab, pi.second);

			// 목적지 슬롯이 비어있는 경우
			if (!destItem)
			{
				// 기존 장비를 인벤토리에 추가
				/*INVEN newItem;
				newItem.itemId = draggedItem.itemId;
				newItem.tab_type = currentInventoryTab;
				newItem.slot_index = pi.second;
				newItem.quantity = 1;*/

				//user->SendAddItemPkt(newItem);
				//서버에서 UnEquip과 동시에 인벤토리 pi.second 위치에 추가
				user->SendUnEquipPkt(draggedItem.itemId, pi.second, Protocol::EQUIP, static_cast<Protocol::EquipmentSlot>(draggedItem.slot_index));
				return true;
			}
		}
		// 인벤토리 내에서 이동
		else
		{
			// 원위치에 놓은 경우 아무것도 하지 않음
			if (draggedItem.tab_type == currentInventoryTab && dragStartSlot == pi.second)
				return true;

			const INVEN* destItem = user->GetItem(currentInventoryTab, pi.second);
			const INVEN* srcItem = user->GetItem(draggedItem.tab_type, dragStartSlot);

			if (srcItem)
			{
				// 목적지 슬롯이 비어있는 경우
				if (!destItem)
				{
					user->SendMoveInventoryItemPkt(draggedItem.tab_type, dragStartSlot, currentInventoryTab, pi.second);
					//user->MoveItem(draggedItem.tab_type, dragStartSlot, currentInventoryTab, pi.second, srcItem->quantity);
					return true;
				}
				// 목적지 슬롯에 다른 아이템이 있는 경우
				else if (destItem->itemId != srcItem->itemId)
				{
					user->SendSwapItemPkt(draggedItem.itemId, draggedItem.tab_type, dragStartSlot, destItem->itemId, currentInventoryTab, pi.second);
					//user->SwapItems(draggedItem.tab_type, dragStartSlot, currentInventoryTab, pi.second);
					return true;
				}
				// 같은 아이템인 경우 (소비 아이템 또는 기타 아이템)
				else if (currentInventoryTab == Protocol::CONSUME || currentInventoryTab == Protocol::MISC)
				{
					int maxQuantity = (currentInventoryTab == Protocol::CONSUME) ? 99 : 200;
					int totalQuantity = destItem->quantity + srcItem->quantity;
					if (totalQuantity <= maxQuantity)
					{
						user->SendUpdateItemPkt(currentInventoryTab, destItem->itemId, destItem->slot_index, totalQuantity);
						//user->UpdateItemQuantity(currentInventoryTab, pi.second, totalQuantity);
						user->SendRemoveItemPkt(srcItem->itemId, currentInventoryTab, srcItem->slot_index);
						//user->RemoveItem(draggedItem.tab_type, dragStartSlot);
					}
					else
					{
						int remainingQuantity = totalQuantity - maxQuantity;
						//user->UpdateItemQuantity(currentInventoryTab, pi.second, maxQuantity);
						//user->UpdateItemQuantity(draggedItem.tab_type, dragStartSlot, remainingQuantity);
						user->SendUpdateItemPkt(currentInventoryTab, destItem->itemId, destItem->slot_index, maxQuantity);
						user->SendUpdateItemPkt(currentInventoryTab, srcItem->itemId, srcItem->slot_index, remainingQuantity);
					}
					return true;
				}
			}
		}
	}
	return false;
}
bool SFSystem::TryDropToEquipment(const sf::Vector2i& _mousePos)
{
	if (!isEquipmentOpen) return false;

	sf::Vector2f equipBase = equipmentSprite.getPosition();

	for (int i = 1; i < 5; ++i)
	{
		sf::FloatRect slotRect(equipBase.x + equipPos[i].x - 20.f, equipBase.y + equipPos[i].y - 20.f, 40.f, 40.f);
		if (slotRect.contains(_mousePos.x, _mousePos.y))
		{
			Protocol::EquipmentSlot slotType = static_cast<Protocol::EquipmentSlot>(i);

			//// 장비창 내 이동인 경우
			//if (draggedItem.tab_type == Protocol::INVENTORY_TAB_NONE)
			//{
			//	// 같은 슬롯으로 이동하는 경우 무시
			//	if (dragStartSlot == i + 1) return true;

			//	// 장비 타입 체크
			//	if (!IsValidEquipmentType(slotType, draggedItem.itemId)) return false;

			//	// 장비 교환
			//	uint32_t oldItemId = user->GetEquip(slotType);
			//	user->SetEquip(slotType, draggedItem.itemId);
			//	user->SetEquip(static_cast<Protocol::EquipmentSlot>(dragStartSlot), oldItemId);
			//	return true;
			//}
			//// 인벤토리에서 장비창으로 이동
			//else
			//{
				// 장비 타입 체크
			if (!IsValidEquipmentType(slotType, draggedItem.itemId)) return false;

			//uint32_t oldEquipId = user->GetEquip(slotType);
			/*user->SetEquip(slotType, draggedItem.itemId);
			user->RemoveItem(draggedItem.tab_type, dragStartSlot);*/

			user->SendEquipPkt(draggedItem.itemId, draggedItem.tab_type, dragStartSlot, slotType);
			
			/*if (oldEquipId != 0)
			{
				INVEN oldEquip;
				oldEquip.itemId = oldEquipId;
				oldEquip.quantity = 1;
				oldEquip.slot_index = dragStartSlot;
				oldEquip.tab_type = draggedItem.tab_type;
				user->AddItem(oldEquip);
			}*/
			return true;
			//}
		}
	}
	return false;
}

bool SFSystem::IsValidEquipmentType(Protocol::EquipmentSlot slotType, uint32_t itemId)
{
	switch (slotType)
	{
	case Protocol::WEAPON:
		return ITEM.IsWeaponType(itemId);
	case Protocol::HELMET:
		return ITEM.IsHelmentType(itemId);
	case Protocol::TOP:
		return ITEM.IsTopType(itemId);
	case Protocol::BOTTOM:
		return ITEM.IsBottomType(itemId);
	default:
		return false;
	}
}

void SFSystem::DrawEquipmentStat()
{
	if (!isEquipmentOpen) return;
	float equipX = equipmentSprite.getPosition().x;
	float equipY = equipmentSprite.getPosition().y;
	float equipW = equipmentTexture.getSize().x * 0.3f;
	float equipH = equipmentTexture.getSize().y * 0.3f;
	float statStartY = equipY + equipH - 240;
	float statX = equipX + 40;
	int fontSize = 20;
	int lineGap = 24;

	// 장비창에 장착된 아이템들의 스탯 합산
	int totalAttack = 0;
	int totalDefence = 0;
	int totalMagicDefence = 0;
	int totalStrength = 0;

	for (int i = 1; i < 5; ++i)
	{
		uint32_t itemId = user->GetEquip(static_cast<Protocol::EquipmentSlot>(i));
		if (itemId != 0)
		{
			auto item = ITEM.GetItem(itemId);
			if (item)
			{
				totalAttack += item->equipmentInfo.attackPower;
				totalDefence += item->equipmentInfo.defensePower;
				totalMagicDefence += item->equipmentInfo.magicPower;
				totalStrength += item->equipmentInfo.strength;
			}
		}
	}

	vector<pair<string, int>> statList = {
		{"Attack", totalAttack},
		{"Defence", totalDefence},
		{"M_Defence", totalMagicDefence},
		{"Strength", totalStrength}
	};

	for (int i = 0; i < statList.size(); ++i) {
		sf::Text statText;
		statText.setFont(font);
		statText.setCharacterSize(fontSize);
		statText.setFillColor(sf::Color::White);
		statText.setStyle(sf::Text::Bold);
		statText.setString(statList[i].first + " : " + std::to_string(statList[i].second));
		statText.setPosition(statX, statStartY + i * lineGap);
		window->draw(statText);
	}
}

void SFSystem::UpdateTooltip(const sf::Vector2i& mousePos)
{
	showTooltip = false;
	if (isInventoryOpen)
	{
		pair<bool, int> slot = FindInventorySlot(mousePos);
		if (slot.first)
		{
			const INVEN* item = user->GetItem(currentInventoryTab, slot.second);
			if (item)
			{
				hoveredItemId = item->itemId;
				hoveredSlotType = currentInventoryTab;
				showTooltip = true;
			}
		}
	}

	if (isEquipmentOpen)
	{
		sf::Vector2f equipBase = equipmentSprite.getPosition();
		for (int i = 0; i < 4; ++i)
		{
			sf::FloatRect slotRect(equipBase.x + equipPos[i].x - 20.f, equipBase.y + equipPos[i].y - 20.f, 40.f, 40.f);
			if (slotRect.contains(mousePos.x, mousePos.y))
			{
				uint32_t itemId = user->GetEquip(static_cast<Protocol::EquipmentSlot>(i));
				if (itemId != 0)
				{
					hoveredItemId = itemId;
					hoveredSlotType = Protocol::INVENTORY_TAB_NONE;
					showTooltip = true;
				}
				break;
			}
		}
	}

	if (showTooltip)
	{
		auto item = ITEM.GetItem(hoveredItemId);
		if (item)
		{
			string tooltipContent;

			// 이름 추가
			if (!item->name.empty())
				tooltipContent += "name = " + item->name + "\n";

			// 효과 값 추가
			if (item->effectValue != 0)
				tooltipContent += "effectValue = " + to_string(item->effectValue) + "\n";

			// 필요 레벨 추가
			if (item->requiredLevel > 0)
				tooltipContent += "requiredLevel = " + to_string(item->requiredLevel) + "\n";

			// 설명 추가
			if (!item->description.empty())
				tooltipContent += "description = " + item->description + "\n";

			// 장비 정보 추가
			if (item->equipmentInfo.attackPower != 0)
				tooltipContent += "attackPower = " + to_string(item->equipmentInfo.attackPower) + "\n";

			if (item->equipmentInfo.defensePower != 0)
				tooltipContent += "defensePower = " + to_string(item->equipmentInfo.defensePower) + "\n";

			if (item->equipmentInfo.magicPower != 0)
				tooltipContent += "magicPower = " + to_string(item->equipmentInfo.magicPower) + "\n";

			if (item->equipmentInfo.strength != 0)
				tooltipContent += "strength = " + to_string(item->equipmentInfo.strength) + "\n";

			// 수량 정보 추가
			if (hoveredSlotType != Protocol::INVENTORY_TAB_NONE) // 인벤토리 아이템인 경우
			{
				pair<bool, int> slot = FindInventorySlot(mousePos);
				if (slot.first)
				{
					const INVEN* invItem = user->GetItem(hoveredSlotType, slot.second);
					if (invItem)
					{
						tooltipContent += "quantity = " + to_string(invItem->quantity) + "\n";
					}
				}
			}
			else // 장비창 아이템인 경우
				tooltipContent += "quantity = 1\n";

			tooltipText.setString(tooltipContent);
			sf::FloatRect textBounds = tooltipText.getLocalBounds();
			tooltipBox.setSize(sf::Vector2f(textBounds.width + 20, textBounds.height + 20));
			tooltipBox.setPosition(mousePos.x + 15, mousePos.y + 15);
			tooltipText.setPosition(mousePos.x + 25, mousePos.y + 25);
		}
	}
}
void SFSystem::DrawTooltip()
{
	if (showTooltip)
	{
		window->draw(tooltipBox);
		window->draw(tooltipText);
	}
}

void SFSystem::InitializeQuickSlots()
{
	// 퀵슬롯 상자 초기화
	for (int i = 0; i < 2; ++i)
	{
		quickSlotBoxes[i].setSize(sf::Vector2f(40, 40));
		quickSlotBoxes[i].setFillColor(sf::Color(50, 50, 50, 200));
		quickSlotBoxes[i].setOutlineColor(sf::Color::White);
		quickSlotBoxes[i].setOutlineThickness(1);

		// EXP 텍스트 아래에 위치하도록 설정
		float expY = text[SystemText::EXP].getPosition().y + 50;
		quickSlotBoxes[i].setPosition(1050 + i * 50, expY);

		// 퀵슬롯 번호 초기화
		quickSlotNumbers[i].setFont(font);
		quickSlotNumbers[i].setCharacterSize(12);
		quickSlotNumbers[i].setFillColor(sf::Color::White);
		quickSlotNumbers[i].setString(std::to_string(i + 1));

		// 번호 위치 설정 (우측 하단)
		sf::FloatRect boxBounds = quickSlotBoxes[i].getGlobalBounds();
		quickSlotNumbers[i].setPosition(boxBounds.left + boxBounds.width - 15,
			boxBounds.top + boxBounds.height - 15);

		// 초기값 설정
		quickSlotItems[i] = -1;
		quickSlotTabs[i] = Protocol::INVENTORY_TAB_NONE;
	}
}

void SFSystem::DrawQuickSlots()
{
	for (int i = 0; i < 2; ++i)
	{
		window->draw(quickSlotBoxes[i]);
		window->draw(quickSlotNumbers[i]);

		// 퀵슬롯에 아이템이 있는 경우 아이템 아이콘 표시
		if (quickSlotItems[i] != -1)
		{
			auto it = std::find_if(user->GetInventory().begin(), user->GetInventory().end(),
				[&](const INVEN& item) {
					return static_cast<int>(item.tab_type) == quickSlotTabs[i] && item.slot_index == quickSlotItems[i];
				});

			if (it != user->GetInventory().end())
			{
				auto itemIconMap = ITEM.GetIconMap();
				auto iconIt = itemIconMap.find(it->itemId);
				if (iconIt != itemIconMap.end())
				{
					sf::Sprite icon;
					icon.setTexture(*iconIt->second.first);
					icon.setTextureRect(iconIt->second.second);

					float scale = 40.0f / std::max(icon.getTextureRect().width, icon.getTextureRect().height);
					icon.setScale(scale, scale);

					sf::FloatRect boxBounds = quickSlotBoxes[i].getGlobalBounds();
					icon.setPosition(boxBounds.left + (boxBounds.width - icon.getGlobalBounds().width) / 2,
						boxBounds.top + (boxBounds.height - icon.getGlobalBounds().height) / 2);

					window->draw(icon);
				}
			}
		}
	}
}

void SFSystem::HandleQuickSlotInput(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Num1 && quickSlotItems[0] != -1)
		{
			UseQuickSlotItem(0);
		}
		else if (event.key.code == sf::Keyboard::Num2 && quickSlotItems[1] != -1)
		{
			UseQuickSlotItem(1);
		}
	}
}

bool SFSystem::TryDropToQuickSlot(const sf::Vector2i& mousePos, uint32 slotIndex)
{
	if (slotIndex < 0 || slotIndex >= 2) return false;

	sf::FloatRect boxBounds = quickSlotBoxes[slotIndex].getGlobalBounds();
	if (boxBounds.contains(mousePos.x, mousePos.y))
	{
		// 소비 아이템인 경우에만 퀵슬롯에 추가 가능
		if (draggedItem.tab_type == Protocol::CONSUME)  // CONSUME 탭인지 명시적으로 체크
		{
			quickSlotItems[slotIndex] = dragStartSlot;
			quickSlotTabs[slotIndex] = currentInventoryTab;
			return true;
		}
	}
	return false;
}

void SFSystem::UseQuickSlotItem(uint32 slotIndex)
{
	if (quickSlotItems[slotIndex] == -1) return;

	auto it = std::find_if(user->GetInventory().begin(), user->GetInventory().end(),
		[&](const INVEN& item) {
			return item.tab_type == quickSlotTabs[slotIndex] && item.slot_index == quickSlotItems[slotIndex];
		});

	if (it != user->GetInventory().end())
	{
		if (it->quantity > 0)
		{
			//user->UpdateItemQuantity(quickSlotTabs[slotIndex], quickSlotItems[slotIndex], newQuantity);
			//user->SendUpdateItemPkt(quickSlotTabs[slotIndex], it->itemId, quickSlotItems[slotIndex], it->quantity - 1);
			user->SendConsumeItemPkt(it->itemId, quickSlotTabs[slotIndex], quickSlotItems[slotIndex]);
			// 왔을때 or 서버에서 처리 후 알려주기 
			/*if (newQuantity <= 0)
			{
				user->RemoveItem(quickSlotTabs[slotIndex], quickSlotItems[slotIndex]);
				quickSlotItems[slotIndex] = -1;
				quickSlotTabs[slotIndex] = Protocol::INVENTORY_TAB_NONE;
			}*/
		}
	}
}
void SFSystem::AddSystemMessage(const std::string& msg)
{
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Bold);
	text.setScale(sf::Vector2f(0.5f, 0.5f));
	text.setString(msg);

	// 최대 개수 초과 시 오래된 메시지 삭제
	if (messageQueue.size() >= maxMessageCount)
		messageQueue.pop_front();
	messageQueue.push_back(text);
}

void SFSystem::DrawSystemMessages()
{
	float startX = window->getSize().x - 400; // 우측상단 기준
	float startY = 20;
	float lineGap = 18;
	int idx = 0;
	for (const auto& msg : messageQueue)
	{
		sf::Text drawText = msg;
		drawText.setPosition(startX, startY + idx * lineGap);

		// 메시지 배경 박스 그리기
		sf::FloatRect textBounds = drawText.getGlobalBounds();
		sf::RectangleShape msgBox(sf::Vector2f(textBounds.width + 20, textBounds.height + 10));
		msgBox.setFillColor(sf::Color(40, 40, 40, 220));
		msgBox.setOutlineColor(sf::Color(255, 215, 0));
		msgBox.setOutlineThickness(1);
		msgBox.setPosition(startX - 10, startY + idx * lineGap - 5);

		window->draw(msgBox);
		window->draw(drawText);
		++idx;
	}
}

void SFSystem::InitializeGoldSlot()
{
	// 좌측 하단 위치 및 크기
	float boxW = 200;
	float boxH = 40;
	float startX = 0;
	float startY = 0;
	goldSlotBox.setSize(sf::Vector2f(boxW, boxH));
	goldSlotBox.setFillColor(sf::Color(40, 40, 40, 220));
	goldSlotBox.setOutlineColor(sf::Color(255, 215, 0));
	goldSlotBox.setOutlineThickness(2);
	goldSlotBox.setPosition(startX, startY);

	// 골드 아이콘
	goldSlotIcon.setTexture(goldConsumeTexture);
	goldSlotIcon.setTextureRect(goldConsumeRects[0]);
	goldSlotIcon.setScale(0.05f, 0.05f);
	goldSlotIcon.setPosition(startX + 8, startY + 4);

	// 골드 텍스트
	goldSlotText.setFont(font);
	goldSlotText.setCharacterSize(20);
	goldSlotText.setFillColor(sf::Color(255, 215, 0));
	goldSlotText.setStyle(sf::Text::Bold);
	goldSlotText.setString(std::to_string(myGold));
	goldSlotText.setPosition(startX + 50, startY + 8);
}

void SFSystem::DrawGoldSlot()
{
	goldSlotText.setString(std::to_string(myGold));
	window->draw(goldSlotBox);
	window->draw(goldSlotIcon);
	window->draw(goldSlotText);
}

void SFSystem::SetQuickSlotReset(uint64 _slotIndex)
{
	quickSlotItems[_slotIndex] = -1;
	quickSlotTabs[_slotIndex] = Protocol::INVENTORY_TAB_NONE;
}

void SFSystem::UpdateGoldRanking(const vector<GoldRanking>& rankings)
{
	goldRankings = rankings;
	for (int i = 0; i < 5; ++i)
	{
		if (i < rankings.size())
		{
			goldRankingTexts[i].setString(rankings[i].name + " : " + to_string(rankings[i].gold));
		}
		else
		{
			goldRankingTexts[i].setString("---");
		}
	}
}

void SFSystem::DrawGoldRanking()
{
	window->draw(goldRankingBox);
	window->draw(goldRankingTitle);
	for (int i = 0; i < 5; ++i)
	{
		window->draw(goldRankingTexts[i]);
	}
}



