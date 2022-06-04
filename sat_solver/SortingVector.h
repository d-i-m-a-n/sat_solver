#pragma once
#include <vector>

struct Node
{
	int var_num;
	bool var_value;
	int min_clause_weight;
	int var_weight;

	bool operator > (Node& obj);
	bool operator < (Node& obj);
	bool operator == (Node& obj);
	Node& operator = (Node&& obj);
};

class SortingVector : public std::vector<Node>
{
public:

	SortingVector(int memAlloc);

	int getNodeIndex(Node& obj);

private:

	int length;

};

