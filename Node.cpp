#include "pch.h"
#include "Node.h"


LeafNode::LeafNode():_tag_name("div")
{
}

LeafNode::LeafNode(std::string tag): _tag_name(tag)
{
}

bool LeafNode::check(std::string str)
{
	return false;
}

LeafNode::~LeafNode()
{
}
