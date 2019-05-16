#include "pch.h"
#include "Node.h"
bool operator==(const LeafNode& l1, const LeafNode& l2) {
	return l1._id == l2._id;
}

bool operator<(const LeafNode& l1, const LeafNode& l2) {
	return l1._id < l2._id;
}
