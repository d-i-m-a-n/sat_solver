#include "CDCLsolver.h"
#include <fstream>
#include <stack>

CDCLsolver::ImplicationGraph::MainNode::MainNode()
{
	var.var_num = 0;
	var.decision_level = 0;
	next = nullptr;
	prev = nullptr;
}

CDCLsolver::ImplicationGraph::ImplicationGraph()
{
	root = new MainNode;
	root->var.var_num = -1;
	root->var.decision_level = 0;
}

CDCLsolver::ImplicationGraph::~ImplicationGraph()
{
	delete root;
}

CDCLsolver::CDCLsolver()
{
}

CDCLsolver::~CDCLsolver()
{
	clearGraph();
}

bool CDCLsolver::backTrackAlg()
{
	if (backtrack_decision_level == -1)
	{
		//std::cout << "conflict var: " << conflict_var << std::endl;
		return false;
	}
	/*for (auto& ptr : conflict_var_false->parents_nodes_ptrs)
	{
		ptr.second->nodes_ptrs.erase(-conflict_var);
	}
	for (auto& ptr : conflict_var_true->parents_nodes_ptrs)
	{
		ptr.second->nodes_ptrs.erase(conflict_var);
	}*/

	delete conflict_var_false;
	delete conflict_var_true;
	conflict_var_false = nullptr;
	conflict_var_true = nullptr;

	for (int level = cur_main_node->var.decision_level; level > backtrack_decision_level - 1 && level > 0; level--)
	{
		//std::cout << cur_main_node->var.decision_level << std::endl;
		// ��������� ��������� �� ������ � �������
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

		// ��������� ���������� ������� � ��������� � ����������� ����
		/*for (auto id = cur_main_node->vars_removed_from_clausesM0_id.begin(); id != cur_main_node->vars_removed_from_clausesM0_id.end(); id++)
		{
			for (auto var = cur_main_node->vars_in_clausesM0[*id].begin(); var != cur_main_node->vars_in_clausesM0[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM0[*var]++;
				vars_in_clausesM0[*id].push_back(*var);
			}
			cur_main_node->vars_in_clausesM0[*id].clear();
		}

		for (auto id = cur_main_node->vars_removed_from_clausesM1_id.begin(); id != cur_main_node->vars_removed_from_clausesM1_id.end(); id++)
		{
			for (auto var = cur_main_node->vars_in_clausesM1[*id].begin(); var != cur_main_node->vars_in_clausesM1[*id].end(); var++)
			{
				weightOfRows[*id]++;
				weightOfColumnsM1[*var]++;
				vars_in_clausesM1[*id].push_back(*var);
			}
			cur_main_node->vars_in_clausesM1[*id].clear();
		}*/

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

		// ��������� �������� ������� ������� � �������
		for (auto ptr : cur_main_node->nodes_ptrs)
		{
			if (!deleted_nodes[ptr.first])
			{
				deleted_nodes[ptr.first] = 1;
				del_queue.push(ptr.second);
			}

			ptr.second->parents_nodes_ptrs.erase(cur_main_node->var.var_num);
		}
		//cur_main_node->nodes_ptrs.clear();


		while (!del_queue.empty())
		{
			auto del_ptr = del_queue.front();
			// ��������� ���� �������� � �������
 			for (auto& ptr : del_ptr->nodes_ptrs)
			{
				if (!deleted_nodes[ptr.first])
				{
					deleted_nodes[ptr.first] = 1;
					del_queue.push(ptr.second);
				}
				
				ptr.second->parents_nodes_ptrs.erase(del_ptr->var.var_num);
			}

			// ������� � ���� ��������� ��������� �� ������� �������
			for (auto& par_ptr : del_ptr->parents_nodes_ptrs)
			{
				par_ptr.second->nodes_ptrs.erase(del_ptr->var.var_num);
			}
			del_ptr->parents_nodes_ptrs.clear();

			// ������� ��������� �� ������ ������ � ���� �������
			ptrs_to_vars.erase(del_ptr->var.var_num);
			delete del_ptr;
			del_queue.pop();
		}

 		if (level > backtrack_decision_level)
		{
			// ������� ������� ������� � ��������� ��������� �� ������� ����
			ptrs_to_vars.erase(cur_main_node->var.var_num);
			ptrs_to_main_nodes.erase(cur_main_node->var.decision_level);
			cur_main_node = cur_main_node->prev;
			delete cur_main_node->next;
			cur_main_node->next = nullptr;
		}
	}

	// ������� �������, �� ������� ����������
	cur_main_node->nodes_ptrs.clear();
	cur_main_node->V1.clear();
	cur_main_node->V0.clear();
	cur_main_node->removed_clause_id.clear();
	/*cur_main_node->vars_removed_from_clausesM0_id.clear();
	cur_main_node->vars_removed_from_clausesM1_id.clear();*/
	cur_main_node->vars_in_clausesM0.clear();
	cur_main_node->vars_in_clausesM1.clear();

	//std::cout << M0 << std::endl << M1 << std::endl;
	//std::cout << "after backtrack matrix:\n";
	//printIntervals(); 

	if (backtrack_decision_level)
	{
		setVarVal(cur_main_node->var.var_num, cur_main_node->var.var_value);

		//std::cout << M0 << std::endl << M1 << std::endl;
	}

	return true;
}

void CDCLsolver::clearGraph()
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

bool CDCLsolver::chooseVarAlg()
{
	//int minClauseWeight = INT_MAX;
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

	if (choosenVar == -1)
	{
		//std::cout << V0 << ' ' << V1 << std::endl;
		return false;
	}

	// ����� ������� �������
	cur_main_node->next = new ImplicationGraph::MainNode;
	// ������������� ������
	cur_main_node->next->prev = cur_main_node;
	// ��������� ������� ���������
	cur_main_node = cur_main_node->next;
	// ������������ �������� ��������� ����������
	cur_main_node->var.var_num = choosenVar;
	cur_main_node->var.var_value = varVal;
	cur_main_node->var.decision_level = cur_main_node->prev->var.decision_level + 1;
	cur_main_node->V1.resize(varCount);
	cur_main_node->V0.resize(varCount);

	// ���������� ��������� �� �������
	ptrs_to_vars[choosenVar] = cur_main_node;
	ptrs_to_main_nodes[cur_main_node->var.decision_level] = cur_main_node;

	//std::cout << "choosen var: " <<  choosenVar << " var value: " << varVal << std::endl;

	setVarVal(choosenVar, varVal);

	//printIntervals();

	return true;
}

void CDCLsolver::createKNFfromDIMACS(const std::string& DIMACS_filepath)
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

	// clause_weight1_id.clear();
	var_in_clause_weight1.clear();

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
	}
	in.close();
}

bool CDCLsolver::deduceAlg()
{
	if (queue_vars_in_clauses_weight1.empty())
		return false;
	//std::cout << "clause count: " << clauseCount << std::endl;
	// ����� ������ ���������� �������
	int first_UIP = cur_main_node->var.var_num;
	std::stack<int> UIPs_stack;
	UIPs_stack.push(cur_main_node->var.var_num);
	// ������� �������� ����������, ����� �������� ���������� �� ����������� �������(�������� ������ ��� � firstUIP �������)
	std::map<int, int> topological_position_of_nodes;
	int topological_counter = 1;
	topological_position_of_nodes[cur_main_node->var.var_num] = 0;
	
	//std::cout << cur_main_node->var.var_num << "->";
	//std::cout << M0 << std::endl << M1 << std::endl;

	while(!conflict_exists && !queue_vars_in_clauses_weight1.empty())
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
		* ����� ������ ���������� � ���� ����������
		* ��� ����� ����������
		* 1. ������� ����� �������
		* 2. ���������� ��� ����������, �������� �� ����� �������� ���� ����������
		* �������� ���������� ��� ��, ������� ���������� ���������� � ����� ���������� ������ � �������, �� ��� �������� ���� �������
		*/

		// ������� �������
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
		// ��������� ���������
		ptrs_to_vars[var_num] = new_node;
		// ���������� ��� ������������ �������
		// ���������� �� ����������, � ������� ���������� ����������, �������� �� �������� ������� ����������
		for (int& clause : var_in_clause_weight1[var])
		{
			BoolVector mask(varCount);
			mask[0] = 1;
			// ���� ������ ����������
			for (int i = 0; i < varCount; i++, mask <<= 1)
			{
				// ������� ��� i-�� ���������� ���� � ����������� ���������, ������������� ��� ������ �� ����� ������� ����������
				if (i != var_num && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
				{
					// ���������� ��������� �� ������� ����������
					ptrs_to_vars[i]->nodes_ptrs[var_num] = new_node;
					// � �� ������������ ��� �������
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

	if (conflict_var == 17)
	{
		int a = 1;
	}

	if (cur_main_node->var.decision_level == 0)
	{
		backtrack_decision_level = -1;
		while (!queue_vars_in_clauses_weight1.empty())
			queue_vars_in_clauses_weight1.pop();
		var_in_clause_weight1.clear();
		return true;
	}
	// ��������� �������� ����� ������
	// ???��������� ���� ����������???
	// ???����� ����???
	
	conflict_var_false = new ImplicationGraph::SideNode;
	conflict_var_true = new ImplicationGraph::SideNode;

	conflict_var_true->var.var_num = conflict_var;

	for (int& clause : var_in_clause_weight1[conflict_var + 1])
	{
		BoolVector mask(varCount);
		mask[0] = 1;
		// ���� ������ ����������
		for (int i = 0; i < varCount; i++, mask <<= 1)
		{
			// ������� ��� i-�� ���������� ���� � ����������� ���������, ������������� ��� ������ �� ����� ������� ����������
			if (i != conflict_var && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
			{
				// ���������� ��������� �� ������������ ��� �������
				conflict_var_true->parents_nodes_ptrs[i] = ptrs_to_vars[i];
			}
		}
	}

	for (int& clause : var_in_clause_weight1[-(conflict_var + 1)])
	{
		BoolVector mask(varCount);
		mask[0] = 1;
		// ���� ������ ����������
		for (int i = 0; i < varCount; i++, mask <<= 1)
		{
			// ������� ��� i-�� ���������� ���� � ����������� ���������, ������������� ��� ������ �� ����� ������� ����������
			if (i != conflict_var && ((mask & *(M0[clause])) || (mask & *(M1[clause]))))
			{
				// ���������� ��������� �� ������������ ��� �������
				conflict_var_false->parents_nodes_ptrs[i] = ptrs_to_vars[i];
			}
		}
	}

	while(!queue_vars_in_clauses_weight1.empty())
		queue_vars_in_clauses_weight1.pop();
	var_in_clause_weight1.clear();

	// ���������� �������
	// ����� ������� ������ �� ������ ���������� ����������
	// ���� �� ����������� ������
	// ������ � ���������� ������� ���� � ������������ �������� � ��������� ��������, � � �������� ������ � �������
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

	/*std::cout << "conflict var: " << conflict_var << std::endl;
	std::cout << "learned clause: " << *learn_clause_V0 << " " << *learn_clause_V1 << std::endl;*/

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

	// ����� �������� ���������� �������� � �������
	// ��������� ������ ������� � �������� ����������
	clauseCount++;

	M1.addRow(learn_clause_V1);
	M0.addRow(learn_clause_V0);

	BoolVector* null = nullptr;
	bufM0.addRow(null);
	bufM1.addRow(null);

	return true;
}

void CDCLsolver::printIntervals()
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

void CDCLsolver::setVarVal(int var, bool varVal)
{
	result[var] = 1;

	if (varVal)
	{
		V1[var] = 1;
		cur_main_node->V1[var] = 1;

		for (int i = 0; i < clauseCount; i++)
		{
			// ���� ���������, ������� ����� ����� 1 � �� ����� ������
			if (M1[i] && (*M1[i])[var])
			{
				cur_main_node->removed_clause_id.push_back(i);
				bufM1.insertRow(M1.extractRow(i), i);
				bufM0.insertRow(M0.extractRow(i), i);
				// ��������� ��� ��������, ������ ���������� ������� ���������� � ��������� ����������
				for (int j = 0; j < vars_in_clausesM1[i].size() || j < vars_in_clausesM0[i].size(); j++)
				{
					if (j < vars_in_clausesM1[i].size())
						weightOfColumnsM1[vars_in_clausesM1[i][j]]--;
					if (j < vars_in_clausesM0[i].size())
						weightOfColumnsM0[vars_in_clausesM0[i][j]]--;
				}
			}
			// ������� ���������� �� ���������� � ��������������� �������, �.�. ��� ����� 0
			if (M0[i] && (*M0[i])[var])
			{
				weightOfColumnsM0[var]--;
				cur_main_node->vars_in_clausesM0[i].push_back(var);
				//cur_main_node->vars_removed_from_clausesM0_id.push_back(i);
				// ��������� ���������� �� ���������� � ����
				for (auto iter = vars_in_clausesM0[i].begin(); iter != vars_in_clausesM0[i].end(); iter++)
				{
					if (*iter == var)
					{
						vars_in_clausesM0[i].erase(iter);
						break;
					}
				}

				// ��������� ���� ����� � ��������� �� ���� �� ��� ����� 1
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

bool CDCLsolver::solve(const std::string& DIMACS_filepath, BoolVector& result_, int& time)
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

			std::cout << V0 << ' ' << V1 << std::endl;

			result_ = std::move(result);
			return true;
		}
	}
}

bool CDCLsolver::checkSolution(const std::string& DIMACS_filepath)
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
