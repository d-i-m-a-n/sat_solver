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
		// номера удаленных дизъюнктов
		std::vector<int> clause_id;
		std::vector<int> clauses0_id;
		std::vector<int> clauses1_id;
		bool varVal;
		bool swapped;
		// храним обнуленные переменные из дизъюнктов
		std::map<int, std::vector<int>> clauses0;
		std::map<int, std::vector<int>> clauses1;
		// переменные, зафиксированные на текущем этапе
		BoolVector V1;
		BoolVector V0;

		StackData() = default;
	};

	bool backTrackAlg();

	void createKNFfromDIMACS(const std::string& DIMACS_filepath);

	bool chooseVarAlg();

	bool deduceAlg();

	void setVarVal(int var, bool varVal);

	// дерево ветвления
	std::stack<StackData> S;

	// дизъюнкты с одной оставшейся переменной
	std::vector<int> clause_weight1;
	// представление матрицы
	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	// зафиксированные переменные
	BoolVector V0;
	BoolVector V1;
	// переменные, находящиеся в i-тых дизъюнктах
	std::vector<std::vector<int>> clauses0;
	std::vector<std::vector<int>> clauses1;
	// текущий вес строк/столбцов
	std::vector<int> weightOfRows;
	std::vector<int> weightOfColumnsM0;
	std::vector<int> weightOfColumnsM1;


};

