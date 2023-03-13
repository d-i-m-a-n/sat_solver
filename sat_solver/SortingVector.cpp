#include "SortingVector.h"

bool Node::operator>(Node& obj)
{
	if(min_clause_weight < obj.min_clause_weight)
		return true;
	if (min_clause_weight > obj.min_clause_weight)
		return false;
	if (var_weight < obj.var_weight)
		return false;

	return true;
}

bool Node::operator<(Node& obj)
{
	if (min_clause_weight > obj.min_clause_weight)
		return true;
	if (min_clause_weight < obj.min_clause_weight)
		return false;
	if (var_weight > obj.var_weight)
		return false;

	return true;


	return false;
}

bool Node::operator==(Node& obj)
{
	if (var_num == obj.var_num && var_value == obj.var_value && min_clause_weight == obj.min_clause_weight && var_weight == obj.var_weight)
		return true;
	return false;
}

Node& Node::operator=(Node&& obj)
{
	var_num = std::move(obj.var_num);
	var_value = std::move(obj.var_value);
	var_weight = std::move(obj.var_weight);
	min_clause_weight = std::move(obj.min_clause_weight);

	return *this;
}

SortingVector::SortingVector(int memAlloc) : vector(memAlloc)
{
	length = 0;
}

int SortingVector::getNodeIndex(Node& obj)
{
	int R = length;
	int L = 0;
	while (L != R)
	{
		int pos = (L + R) / 2;
		if (this->operator[](pos) == obj)
		{
			return pos;
		}

		if (this->operator[](pos) > obj)
			R = pos;
		if (this->operator[](pos) < obj)
			L = pos;
	}
}
