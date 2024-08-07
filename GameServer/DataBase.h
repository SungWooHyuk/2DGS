#pragma once

#include "protocol.h"
#include "pch.h"
#include "utils.h"
#include "GameSession.h"

#include <sqlext.h>

class DataBase
{
private:
	static DataBase* d_instance;
	DataBase() {};

	
public:
	~DataBase();
	static	DataBase*	GetInstance();

	bool				CheckDB(string _name, GameSessionRef &_gamesession);
	void				InitDB();
	bool				SaveDB(SAVEDB _sd);

private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR szUser_ID[NAME_LEN];

	SQLLEN cbLoginID = 0;
	SQLLEN cbX, cbY = 0, cbHP = 0, cbEXP = 0, cbLEVEL = 0, cbMp = 0;
	SQLLEN cbMaxHp = 0, cbMaxMp = 0, cbMaxExp = 0;

	SQLINTEGER  pos_x, pos_y, user_lv, user_hp, user_maxhp;
	SQLINTEGER user_mp, user_exp, user_maxexp, user_maxmp;

	SQLLEN cb0 = 0, cb1 = 0;
	SQLLEN cb2 = 0, cb4 = 0;
	SQLLEN cb3 = 0, cb5 = 0;
	SQLLEN cb6 = 0, cb7 = 0;
	SQLLEN cb8 = 0;
};

