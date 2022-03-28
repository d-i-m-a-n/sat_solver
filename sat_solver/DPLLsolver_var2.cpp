#include "DPLLsolver_var2.h"
#include <fstream>

DPLLsolver_var2::~DPLLsolver_var2()
{
	/*while (!M0.empty())
	{
		delete[] M0.back();
		M0.pop_back();
	}
	while (!M1.empty())
	{
		delete[] M1.back();
		M1.pop_back();
	}*/
}

bool DPLLsolver_var2::backTrackAlg()
{
	while (true)
	{
		StackData& stack_top = S.top();
		for (auto& id : stack_top.clause_id)
		{
			M1.insertRow(bufM1.extractRow(id), id);
			M0.insertRow(bufM0.extractRow(id), id);

			for (int j = 0; j < clauses1[id].size() || j < clauses0[id].size(); j++)
			{
				if (j < clauses1[id].size())
					weightOfColumnsM1[clauses1[id][j]]++;
				if (j < clauses0[id].size())
					weightOfColumnsM0[clauses0[id][j]]++;
			}

		}
		
		// переносим переменные обратно в дизъюнкты и увеличиваем веса
		for (auto id = stack_top.clauses0_id.begin(); id != stack_top.clauses0_id.end(); id++)
		{
			for (auto var = stack_top.clauses0[*id].begin(); var != stack_top.clauses0[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM0[*var]++;
				clauses0[*id].push_back(*var);
			}
			stack_top.clauses0[*id].clear();
		}
		
		for (auto id = stack_top.clauses1_id.begin(); id != stack_top.clauses1_id.end(); id++)
		{
			for (auto var = stack_top.clauses1[*id].begin(); var != stack_top.clauses1[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM1[*var]++;
				clauses1[*id].push_back(*var);
			}
			stack_top.clauses1[*id].clear();
		}

		stack_top.clause_id.clear();
		stack_top.clauses0_id.clear();
		stack_top.clauses1_id.clear();
		stack_top.clauses0.clear();
		stack_top.clauses1.clear();
		clause_weight1.clear();

		V1 &= ~stack_top.V1;
		V0 &= ~stack_top.V0;
		result &= ~stack_top.V1;
		result &= ~stack_top.V0;
		if (stack_top.swapped)
		{
			S.pop();
			if (S.empty())
				return false;
		}
		else
		{
			stack_top.V1.clear();
			stack_top.V0.clear();
			stack_top.swapped = true;
			stack_top.varVal = !stack_top.varVal;
			break;
		}
	}

	/*
	* перенос дизъюнктов в буферы
	*/

	setVarVal(S.top().var, S.top().varVal);

	return true;
}

void DPLLsolver_var2::createKNFfromDIMACS(const std::string& DIMACS_filepath)
{
	std::ifstream in(DIMACS_filepath);

	while (in.peek() != 'p')
	{
		in.ignore(50, '\n');
	}

	in.ignore(10, 'f');

	in >> varCount;
	in >> clauseCount;

	clauses0.resize(clauseCount);
	clauses1.resize(clauseCount);
	M0.resize(clauseCount, varCount);
	M1.resize(clauseCount, varCount);
	bufM1.resize(clauseCount);
	bufM0.resize(clauseCount);
	V0.resize(varCount);
	V1.resize(varCount);
	weightOfColumnsM0.clear();
	weightOfColumnsM1.clear();
	weightOfColumnsM0.resize(varCount, 0);
	weightOfColumnsM1.resize(varCount, 0);
	weightOfRows.clear();
	weightOfRows.resize(clauseCount, 0);
	clause_weight1.clear();
	
	result.resize(varCount);

	while (!S.empty())
		S.pop();

	for (int i = 0; i < clauseCount; i++)
	{
		clauses0[i].resize(0);
		clauses1[i].resize(0);
		int var;
		in >> var;
		while (var)
		{
			weightOfRows[i]++;
			if (var > 0)
			{
				(*M1[i])[var - 1] = 1;
				clauses1[i].push_back(var - 1);
				weightOfColumnsM1[var - 1]++;
			}
			else
			{
				(*M0[i])[var * (-1) - 1] = 1;
				clauses0[i].push_back(var * (-1) - 1);
				weightOfColumnsM0[var * (-1) - 1]++;
			}
			in >> var;
		}
	}
	in.close();
}

bool DPLLsolver_var2::chooseVarAlg()
{
	int maxWeight = 0;
	int choosenVar = -1;

	bool varVal;
	// ищем переменную, которая находится в наибольшем кол-ве дизъюнктов
	for (int i = 0; i < varCount; i++)
	{
		if (weightOfColumnsM0[i] > maxWeight)
		{
			choosenVar = i;
			maxWeight = weightOfColumnsM0[i];
			varVal = false;
		}
		if (weightOfColumnsM1[i] > maxWeight)
		{
			choosenVar = i;
			maxWeight = weightOfColumnsM1[i];
			varVal = true;
		}
	}

	if (choosenVar == -1)
		return false;

	S.push(StackData());
	StackData& stack_top = S.top();
	stack_top.V1.resize(varCount);
	stack_top.V0.resize(varCount);
	stack_top.var = choosenVar;
	stack_top.varVal = varVal;

	setVarVal(choosenVar, varVal);

	return true;
}

bool DPLLsolver_var2::deduceAlg()
{
	// проверяем дизъюнкты в которых осталась 1 переменная

	// нужно зафиксировать переменные для всех дизъюнктов из вектора
	for (auto iter = clause_weight1.begin(); iter != clause_weight1.end();)
	{
		int cur_clause = *iter;

		if (!clauses0[cur_clause].empty())
		{
			// проверка конфликта
			for (auto iter2 = iter + 1; iter2 != clause_weight1.end(); )
			{
				if (!clauses1[*iter2].empty() && clauses0[cur_clause][0] == clauses1[*iter2][0])
				{
					return true;
				}

				if (iter != iter2 && !clauses0[*iter2].empty() && clauses0[cur_clause][0] == clauses0[*iter2][0])
					iter2 = clause_weight1.erase(iter2);
				else
					iter2++;
			}
			setVarVal(clauses0[cur_clause][0], false);
		}

		if (!clauses1[cur_clause].empty())
		{
			// проверка конфликта
			for (auto iter2 = iter + 1; iter2 != clause_weight1.end();)
			{
				if (!clauses0[*iter2].empty() && clauses1[cur_clause][0] == clauses0[*iter2][0])
				{
					return true;
				}

				if (iter != iter2 && !clauses1[*iter2].empty() && clauses1[cur_clause][0] == clauses1[*iter2][0])
					iter2 = clause_weight1.erase(iter2);
				else
					iter2++;
			}
			setVarVal(clauses1[cur_clause][0], true);
		}

		iter = clause_weight1.begin();
		iter = clause_weight1.erase(iter);
	}
	return false;
}

void DPLLsolver_var2::setVarVal(int var, bool varVal)
{
	result[var] = 1;
	StackData& stack_top = S.top();
	if (varVal)
	{
		V1[var] = 1;
		stack_top.V1[var] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
			if (M1[i] && (*M1[i])[var])
			{
				stack_top.clause_id.push_back(i);
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
				for (int j = 0; j < clauses1[i].size() || j < clauses0[i].size(); j++)
				{
					if (j < clauses1[i].size())
						weightOfColumnsM1[clauses1[i][j]]--;
					if (j < clauses0[i].size())
						weightOfColumnsM0[clauses0[i][j]]--;
				}
			}
			// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
			if (M0[i] && (*M0[i])[var])
			{
				weightOfColumnsM0[var]--;
				stack_top.clauses0[i].push_back(var);
				stack_top.clauses0_id.push_back(i);
				// переносим переменные из дизъюнктов в стэк
				for (auto iter = clauses0[i].begin(); iter != clauses0[i].end(); iter++)
				{
					if (*iter == var)
					{
						clauses0[i].erase(iter);
						break;
					}
				}
				// уменьшаем веса строк и проверяем не стал ли вес равен 1
				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
					clause_weight1.push_back(i);
			}
		}
	}
	else
	{
		V0[var] = 1;
		stack_top.V0[var] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			if (M0[i] && (*M0[i])[var])
			{
				stack_top.clause_id.push_back(i);
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				for (int j = 0; j < clauses1[i].size() || j < clauses0[i].size(); j++)
				{
					if (j < clauses1[i].size())
						weightOfColumnsM1[clauses1[i][j]]--;
					if (j < clauses0[i].size())
						weightOfColumnsM0[clauses0[i][j]]--;
				}
			}

			if (M1[i] && (*M1[i])[var])
			{
				weightOfColumnsM1[var]--;
				stack_top.clauses1[i].push_back(var);
				stack_top.clauses1_id.push_back(i);
				for (auto iter = clauses1[i].begin(); iter != clauses1[i].end(); iter++)
				{
					if (*iter == var)
					{
						clauses1[i].erase(iter);
						break;
					}
				}
				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
					clause_weight1.push_back(i);
			}
		}
	}

}
