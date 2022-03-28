#include "BoolMatrix.h"

BoolMatrix::BoolMatrix()
{
	matrix = nullptr;
	columnCount = rowCount = 0;
}

BoolMatrix::BoolMatrix(int ROW, int COLUMN = 8)
{
	matrix = nullptr;
	columnCount = rowCount = 0;
	if (ROW > 0)
	{
		rowCount = ROW;
		columnCount = COLUMN;
		matrix = new BoolVector * [rowCount];

		for (int i = 0; i < rowCount; i++)
			matrix[i] = new BoolVector(columnCount);
	}
}

BoolMatrix::BoolMatrix(char** mx, int ROW, int COLUMN)
{
	if (ROW < 1 || COLUMN < 1)
	{
		ROW = 8;
		COLUMN = 8;
		mx = nullptr;
	}

	rowCount = ROW;
	columnCount = COLUMN;
	matrix = new BoolVector * [rowCount];

	if (mx)
		for (int i = 0; i < rowCount; i++)
			matrix[i] = new BoolVector(mx[i], COLUMN);
	else
		for (int i = 0; i < rowCount; i++)
			matrix[i] = new BoolVector(COLUMN);

}

BoolMatrix::BoolMatrix(BoolMatrix& obj)
{
	matrix = nullptr;
	if (obj.rowCount > 0)
	{
		rowCount = obj.rowCount;
		columnCount = obj.columnCount;
		matrix = new BoolVector * [rowCount];

		for (int i = 0; i < rowCount; i++)
			matrix[i] = new BoolVector(*obj[i]);
	}
}

BoolMatrix::BoolMatrix(BoolMatrix&& obj)
{
	matrix = std::move(obj.matrix);
	rowCount = std::move(obj.rowCount);
	columnCount = std::move(obj.columnCount);

	obj.matrix = nullptr;
	obj.rowCount = 0;
	obj.columnCount = 0;
}

BoolMatrix::~BoolMatrix()
{
	for (int i = 0; i < rowCount; i++)
		delete matrix[i];
	if(matrix)
		delete[] matrix;
	matrix = nullptr;
	rowCount = 0;
	columnCount = 0;
}

void BoolMatrix::clear()
{
	for (int i = 0; i < rowCount; i++)
		delete matrix[i];
	if (matrix)
		delete[] matrix;
	matrix = nullptr;
	rowCount = 0;
	columnCount = 0;
}

BoolMatrix& BoolMatrix::operator = (BoolMatrix& obj)
{
	if (obj.matrix)
	{
		if (rowCount != obj.rowCount)
		{
			this->clear();
			rowCount = obj.rowCount;
			matrix = new BoolVector * [rowCount];
			for (int i = 0; i < rowCount; i++)
				matrix[i] = new BoolVector;
		}

		columnCount = obj.columnCount;

		for (int i = 0; i < rowCount; i++)
		{
			if (*obj.matrix[i])
			{
				if (!matrix[i])
					matrix[i] = new BoolVector;
				*matrix[i] = *obj.matrix[i];
			}
			else
			{
				if(matrix[i])
					delete matrix[i];
				matrix[i] = nullptr;
			}
		}
	}
	else
		this->clear();

	return *this;
}

BoolMatrix BoolMatrix::operator & (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		BoolMatrix res(rowCount, columnCount);
		if (matrix)
		{
			int size = columnCount / 8;
			if (columnCount % 8)
				size++;
			for (int i = 0; i < rowCount; i++)
			{
				if(matrix[i] && obj.matrix[i])
					for (int j = 0; j < size; res.matrix[i]->bits[j] = matrix[i]->bits[j] & obj.matrix[i]->bits[j], j++);
			}
		}
		return res;
	}
	throw "dimension error";
}

BoolMatrix& BoolMatrix::operator &= (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		int size = columnCount / 8;
		if (columnCount % 8)
			size++;
		for (int i = 0; i < rowCount; i++)
		{
			for (int j = 0; j < size; matrix[i]->bits[j] &= obj.matrix[i]->bits[j], j++);
		}
		return *this;
	}
	throw "dimension error";
}

BoolMatrix BoolMatrix::operator | (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		BoolMatrix res(rowCount, columnCount);
		int size = columnCount / 8;
		if (columnCount % 8)
			size++;
		for (int i = 0; i < rowCount; i++)
		{
			for (int j = 0; j < size; res.matrix[i]->bits[j] = matrix[i]->bits[j] | obj.matrix[i]->bits[j], j++);
		}
		return res;
	}
	throw "dimension error";
}

BoolMatrix& BoolMatrix::operator |= (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		int size = columnCount / 8;
		if (columnCount % 8)
			size++;
		for (int i = 0; i < rowCount; i++)
		{
			for (int j = 0; j < size; matrix[i]->bits[j] |= obj.matrix[i]->bits[j], j++);
		}
		return *this;
	}
	throw "dimension error";
}

BoolMatrix BoolMatrix::operator ^ (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		BoolMatrix res(rowCount, columnCount);
		int size = columnCount / 8;
		if (columnCount % 8)
			size++;
		for (int i = 0; i < rowCount; i++)
		{
			for (int j = 0; j < size; res.matrix[i]->bits[j] = matrix[i]->bits[j] | obj.matrix[i]->bits[j], j++);
		}
		return res;
	}
	throw "dimension error";
}

BoolMatrix& BoolMatrix::operator ^= (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		int size = columnCount / 8;
		if (columnCount % 8)
			size++;
		for (int i = 0; i < rowCount; i++)
		{
			for (int j = 0; j < size; matrix[i]->bits[j] ^= obj.matrix[i]->bits[j], j++);
		}
		return *this;
	}
	throw "dimension error";
}

BoolMatrix BoolMatrix::operator ~ ()
{
	BoolMatrix res(rowCount, columnCount);
	int size = columnCount / 8;
	if (columnCount % 8)
		size++;
	for (int i = 0; i < rowCount; i++)
		for (int j = 0; j < size; j++)
		{
			res.matrix[i]->bits[j] = ~matrix[i]->bits[j];

			int lastB = matrix[i]->len % 8;
			if (lastB)
			{
				unsigned char mask = (1 << lastB) - 1;
				res.matrix[i]->bits[size - 1] &= mask;
			}
		}
	return res;
}

BoolVector* BoolMatrix::operator [] (int i)
{
	if (i > -1 && i < rowCount)
		return matrix[i];
	throw "memory error";
}

bool BoolMatrix::operator == (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		for (int i = 0; i < rowCount; i++)
			if (*(matrix[i]) != *(obj.matrix[i]))
				return false;
		return true;
	}
	return false;
}

bool BoolMatrix::operator != (BoolMatrix& obj)
{
	if (rowCount == obj.rowCount && columnCount == obj.columnCount)
	{
		for (int i = 0; i < rowCount; i++)
			if (*(matrix[i]) != *(obj.matrix[i]))
				return true;
		return false;
	}
	return true;
}

BoolVector BoolMatrix::conjuctionAllRows()
{
	BoolVector res(columnCount);

	for (int i = 0; i < rowCount; i++)
		res &= *matrix[i];

	return res;
}

BoolVector BoolMatrix::disjuncionAllRows()
{
	BoolVector res(columnCount);

	for (int i = 0; i < rowCount; i++)
		res |= *matrix[i];

	return res;
}

BoolVector*&& BoolMatrix::extractRow(int pos)
{
	return std::move(matrix[pos]);
}

void BoolMatrix::set0InRangeOfRow(int beg, int Len, int N)
{
	matrix[N]->Set0InRange(beg, Len);
}

void BoolMatrix::set1InRangeOfRow(int beg, int Len, int N)
{
	matrix[N]->Set1InRange(beg, Len);
}

void BoolMatrix::inversionInRangeOfRow(int beg, int Len, int N)
{
	matrix[N]->InversionInRange(beg, Len);
}

void BoolMatrix::insertRow(BoolVector*&& vec, int pos)
{
	if (matrix[pos])
		delete matrix[pos];
	matrix[pos] = vec;
	vec = nullptr;
}

void BoolMatrix::resize(int rows, int columns)
{
	if (matrix)
	{
		for (int i = 0; i < rowCount; i++)
			if (matrix[i])
				delete matrix[i];
		delete[] matrix;
	}
	rowCount = rows;
	columnCount = columns;
	matrix = new BoolVector * [rowCount];


	for (int i = 0; i < rowCount; i++)
		if (columns)
			matrix[i] = new BoolVector(columnCount);
		else
			matrix[i] = nullptr;
}

int BoolMatrix::weightOfRow(int i)
{
	return matrix[i]->Weight();
}

int BoolMatrix::weightOfColumn(int i)
{
	int weight = 0;

	for (int j = 0; j < rowCount; j++)
	{
		if(matrix[j])
			if ((*matrix[j])[i])
				weight++;
	}
	return weight;
}

int BoolMatrix::weight()
{
	int res = 0;
	for (int i = 0; i < rowCount; i++)
		res += matrix[i]->Weight();

	return res;
}

BoolMatrix BoolMatrix::operator - (BoolVector& vector)
{
	BoolMatrix res;
	if (matrix && vector.len == columnCount)
	{
		res.rowCount = rowCount;
		res.columnCount = columnCount;
		res.matrix = new BoolVector * [rowCount];
		for (int i = 0; i < rowCount; i++)
		{
			if (matrix[i])
			{
				res.matrix[i] = new BoolVector(*matrix[i] - vector);
			}
			else
			{
				res.matrix[i] = nullptr;
			}
		}
	}
	return res;
}

int BoolMatrix::getRowsCount()
{
	return rowCount;
}

int BoolMatrix::getColumnsCount()
{
	return columnCount;
}

BoolMatrix::operator bool()
{
	BoolVector res(columnCount);

	for (int i = 0; i < rowCount; i++)
		res |= *matrix[i];

	if (res)
		return true;
	return false;
}

std::istream& operator >> (std::istream& in, BoolMatrix& obj)
{
	if (obj.matrix)
	{
		for (int i = 0; i < obj.rowCount; i++)
			delete obj.matrix[i];
		delete[] obj.matrix;
	}

	in >> obj.rowCount >> obj.columnCount;

	obj.matrix = new BoolVector * [obj.rowCount];

	in.get();
	for (int i = 0; i < obj.rowCount; i++)
	{
		std::string str;
		std::getline(in, str);
		obj.matrix[i] = new BoolVector(str);
	}

	return in;
}

std::ostream& operator << (std::ostream& out, BoolMatrix& obj)
{
	for (int i = 0; i < obj.rowCount; i++)
		out << *obj.matrix[i] << std::endl;
	return out;
}
