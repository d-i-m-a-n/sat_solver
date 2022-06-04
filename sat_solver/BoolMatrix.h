#pragma once
#include <string>
#include "BoolVector.h"

class DPLLsolver;

class BoolMatrix
{
public:
	BoolMatrix();

	BoolMatrix(int rowsCount);

	explicit BoolMatrix(int rowsCount, int columnsCount);

	BoolMatrix(char**, int, int);

	BoolMatrix(BoolMatrix&);

	BoolMatrix(BoolMatrix&& obj);

	~BoolMatrix();

	void addRow(BoolVector*& vector);

	void clear ();

	BoolVector conjuctionAllRows();

	BoolVector disjuncionAllRows();

	BoolVector* extractRow(int pos);

	int getRowsCount();

	int getColumnsCount();

	void inversionInRangeOfRow(int, int, int);

	void insertRow(BoolVector* vec, int pos);

	void resize(int rows, int columns = 0);

	void set0InRangeOfRow(int, int, int);

	void set1InRangeOfRow(int, int, int);

	int weightOfRow(int);

	int weightOfColumn(int);

	int weight();

	BoolMatrix operator - (BoolVector& vector);

	BoolMatrix operator + (BoolMatrix& matrix);

	BoolMatrix& operator = (BoolMatrix&);

	BoolMatrix operator & (BoolMatrix&);

	BoolMatrix& operator &= (BoolMatrix&);

	BoolMatrix operator | (BoolMatrix&);

	BoolMatrix& operator |= (BoolMatrix&);

	BoolMatrix operator ^ (BoolMatrix&);

	BoolMatrix& operator ^= (BoolMatrix&);

	BoolMatrix operator ~ ();

	BoolVector* operator [](int);

	bool operator == (BoolMatrix&);

	bool operator != (BoolMatrix&);

	operator bool();

	friend std::istream& operator >> (std::istream&, BoolMatrix& obj);
	friend std::ostream& operator << (std::ostream&, BoolMatrix& obj);

private:
	BoolVector** matrix;
	int rowCount;
	int columnCount;

};

