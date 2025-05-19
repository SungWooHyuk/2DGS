#pragma once
#include "pch.h"
#include "ServerSession.h"
#include "Player.h"
#include "Client.h"
#include "GLogger.h"
Player::Player(sf::Texture& _attackT, sf::Texture& _playerT, int _x, int _y, int _x2, int _y2, POS _pos, TP _tp, int32 _id, const string& _name, const uint64 _gold)
	:Client(Protocol::PLAYER_TYPE_CLIENT, _pos, _id, _name), myTp(_tp), myGold(_gold)
{
	Sprite.setTexture(_playerT);
	Sprite.setTextureRect(sf::IntRect(_x, _y, _x2, _y2));

	attackSprite.setTexture(_attackT);
	attackSprite.setTextureRect(sf::IntRect(_x, _y, _x2, _y2));

	Move(myPos.posx, myPos.posy);
}

void Player::SetAttackTime()
{
	myTp.attackEndTime = NOW + chrono::milliseconds(500);
	myTp.attackTime = NOW + chrono::milliseconds(1000);
}

void Player::SetStat(STAT _st)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "SetStat", "Level = {} HP = {} / {} MP = {} / {} EXP = {} / {} AP = {} DP = {} MPower = {} STR = {}",
		_st.level, _st.hp, _st.maxHp, _st.mp, _st.maxMp,
		_st.exp, _st.maxExp, _st.attackPower, _st.defencePower, _st.magicPower, _st.strength);

	myStat.hp = _st.hp;
	myStat.maxHp = _st.maxHp;
	myStat.mp = _st.mp;
	myStat.maxMp = _st.maxMp;
	myStat.exp = _st.exp;
	myStat.maxExp = _st.maxExp;
	myStat.level = _st.level;

	myStat.attackPower = _st.attackPower;
	myStat.defencePower = _st.defencePower;
	myStat.magicPower = _st.magicPower;
	myStat.strength = _st.strength;
}

const uint32 Player::GetEquip(Protocol::EquipmentSlot _slot) const
{
	auto it = myEquipments.find(_slot);
	return (it != myEquipments.end()) ? it->second : 0;
}
void Player::Draw()
{
	float fX = (myPos.posx - g_left_x) * 65.f + 1;
	float fY = (myPos.posy - g_top_y) * 65.f + 1;

	Sprite.setPosition(fX, fY);
	SFSYSTEM.GetWindow()->draw(Sprite);

	if (myTp.attackEndTime > NOW)
	{
		attackSprite.setPosition(fX, fY - 65);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX, fY + 65);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX + 65, fY);
		SFSYSTEM.GetWindow()->draw(attackSprite);
		attackSprite.setPosition(fX - 65, fY);
		SFSYSTEM.GetWindow()->draw(attackSprite);
	}
}

bool Player::Respawn(uint64 _exp, uint64 _hp, uint64 _x, uint64 _y)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "Respawn","EXP={} HP={} Pos=({}, {})", _exp, _hp, _x, _y);
	myStat.exp = _exp;
	myStat.hp = _hp;
	myPos.posx = _x;
	myPos.posy = _y;
	return true;
}

void Player::AddItem(const INVEN& _item)
{
	GLogger::LogWithContext(spdlog::level::info, myName ,"AddItem","ItemID = {} Slot = {} Tab = {}", 
		_item.itemId, _item.slot_index,static_cast<int>(_item.tab_type));
	myInventory.push_back(_item);
}
void Player::RemoveItem(Protocol::InventoryTab tabType, uint64 slotIndex)
{
	auto it = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tabType && item.slot_index == slotIndex;
		});
	if (it != myInventory.end())
	{
		GLogger::LogWithContext(spdlog::level::info, myName,"RemoveItem", "Removed ItemID = {} Slot = {}", it->itemId, it->slot_index);
		myInventory.erase(it);
	}
	else
	{
		GLogger::LogWithContext(spdlog::level::warn, myName, "RemoveItem", "Item not found PlayerID={} Tab={} Slot={}", static_cast<int>(tabType), slotIndex);
	}
}

void Player::MoveItem(Protocol::InventoryTab fromTab, uint64 fromSlot, Protocol::InventoryTab toTab, uint64 toSlot, uint64 quantity)
{
	auto it = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == fromTab && item.slot_index == fromSlot;
		});
	if (it != myInventory.end())
	{
		GLogger::LogWithContext(spdlog::level::info, myName, "MoveItem", "ItemID={} From[Tab={}, Slot={}] To[Tab={}, Slot={}]",
			it->itemId, static_cast<int>(fromTab), fromSlot, static_cast<int>(toTab), toSlot);
		it->tab_type = toTab;
		it->slot_index = toSlot;
	}
	else
	{
		GLogger::LogWithContext(spdlog::level::warn, myName, "MoveItem", "Tab={} Slot={}", static_cast<int>(fromTab), fromSlot);
	}
}

void Player::SetEquipToInventory(uint64 slotIndex, const INVEN& item)
{
	auto it = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& invItem) {
			return invItem.tab_type == item.tab_type && invItem.slot_index == slotIndex;
		});
	if (it != myInventory.end())
	{
		RemoveItem(it->tab_type, it->slot_index);
		AddItem(item);
	}
	else
		AddItem(item);
	
}

void Player::SwapItems(Protocol::InventoryTab tab1, uint64 slot1, Protocol::InventoryTab tab2, uint64 slot2)
{
	auto it1 = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tab1 && item.slot_index == slot1;
		});
	auto it2 = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tab2 && item.slot_index == slot2;
		});
	
	if (it1 != myInventory.end() && it2 != myInventory.end())
	{
		GLogger::LogWithContext(spdlog::level::info, myName, "SwapItems", "Swap Item1[ID={}, Tab={}, Slot={}] <--> Item2[ID={}, Tab={}, Slot={}]",
			it1->itemId, static_cast<int>(tab1), slot1, it2->itemId, static_cast<int>(tab2), slot2);
		swap(it1->tab_type, it2->tab_type);
		swap(it1->slot_index, it2->slot_index);
	}
	else
	{
		GLogger::LogWithContext(spdlog::level::warn, myName, "SwapItems", "Swap failed. Found: Item1[{}], Item2[{}]",
			it1 != myInventory.end(), it2 != myInventory.end());
	}
}

void Player::UpdateItemQuantity(Protocol::InventoryTab tabType, uint64 slotIndex, uint64 newQuantity)
{
	auto it = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tabType && item.slot_index == slotIndex;
		});
	if (it != myInventory.end())
	{
		GLogger::LogWithContext(spdlog::level::info, myName, "UpdateItemQuantity", "ItemID={} Slot={} Tab={} OldQty={} NewQty={}",
			it->itemId, slotIndex, static_cast<int>(tabType), it->quantity, newQuantity);

		it->quantity = newQuantity;
		if (newQuantity <= 0)
		{
			GLogger::LogWithContext(spdlog::level::info, myName, "UpdateItemQuantity", "Removing ItemID={} due to zero quantity",
				it->itemId);
			myInventory.erase(it);
		}
	}
	else
	{
		GLogger::LogWithContext(spdlog::level::warn, myName, "UpdateItemQuantity", "Item not found Tab={} Slot={}",
			static_cast<int>(tabType), slotIndex);
	}
}

void Player::UnEquip(Protocol::EquipmentSlot _slot, uint64 _itemId)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "UnEquip", "Slot={} UnEquipped ItemID={}", static_cast<int>(_slot), _itemId);
	myEquipments[_slot] = 0;
}

void Player::SetEquip(Protocol::EquipmentSlot _slot, uint32 _itemId)
{
	GLogger::LogWithContext(spdlog::level::info, myName, "SetEquip","Slot = {} Equipped ItemID = {}", static_cast<int>(_slot), _itemId);
	myEquipments[_slot] = _itemId;
}
bool Player::HasItem(Protocol::InventoryTab tabType, uint64 slotIndex) const
{
	return any_of(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tabType && item.slot_index == slotIndex;
		});
}

const INVEN* Player::GetItem(Protocol::InventoryTab tabType, uint64 slotIndex) const
{
	auto it = find_if(myInventory.begin(), myInventory.end(),
		[&](const INVEN& item) {
			return item.tab_type == tabType && item.slot_index == slotIndex;
		});
	return it != myInventory.end() ? &(*it) : nullptr;
}


void Player::SendEquipPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _inv_slot_index, Protocol::EquipmentSlot _slotIndex)
{
	if (ownerSession)
		ownerSession->EquipPkt(_itemId, _tabType, _inv_slot_index, _slotIndex);
}

void Player::SendUnEquipPkt(uint64 _itemId, uint64 _inv_slot_index, Protocol::InventoryTab _tabType, Protocol::EquipmentSlot _slotIndex)
{
	if (ownerSession)
		ownerSession->UnEquipPkt(_itemId, _inv_slot_index, _tabType, _slotIndex);

}

void Player::SendMoveInventoryItemPkt(Protocol::InventoryTab _fromTab, uint64 _fromIndex, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	if (ownerSession)
		ownerSession->MoveInventoryItemPkt(_fromTab, _fromIndex, _toTab, _toIndex);

}

void Player::SendConsumeItemPkt(uint64 _itemid, Protocol::InventoryTab _tabType, uint64 _slotIndex)
{
	if (ownerSession)
		ownerSession->ConsumeItemPkt(_itemid, _tabType, _slotIndex);
}

void Player::SendRemoveItemPkt(uint64 _itemId, Protocol::InventoryTab _tabType, uint64 _slotIndex)
{
	if (ownerSession)
		ownerSession->RemoveItemPkt(_itemId, _tabType, _slotIndex);

}

void Player::SendAddItemPkt(const INVEN& _inv)
{
	if (ownerSession)
		ownerSession->AddItemPkt(_inv);

}

void Player::SendSwapItemPkt(uint64 _fromItemId, Protocol::InventoryTab _fromTab, uint64 _fromIndex, uint64 _toItemId, Protocol::InventoryTab _toTab, uint64 _toIndex)
{
	if (ownerSession)
		ownerSession->SwapItemPkt(_fromItemId, _fromTab, _fromIndex, _toItemId, _toTab, _toIndex);
}

void Player::SendUpdateItemPkt(Protocol::InventoryTab _tabType, uint64 _itemId, uint64 _slotIndex, uint64 _quantity)
{
	if (ownerSession)
		ownerSession->UpdateItemPkt(_tabType, _itemId, _slotIndex, _quantity);
}
