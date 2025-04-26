#include "pch.h"
#include "DBPacketHandler.h"
#include "DBSession.h"
#include "DBProtocol.pb.h"
#include "Struct.pb.h"
void DBSession::OnConnected()
{
	cout << "DB Server Connected" << endl;
	Init();
	cout << "DB Init()" << endl;
}

void DBSession::OnDisconnected()
{
	cout << "DB Server Disconnected" << endl;
}

void DBSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	DBPacketHandler::HandlePacket(session, buffer, len);
}

void DBSession::OnSend(int32 len)
{
	// LOG
}
void DBSession::Init()
{
	setlocale(LC_ALL, "korean");
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2022FALLODBC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					printf("ODBC Connect OK \n");
				}
			}
		}
	}
}

string DBSession::WstringToString(const wstring& wstr)
{
	if (wstr.empty()) return std::string();

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(),
		NULL, 0, NULL, NULL);

	std::string result(size_needed, 0);

	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(),
		&result[0], size_needed, NULL, NULL);

	return result;
}

bool DBSession::GetAllItem()
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring proc = L"{CALL sp_get_equipment_item}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// 결과 처리 가능 (생략 또는 Result 처리 코드 삽입 가능)
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}
}

bool DBSession::CheckUserHaveItem(string _name, uint32 _itemId)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	int result = 0;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_check_item_user (?, ?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_itemId, 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_LONG, &result, 0, NULL);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return result == 1;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

bool DBSession::FarmingItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_farming_item (?, ?, ?, ?, ?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_itemId, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_amount, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_tab_type, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_slot_index, 0, NULL);

	// _tab_type이 1이면 그냥 새로 추가(수량증가가 아님)
	// _tab_type이 2,3 - 소비,기타면 수량증가 일 수 있음.
	// 그런데 서버에서 slot_index를 보낼 때 장비라면 어차피 다음 슬롯으로 지정해줬을것이고, 25칸을 넘는지도 체크했을테니 상관없으며
	// 소비,기타도 99가 넘는지 수량체크를 서버에서 진행한 뒤 디비에 넘기는 것이기에 그냥 보내고 결과만 받으면 된다.

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "FarmingItem add success" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "FarmingItem add fail" << endl;
		return false;
	}
}

bool DBSession::GetUserEquipment(string _name, vector<Protocol::EquipmentItem>& _equip)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_get_user_equipment (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		cout << "GetUserEquipment: sp_get_equipment fail" << endl;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}
	UserBindEquipment bindInfo;

	SQLBindCol(hstmt, 1, SQL_C_WCHAR, bindInfo.user_id, sizeof(bindInfo.user_id), &bindInfo.cb_user_id);
	SQLBindCol(hstmt, 2, SQL_C_LONG, &bindInfo.slot_weapon, 0, &bindInfo.cb_slot_weapon);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &bindInfo.slot_helmet, 0, &bindInfo.cb_slot_helmet);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &bindInfo.slot_top, 0, &bindInfo.cb_slot_top);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &bindInfo.slot_bottom, 0, &bindInfo.cb_slot_bottom);

	retcode = SQLFetch(hstmt);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		Protocol::EquipmentItem equip;
		equip.set_weapon(bindInfo.slot_weapon);
		equip.set_helmet(bindInfo.slot_helmet);
		equip.set_top(bindInfo.slot_top);
		equip.set_bottom(bindInfo.slot_bottom);
		
		_equip.push_back(equip);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return true;
	}
	else
	{
		cout << "equip fail" << endl;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}
}

bool DBSession::GetUserInfo(string _name, uint64 _pktId)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_get_user_info (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		cout << "sp_get_user_info fail" << endl;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	UserBindInfo bindInfo;

	SQLBindCol(hstmt, 1, SQL_C_WCHAR, bindInfo.user_id, sizeof(bindInfo.user_id), &bindInfo.cb_user_id);
	SQLBindCol(hstmt, 2, SQL_C_LONG, &bindInfo.pos_x, 0, &bindInfo.cb_pos_x);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &bindInfo.pos_y, 0, &bindInfo.cb_pos_y);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &bindInfo.level, 0, &bindInfo.cb_level);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &bindInfo.hp, 0, &bindInfo.cb_hp);
	SQLBindCol(hstmt, 6, SQL_C_LONG, &bindInfo.maxhp, 0, &bindInfo.cb_maxhp);
	SQLBindCol(hstmt, 7, SQL_C_LONG, &bindInfo.mp, 0, &bindInfo.cb_mp);
	SQLBindCol(hstmt, 8, SQL_C_LONG, &bindInfo.maxmp, 0, &bindInfo.cb_maxmp);
	SQLBindCol(hstmt, 9, SQL_C_LONG, &bindInfo.exp, 0, &bindInfo.cb_exp);
	SQLBindCol(hstmt, 10, SQL_C_LONG, &bindInfo.maxexp, 0, &bindInfo.cb_maxexp);
	SQLBindCol(hstmt, 11, SQL_C_LONG, &bindInfo.gold, 0, &bindInfo.cb_gold);

	retcode = SQLFetch(hstmt);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

		vector<Protocol::InventorySlot> inventoryList;
		vector<Protocol::EquipmentItem> equipmentList;

		string name = WstringToString(bindInfo.user_id);
		if (!GetUserInventory(name, inventoryList))
			return false;

		if (!GetUserEquipment(name, equipmentList))
			return false;
	
		DBProtocol::DS_LOGIN pkt;
		
		pkt.set_user_id(_pktId);
		pkt.set_success(true);
		auto player = pkt.add_player();
		player->add_playertype(Protocol::PlayerType::PLAYER_TYPE_CLIENT);
		player->set_id(_pktId);
		player->set_name(name);
		player->set_x(bindInfo.pos_x);
		player->set_y(bindInfo.pos_y);
		player->set_level(bindInfo.level);
		player->set_hp(bindInfo.hp);
		player->set_maxhp(bindInfo.maxhp);
		player->set_mp(bindInfo.mp);
		player->set_maxmp(bindInfo.maxmp);
		player->set_exp(bindInfo.exp);
		player->set_maxexp(bindInfo.maxexp);
		player->set_gold(bindInfo.gold);

		for (const auto& slot : inventoryList)
			*pkt.add_inventory() = slot;

		for (const auto& equip : equipmentList)
			*pkt.add_equipment() = equip;

		SendBufferRef sendBuffer = DBPacketHandler::MakeSendBuffer(pkt);
		Send(sendBuffer);
		
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return true;
	}
	else
	{
		cout << "Login Bind fail" << endl;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}
}

bool DBSession::GetUserInventory(string _name, vector<Protocol::InventorySlot>& _inven)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_get_user_inventory (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}
	UserBindInventory bindInfo;

	SQLBindCol(hstmt, 1, SQL_C_LONG, &bindInfo.item_id, 0, &bindInfo.cb_item_id);
	SQLBindCol(hstmt, 2, SQL_C_WCHAR, bindInfo.user_id, sizeof(bindInfo.user_id), &bindInfo.cb_user_id);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &bindInfo.quantity, 0, &bindInfo.cb_quantity);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &bindInfo.tab_type, 0, &bindInfo.cb_tab_type);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &bindInfo.slot_index, 0, &bindInfo.cb_slot_index);

	while (SQLFetch(hstmt) == SQL_SUCCESS)
	{
		Protocol::InventorySlot slot;
		slot.set_item_id(bindInfo.item_id);
		slot.set_quantity(bindInfo.quantity);
		slot.set_tab_type(bindInfo.tab_type);
		slot.set_slot_index(bindInfo.slot_index);

		_inven.push_back(slot);
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}

bool DBSession::InitializeUserInventoryEquipment(string _name)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_initialize_user_inventory_equipment (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "Initialize and reset success" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "Initialize and reset fail" << endl;
		return false;
	}
}

bool DBSession::UnEquipItem(string _name, E_EQUIP _type, uint32 _tab_type, uint32 _slot_index)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_unequip_item(? ,?, ?, ?)}";

	wstring typeStr;
	switch (_type) {
	case E_EQUIP::WEAPON:  typeStr = L"weapon"; break;
	case E_EQUIP::HELMET:  typeStr = L"helmet"; break;
	case E_EQUIP::TOP:     typeStr = L"top";    break;
	case E_EQUIP::BOTTOM:  typeStr = L"bottom"; break;
	default:
		cout << "잘못된 장비 타입" << endl;
		return false;
	}

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		cout << "SQL 문 핸들 할당 실패 (UnEquipItem)" << endl;
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)typeStr.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_tab_type, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_slot_index, 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		return true;
	}
	else {
		return false;
	}
}

bool DBSession::UpdateUserInfo(USER_INFO _userInfo)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_userInfo.name.begin(), _userInfo.name.end());
	wstring proc = L"{CALL sp_update_user_info (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";  // 파라미터화된 쿼리 사용

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	// 파라미터 바인딩
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.posx, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.posy, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.level, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.hp, 0, NULL);
	SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.maxHp, 0, NULL);
	SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.mp, 0, NULL);
	SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.maxMp, 0, NULL);
	SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.exp, 0, NULL);
	SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.maxExp, 0, NULL);
	SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_userInfo.gold, 0, NULL);

	// 프로시저 실행
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "user info update success" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "user info update fail" << endl;
		return false;
	}
}

bool DBSession::UseItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index)
{

	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());

	wstring proc = L"{CALL sp_use_item(?, ?, ?, ?, ?)}";


	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		cout << "SQL 문 핸들 할당 실패 (UseItem)" << endl;
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_itemId, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_amount, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_tab_type, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_slot_index, 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		cout << "UseItem success" << endl;
		return true;
	}
	else {
		cout << "UseItem fail" << endl;
		return false;
	}

}

bool DBSession::InventoryToEquip(string _name, uint32 _itemId, uint32 _tab_type, uint32 _slot_index)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());

	wstring proc = L"{CALL sp_user_inventory_to_equip (?, ?, ?, ?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_itemId, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_tab_type, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &_slot_index, 0, NULL);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "Inventory to equip success" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "Inventory to equip fail" << endl;
		return false;
	}
}

bool DBSession::IsUserExists(string _name)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	int result = 0;

	wstring wname = wstring(_name.begin(), _name.end());
	wstring proc = L"{CALL sp_user_exists (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_LONG, &result, 0, NULL);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return result == 1;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

bool DBSession::RegisterNewUser(string _name)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	wstring wname = wstring(_name.begin(), _name.end());

	wstring proc = L"{CALL sp_register_new_user (?)}";

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 20, 0, (SQLWCHAR*)wname.c_str(), 0, NULL);
	
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "reigster success" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "register fail" << endl;
		return false;
	}
}
