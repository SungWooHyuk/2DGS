#pragma once

#include "SFSystem.h"
#include "Enum.pb.h"

class Client
{
private:

public:
	Client();
	Client(Protocol::PlayerType pt, POS _pos, int _id, const string& _name);
	~Client();

	virtual void	Move(int _x, int _y);
	virtual void	Draw();
	void			SetNameColor(const string& _name);
	void			SetTexture(Protocol::PlayerType pt);

	int		GetId() { return myId; };
	void	SetId(uint64 _id) { myId = _id; }
	string	GetName() { return myName; }
	void	SetName(string _name) { myName = _name; }

	POS		GetPos() const { return myPos; };
	void	SetPos(POS _pos) { myPos = _pos; }
	
public:
	sf::Sprite	Sprite;
protected:
	uint64		myId;
	string		myName;
	POS			myPos;
	sf::Text	myTextName;
};

