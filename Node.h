#pragma once
#include <string>
#include "Token.h"
class LeafNode
{
protected:
	// ����
	std::string _tag_name;
public:

	// ������
	LeafNode();
	LeafNode(std::string tag);

	bool check(std::string);

	virtual ~LeafNode();
};

