#pragma once

#include "BoolVector.h"

class DPLLsolver
{
public:

	DPLLsolver() = default;

	virtual ~DPLLsolver() = default;

	bool solve(const std::string& DIMACS_filepath, BoolVector& result_, int& time);

protected:
	virtual bool backTrackAlg() = 0;

	virtual void createKNFfromDIMACS(const std::string& DIMACS_filepath) = 0;

	virtual bool chooseVarAlg() = 0;

	virtual bool deduceAlg() = 0;

	BoolVector result;

	int clauseCount;
	int varCount;
};

