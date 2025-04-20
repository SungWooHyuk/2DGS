#pragma once

#include <sqlext.h>
#include <session.h>

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
    
	bool GetAllItem();											// 테이블에 있는 모든 아이템 가져오기
	bool CheckUserHaveItem(string _name, uint32 _itemId);		// 유저가 아이템 갖고있는지 체크 프로시저
	bool FarmingItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index);	// 아이템 획득 인벤토리로 
	bool GetUserEquipment(string _name);						// 유저 장비창 정보
	bool GetUserInfo(string _name);								// 유저 정보 가져오기 (장비,인벤 X)
	bool GetUserInventory(string _name);						// 유저 인벤토리 정보
	bool InitializeUserInventoryEquipment(string _name);		// 초기상태로 
	bool UnEquipItem(string _name, Equipment _type, uint32 _tab_type, uint32 _slot_index);			// 장비 해제
	bool UpdateUserInfo(USER_INFO _userInfo);						// 유저 업데이트 ( 저장 용도 )
	bool UseItem(string _name, uint32 _itemId, uint32 _amount, uint32 _tab_type, uint32 _slot_index); // 아이템 사용
	bool InventoryToEquip(string _name, uint32 _itemId, uint32 _tab_type, uint32 _slot_index);
	string WstringToString(const wstring& wstr); // wstring -> string 변환

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
};

