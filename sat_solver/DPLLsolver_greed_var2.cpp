#include "DPLLsolver_greed_var2.h"
#include <fstream>

DPLLsolver_greed_var2::~DPLLsolver_greed_var2()
{

}

bool DPLLsolver_greed_var2::backTrackAlg()
{
	while (true)
	{
		StackData& stack_top = S.top();
		
		// переносим дизъюнкты из буфера в матрицы
		for (auto& id : stack_top.removed_clause_id)
		{
			M1.insertRow(bufM1.extractRow(id), id);
			M0.insertRow(bufM0.extractRow(id), id);

			for (int j = 0; j < vars_in_clausesM1[id].size() || j < vars_in_clausesM0[id].size(); j++)
			{
				if (j < vars_in_clausesM1[id].size())
					weightOfColumnsM1[vars_in_clausesM1[id][j]]++;
				if (j < vars_in_clausesM0[id].size())
					weightOfColumnsM0[vars_in_clausesM0[id][j]]++;
			}
		}

		// переносим переменные обратно в дизъюнкты и увеличиваем веса
		for (auto id = stack_top.vars_removed_from_clausesM0_id.begin(); id != stack_top.vars_removed_from_clausesM0_id.end(); id++)
		{
			for (auto var = stack_top.vars_in_clausesM0[*id].begin(); var != stack_top.vars_in_clausesM0[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM0[*var]++;
				vars_in_clausesM0[*id].push_back(*var);
			}
			stack_top.vars_in_clausesM0[*id].clear();
		}
		
		for (auto id = stack_top.vars_removed_from_clausesM1_id.begin(); id != stack_top.vars_removed_from_clausesM1_id.end(); id++)
		{
			for (auto var = stack_top.vars_in_clausesM1[*id].begin(); var != stack_top.vars_in_clausesM1[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM1[*var]++;
				vars_in_clausesM1[*id].push_back(*var);
			}
			stack_top.vars_in_clausesM1[*id].clear();
		}

		stack_top.removed_clause_id.clear();
		stack_top.vars_removed_from_clausesM0_id.clear();
		stack_top.vars_removed_from_clausesM1_id.clear();
		stack_top.vars_in_clausesM0.clear();
		stack_top.vars_in_clausesM1.clear();
		clause_weight1_id.clear();

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

	positive_vectors.clear();
	negative_vectors.clear();

	for (int i = 0; i < clauseCount; i++)
	{
		if (M0[i] && vars_in_clausesM0[i].empty())
			positive_vectors[i] = true;
		if (M1[i] && vars_in_clausesM1[i].empty())
			negative_vectors[i] = true;
	}


	/*
	* перенос дизъюнктов в буферы
	*/

	setVarVal(S.top().var, S.top().varVal);

	return true;
}

void DPLLsolver_greed_var2::createKNFfromDIMACS(const std::string& DIMACS_filepath)
{
	std::ifstream in(DIMACS_filepath);

	while (in.peek() != 'p')
	{
		in.ignore(50, '\n');
	}

	in.ignore(10, 'f');

	in >> varCount;
	in >> clauseCount;

	vars_in_clausesM0.resize(clauseCount);
	vars_in_clausesM1.resize(clauseCount);

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

	clause_weight1_id.clear();

	positive_vectors.clear();
	negative_vectors.clear();
	
	result.resize(varCount);

	while (!S.empty())
		S.pop();

	for (int i = 0; i < clauseCount; i++)
	{
		vars_in_clausesM0[i].resize(0);
		vars_in_clausesM1[i].resize(0);
		int var;
		in >> var;
		while (var)
		{
			weightOfRows[i]++;
			if (var > 0)
			{
				(*M1[i])[var - 1] = 1;
				vars_in_clausesM1[i].push_back(var - 1);
				weightOfColumnsM1[var - 1]++;
			}
			else
			{
				(*M0[i])[(-1) - var] = 1;
				vars_in_clausesM0[i].push_back((-1) - var);
				weightOfColumnsM0[(-1) - var]++;
			}
			in >> var;
		}
		if (!M0[i]->operator bool())
			positive_vectors[i] = true;
		if (!M1[i]->operator bool())
			negative_vectors[i] = true;
	}
	in.close();

	std::cout << M0 << std::endl << M1 << std::endl;
}

bool DPLLsolver_greed_var2::checkMatrix()
{
	BoolVector checkVectorM1(varCount);
	BoolVector checkVectorM0(varCount);
	bool M_pos_conflict = false;
	bool M_neg_conflict = false;

	for (int i = 0; i < clauseCount; i++)
	{
		if (M0[i] && vars_in_clausesM0[i].size() == 1)
		{
			checkVectorM0 |= *(M0[i]);
		}
		if (M1[i] && vars_in_clausesM1[i].size() == 1)
		{
			checkVectorM1 |= *(M1[i]);
		}
	}

	for (auto& i : positive_vectors)
	{
		if ((*(M1[i.first]) & checkVectorM0) == *(M1[i.first]))
		{
			M_pos_conflict = true;
			break;
		}
	}

	for (auto& i : negative_vectors)
	{
		if ((*(M0[i.first]) & checkVectorM1) == *(M0[i.first]))
		{
			M_neg_conflict = true;
			break;
		}
	}

	if (M_pos_conflict && M_neg_conflict)
		return true;

	return false;
}

bool DPLLsolver_greed_var2::chooseVarAlg()
{
	//checkMatrix();

	//// М = М*
	//if (positive_vectors.empty() && negative_vectors.empty())
	//{
	//	findCoverage(M1, weightOfColumnsM1, vars_in_clausesM1);
	//	return false;
	//}
	//// М = М* + М-
	//if (positive_vectors.empty())
	//{
	//	findCoverage(M0, weightOfColumnsM0, vars_in_clausesM0);
	//	return false;
	//}
	//// М = М* + М+
	//if (negative_vectors.empty())
	//{
	//	findCoverage(M1, weightOfColumnsM1, vars_in_clausesM1);
	//	return false;
	//}

	int maxWeight = 0;
	int choosenVar = -1;

	bool varVal;
	
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

	//// ищем переменную из М+ или М-, которая находится в наибольшем кол-ве дизъюнктов
	//for (auto& vector : positive_vectors)
	//{
	//	for (auto& variable : vars_in_clausesM1[vector.first])
	//	{
	//		if (weightOfColumnsM1[variable] > maxWeight)
	//		{
	//			maxWeight = weightOfColumnsM1[variable];
	//			choosenVar = variable;
	//			varVal = true;
	//		}
	//	}
	//}
	//for (auto& vector : negative_vectors)
	//{
	//	for (auto& variable : vars_in_clausesM0[vector.first])
	//	{
	//		if (weightOfColumnsM0[variable] > maxWeight)
	//		{
	//			maxWeight = weightOfColumnsM0[variable];
	//			choosenVar = variable;
	//			varVal = false;
	//		}
	//	}
	//}

	if (choosenVar == -1)
	{
		std::cout << V0 << ' ' << V1 << std::endl;
		return false;
	}
	S.push(StackData());
	StackData& stack_top = S.top();
	stack_top.V1.resize(varCount);
	stack_top.V0.resize(varCount);
	stack_top.var = choosenVar;
	stack_top.varVal = varVal;

	setVarVal(choosenVar, varVal);

	return true;
}

bool DPLLsolver_greed_var2::deduceAlg()
{
	// проверяем дизъюнкты в которых осталась 1 переменная

	// нужно зафиксировать переменные для всех дизъюнктов из вектора
	for (auto iter = clause_weight1_id.begin(); iter != clause_weight1_id.end();)
	{
		int cur_clause = *iter;

		if (!vars_in_clausesM0[cur_clause].empty())
		{
			// проверка конфликта
			for (auto iter2 = iter + 1; iter2 != clause_weight1_id.end(); )
			{
				if (!vars_in_clausesM1[*iter2].empty() && vars_in_clausesM0[cur_clause][0] == vars_in_clausesM1[*iter2][0])
				{
					return true;
				}

				if (iter != iter2 && !vars_in_clausesM0[*iter2].empty() && vars_in_clausesM0[cur_clause][0] == vars_in_clausesM0[*iter2][0])
					iter2 = clause_weight1_id.erase(iter2);
				else
					iter2++;
			}
			setVarVal(vars_in_clausesM0[cur_clause][0], false);
		}

		if (!vars_in_clausesM1[cur_clause].empty())
		{
			// проверка конфликта
			for (auto iter2 = iter + 1; iter2 != clause_weight1_id.end();)
			{
				if (!vars_in_clausesM0[*iter2].empty() && vars_in_clausesM1[cur_clause][0] == vars_in_clausesM0[*iter2][0])
				{
					return true;
				}

				if (iter != iter2 && !vars_in_clausesM1[*iter2].empty() && vars_in_clausesM1[cur_clause][0] == vars_in_clausesM1[*iter2][0])
					iter2 = clause_weight1_id.erase(iter2);
				else
					iter2++;
			}
			setVarVal(vars_in_clausesM1[cur_clause][0], true);
		}

		iter = clause_weight1_id.begin();
		iter = clause_weight1_id.erase(iter);
	}

	//return checkMatrix();
	return false;
}

void DPLLsolver_greed_var2::findCoverage(BoolMatrix& M, std::vector<int>& weightOfColumns, std::vector<std::vector<int>>& vars_in_clauses)
{
	int maxWeight = 0;
	int choosenVar = -1;

	//::cout << "Entered findCoverage\n";

	for (int i = 0; i < clauseCount; i++)
	{
		if (M[i])
		{
			if (vars_in_clauses[i].size() == 1)
			{
				choosenVar = vars_in_clauses[i].front();
				result[choosenVar] = 1;

				for (int j = 0; j < clauseCount; j++)
				{
					if (M[j] && (*M[j])[choosenVar])
					{
						M.extractRow(j);

						for (int k = 0; k < vars_in_clauses[j].size(); k++)
						{
							weightOfColumns[vars_in_clauses[j][k]]--;
						}
					}
				}
			}
		}
	}

	while (true)
	{
		maxWeight = 0;
		choosenVar = -1;

		for (int i = 0; i < varCount; i++)
		{
			if (weightOfColumns[i] > maxWeight)
			{
				choosenVar = i;
				maxWeight = weightOfColumns[i];
			}
		}

		if (choosenVar == -1)
			return;

		result[choosenVar] = 1;
		for (int j = 0; j < clauseCount; j++)
		{
			if (M[j] && (*M[j])[choosenVar])
			{
				M.extractRow(j);

				for (int k = 0; k < vars_in_clauses[j].size(); k++)
				{
					weightOfColumns[vars_in_clauses[j][k]]--;
				}
			}
		}

	}
}

void DPLLsolver_greed_var2::setVarVal(int var, bool varVal)
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
				positive_vectors.erase(i);

				stack_top.removed_clause_id.push_back(i);
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
				for (int j = 0; j < vars_in_clausesM1[i].size() || j < vars_in_clausesM0[i].size(); j++)
				{
					if (j < vars_in_clausesM1[i].size())
						weightOfColumnsM1[vars_in_clausesM1[i][j]]--;
					if (j < vars_in_clausesM0[i].size())
						weightOfColumnsM0[vars_in_clausesM0[i][j]]--;
				}
			}
			// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
			if (M0[i] && (*M0[i])[var])
			{
				weightOfColumnsM0[var]--;
				stack_top.vars_in_clausesM0[i].push_back(var);
				stack_top.vars_removed_from_clausesM0_id.push_back(i);
				// переносим переменные из дизъюнктов в стэк
				for (auto iter = vars_in_clausesM0[i].begin(); iter != vars_in_clausesM0[i].end(); iter++)
				{
					if (*iter == var)
					{
						vars_in_clausesM0[i].erase(iter);
						break;
					}
				}

				if (vars_in_clausesM0[i].empty())
				{
					positive_vectors[i] = true;
				}

				// уменьшаем веса строк и проверяем не стал ли вес равен 1
				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
					clause_weight1_id.push_back(i);
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
				negative_vectors.erase(i);

				stack_top.removed_clause_id.push_back(i);
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				for (int j = 0; j < vars_in_clausesM1[i].size() || j < vars_in_clausesM0[i].size(); j++)
				{
					if (j < vars_in_clausesM1[i].size())
						weightOfColumnsM1[vars_in_clausesM1[i][j]]--;
					if (j < vars_in_clausesM0[i].size())
						weightOfColumnsM0[vars_in_clausesM0[i][j]]--;
				}
			}

			if (M1[i] && (*M1[i])[var])
			{
				weightOfColumnsM1[var]--;
				stack_top.vars_in_clausesM1[i].push_back(var);
				stack_top.vars_removed_from_clausesM1_id.push_back(i);
				for (auto iter = vars_in_clausesM1[i].begin(); iter != vars_in_clausesM1[i].end(); iter++)
				{
					if (*iter == var)
					{
						vars_in_clausesM1[i].erase(iter);
						break;
					}
				}

				if (vars_in_clausesM1[i].empty())
				{
					negative_vectors[i] = true;
				}

				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
					clause_weight1_id.push_back(i);
			}
		}
	}

	std::cout << "Fixed var: " << var << " valuse: " << varVal << std::endl;
	std::cout << M0 << std::endl << M1 << std::endl;

}
