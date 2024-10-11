#include "pch.h"
#include "DataBase.h"
#include "MapData.h"
#include "Player.h"

DataBase* DataBase::d_instance = nullptr;

DataBase* DataBase::GetInstance()
{
	if (d_instance == nullptr)
	{
		d_instance = new DataBase();
	}
	return d_instance;
}

bool DataBase::CheckDB(string _name, GameSessionRef& _gamesession)
{
	string temp = _name;
	string ttemp = "";

	const string DUMMY_PREFIX = "dummy";

	if (_name.compare(0, DUMMY_PREFIX.size(), DUMMY_PREFIX) == 0)
	{
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 generator(seed);
		std::uniform_int_distribution<int> distribution(10, 1900);

		POS pos;
		STAT st;

		while (true) {
			int	x = distribution(generator);
			int	y = distribution(generator);

			if (MAPDATA->GetTile(y, x) != MAPDATA->e_OBSTACLE) {
				pos.posx = x;
				pos.posy = y;
				break;
			};
		}
		
		st.hp = 5000;
		st.maxHp = 5000;
		st.exp = 0;
		st.maxExp = 5000;
		st.mp = 5000;
		st.maxMp = 5000;
		st.level = 5000;

		PlayerRef player = MakeShared<Player>(_name, st, pos, ST_INGAME, 9999, Protocol::PLAYER_TYPE_CLIENT);
		_gamesession->SetCurrentPlayer(player);

		return true;
	}

	wstring wtemp = L"";
	wtemp.assign(temp.begin(), temp.end());

	wstring proc = L"EXEC select_user_id ";
	proc += wtemp;

	setlocale(LC_ALL, "korean");

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szUser_ID, NAME_LEN, &cbLoginID);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &pos_x, 100, &cb0);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &user_lv, 100, &cb2);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &pos_y, 100, &cb1);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &user_hp, 100, &cb3);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &user_maxhp, 100, &cb4);
		retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &user_mp, 100, &cb5);
		retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &user_maxmp, 100, &cb6);
		retcode = SQLBindCol(hstmt, 9, SQL_C_LONG, &user_exp, 100, &cb7);
		retcode = SQLBindCol(hstmt, 10, SQL_C_LONG, &user_maxexp, 100, &cb8);

		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
					//HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
				}
				//검색 완료( 존재 )
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					STAT st;
					POS pos;
					st.level = user_lv;
					st.hp = user_hp;
					st.maxHp = user_maxhp;
					st.mp = user_mp;
					st.maxMp = user_maxmp;
					st.exp = user_exp;
					st.maxExp = user_maxexp;
					pos.posx = pos_x;
					pos.posy = pos_y;

					PlayerRef player = MakeShared<Player>(_name, st, pos, ST_INGAME, 9999, Protocol::PLAYER_TYPE_CLIENT);
					_gamesession->SetCurrentPlayer(player);

					SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					return true;


				}
				else {
					SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					return false;
				}
			}
			else {
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				return false;
			}
		}
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		return false;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return false;
}

void DataBase::InitDB()
{
	setlocale(LC_ALL, "korean");


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
bool DataBase::SaveDB(SAVEDB _sd)
{
	string temp(_sd.name);
	wstring wtemp;
	wtemp.assign(temp.begin(), temp.end());

	wstring proc = L"EXECUTE select_user_pos ";
	proc += wtemp;
	proc += L", ";
	proc += to_wstring(_sd.posy);
	proc += L", ";
	proc += to_wstring(_sd.level);
	proc += L", ";
	proc += to_wstring(_sd.hp);
	proc += L", ";
	proc += to_wstring(_sd.maxHp);
	proc += L", ";
	proc += to_wstring(_sd.mp);
	proc += L", ";
	proc += to_wstring(_sd.maxMp);
	proc += L", ";
	proc += to_wstring(_sd.exp);
	proc += L", ";
	proc += to_wstring(_sd.maxExp);
	proc += L", ";
	proc += to_wstring(_sd.posx);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)proc.c_str(), SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "POS 저장 완료" << endl;
		return true;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		cout << "저장 실패" << endl;
		return false;
	}
}
