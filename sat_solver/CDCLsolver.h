#pragma once
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>

#include "BoolMatrix.h"
#include "BoolVector.h"

class CDCLsolver
{
public:
	CDCLsolver();

	~CDCLsolver();

	bool solve(const std::string& DIMACS_filepath, BoolVector& result_, int& time);

	bool checkSolution(const std::string& DIMACS_filepath);

private:

	struct ImplicationGraph
	{
		struct SideNode;

		struct Variable
		{
			Variable() = default;

			int var_num;
			bool var_value;
			int decision_level;
			bool UIP;
		};

		struct Node
		{
			virtual ~Node() = default;

			Variable var;
			std::map<int, SideNode*> nodes_ptrs;
		};

		struct MainNode : public Node
		{
			MainNode();
			~MainNode() = default;

			// ������ ��������� ����������
			std::vector<int> removed_clause_id;
			// ������ ���������� �� ������� ������� ��������� ����������
			/*std::vector<int> vars_removed_from_clausesM0_id;
			std::vector<int> vars_removed_from_clausesM1_id;*/
			// ������ ���������� ���������� �� ����������
			std::map<int, std::vector<int>> vars_in_clausesM0;
			std::map<int, std::vector<int>> vars_in_clausesM1;
			// ����������, ��������������� �� ������� �����
			BoolVector V1;
			BoolVector V0;

			MainNode* next;
			MainNode* prev;
		};

		struct SideNode : public Node
		{
			~SideNode() = default;

			std::map<int, Node*> parents_nodes_ptrs;
		};

		ImplicationGraph();

		~ImplicationGraph();

		MainNode* root;
	};

	bool backTrackAlg();

	bool chooseVarAlg();

	void clearGraph();

	void createKNFfromDIMACS(const std::string& DIMACS_filepath);

	bool deduceAlg();

	void printIntervals();

	void setVarVal(int var, bool varVal);

	// ���� ����������
	ImplicationGraph implication_graph;
	// ��������� �� ���������� � ����� ����������
	std::map<int, ImplicationGraph::Node*> ptrs_to_vars;
	// ��������� �� ������ � ����� ����������
	std::map<int, ImplicationGraph::MainNode*> ptrs_to_main_nodes;

	ImplicationGraph::MainNode* cur_main_node;

	// ����� ����������, ������� ��������� ���� � ���������; �� ������� ���� ����������
	// ������ ���������� � "+" � "-" � ����������� � "1" ��� ����� ������ ����������
	std::map<int, std::vector<int>> var_in_clause_weight1;
	std::queue<int> queue_vars_in_clauses_weight1;

	// ������������� �������
	BoolMatrix bufM1;
	BoolMatrix bufM0;

	BoolMatrix M1;
	BoolMatrix M0;

	// ��������������� ����������
	BoolVector V0;
	BoolVector V1;

	// ����������, ����������� � i-��� ����������
	std::vector<std::vector<int>> vars_in_clausesM0;
	std::vector<std::vector<int>> vars_in_clausesM1;
	// ������� ��� �����/��������
	std::vector<int> weightOfRows;
	std::vector<int> weightOfColumnsM0;
	std::vector<int> weightOfColumnsM1;

	ImplicationGraph::SideNode* conflict_var_true;
	ImplicationGraph::SideNode* conflict_var_false;
	int conflict_var;
	bool conflict_exists;

	int backtrack_decision_level;

	bool potential_UIP;

	BoolVector result;

	int varCount;
	int clauseCount;
};