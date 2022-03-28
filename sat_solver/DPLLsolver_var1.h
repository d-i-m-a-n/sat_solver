#pragma once
#include <stack>
#include <vector>

#include "DPLLsolver.h"
#include "BoolMatrix.h"


class DPLLsolver_var1 : public DPLLsolver
{
public:
	DPLLsolver_var1() = default;

	~DPLLsolver_var1() = default;

private:
	struct StackData
	{
		int var;
		std::vector<int> clause_id;
		bool varVal;
		bool swapped;

		BoolVector V1;
		BoolVector V0;

		StackData() = default;
	};

	bool backTrackAlg();

	void createKNFfromDIMACS(const std::string& DIMACS_filepath);

	bool chooseVarAlg();

	bool deduceAlg();

	bool deduceM1(BoolMatrix& M1_, BoolMatrix& bufM1_, BoolVector& V1_, BoolMatrix& M0_, BoolMatrix& bufM0_, BoolVector& V0_, BoolVector& stackV, bool& changes);

	std::stack<StackData> S;

	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	BoolVector V0;
	BoolVector V1;
};

