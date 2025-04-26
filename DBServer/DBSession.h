#pragma once

#include <sqlext.h>
#include <session.h>
#include "Struct.pb.h"

class DBSession : public PacketSession
{
public:
    DBSession() {};
    ~DBSession() { cout << "~DBSession" << endl; };

    virtual void OnConnected() override;
    virtual void OnDisconnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
    virtual void OnSend(int32 len) override;

    void Init();
    
	bool GetAllItem();											// ���̺� �ִ� ��� ������ ��������
	bool CheckUserHaveItem(string _name, uint32 _itemId);		// ������ ������ �����ִ��� üũ ���ν���
	bool FarmingItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index);	// ������ ȹ�� �κ��丮�� 
	bool GetUserEquipment(string _name, vector<Protocol::EquipmentItem>& equip);						// ���� ���â ����
	bool GetUserInfo(string _name, uint64 _pktId);								// ���� ���� �������� (���,�κ� X)
	bool GetUserInventory(string _name, vector<Protocol::InventorySlot>& _inven);						// ���� �κ��丮 ����
	bool InitializeUserInventoryEquipment(string _name);		// �ʱ���·� 
	bool UnEquipItem(string _name, E_EQUIP _type, uint32 _tab_type, uint32 _slot_index);			// ��� ����
	bool UpdateUserInfo(USER_INFO _userInfo);						// ���� ������Ʈ ( ���� �뵵 )
	bool UseItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index); // ������ ���
	bool InventoryToEquip(string _name, uint32 _itemId, uint32 _tab_type, uint32 _slot_index);
	bool IsUserExists(string _name);
	bool RegisterNewUser(string _name);
	string WstringToString(const wstring& wstr); // wstring -> string ��ȯ

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
};

