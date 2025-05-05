#pragma once
#include "pch.h"
#include "queue"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "ServerSession.h"
#include <deque>

class SFSystem
{
private:
    SFSystem();
    ~SFSystem();
    SFSystem(const SFSystem&) = delete;
    SFSystem& operator=(const SFSystem&) = delete;

public:
	static SFSystem& GetInstance()
	{
		static SFSystem instance;
		return instance;
	}
	
	void InitText();
	void InitBox();

	void SetText(SystemText _st, const char _str[]);
	void SetChat(const string& _mess);
	void SetInnerBoxSize(SystemBox _sb, int _inner, int _maxinner);
	void Update(ServerSessionRef& _session);
	void Draw();
	
    sf::RenderWindow* GetWindow() const { return window;}
    void SetWindow(sf::RenderWindow* _win) {window = _win;}

    sf::Font& GetFont() {return font;}
    void SetFont(const sf::Font& _f) {font = _f;}

    sf::Texture* GetMonster() const { return monster;}
    void SetMonster(sf::Texture* _tex) { monster = _tex;}

    sf::Texture* GetPlayer() const { return player;}
    void SetPlayer(sf::Texture* _tex) {player = _tex;}

    sf::Texture* GetPlayerAttack() const { return player_attack;}
    void SetPlayerAttack(sf::Texture* _tex) {player_attack = _tex;}

    sf::Texture* GetBoard() const {return board;}
    void SetBoard(sf::Texture* _tex) { board = _tex;}

    const PlayerRef& GetUser() const { return user; }
    void SetUser(PlayerRef& _player) { user = _player; }

    // UI 관련 함수들
    void InitializeUI();
    void LoadUITextures();
    void ToggleInventory();
    void ToggleEquipment();
    void DrawUI();
    void HandleUIInput(const sf::Event& event);
    void UpdateInventoryTab(Protocol::InventoryTab _tab);
    // 골드 관련 함수
    void SetGold(uint64 _gold);
    void UpdateGoldRanking(const vector<GoldRanking>& rankings);
    void DrawGoldRanking();

    void HandleMousePressed(const sf::Vector2i& _mousePos);
    void HandleMouseReleased(const sf::Vector2i& _mousePos);
    void HandleMouseMoved(const sf::Vector2i& _mousePos);
    void StartDraggingWindow(sf::Sprite& sprite, bool& _isDragging);
    bool TryHandleInventoryTabClick(const sf::Vector2i& _mousPos);
    bool TryStartDraggingItem(const sf::Vector2i& _mousePos);
    bool TryStartDraggingEquip(const sf::Vector2i& _mousePos);
    void TryDropItem(const sf::Vector2i& _mousePos);
    bool TryDropToInventory(const sf::Vector2i& _mousePos);
    bool TryDropToEquipment(const sf::Vector2i& _mousePos);
    bool IsValidEquipmentType(Protocol::EquipmentSlot slotType, uint32_t itemId);
    pair<bool, int> FindInventorySlot(const sf::Vector2i& _mousePos);

    void UpdateTooltip(const sf::Vector2i& mousePos);
    void DrawTooltip();

    void InitializeQuickSlots();
    void DrawQuickSlots();
    void HandleQuickSlotInput(const sf::Event& event);
    bool TryDropToQuickSlot(const sf::Vector2i& mousePos, uint32 _slotIndex);
    void UseQuickSlotItem(uint32 _slotIndex);

    void AddSystemMessage(const std::string& msg);
    void DrawSystemMessages();

    void InitializeGoldSlot();
    void DrawGoldSlot();

    void SetQuickSlotReset(uint64 _slotIndex);
private:
    void UpdatePlayerStats(const PlayerRef& _player);

public:
	sf::TcpSocket				socket;

private:
    PlayerRef                   user;

	vector<sf::Text>			text;
	vector<sf::RectangleShape>	box;
	vector<sf::Text>			systemChat;

	sf::RenderWindow*			window;
	sf::Font					font;

	sf::Texture*				monster;
	sf::Texture*				player;	
	sf::Texture*				player_attack;
	sf::Texture*				board;

    // UI 관련 변수들
    bool isInventoryOpen;
    bool isEquipmentOpen;
    Protocol::InventoryTab currentInventoryTab;  // EQUIP: 장비, CONSUM: 소비, MISC: 기타
    
    // UI 텍스처들
    sf::Texture equipmentTexture;
    sf::Sprite equipmentSprite;

    sf::Texture inventoryTexture;
    sf::Sprite inventorySprite;

    // UI 렉트들 (4등분)
    sf::Sprite goldConsumeSprite;
    sf::Texture goldConsumeTexture;
    std::vector<sf::IntRect> goldConsumeRects;
    
    // 마우스 드래그 관련 변수
    bool isDraggingInventory;
    bool isDraggingEquipment;
    sf::Vector2f dragOffset;

    // 골드 관련 변수
    int myGold;
    sf::Text goldText;
    sf::Sprite goldCoinSprite;

    void DrawInventorySlots();
    void DrawEquipmentSlots();
    void DrawEquipmentStat();

    const vector<string> equipStr = { "NONE", "WEAPON", "HELMET", "TOP", "BOTTOM" };
    const sf::Vector2f equipPos[5] = { {0,0}, {67,119},{125,119},{182,119},{239,118} };
    const int baseX[5] = { 38, 90, 145, 198, 250 };
    const int baseY[5] = { 80, 135, 190, 245, 300 };

    bool isDraggingItem = false;
    int dragStartSlot = -1;
    int dragStartTab = -1;
    INVEN draggedItem;

    sf::Sprite draggedItemSprite;  // 드래그 중인 아이템 이미지

    // 아이템 툴팁 관련 변수
    sf::RectangleShape tooltipBox;
    sf::Text tooltipText;
    bool showTooltip;
    int hoveredItemId;
    Protocol::InventoryTab hoveredSlotType;  // -1: 장비창, 0~2: 인벤토리 탭

    // 퀵슬롯 관련 변수
    sf::RectangleShape quickSlotBoxes[2];  // 퀵슬롯 상자 2개
    sf::Text quickSlotNumbers[2];          // 퀵슬롯 번호 (1, 2)
    int quickSlotItems[2];                 // 퀵슬롯에 들어있는 아이템의 slot_index
    Protocol::InventoryTab quickSlotTabs[2];                  // 퀵슬롯에 들어있는 아이템의 tab_type

    deque<sf::Text> messageQueue;           // 우측상단 메시지 큐
    const size_t maxMessageCount = 10;


    sf::RectangleShape goldSlotBox;         // 우측 상단 골드 박스
    sf::Sprite goldSlotIcon;                // 골드 아이콘
    sf::Text goldSlotText;                  // 골드 수치 텍스트

    
    vector<GoldRanking> goldRankings;
    sf::RectangleShape goldRankingBox;
    sf::Text goldRankingTitle;
    sf::Text goldRankingTexts[5];

};

