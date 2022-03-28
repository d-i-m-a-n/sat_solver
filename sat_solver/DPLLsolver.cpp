#include "DPLLsolver.h"
#include <time.h>

bool DPLLsolver::solve(const std::string& DIMACS_filepath, BoolVector& result_, int& time)
{
	int beg = clock();

	createKNFfromDIMACS(DIMACS_filepath);

	/*if (deduceAlg())
	{
		time = clock() - beg;
		return false;
	}*/

	while (true)
	{
		if (chooseVarAlg())
		{
			while (deduceAlg())
			{
				if (!backTrackAlg())
				{
					time = clock() - beg;
					result_ = std::move(result);
					return false;
				}
			}
		}
		else
		{
			time = clock() - beg;
			result_ = std::move(result);
			return true;
		}
	}
}
