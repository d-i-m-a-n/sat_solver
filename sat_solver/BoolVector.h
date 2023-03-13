#pragma once
#include <iostream>
#include <string>

#define uc unsigned int

class BoolMatrix;

class BoolVector
{
	friend BoolMatrix;
public:

	class bitK
	{
	public:
		bitK(uc* BAIT, uc MASK);

		bitK& operator ~ ();

		bitK& operator = (bitK& obj);

		bitK& operator = (bool x);

		operator bool();

		friend std::istream& operator >> (std::istream& in, bitK& obj);
		friend std::ostream& operator << (std::ostream& out, bitK& obj);

	private:
		uc* bait;
		uc mask;
	};

	BoolVector();

	explicit BoolVector(int LEN);

	BoolVector(BoolVector& obj);

	BoolVector(BoolVector&& obj);

	explicit BoolVector(std::string& str);

	explicit BoolVector(char* str);

	BoolVector(char* str, int LEN);

	~BoolVector();

	void clear();

	int GetLength();

	int GetSize();

	void Inversion(int i);

	void InversionInRange(int i, int len);

	void resize(int len);

	void Set0(int i);

	void Set0InRange(int i, int len);

	void Set1(int i);

	void Set1InRange(int i, int len);

	int Weight();

	BoolVector operator - (BoolVector& obj);

	BoolVector& operator = (BoolVector& obj);

	BoolVector& operator = (BoolVector&& obj);

	BoolVector operator & (BoolVector& obj);

	BoolVector& operator &= (BoolVector& obj);

	BoolVector& operator &= (BoolVector&& obj);

	BoolVector operator | (BoolVector& obj);

	BoolVector& operator |= (BoolVector& obj);

	BoolVector& operator |= (BoolVector&& obj);

	BoolVector operator ^ (BoolVector& obj);

	BoolVector& operator ^= (BoolVector& obj);

	BoolVector operator ~ ();

	BoolVector operator >> (int N);

	BoolVector operator << (int N);

	BoolVector& operator >>= (int N);

	BoolVector& operator <<= (int N);

	bitK operator [] (int N);

	bool operator == (BoolVector& obj);

	bool operator != (BoolVector& obj);

	operator char* ();

	operator bool();

	operator std::string();

	friend std::istream& operator >> (std::istream& in, BoolVector& obj);
	friend std::ostream& operator << (std::ostream& out, BoolVector& obj);

protected:
	uc* bits;
	int len;
	int size;

	static const int trailSize;
};

