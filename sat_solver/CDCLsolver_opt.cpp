#include "CDCLsolver_opt.h"

#include <fstream>
#include <stack>

bool debug_mode = false;

CDCLsolver_opt::ImplicationGraph::MainNode::MainNode()
{
	var.var_num = 0;
	var.decision_level = 0;
	next = nullptr;
	prev = nullptr;
}

CDCLsolver_opt::ImplicationGraph::ImplicationGraph()
{
	root = new MainNode;
	root->var.var_num = -1;
	root->var.decision_level = 0;
}

CDCLsolver_opt::ImplicationGraph::~ImplicationGraph()
{
	delete root;
}

CDCLsolver_opt::CDCLsolver_opt()
{
}

CDCLsolver_opt::~CDCLsolver_opt()
{
	clearGraph();
}

bool CDCLsolver_opt::backTrackAlg()
{
	if (backtrack_decision_level == -1)
	{
		if(debug_mode)
			std::cout << "conflict var: " << conflict_var << std::endl;
		return false;
	}

	delete conflict_var_false;
	delete conflict_var_true;
	conflict_var_false = nullptr;
	conflict_var_true = nullptr;

	for (int level = cur_main_node->var.decision_level; level > backtrack_decision_level - 1 && level > 0; level--)
	{
		if(debug_mode)
			std::cout << cur_main_node->var.decision_level << std::endl;
		// переносим дизъюнкты из буфера в матрицы
		for (auto& id : cur_main_node->removed_clause_id)
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

		for (auto& iter : cur_main_node->vars_in_clausesM0)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				weightOfColumnsM0[var]++;
				vars_in_clausesM0[iter.first].push_back(var);
			}
		}

		for (auto& iter : cur_main_node->vars_in_clausesM1)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				weightOfColumnsM1[var]++;
				vars_in_clausesM1[iter.first].push_back(var);
			}
		}

		V1 &= ~cur_main_node->V1;
		V0 &= ~cur_main_node->V0;
		result &= ~cur_main_node->V1;
		result &= ~cur_main_node->V0;

		std::queue<ImplicationGraph::SideNode*> del_queue;
		BoolVector deleted_nodes(varCount);

		// добавляем потомков главной вершины в очередь
		for (auto ptr : cur_main_node->nodes_ptrs)
		{
			if (!deleted_nodes[ptr.first])
			{
				deleted_nodes[ptr.first] = 1;
				del_queue.push(ptr.second);
			}

			ptr.second->parents_nodes_ptrs.erase(cur_main_node->var.var_num);
		}

		while (!del_queue.empty())
		{
			auto del_ptr = del_queue.front();
			// добавляем всех потомков в очередь
			for (auto& ptr : del_ptr->nodes_ptrs)
			{
				if (!deleted_nodes[ptr.first])
				{
					deleted_nodes[ptr.first] = 1;
					del_queue.push(ptr.second);
				}

				ptr.second->parents_nodes_ptrs.erase(del_ptr->var.var_num);
			}

			// удаляем у всех родителей указатель на текущую вершину
			for (auto& par_ptr : del_ptr->parents_nodes_ptrs)
			{
				par_ptr.second->nodes_ptrs.erase(del_ptr->var.var_num);
			}
			del_ptr->parents_nodes_ptrs.clear();

			// удаляем указатель из общего списка и саму вершину
			ptrs_to_vars.erase(del_ptr->var.var_num);
			delete del_ptr;
			del_queue.pop();
		}

		if (level > backtrack_decision_level)
		{
			// удаляем главную вершину и переносим указатель на уровень выше
			ptrs_to_vars.erase(cur_main_node->var.var_num);
			ptrs_to_main_nodes.erase(cur_main_node->var.decision_level);
			cur_main_node = cur_main_node->prev;
			delete cur_main_node->next;
			cur_main_node->next = nullptr;
		}
	}

	// очищаем уровень, на который откатились
	cur_main_node->nodes_ptrs.clear();
	cur_main_node->V1.clear();
	cur_main_node->V0.clear();
	cur_main_node->removed_clause_id.clear();
	cur_main_node->vars_in_clausesM0.clear();
	cur_main_node->vars_in_clausesM1.clear();

	if (debug_mode)
	{
		std::cout << "after backtrack matrix:\n";
		printIntervals();

	} 
	positive_vectors = cur_main_node->positive_vectors;
	negative_vectors = cur_main_node->negative_vectors;

	if (debug_mode)
	{
		std::cout << "positive vectors: ";
		for (auto& clause : positive_vectors)
			std::cout << clause.first << ' ';
		std::cout << "\nnegative vectors: ";
		for (auto& clause : negative_vectors)
			std::cout << clause.first << ' ';
		std::cout << std::endl;
	}

	if (backtrack_decision_level > 0)
	{
		setVarVal(cur_main_node->var.var_num, cur_main_node->var.var_value);
	}

	return true;
}

bool CDCLsolver_opt::checkMatrix()
{
	// М = М*
	if (positive_vectors.empty() && negative_vectors.empty())
	{
		BoolVector bufV1(varCount);
		BoolVector bufV0(varCount);
		findCoverage(M1, weightOfColumnsM1, vars_in_clausesM1, bufV1);
		findCoverage(M0, weightOfColumnsM0, vars_in_clausesM0, bufV0);

		if (bufV1.Weight() > bufV0.Weight())
		{
			V0 |= bufV0;
			result |= bufV0;
		}
		else
		{
			V1 |= bufV1;
			result |= bufV1;
		}

		return true;
	}
	// М = М* + М-
	if (positive_vectors.empty())
	{
		findCoverage(M0, weightOfColumnsM0, vars_in_clausesM0, V0);
		result |= V0;

		return true;
	}
	// М = М* + М+
	if (negative_vectors.empty())
	{
		findCoverage(M1, weightOfColumnsM1, vars_in_clausesM1, V1);
		result |= V1;

		return true;
	}

	BoolVector checkVectorM1(varCount);
	BoolVector checkVectorM0(varCount);
	bool M_positive_conflict = false;
	bool M_negative_conflict = false;

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
			M_positive_conflict = true;
			break;
		}
	}

	for (auto& i : negative_vectors)
	{
		if ((*(M0[i.first]) & checkVectorM1) == *(M0[i.first]))
		{
			M_negative_conflict = true;
			break;
		}
	}

	if (M_positive_conflict && M_negative_conflict)
		return false;


	bool solution_in_M1 = false;
	BoolVector coverageM1_V0(varCount);
	BoolVector coverageM1_V1(varCount);

	bool solution_in_M0 = false;
	BoolVector coverageM0_V0(varCount);
	BoolVector coverageM0_V1(varCount);

	if (!M_positive_conflict)
	{
		bool conflict = false;
		int min_clause_weight;
		int max_var_weight;
		int choosen_var;
		bool choosen_var_value;
		std::vector<int> removed_clause_id;
		// храним обнуленные переменные из дизъюнктов
		std::map<int, std::vector<int>> bt_vars_in_clausesM0;
		std::map<int, std::vector<int>> bt_vars_in_clausesM1;

		std::map<int, bool> positive_vectors(this->positive_vectors);

		std::vector<int> weightOfColumnsM1(varCount,0);
		for (auto& clause : positive_vectors)
		{
			for (auto& var : vars_in_clausesM1[clause.first])
			{
				weightOfColumnsM1[var]++;
			}
		}

		while (!conflict)
		{
			min_clause_weight = INT_MAX;
			max_var_weight = 0;
			choosen_var = -1;

			for (int i = 0; i < clauseCount; i++)
			{
				if (M0[i])
				{
					if (vars_in_clausesM0[i].size() > 0 && vars_in_clausesM0[i].size() <= min_clause_weight)
					{
						if (min_clause_weight > weightOfRows[i])
						{
							min_clause_weight = vars_in_clausesM0[i].size();
							max_var_weight = 0;
						}
						
						for (auto& var : vars_in_clausesM0[i])
						{
							if (weightOfColumnsM0[var] > max_var_weight)
							{
								max_var_weight = weightOfColumnsM0[var];
								choosen_var = var;
								choosen_var_value = false;
							}
						}
					}
				}
			}

			for (auto& clause : positive_vectors)
			{
				if (vars_in_clausesM1[clause.first].size() <= min_clause_weight)
				{
					if (min_clause_weight > weightOfRows[clause.first])
					{
						min_clause_weight = vars_in_clausesM1[clause.first].size();
						max_var_weight = 0;
					}

					for (auto& var : vars_in_clausesM1[clause.first])
					{
						if (weightOfColumnsM1[var] > max_var_weight)
						{
							max_var_weight = weightOfColumnsM1[var];
							choosen_var = var;
							choosen_var_value = true;
						}
					}
				}
			}

			if (choosen_var == -1)
			{
				solution_in_M1 = true;
				break;
			}

			if (choosen_var_value)
			{
				coverageM1_V1[choosen_var] = 1;

				for (int i = 0; i < clauseCount; i++)
				{
					// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
					if (positive_vectors.contains(i) && (*M1[i])[choosen_var])
					{
						positive_vectors.erase(i);
						removed_clause_id.push_back(i);
						bufM1.insertRow(M1.extractRow(i), i);
						bufM0.insertRow(M0.extractRow(i), i);
						// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
						for (auto& var : vars_in_clausesM1[i])
						{
							weightOfColumnsM1[var]--;
						}
					}
					// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
					if (M0[i] && (*M0[i])[choosen_var])
					{
						weightOfColumnsM0[choosen_var]--;
						bt_vars_in_clausesM0[i].push_back(choosen_var);
						//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
						// переносим переменные из дизъюнктов в стэк
						for (auto iter = vars_in_clausesM0[i].begin(); iter != vars_in_clausesM0[i].end(); iter++)
						{
							if (*iter == choosen_var)
							{
								vars_in_clausesM0[i].erase(iter);
								break;
							}
						}

						weightOfRows[i]--;
						if (vars_in_clausesM0[i].empty())
						{
							conflict = true;
							break;
						}
					}
				}
			}
			else
			{
				coverageM1_V0[choosen_var] = 1;

				for (int i = 0; i < clauseCount; i++)
				{
					// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
					if (M0[i] && (*M0[i])[choosen_var])
					{
						removed_clause_id.push_back(i);
						bufM1.insertRow(M1.extractRow(i), i);
						bufM0.insertRow(M0.extractRow(i), i);
						// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
						for (auto& var : vars_in_clausesM0[i])
						{
							weightOfColumnsM0[var]--;
						}
					}
					// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
					if (positive_vectors.contains(i) && (*M1[i])[choosen_var])
					{
						weightOfColumnsM1[choosen_var]--;
						bt_vars_in_clausesM1[i].push_back(choosen_var);
						//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
						// переносим переменные из дизъюнктов в стэк
						for (auto iter = vars_in_clausesM1[i].begin(); iter != vars_in_clausesM1[i].end(); iter++)
						{
							if (*iter == choosen_var)
							{
								vars_in_clausesM1[i].erase(iter);
								break;
							}
						}

						weightOfRows[i]--;
						if (vars_in_clausesM1[i].empty())
						{
							conflict = true;
							break;
						}
					}
				}
			}
		}
		// восстанавливаем матрицу
		for (auto& id : removed_clause_id)
		{
			M1.insertRow(bufM1.extractRow(id), id);
			M0.insertRow(bufM0.extractRow(id), id);
			
			for (auto& var : vars_in_clausesM0[id])
			{
				weightOfColumnsM0[var]++;
			}
		}

		for (auto& iter : bt_vars_in_clausesM0)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				weightOfColumnsM0[var]++;
				vars_in_clausesM0[iter.first].push_back(var);
			}
		}

		for (auto& iter : bt_vars_in_clausesM1)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				//weightOfColumnsM1[var]++;
				vars_in_clausesM1[iter.first].push_back(var);
			}
		}

	}

	if (!M_negative_conflict)
	{
		bool conflict = false;
		int min_clause_weight;
		int max_var_weight;
		int choosen_var;
		bool choosen_var_value;
		std::vector<int> removed_clause_id;
		// храним обнуленные переменные из дизъюнктов
		std::map<int, std::vector<int>> bt_vars_in_clausesM0;
		std::map<int, std::vector<int>> bt_vars_in_clausesM1;

		std::map<int, bool> negative_vectors(this->negative_vectors);

		std::vector<int> weightOfColumnsM0(varCount, 0);
		for (auto& clause : negative_vectors)
		{
			for (auto& var : vars_in_clausesM0[clause.first])
			{
				weightOfColumnsM0[var]++;
			}
		}

		while (!conflict)
		{
			min_clause_weight = INT_MAX;
			max_var_weight = 0;
			choosen_var = -1;

			for (int i = 0; i < clauseCount; i++)
			{
				if (M1[i])
				{
					if (vars_in_clausesM1[i].size() > 0 && vars_in_clausesM1[i].size() <= min_clause_weight)
					{
						if (min_clause_weight > weightOfRows[i])
						{
							min_clause_weight = vars_in_clausesM1[i].size();
							max_var_weight = 0;
						}

						for (auto& var : vars_in_clausesM1[i])
						{
							if (weightOfColumnsM1[var] > max_var_weight)
							{
								max_var_weight = weightOfColumnsM1[var];
								choosen_var = var;
								choosen_var_value = true;
							}
						}
					}
				}
			}

			for (auto& clause : negative_vectors)
			{
				if (vars_in_clausesM0[clause.first].size() <= min_clause_weight)
				{
					if (min_clause_weight > weightOfRows[clause.first])
					{
						min_clause_weight = vars_in_clausesM0[clause.first].size();
						max_var_weight = 0;
					}				

					for (auto& var : vars_in_clausesM0[clause.first])
					{
						if (weightOfColumnsM0[var] > max_var_weight)
						{
							max_var_weight = weightOfColumnsM0[var];
							choosen_var = var;
							choosen_var_value = false;
						}
					}
				}
			}

			if (choosen_var == -1)
			{
				solution_in_M0 = true;
				break;
			}

			//std::cout << "choosen var: " << choosen_var << std::endl << "var value: " << choosen_var_value << std::endl;

			if (choosen_var_value)
			{
				coverageM0_V1[choosen_var] = 1;

				for (int i = 0; i < clauseCount; i++)
				{
					// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
					if (M1[i] && (*M1[i])[choosen_var])
					{
						removed_clause_id.push_back(i);
						bufM1.insertRow(M1.extractRow(i), i);
						bufM0.insertRow(M0.extractRow(i), i);
						// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
						for (auto& var : vars_in_clausesM1[i])
						{
							weightOfColumnsM1[var]--;
						}
					}
					// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
					if (negative_vectors.contains(i) && (*M0[i])[choosen_var])
					{
						weightOfColumnsM0[choosen_var]--;
						bt_vars_in_clausesM0[i].push_back(choosen_var);
						//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
						// переносим переменные из дизъюнктов в стэк
						for (auto iter = vars_in_clausesM0[i].begin(); iter != vars_in_clausesM0[i].end(); iter++)
						{
							if (*iter == choosen_var)
							{
								vars_in_clausesM0[i].erase(iter);
								break;
							}
						}

						weightOfRows[i]--;
						if (vars_in_clausesM0[i].empty())
						{
							conflict = true;
							break;
						}
					}
				}
			}
			else
			{
				coverageM0_V0[choosen_var] = 1;

				for (int i = 0; i < clauseCount; i++)
				{
					// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
					if (negative_vectors.contains(i) && (*M0[i])[choosen_var])
					{
						negative_vectors.erase(i);

						removed_clause_id.push_back(i);
						bufM1.insertRow(M1.extractRow(i), i);
						bufM0.insertRow(M0.extractRow(i), i);
						// уменьшаем вес столбцов, убирая переменные которые находились в удаляемых дизъюнктах
						for (auto& var : vars_in_clausesM0[i])
						{
							weightOfColumnsM0[var]--;
						}
					}
					// убираем переменные из дизъюнктов в противоположной матрице, т.к. они равны 0
					if (M1[i] && (*M1[i])[choosen_var])
					{
						weightOfColumnsM1[choosen_var]--;
						bt_vars_in_clausesM1[i].push_back(choosen_var);
						//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
						// переносим переменные из дизъюнктов в стэк
						for (auto iter = vars_in_clausesM1[i].begin(); iter != vars_in_clausesM1[i].end(); iter++)
						{
							if (*iter == choosen_var)
							{
								vars_in_clausesM1[i].erase(iter);
								break;
							}
						}

						weightOfRows[i]--;
						if (vars_in_clausesM1[i].empty())
						{
							conflict = true;
							break;
						}
					}
				}
			}
			//printIntervals();
		}
		// восстанавливаем матрицу
		for (auto& id : removed_clause_id)
		{
			M1.insertRow(bufM1.extractRow(id), id);
			M0.insertRow(bufM0.extractRow(id), id);

			for (auto& var : vars_in_clausesM1[id])
			{
				weightOfColumnsM1[var]++;
			}
		}

		for (auto& iter : bt_vars_in_clausesM0)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				weightOfColumnsM0[var]++;
				vars_in_clausesM0[iter.first].push_back(var);
			}
		}

		for (auto& iter : bt_vars_in_clausesM1)
		{
			for (auto& var : iter.second)
			{
				weightOfRows[iter.first]++;
				weightOfColumnsM1[var]++;
				vars_in_clausesM1[iter.first].push_back(var);
			}
		}

	}

	if (solution_in_M0 && solution_in_M1)
	{
		if (coverageM0_V0.Weight() + coverageM0_V1.Weight() > coverageM1_V0.Weight() + coverageM1_V1.Weight())
		{
			V0 |= coverageM1_V0;
			V1 |= coverageM1_V1;
			result |= coverageM1_V0;
			result |= coverageM1_V1;
		}
		else
		{
			V0 |= coverageM0_V0;
			V1 |= coverageM0_V1;
			result |= coverageM0_V0;
			result |= coverageM0_V1;
		}
		return true;
	}

	if (solution_in_M0)
	{
		V0 |= coverageM0_V0;
		V1 |= coverageM0_V1;
		result |= coverageM0_V0;
		result |= coverageM0_V1;
		return true;
	}

	if (solution_in_M1)
	{
		V0 |= coverageM1_V0;
		V1 |= coverageM1_V1;
		result |= coverageM1_V0;
		result |= coverageM1_V1;
		return true;
	}


	return false;
}

bool CDCLsolver_opt::checkSolution(const std::string& DIMACS_filepath)
{
	std::ifstream in(DIMACS_filepath);

	while (in.peek() != 'p')
	{
		in.ignore(50, '\n');
	}

	in.ignore(10, 'f');

	in >> varCount;
	in >> clauseCount;
	
	BoolVector bufV0(varCount);
	BoolVector bufV1(varCount);

	for (int i = 0; i < clauseCount; i++)
	{
		bufV0.clear();
		bufV1.clear();
		int var;
		in >> var;
		while (var)
		{
			if (var > 0)
			{
				bufV1[var - 1] = 1;
			}
			else
			{
				bufV0[(-1) - var] = 1;
			}
			in >> var;
		}

		if (!(bufV1 & V1).operator bool() && !(bufV0 & V0).operator bool())
		{
			std::cout << "wrong solution\n" << V0 << ' ' << V1 << std::endl << bufV0 << ' ' << bufV1 << std::endl;
			in.close();
			return false;
		}

	}
	in.close();


	return true;
}

bool CDCLsolver_opt::chooseVarAlg()
{
	if (debug_mode)
	{
		std::cout << "intervals before choosing:\n";
		printIntervals();

		std::cout << "positive vectors: ";
		for (auto& clause : positive_vectors)
			std::cout << clause.first << ' ';
		std::cout << "\nnegative vectors: ";
		for (auto& clause : negative_vectors)
			std::cout << clause.first << ' ';
		std::cout << std::endl;
	}
	if (checkMatrix())
		return false;

	int minClauseWeight = INT_MAX;
	int maxWeight = 0;
	int choosenVar = -1;

	bool varVal;

	for (auto& clause : positive_vectors)
	{
		if (minClauseWeight >= weightOfRows[clause.first])
		{
			if (minClauseWeight > weightOfRows[clause.first])
			{
				minClauseWeight = weightOfRows[clause.first];
				maxWeight = 0;
			}
			for (auto& var : vars_in_clausesM1[clause.first])
			{
				if (maxWeight < weightOfColumnsM1[var])
				{
					maxWeight = weightOfColumnsM1[var];
					choosenVar = var;
					varVal = true;
				}
			}
		}
	}

	for (auto& clause : negative_vectors)
	{
		if (minClauseWeight >= weightOfRows[clause.first])
		{
			if (minClauseWeight > weightOfRows[clause.first])
			{
				minClauseWeight = weightOfRows[clause.first];
				maxWeight = 0;
			}
			for (auto& var : vars_in_clausesM0[clause.first])
			{
				if (maxWeight < weightOfColumnsM0[var])
				{
					maxWeight = weightOfColumnsM0[var];
					choosenVar = var;
					varVal = false;
				}
			}
		}
	}

	if (choosenVar == -1)
	{
		if(debug_mode)
			std::cout << "solution:\n" << V0 << ' ' << V1 << std::endl;
		return false;
	}

	// новая главная вершина
	cur_main_node->next = new ImplicationGraph::MainNode;
	// устанваливаем предка
	cur_main_node->next->prev = cur_main_node;
	// переносим текущай указатель
	cur_main_node = cur_main_node->next;
	// устанавливам значение выбранной переменной
	cur_main_node->var.var_num = choosenVar;
	cur_main_node->var.var_value = varVal;
	cur_main_node->var.decision_level = cur_main_node->prev->var.decision_level + 1;
	cur_main_node->V1.resize(varCount);
	cur_main_node->V0.resize(varCount);
	cur_main_node->positive_vectors = positive_vectors;
	cur_main_node->negative_vectors = negative_vectors;

	// запоминаем указатель на вершину
	ptrs_to_vars[choosenVar] = cur_main_node;
	ptrs_to_main_nodes[cur_main_node->var.decision_level] = cur_main_node;

	if(debug_mode)
		std::cout << "choosen var: " <<  choosenVar << " var value: " << varVal << std::endl;

	setVarVal(choosenVar, varVal);

	if(debug_mode)
		printIntervals();

	return true;
}

void CDCLsolver_opt::clearGraph()
{
	for (auto& ptr : ptrs_to_vars)
	{
		delete ptr.second;
	}

	implication_graph.root->next = nullptr;
	implication_graph.root->removed_clause_id.clear();
	implication_graph.root->vars_in_clausesM0.clear();
	implication_graph.root->vars_in_clausesM1.clear();

	ptrs_to_vars.clear();
	ptrs_to_main_nodes.clear();
}

void CDCLsolver_opt::createKNFfromDIMACS(const std::string& DIMACS_filepath)
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

	var_in_clause_weight1.clear();

	positive_vectors.clear();
	negative_vectors.clear();

	result.resize(varCount);

	clearGraph();
	cur_main_node = implication_graph.root;
	cur_main_node->V0.resize(varCount);
	cur_main_node->V1.resize(varCount);

	ptrs_to_main_nodes.clear();
	ptrs_to_main_nodes[0] = cur_main_node;

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
		
		if (vars_in_clausesM0[i].empty())
			positive_vectors[i] = true;
		if (vars_in_clausesM1[i].empty())
			negative_vectors[i] = true;
	}
	in.close();
	
	cur_main_node->negative_vectors = negative_vectors;
	cur_main_node->positive_vectors = positive_vectors;

}

bool CDCLsolver_opt::deduceAlg()
{
	if (queue_vars_in_clauses_weight1.empty())
		return false;
	//std::cout << "clause count: " << clauseCount << std::endl;
	// номер первой уникальной вершины
	int first_UIP = cur_main_node->var.var_num;
	std::stack<int> UIPs_stack;
	UIPs_stack.push(cur_main_node->var.var_num);
	// порядок фиксации переменных, чтобы отличать переменные на конфликтной стороне(значение больше чем у firstUIP вершины)
	std::map<int, int> topological_position_of_nodes;
	int topological_counter = 1;
	topological_position_of_nodes[cur_main_node->var.var_num] = 0;

	//std::cout << cur_main_node->var.var_num << "->";
	//std::cout << M0 << std::endl << M1 << std::endl;

	while (!conflict_exists && !queue_vars_in_clauses_weight1.empty())
	{
		auto var = queue_vars_in_clauses_weight1.front();
		int var_num;
		bool var_val;
		if (var > 0)
		{
			var_num = var - 1;
			var_val = true;
		}
		if (var < 0)
		{
			var_num = -(var + 1);
			var_val = false;
		}

		//std::cout << var_num << "->";

		topological_position_of_nodes[var_num] = topological_counter;
		topological_counter++;

		/*
		* нужно внести переменную в граф импликаций
		* для этого необходимо
		* 1. создать новую вершину
		* 2. определить все переменные, влияющие на выбор значения этой переменной
		* влияющие переменные это те, которые изначально находились в одних дизъюнктах вместе с текущей, но при фиксации были удалены
		*/

		// создаем вершину
		ImplicationGraph::SideNode* new_node = new ImplicationGraph::SideNode;
		new_node->var.var_num = var_num;
		new_node->var.var_value = var_val;
		new_node->var.decision_level = cur_main_node->var.decision_level;
		if (queue_vars_in_clauses_weight1.size() == 1 && potential_UIP)
		{
			new_node->var.UIP = true;
			UIPs_stack.push(new_node->var.var_num);
			first_UIP = new_node->var.var_num;
		}
		potential_UIP = false;
		// запомнили указатель
		ptrs_to_vars[var_num] = new_node;
		// определить все родительские вершины
		// проходимся по дизъюнктам, в которых находились переменные, влияющие на значение текущей переменной
		for (int& clause : var_in_clause_weight1[var])
		{
			BoolVector mask(varCount);
			mask[0] = 1;
			// ищем номера переменных
			for (int i = 0; i < varCount; i++, mask <<= 1)
			{
				// находим что i-ая переменная была в проверяемом дизъюнкте, следовательно она влияла на выбор текущей переменной
				if (i != var_num && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
				{
					// записываем указатели на текущую переменную
					ptrs_to_vars[i]->nodes_ptrs[var_num] = new_node;
					// и на родительскую для текущей
					new_node->parents_nodes_ptrs[i] = ptrs_to_vars[i];
				}
			}
		}

		queue_vars_in_clauses_weight1.pop();
		setVarVal(var_num, var_val);
	}
	//std::cout << conflict_var << std::endl;
	if (!conflict_exists)
	{
		var_in_clause_weight1.clear();
		return false;
	}

	conflict_exists = false;

	if (cur_main_node->var.decision_level == 0)
	{
		backtrack_decision_level = -1;
		while (!queue_vars_in_clauses_weight1.empty())
			queue_vars_in_clauses_weight1.pop();
		var_in_clause_weight1.clear();
		return true;
	}
	// встретили конфликт нужно решать
	// ???достроить граф импликаций???
	// ???найти срез???

	conflict_var_false = new ImplicationGraph::SideNode;
	conflict_var_true = new ImplicationGraph::SideNode;

	conflict_var_true->var.var_num = conflict_var;

	for (int& clause : var_in_clause_weight1[conflict_var + 1])
	{
		BoolVector mask(varCount);
		mask[0] = 1;
		// ищем номера переменных
		for (int i = 0; i < varCount; i++, mask <<= 1)
		{
			// находим что i-ая переменная была в проверяемом дизъюнкте, следовательно она влияла на выбор текущей переменной
			if (i != conflict_var && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
			{
				// записываем указатель на родительскую для текущей
				conflict_var_true->parents_nodes_ptrs[i] = ptrs_to_vars[i];
			}
		}
	}

	for (int& clause : var_in_clause_weight1[-(conflict_var + 1)])
	{
		BoolVector mask(varCount);
		mask[0] = 1;
		// ищем номера переменных
		for (int i = 0; i < varCount; i++, mask <<= 1)
		{
			// находим что i-ая переменная была в проверяемом дизъюнкте, следовательно она влияла на выбор текущей переменной
			if (i != conflict_var && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
			{
				// записываем указатель на родительскую для текущей
				conflict_var_false->parents_nodes_ptrs[i] = ptrs_to_vars[i];
			}
		}
	}

	while (!queue_vars_in_clauses_weight1.empty())
		queue_vars_in_clauses_weight1.pop();
	var_in_clause_weight1.clear();

	// ПОСТРОЕНИЕ РАЗРЕЗА
	// будем строить разрез по первой уникальной переменной
	// идем от конфликтных вершин
	// предки с предыдущих уровней идут в запоминаемый дизъюнкт с инверсией значений, а с текущего уровня в очередь
	BoolVector* learn_clause_V0 = new BoolVector(varCount);
	BoolVector* learn_clause_V1 = new BoolVector(varCount);

	BoolVector checked_nodes(varCount);

	std::queue<ImplicationGraph::Node*> queue_nodes;

	vars_in_clausesM0.resize(clauseCount + 1);
	vars_in_clausesM1.resize(clauseCount + 1);
	weightOfRows.resize(clauseCount + 1, 0);

	queue_nodes.push(conflict_var_false);
	queue_nodes.push(conflict_var_true);

	backtrack_decision_level = 0;

	UIPs_stack.pop();

	while (!queue_nodes.empty())
	{
		for (auto& ptr : static_cast<ImplicationGraph::SideNode*>(queue_nodes.front())->parents_nodes_ptrs)
		{

			if (ptr.second->var.decision_level == cur_main_node->var.decision_level)
			{
				if (topological_position_of_nodes[ptr.first] < topological_position_of_nodes[first_UIP])
				{
					queue_nodes.push(ptrs_to_vars[first_UIP]);
					checked_nodes[first_UIP] = 1;

					while (topological_position_of_nodes[UIPs_stack.top()] > topological_position_of_nodes[ptr.first])
					{
						UIPs_stack.pop();
					}

					first_UIP = UIPs_stack.top();
					UIPs_stack.pop();
				}

				if (ptr.first == first_UIP)
					continue;

				if (topological_position_of_nodes[ptr.first] > topological_position_of_nodes[first_UIP])
				{
					if (!checked_nodes[ptr.first])
					{
						checked_nodes[ptr.first] = 1;
						queue_nodes.push(ptr.second);
					}

					continue;
				}
			}

			if (backtrack_decision_level < ptr.second->var.decision_level)
				backtrack_decision_level = ptr.second->var.decision_level;

			if (ptr.second->var.var_value && !(*learn_clause_V0)[ptr.first])
			{
				ptrs_to_main_nodes[ptr.second->var.decision_level]->vars_in_clausesM0[clauseCount].push_back(ptr.first);
				(*learn_clause_V0)[ptr.first] = 1;
			}
			if (!ptr.second->var.var_value && !(*learn_clause_V1)[ptr.first])
			{
				ptrs_to_main_nodes[ptr.second->var.decision_level]->vars_in_clausesM1[clauseCount].push_back(ptr.first);
				(*learn_clause_V1)[ptr.first] = 1;
			}
		}
		queue_nodes.pop();
	}

	if (ptrs_to_vars[first_UIP]->var.var_value)
	{
		(*learn_clause_V0)[first_UIP] = 1;
		vars_in_clausesM0[clauseCount].push_back(first_UIP);
		weightOfColumnsM0[first_UIP]++;
	}
	else
	{
		(*learn_clause_V1)[first_UIP] = 1;
		vars_in_clausesM1[clauseCount].push_back(first_UIP);
		weightOfColumnsM1[first_UIP]++;
	}
	weightOfRows[clauseCount]++;

	if (debug_mode)
	{
		std::cout << "conflict var: " << conflict_var << std::endl;
		std::cout << "learned clause: " << *learn_clause_V0 << " " << *learn_clause_V1 << std::endl;
	}

	if (backtrack_decision_level == 0)
	{
		if (ptrs_to_vars[first_UIP]->var.var_value)
		{
			var_in_clause_weight1[-(first_UIP + 1)].push_back(clauseCount);
			queue_vars_in_clauses_weight1.push(-(first_UIP + 1));
		}
		if (!ptrs_to_vars[first_UIP]->var.var_value)
		{
			var_in_clause_weight1[first_UIP + 1].push_back(clauseCount);
			queue_vars_in_clauses_weight1.push(first_UIP + 1);
		}
	}

	bool var_value_in_new_clause = !ptrs_to_vars[first_UIP]->var.var_value;

	ImplicationGraph::MainNode* main_node_ptr = cur_main_node;

	while (main_node_ptr)
	{
		if (var_value_in_new_clause && main_node_ptr->vars_in_clausesM0.contains(clauseCount))
		{
			break;
		}

		if (!var_value_in_new_clause && main_node_ptr->vars_in_clausesM1.contains(clauseCount))
		{
			break;
		}

		if (var_value_in_new_clause)
			main_node_ptr->positive_vectors[clauseCount] = true;
		else
			main_node_ptr->negative_vectors[clauseCount] = true;

		main_node_ptr = main_node_ptr->prev;
	}

	// нужно добавить полученный дизъюнкт в матрицу
	// увеличить размер буферов и счетчика дизъюнктов
	clauseCount++;

	M1.addRow(learn_clause_V1);
	M0.addRow(learn_clause_V0);

	BoolVector* null = nullptr;
	bufM0.addRow(null);
	bufM1.addRow(null);

	return true;
}

void CDCLsolver_opt::findCoverage(BoolMatrix& M, std::vector<int>& weightOfColumns, std::vector<std::vector<int>>& vars_in_clauses, BoolVector& V)
{
	int min_clause_weight = INT_MAX;
	int maxWeight = 0;
	int choosenVar = -1;


	while (true)
	{
		min_clause_weight = INT_MAX;
		maxWeight = 0;
		choosenVar = -1;

		for (int i = 0; i < clauseCount; i++)
		{
			if (M[i])
			{
				if (min_clause_weight >= vars_in_clauses[i].size())
				{
					min_clause_weight = vars_in_clauses[i].size();

					for (auto& var : vars_in_clauses[i])
					{
						if (maxWeight < weightOfColumns[var])
						{
							maxWeight = weightOfColumns[var];
							choosenVar = var;
						}
					}
				}
			}
		}

		if (choosenVar == -1)
			return;

		V[choosenVar] = 1;
		for (int j = 0; j < clauseCount; j++)
		{
			if (M[j] && (*M[j])[choosenVar])
			{
				M.extractRow(j);

				for (auto& var : vars_in_clauses[j])
				{
					weightOfColumns[var]--;
				}
			}
		}
	}
}

void CDCLsolver_opt::printIntervals()
{
	BoolMatrix bufM1 = M1 - V0;
	BoolMatrix bufM0 = M0 - V1;
	for (int i = 0; i < clauseCount; i++)
	{
		if (M0[i])
			std::cout << *(bufM0[i]) << "  " << *(bufM1[i]) << "  :" << i << std::endl;
		/*else
			std::cout << i << ":--------------DELETED ROW--------------\n";*/
	}
}

void CDCLsolver_opt::setVarVal(int var, bool varVal)
{
	result[var] = 1;

	if (varVal)
	{
		V1[var] = 1;
		cur_main_node->V1[var] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			// ищем дизъюнкты, которые стали равны 1 и их нужно убрать
			if (M1[i] && (*M1[i])[var])
			{
				if (cur_main_node->var.decision_level == 0)
					cur_main_node->positive_vectors.erase(i);
				positive_vectors.erase(i);

				cur_main_node->removed_clause_id.push_back(i);
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
				cur_main_node->vars_in_clausesM0[i].push_back(var);
				//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
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
					if (cur_main_node->var.decision_level == 0)
						cur_main_node->positive_vectors[i] = true;
					positive_vectors[i] = true;
				}

				// уменьшаем веса строк и проверяем не стал ли вес равен 1
				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
				{
					if (!vars_in_clausesM0[i].empty())
					{
						if (!var_in_clause_weight1.contains(-(vars_in_clausesM0[i].front() + 1)))
						{
							queue_vars_in_clauses_weight1.push(-(vars_in_clausesM0[i].front() + 1));
							if (queue_vars_in_clauses_weight1.size() == 1)
								potential_UIP = true;
						}

						var_in_clause_weight1[-(vars_in_clausesM0[i].front() + 1)].push_back(i);

						if (var_in_clause_weight1.contains(vars_in_clausesM0[i].front() + 1))
						{
							conflict_exists = true;
							conflict_var = vars_in_clausesM0[i].front();
						}
					}
					if (!vars_in_clausesM1[i].empty())
					{
						if (!var_in_clause_weight1.contains(vars_in_clausesM1[i].front() + 1))
						{
							queue_vars_in_clauses_weight1.push(vars_in_clausesM1[i].front() + 1);
							if (queue_vars_in_clauses_weight1.size() == 1)
								potential_UIP = true;
						}

						var_in_clause_weight1[vars_in_clausesM1[i].front() + 1].push_back(i);

						if (var_in_clause_weight1.contains(-(vars_in_clausesM1[i].front() + 1)))
						{
							conflict_exists = true;
							conflict_var = vars_in_clausesM1[i].front();
						}
					}
				}
			}
		}
	}
	else
	{
		V0[var] = 1;
		cur_main_node->V0[var] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			if (M0[i] && (*M0[i])[var])
			{
				if (cur_main_node->var.decision_level == 0)
					cur_main_node->negative_vectors.erase(i);
				negative_vectors.erase(i);

				cur_main_node->removed_clause_id.push_back(i);
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
				cur_main_node->vars_in_clausesM1[i].push_back(var);
				//cur_main_node->vars_removed_from_clausesM1_id.push_back(i);
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
					if (cur_main_node->var.decision_level == 0)
						cur_main_node->negative_vectors[i] = true;
					negative_vectors[i] = true;
				}

				weightOfRows[i]--;
				if (weightOfRows[i] == 1)
				{
					if (!vars_in_clausesM0[i].empty())
					{
						if (!var_in_clause_weight1.contains(-(vars_in_clausesM0[i].front() + 1)))
						{
							queue_vars_in_clauses_weight1.push(-(vars_in_clausesM0[i].front() + 1));
							if (queue_vars_in_clauses_weight1.size() == 1)
								potential_UIP = true;
						}

						var_in_clause_weight1[-(vars_in_clausesM0[i].front() + 1)].push_back(i);

						if (var_in_clause_weight1.contains(vars_in_clausesM0[i].front() + 1))
						{
							conflict_exists = true;
							conflict_var = vars_in_clausesM0[i].front();
						}
					}
					if (!vars_in_clausesM1[i].empty())
					{
						if (!var_in_clause_weight1.contains(vars_in_clausesM1[i].front() + 1))
						{
							queue_vars_in_clauses_weight1.push(vars_in_clausesM1[i].front() + 1);
							if (queue_vars_in_clauses_weight1.size() == 1)
								potential_UIP = true;
						}

						var_in_clause_weight1[vars_in_clausesM1[i].front() + 1].push_back(i);

						if (var_in_clause_weight1.contains(-(vars_in_clausesM1[i].front() + 1)))
						{
							conflict_exists = true;
							conflict_var = vars_in_clausesM1[i].front();
						}
					}
				}
			}
		}
	}
}

bool CDCLsolver_opt::solve(const std::string& DIMACS_filepath, BoolVector& result_, int& time)
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
		//std::cout << counter++ << std::endl;
		/*if (++counter == 640)
			debug_mode = true;
		if (counter == 655)
		{
			int a = 1;
		}*/
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
			
			if(debug_mode)
				std::cout << V0 << ' ' << V1 << std::endl;

			result_ = std::move(result);
			return true;
		}
	}
}
