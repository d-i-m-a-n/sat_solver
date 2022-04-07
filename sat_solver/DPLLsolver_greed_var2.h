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
		// номера удаленных дизъюнктов
		std::vector<int> removed_clause_id;
		// номера дизъюнктов из которых удалены некоторые переменные
		std::vector<int> vars_removed_from_clausesM0_id;
		std::vector<int> vars_removed_from_clausesM1_id;
		bool varVal;
		bool swapped;
		// храним обнуленные переменные из дизъюнктов
		std::map<int, std::vector<int>> vars_in_clausesM0;
		std::map<int, std::vector<int>> vars_in_clausesM1;
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
	std::vector<int> clause_weight1_id;
	// представление матрицы
	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	// зафиксированные переменные
	BoolVector V0;
	BoolVector V1;

	// переменные, находящиеся в i-тых дизъюнктах
	std::vector<std::vector<int>> vars_in_clausesM0;
	std::vector<std::vector<int>> vars_in_clausesM1;
	// текущий вес строк/столбцов
	std::vector<int> weightOfRows;
	std::vector<int> weightOfColumnsM0;
	std::vector<int> weightOfColumnsM1;


};

