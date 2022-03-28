#pragma once
#include <vector>
#include <stack>
#include <map>

#include "DPLLsolver.h"
#include "BoolMatrix.h"

class DPLLsolver_var2 :
    public DPLLsolver
{
public:
	DPLLsolver_var2() = default;

	~DPLLsolver_var2();

private:
	struct StackData
	{
		int var;
		// ������ ��������� ����������
		std::vector<int> clause_id;
		std::vector<int> clauses0_id;
		std::vector<int> clauses1_id;
		bool varVal;
		bool swapped;
		// ������ ���������� ���������� �� ����������
		std::map<int, std::vector<int>> clauses0;
		std::map<int, std::vector<int>> clauses1;
		// ����������, ��������������� �� ������� �����
		BoolVector V1;
		BoolVector V0;

		StackData() = default;
	};

	bool backTrackAlg();

	void createKNFfromDIMACS(const std::string& DIMACS_filepath);

	bool chooseVarAlg();

	bool deduceAlg();

	void setVarVal(int var, bool varVal);

	// ������ ���������
	std::stack<StackData> S;

	// ��������� � ����� ���������� ����������
	std::vector<int> clause_weight1;
	// ������������� �������
	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	// ��������������� ����������
	BoolVector V0;
	BoolVector V1;
	// ����������, ����������� � i-��� ����������
	std::vector<std::vector<int>> clauses0;
	std::vector<std::vector<int>> clauses1;
	// ������� ��� �����/��������
	std::vector<int> weightOfRows;
	std::vector<int> weightOfColumnsM0;
	std::vector<int> weightOfColumnsM1;


};

