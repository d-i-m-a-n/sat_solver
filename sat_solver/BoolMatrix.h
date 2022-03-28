#pragma once
#include <string>
#include "BoolVector.h"

class BoolMatrix
{
public:
	BoolMatrix();

	explicit BoolMatrix(int, int);

	BoolMatrix(char**, int, int);

	BoolMatrix(BoolMatrix&);

	BoolMatrix(BoolMatrix&& obj);

	~BoolMatrix();

	void clear ();

	BoolVector conjuctionAllRows();

	BoolVector disjuncionAllRows();

	BoolVector*&& extractRow(int pos);

	int getRowsCount();

	int getColumnsCount();

	void inversionInRangeOfRow(int, int, int);

	void insertRow(BoolVector*&& vec, int pos);

	void resize(int rows, int columns = 0);

	void set0InRangeOfRow(int, int, int);

	void set1InRangeOfRow(int, int, int);

	int weightOfRow(int);

	int weightOfColumn(int);

	int weight();

	BoolMatrix operator - (BoolVector & vector);

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

