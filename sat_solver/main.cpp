#include <iostream>
#include <string>
#include <fstream>

#include <filesystem>

#include "DPLLsolver.h"

#include "DPLLsolver_greed_var1.h"
#include "DPLLsolver_rand_var1.h"

#include "DPLLsolver_greed_var2.h"
#include "DPLLsolver_rand_var2.h"

#include "CDCLsolver.h"
#include "CDCLsolver_opt.h"

void run_test(DPLLsolver* solver, std::filesystem::path& test_dir_path, std::string&& out_file_name, bool SAT)
{
	bool allSAT;
	int counter = 0;
	int sumTime = 0;
	int sumRank = 0;
	int time;

	BoolVector result;

	std::ofstream out;

	for (auto& entry : std::filesystem::directory_iterator(test_dir_path))
	{
		out.open(out_file_name, std::ios_base::app);
		out << entry.path().filename();
		allSAT = SAT;
		sumTime = 0;
		sumRank = 0;
		counter = 0;

		for (auto& file : std::filesystem::directory_iterator(entry.path()))
		{
			// ������� � ������� ����� ���� ������ ��������������
			std::cout << file.path().string() + '\n';
			if ((*solver).solve(file.path().string(), result, time) != SAT)
			{
				std::cout << "\ntest " << file.path().filename() << " not passed\n";
				allSAT = !SAT;
			}
			sumTime += time;
			sumRank += result.Weight();
			counter++;
		}
		if (allSAT == SAT)
			out << "\nall "; SAT ? out << "" : out << "UN"; out << "SAT\ntests count: " << counter << "\nTOTAL time: " << sumTime << "\nAVG time(ms): " << sumTime / counter << "\nAVG rank: " << sumRank / counter << std::endl;
		out << "-----------------------\n";
		out.close();
	}

}

void run_test_CDCL(CDCLsolver& solver, std::filesystem::path& test_dir_path, std::string&& out_file_name, bool SAT)
{
	bool allSAT;
	int counter = 0;
	int sumTime = 0;
	int sumRank = 0;
	int time;

	BoolVector result;

	std::ofstream out;

	for (auto& entry : std::filesystem::directory_iterator(test_dir_path))
	{
		out.open(out_file_name, std::ios_base::app);
		out << entry.path().filename();
		allSAT = SAT;
		sumTime = 0;
		sumRank = 0;
		counter = 0;

		for (auto& file : std::filesystem::directory_iterator(entry.path()))
		{
			// ������� � ������� ����� ���� ������ ��������������
			std::cout << file.path().string() + '\n';
			if (solver.solve(file.path().string(), result, time) != SAT)
			{
				std::cout << "\ntest " << file.path().filename() << " not passed\n";
				allSAT = !SAT;
			}
			sumTime += time;
			sumRank += result.Weight();
			counter++;
		}
		if (allSAT == SAT)
			out << "\nall "; SAT ? out << "" : out << "UN"; out << "SAT\ntests count: " << counter << "\nTOTAL time: " << sumTime << "\nAVG time(ms): " << sumTime / counter << "\nAVG rank: " << sumRank / counter << std::endl;
		out << "-----------------------\n";
		out.close();
	}

}

void run_test_CDCL_opt(CDCLsolver_opt& solver, std::filesystem::path& test_dir_path, std::string&& out_file_name, bool SAT)
{
	bool allSAT;
	int counter = 0;
	int sumTime = 0;
	int sumRank = 0;
	int time;

	BoolVector result;

	std::ofstream out;

	for (auto& entry : std::filesystem::directory_iterator(test_dir_path))
	{
		out.open(out_file_name, std::ios_base::app);
		out << entry.path().filename();
		allSAT = SAT;
		sumTime = 0;
		sumRank = 0;
		counter = 0;

		for (auto& file : std::filesystem::directory_iterator(entry.path()))
		{
			// ������� � ������� ����� ���� ������ ��������������
			std::cout << file.path().string() + '\n';
			if (solver.solve(file.path().string(), result, time) != SAT)
			{
				std::cout << "\ntest " << file.path().filename() << " not passed\n";
				allSAT = !SAT;
			}
			sumTime += time;
			sumRank += result.Weight();
			counter++;
		}
		if (allSAT == SAT)
			out << "\nall "; SAT ? out << "" : out << "UN"; out << "SAT\ntests count: " << counter << "\nTOTAL time: " << sumTime << "\nAVG time(ms): " << sumTime / counter << "\nAVG rank: " << sumRank / counter << std::endl;
		out << "-----------------------\n";
		out.close();
	}

}

void run_single_test_CDCL_opt(CDCLsolver_opt& solver, std::filesystem::path& test_dir_path, std::string&& out_file_name, bool SAT)
{
	bool allSAT;
	int counter = 0;
	int sumTime = 0;
	int sumRank = 0;
	int time;

	BoolVector result;

	std::ofstream out;
	out.open(out_file_name, std::ios_base::app);
	for (auto& entry : std::filesystem::directory_iterator(test_dir_path))
	{
		out << entry.path().filename();
		allSAT = SAT;
		sumTime = 0;
		sumRank = 0;
		counter = 0;

		// ������� � ������� ����� ���� ������ ��������������
		std::cout << entry.path().string() + '\n';
		if (solver.solve(entry.path().string(), result, time) != SAT)
		{
			std::cout << "\ntest " << entry.path().filename() << " not passed\n";
			allSAT = !SAT;
		}

		solver.checkSolution(entry.path().string());

		sumTime += time;
		sumRank += result.Weight();
		counter++;
		if (allSAT == SAT)
			out << "\n"; SAT ? out << "" : out << "UN"; out << "SAT\ntests count: " << counter << "\nTOTAL time: " << sumTime <<  "\nrank: " << sumRank / counter << std::endl;
		out << "-----------------------\n";
	}
	out.close();
}

void run_single_test_CDCL(CDCLsolver& solver, std::filesystem::path& test_dir_path, std::string&& out_file_name, bool SAT)
{
	bool allSAT;
	int counter = 0;
	int sumTime = 0;
	int sumRank = 0;
	int time;

	BoolVector result;

	std::ofstream out;
	out.open(out_file_name, std::ios_base::app);
	for (auto& entry : std::filesystem::directory_iterator(test_dir_path))
	{
		out << entry.path().filename();
		allSAT = SAT;
		sumTime = 0;
		sumRank = 0;
		counter = 0;

		// ������� � ������� ����� ���� ������ ��������������
		std::cout << entry.path().string() + '\n';
		if (solver.solve(entry.path().string(), result, time) != SAT)
		{
			std::cout << "\ntest " << entry.path().filename() << " not passed\n";
			allSAT = !SAT;
		}

		solver.checkSolution(entry.path().string());

		sumTime += time;
		sumRank += result.Weight();
		counter++;
		if (allSAT == SAT)
			out << "\n"; SAT ? out << "" : out << "UN"; out << "SAT\ntests count: " << counter << "\nTOTAL time: " << sumTime << "\nrank: " << sumRank / counter << std::endl;
		out << "-----------------------\n";
	}
	out.close();
}

int main()
{
	// ���� �� ����� � ��������� �������
	std::filesystem::path sat_tests_dp = std::filesystem::current_path() / "test" / "SAT";
	std::filesystem::path unsat_tests_dp = std::filesystem::current_path() / "test" / "UNSAT";


	std::filesystem::path sat_tests_dp2 = std::filesystem::current_path() / "test" / "blocksworld";
	// ������ �����������
	std::string sat_tests_out("./TESTS_RESULT/SAT_TESTS_WITH_RANK2.txt");
	std::string unsat_tests_out("./TESTS_RESULT/UNSAT_TESTS_WITH_RANK2.txt");

	CDCLsolver cdcl_solver;
	
	//run_single_test_CDCL(cdcl_solver, sat_tests_dp2, "./TESTS_RESULT/SAT_TESTS_cdcl_solver.txt", true);
	//run_test_CDCL(cdcl_solver, sat_tests_dp, "./TESTS_RESULT/SAT_TESTS_cdcl_solver.txt", true);
	//run_test_CDCL(cdcl_solver, unsat_tests_dp, "./TESTS_RESULT/UNSAT_TESTS_cdcl_solver.txt", false);

	CDCLsolver_opt cdcl_solver_opt;
	
	run_single_test_CDCL_opt(cdcl_solver_opt, sat_tests_dp2, "./TESTS_RESULT/SAT_TESTS_cdcl_solver_optimized.txt", true);
	//run_test_CDCL_opt(cdcl_solver_opt, sat_tests_dp, "./TESTS_RESULT/SAT_TESTS_cdcl_solver_optimized.txt", true);
	//run_test_CDCL(cdcl_solver, unsat_tests_dp, "./TESTS_RESULT/UNSAT_TESTS_cdcl_solver_optimized.txt", false);

	DPLLsolver_greed_var2 solver_greed_var2;
	//run_test(&solver_greed_var2, sat_tests_dp, "./TESTS_RESULT/SAT_TESTS_greed_var2.txt", true);
	//run_test(&solver_greed_var2, unsat_tests_dp, "./TESTS_RESULT/UNSAT_TESTS_greed_var2.txt", false);

	return 0;
}