#pragma once
#include <vector>
#include <stack>
#include <map>

#include "DPLLsolver.h"
#include "BoolMatrix.h"

class DPLLsolver_greed_var2 :
    public DPLLsolver
{
public:
	DPLLsolver_greed_var2() = default;

	~DPLLsolver_greed_var2();

private:
	struct StackData
	{
		int var;
		// ������ ��������� ����������
		std::vector<int> removed_clause_id;
		// ������ ���������� �� ������� ������� ��������� ����������
		std::vector<int> vars_removed_from_clausesM0_id;
		std::vector<int> vars_removed_from_clausesM1_id;
		bool varVal;
		bool swapped;
		// ������ ���������� ���������� �� ����������
		std::map<int, std::vector<int>> vars_in_clausesM0;
		std::map<int, std::vector<int>> vars_in_clausesM1;
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
	std::vector<int> clause_weight1_id;
	// ������������� �������
	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	// ��������������� ����������
	BoolVector V0;
	BoolVector V1;

	// ����������, ����������� � i-��� ����������
	std::vector<std::vector<int>> vars_in_clausesM0;
	std::vector<std::vector<int>> vars_in_clausesM1;
	// ������� ��� �����/��������
	std::vector<int> weightOfRows;
	std::vector<int> weightOfColumnsM0;
	std::vector<int> weightOfColumnsM1;


};

