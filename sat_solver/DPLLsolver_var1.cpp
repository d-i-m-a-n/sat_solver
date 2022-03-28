#include "DPLLsolver_var1.h"
#include <fstream>

/*
* TODO:
* восстановить векторы V1 V0 при возврате (done)
* в deduce надо добавлять в векторы в стеке информацию об зафиксированных переменных(done)
* добавить выбор перемнной из M0
*/

// М0 - переменные с инверсией, М1 - без инверсии
void DPLLsolver_var1::createKNFfromDIMACS(const std::string& DIMACS_filepath)
{
	std::ifstream in(DIMACS_filepath);

	while (in.peek() != 'p')
	{
		in.ignore(50, '\n');
	}

	in.ignore(10, 'f');

	in >> varCount;
	in >> clauseCount;

	M0.resize(clauseCount, varCount);
	M1.resize(clauseCount, varCount);
	V0.resize(varCount);
	V1.resize(varCount);
	bufM1.resize(clauseCount);
	bufM0.resize(clauseCount);

	result.resize(varCount);

	while (!S.empty())
		S.pop();

	for (int i = 0; i < clauseCount; i++)
	{
		int var;
		in >> var;
		while (var)
		{
			if (var > 0)
			{
				(*M1[i])[var - 1] = 1;
			}
			else
			{
				(*M0[i])[var * (-1) - 1] = 1;
			}
			in >> var;
		}
	}
	in.close();
}

bool DPLLsolver_var1::backTrackAlg() 
{
	/*
	* 1. возвращаем дизъюнкты из буферов
	* 2. меняем значение переменной
	* 3. если уже свапали, переходим к следующей вершине на п.1
	* 4. после свапа перекинуть дизъюнкты в буфер
	*/

	while (true)
	{
		StackData& stack_top = S.top();
		for (auto& id : stack_top.clause_id)
		{
			/*
			* перенос дизъюнктов из буфера обратно в матрицы
			*/
			M1.insertRow(bufM1.extractRow(id), id);
			M0.insertRow(bufM0.extractRow(id), id);
		}
		stack_top.clause_id.clear();
		
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
			result[stack_top.var] = 1;
			if (stack_top.varVal)
			{
				V1[stack_top.var] = 1;
				stack_top.V1[stack_top.var] = 1;
			}
			else
			{
				V0[stack_top.var] = 1;
				stack_top.V0[stack_top.var] = 1;
			}


			break;
		}
	}

	/*
	* перенос дизъюнктов в буферы
	*/
	if (S.top().varVal)
	{
		for (int i = 0; i < clauseCount; i++)
		{
			if (M1[i] && (*M1[i])[S.top().var])
			{
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				S.top().clause_id.push_back(i);
			}
		}
	}
	else
	{
		for (int i = 0; i < clauseCount; i++)
		{
			if (M0[i] && (*M0[i])[S.top().var])
			{
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				S.top().clause_id.push_back(i);
			}
		}
	}
}

bool DPLLsolver_var1::chooseVarAlg()
{
	int maxWeight = 0;
	int choosenVar = -1;
	/*
	* вычитание вектора из всех строк матрицы
	*/
	BoolMatrix buf1 = M1 - V0;
	BoolMatrix buf0 = M0 - V1;
	bool varVal;

	for (int i = 0; i < varCount; i++)
	{
		int M1ColumnWeight = 0;
		int M0ColumnWeight = 0;

		if (!V0[i] && !V1[i])
		{
			M1ColumnWeight = buf1.weightOfColumn(i);
			M0ColumnWeight = buf0.weightOfColumn(i);
		}

		if (M1ColumnWeight > maxWeight || M0ColumnWeight > maxWeight)
		{
			if (M1ColumnWeight > M0ColumnWeight)
			{
				maxWeight = M1ColumnWeight;
				varVal = true;
			}
			else
			{
				maxWeight = M0ColumnWeight;
				varVal = false;
			}
			choosenVar = i;
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

	/*
	* перенос строк из M1 M0 в bufM1 bufM0
	* занести данные в стек(номера строк)
	*/
	result[choosenVar] = 1;
	if (varVal)
	{
		V1[choosenVar] = 1;
		stack_top.V1[choosenVar] = 1;
		for (int i = 0; i < clauseCount; i++)
		{
			if (M1[i] && (*M1[i])[choosenVar])
			{
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				stack_top.clause_id.push_back(i);
			}
		}
	}
	else
	{
		V0[choosenVar] = 1;
		stack_top.V0[choosenVar] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			if (M0[i] && (*(M0[i]))[choosenVar])
			{
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				stack_top.clause_id.push_back(i);
			}
		}
	}

	return true;
}

bool DPLLsolver_var1::deduceAlg()
{
	/*
	* цикл вынести в функцию
	* сделать вызов по обеим матрицам в цикле
	* цикл закончится, когда не будет никаких изменений в матрицах
	*/

	bool anyChanges;
	do
	{
		anyChanges = false;
		if (deduceM1(M1, bufM1, V1, M0, bufM0, V0, S.top().V1, anyChanges))
			return true;

		if (deduceM1(M0, bufM0, V0, M1, bufM1, V1, S.top().V0, anyChanges))
			return true;
	} while (anyChanges);
	return false;
}

bool DPLLsolver_var1::deduceM1(BoolMatrix& M1_, BoolMatrix& bufM1_, BoolVector& V1_, BoolMatrix& M0_, BoolMatrix& bufM0_, BoolVector& V0_, BoolVector& stackV, bool& changes)
{

	//ищем перемнные, которые можем зафиксировать
	for (int i = 0; i < clauseCount; i++)
	{
		if (M1_[i])
		{
			BoolVector bufV1 = *(M1_[i]) - V0_;
			BoolVector bufV0 = *(M0_[i]) - V1_;

			if (bufV1.Weight() == 1 && !bufV0.operator bool())
			{
				// проверяем на конфликт
				for (int j = 0; j < clauseCount; j++)
					if (M1_[j] && !(*(M1_[j]) - V0_).operator bool() && (*(M0_[j]) - V1_) == bufV1)
						return true;

				// ищем дизъюнкты которые при фиксации переменной станут 1
				for (int j = 0; j < clauseCount; j++)
				{
					if (M1_[j] && (*(M1_[j]) & bufV1))
					{
						/*
						* вставить перенос дизъюнктов в bufM1 bufM0
						*/
						bufM1_.insertRow(M1_.extractRow(j), j);
						bufM0_.insertRow(M0_.extractRow(j), j);
						S.top().clause_id.push_back(j);
					}
				}
				V1_ |= bufV1;
				result |= bufV1;
				stackV |= bufV1;
				changes = true;
			}
		}
	}
	return false;
}
