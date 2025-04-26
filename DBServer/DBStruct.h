#pragma once

#include "pch.h"
#include <sqlext.h>

struct UserBindInfo {
	SQLWCHAR user_id[NAME_LEN];
	SQLLEN cb_user_id;

	SQLINTEGER pos_x;
	SQLLEN cb_pos_x;

	SQLINTEGER pos_y;
	SQLLEN cb_pos_y;

	SQLINTEGER level;
	SQLLEN cb_level;

	SQLINTEGER hp;
	SQLLEN cb_hp;

	SQLINTEGER maxhp;
	SQLLEN cb_maxhp;

	SQLINTEGER mp;
	SQLLEN cb_mp;

	SQLINTEGER maxmp;
	SQLLEN cb_maxmp;

	SQLINTEGER exp;
	SQLLEN cb_exp;

	SQLINTEGER maxexp;
	SQLLEN cb_maxexp;

	SQLINTEGER gold;
	SQLLEN cb_gold;
};

struct EquipmentInfo {
	SQLINTEGER attack_power;
	SQLLEN cb_attack_power;

	SQLINTEGER defense_power;
	SQLLEN cb_defense_power;

	SQLINTEGER magic_power;
	SQLLEN cb_magic_power;

	SQLINTEGER strength;
	SQLLEN cb_strength;
};

struct ItemDB {
	SQLINTEGER item_id;
	SQLLEN cb_item_id;

	SQLWCHAR name[50];
	SQLLEN cb_name;

	SQLINTEGER item_type;
	SQLLEN cb_item_type;

	SQLINTEGER equip_type;
	SQLLEN cb_equip_type;

	SQLINTEGER effect_type;
	SQLLEN cb_effect_type;

	SQLINTEGER effect_value;
	SQLLEN cb_effect_value;

	SQLINTEGER required_level;
	SQLLEN cb_required_level;

	SQLWCHAR description[100];
	SQLLEN cb_description;

	EquipmentInfo equipmentInfo;
};

struct UserBindEquipment {
	SQLWCHAR user_id[20];
	SQLLEN cb_user_id;

	SQLINTEGER slot_weapon;
	SQLLEN cb_slot_weapon;

	SQLINTEGER slot_helmet;
	SQLLEN cb_slot_helmet;

	SQLINTEGER slot_top;
	SQLLEN cb_slot_top;

	SQLINTEGER slot_bottom;
	SQLLEN cb_slot_bottom;

	SQL_TIMESTAMP_STRUCT updated_at;
	SQLLEN cb_updated_at;
};
struct UserBindInventory {
	SQLWCHAR user_id[20];
	SQLLEN cb_user_id;

	SQLINTEGER item_id;
	SQLLEN cb_item_id;

	SQLINTEGER quantity;
	SQLLEN cb_quantity;

	SQL_TIMESTAMP_STRUCT acquired_at;
	SQLLEN cb_acquired_at;

	SQLINTEGER tab_type;
	SQLLEN cb_tab_type;

	SQLINTEGER slot_index;
	SQLLEN cb_slot_index;
};