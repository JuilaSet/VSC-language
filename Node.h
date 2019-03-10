#pragma once
#include <string>
#include "Token.h"
class LeafNode
{
protected:
	//  Ù–‘
	std::string _tag_name;
public:

	// ππ‘Ï∆˜
	LeafNode();
	LeafNode(std::string tag);

	bool check(std::string);

	virtual ~LeafNode();
};

