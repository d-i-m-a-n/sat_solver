#include "BoolVector.h"

const int BoolVector::trailSize = 32;

BoolVector::bitK::bitK(uc* BAIT, uc MASK)
{
	bait = BAIT;
	mask = MASK;
}

BoolVector::bitK& BoolVector::bitK::operator ~ ()
{
	*bait ^= mask;

	return *this;
}

BoolVector::bitK& BoolVector::bitK::operator = (BoolVector::bitK& obj)
{
	if (*(obj.bait) & obj.mask)
		*bait |= mask;
	else
		*bait &= ~mask;
	return *this;
}

BoolVector::bitK& BoolVector::bitK::operator = (bool x)
{
	if (x)
		*bait |= mask;
	else
		*bait &= ~mask;

	return *this;
}

BoolVector::bitK::operator bool()
{
	if (*bait & mask)
		return true;
	return false;
}

std::ostream& operator << (std::ostream& out, BoolVector::bitK& obj)
{
	if (*(obj.bait) & obj.mask)
		out << '1';
	else
		out << '0';
	return out;
}

std::istream& operator >> (std::istream& in, BoolVector::bitK& obj)
{
	char a;
	in >> a;

	if (a - 48)
		*(obj.bait) |= obj.mask;
	else
		*(obj.bait) &= obj.mask;

	return in;
}

BoolVector::BoolVector()
{
	len = 32;
	size = 1;
	if (len % trailSize != 0)
		size++;
	bits = new uc[size];
	for (int i = 0; i < size; bits[i++] = 0);
}

BoolVector::BoolVector(int LEN)
{
	if (LEN > 0)
	{
		
		len = LEN;
		size = LEN / trailSize;
		if (LEN % trailSize != 0)
			size++;
		bits = new uc[size];
		for (int i = 0; i < size; bits[i++] = 0);
	}
}

BoolVector::BoolVector(BoolVector& obj)
{
	
	len = obj.len;
	size = obj.size;
	bits = new uc[size];

	for (int i = 0; i < size; bits[i] = obj.bits[i], i++);
}

BoolVector::BoolVector(BoolVector&& obj)
{
	
	len = std::move(obj.len);
	size = std::move(obj.size);
	bits = std::move(obj.bits);
	obj.bits = nullptr;
	obj.len = 0;
	obj.size = 0;
}

BoolVector::BoolVector(std::string& str)
{
	if (str.length())
	{
		
		len = str.length();

		size = len / trailSize;
		if (len % trailSize != 0)
			size++;

		bits = new uc[size]{ 0 };

		for (int j = 0, k = 0; str[k]; j++)
		{
			uc mask = 1;
			for (int i = 0; i < trailSize && str[k]; i++, k++, mask = mask << 1)
			{
				if (str[k] - 48)
					bits[j] |= mask;
			}
		}
	}
}

BoolVector::BoolVector(char* str)
{
	if (str)
	{
		
		len = 0;
		for (char* ptr = str; *ptr; len++, ptr++);

		size = len / trailSize;
		if (len % trailSize != 0)
			size++;

		bits = new uc[size]{ 0 };

		for (int j = 0; *str; j++)
		{
			uc mask = 1;
			for (int i = 0; i < trailSize && *str; i++, str++, mask = mask << 1)
			{
				if (*str - 48)
					bits[j] |= mask;
			}
		}
	}
}

BoolVector::BoolVector(char* str, int LEN)
{
	if (str && len > 0)
	{
		
		len = LEN;
		size = len / trailSize;

		if (len % trailSize)
			size++;

		for (int i = 0; i < size; bits[i++] = 0)

			for (int i = 0; i < len && *str; i++, str++)
			{
				uc mask = 1;
				for (int j = 0; j < trailSize && i < len && *str; i++, str++, j++, mask <<= 1)
					if (*str - 48)
						bits[i] |= mask;
			}
	}
}

BoolVector::~BoolVector()
{
	if(bits)
		delete[] bits;
	bits = nullptr;
	len = 0;
	size = 0;
}

void BoolVector::clear()
{
	for (int i = 0; i < size; bits[i] = 0, i++);
}

int BoolVector::GetLength()
{
	return len;
}

int BoolVector::GetSize()
{
	return size;
}

void BoolVector::resize(int LEN)
{
	if (bits)
		delete[] bits;
	len = LEN;
	size = LEN / trailSize;
	if (LEN % trailSize != 0)
		size++;
	bits = new uc[size]{ 0 };
}

void BoolVector::Set0(int i)
{
	if (i > -1 && i < len)
	{
		int k = i / trailSize;
		uc mask = 1 << (i % trailSize);

		bits[k] &= ~mask;
		return;
	}
	throw "memory error";
}

void BoolVector::Set0InRange(int i, int N = 1)
{
	if (i > -1 && N > 0 && (i + N - 1) < len)
	{
		int b1 = i / trailSize;
		int p1 = i % trailSize;
		uc mask1;

		if (i + N > trailSize)
			mask1 = (1 << p1) - 1;
		else
		{
			mask1 = 1 << p1;
			for (int k = 0; k < N - 1; k++)
				mask1 = mask1 | (mask1 << 1);
			mask1 = ~mask1;
		}

		bits[b1] &= mask1;

		N -= (trailSize - p1);
		int b2 = (i + N) / trailSize;
		int p2 = N % trailSize;
		uc mask2 = (1 << p2) - 1;

		if (b1 != b2)
		{
			for (int k = b1 + 1; k < b2; bits[k++] = 0);

			bits[b2] &= ~mask2;
		}
		return;
	}
	throw "memory error";
}

void BoolVector::Set1(int i)
{
	if (i > -1 && i < len)
	{
		int b = i / trailSize;
		int p = i % trailSize;
		uc mask = 1 << p;

		bits[b] |= mask;
		return;
	}
	throw "memory error";
}

void BoolVector::Set1InRange(int i, int N = 1)
{
	if (i > -1 && N > 0 && (i + N - 1) < len)
	{
		int b1 = i / trailSize;
		int p1 = i % trailSize;
		uc mask1;

		if (i + N > trailSize)
			mask1 = (1 << p1) - 1;
		else
		{
			mask1 = 1 << p1;
			for (int k = 0; k < N - 1; k++)
				mask1 = mask1 | (mask1 << 1);
			mask1 = ~mask1;
		}

		bits[b1] |= ~mask1;

		N -= (trailSize - p1);
		int b2 = (i + N) / trailSize;
		int p2 = N % trailSize;
		uc mask2 = (1 << p2) - 1;

		if (b1 != b2)
		{
			for (int k = b1 + 1; k < b2; bits[k++] = UCHAR_MAX);

			bits[b2] |= mask2;
		}
		return;
	}
	throw "memory error";
}

void BoolVector::Inversion(int i)
{
	if (i > -1 && i < len)
	{
		int b = i / trailSize;
		uc mask = 1 << (i % trailSize);

		bits[b] ^= mask;
		return;
	}
	throw "memory error";
}

void BoolVector::InversionInRange(int i, int N = 1)
{
	if (i > -1 && N > 0 && (i + N - 1) < len)
	{
		int b1 = i / trailSize;
		int p1 = i % trailSize;
		uc mask1;

		if (i + N > trailSize)
			mask1 = (1 << p1) - 1;
		else
		{
			mask1 = 1 << p1;
			for (int k = 0; k < N - 1; k++)
				mask1 = mask1 | (mask1 << 1);
			mask1 = ~mask1;
		}

		bits[b1] ^= ~mask1;

		N -= (trailSize - p1);
		int b2 = (i + N) / trailSize;
		int p2 = N % trailSize;
		uc mask2 = (1 << p2) - 1;

		if (b1 != b2)
		{
			for (int k = b1 + 1; k < b2; bits[k++] = 0);

			bits[b2] ^= mask2;
		}
		return;
	}
	throw "memory error";
}

int BoolVector::Weight()
{
	int res = 0;
	for (int i = 0, k = 0; i < len; k++)
	{
		if (!bits[k])
		{
			i += trailSize;
			continue;
		}
		uc mask = 1;
		for (int j = 0; j < trailSize && i < len; j++, i++, mask <<= 1)
			if (bits[k] & mask)
				res++;
	}
	return res;
}

BoolVector BoolVector::operator-(BoolVector& obj)
{
	if (len == obj.len)
	{
		BoolVector res(len);

		for (int i = 0; i < size; res.bits[i] = ~(bits[i] & obj.bits[i]) & bits[i], i++);

		return res;
	}
	throw "different length";
}

BoolVector& BoolVector::operator = (BoolVector& obj)
{
	if (this != &obj)
	{
		if(bits)
			delete[] bits;
		len = obj.len;
		size = obj.size;
		bits = new uc[size];

		for (int i = 0; i < size; bits[i] = obj.bits[i++]);
	}
	return *this;
}

BoolVector& BoolVector::operator=(BoolVector&& obj)
{
	/*if (this != &obj)
	{
		if(bits)
			delete[] bits;
		len = obj.len;
		size = obj.size;
		bits = new uc[size];

		for (int i = 0; i < size; bits[i] = obj.bits[i++]);
	}*/

	if (this != &obj)
	{
		if(bits)
			delete[] bits;
		len = std::move(obj.len);
		size = std::move(obj.size);
		bits = std::move(obj.bits);
		obj.bits = nullptr;
		obj.len = 0;
		obj.size = 0;
	}
	
	return *this;
}

BoolVector BoolVector::operator & (BoolVector& obj)
{
	if (len == obj.len)
	{
		BoolVector res(len);

		for (int i = 0; i < size; res.bits[i] = bits[i] & obj.bits[i], i++);

		return res;
	}
	throw "different length";
}

BoolVector& BoolVector::operator &= (BoolVector& obj)
{
	if (len == obj.len)
	{
		for (int i = 0; i < size; bits[i] &= obj.bits[i], i++);

		return *this;
	}
	throw "different length";
}

BoolVector& BoolVector::operator&=(BoolVector&& obj)
{
	if (len == obj.len)
	{
		for (int i = 0; i < size; bits[i] &= obj.bits[i], i++);

		return *this;
	}
	throw "different length";
}

BoolVector BoolVector::operator | (BoolVector& obj)
{
	if (len == obj.len)
	{
		BoolVector res(len);

		for (int i = 0; i < size; res.bits[i] = bits[i] | obj.bits[i], i++);

		return res;
	}
	throw "different length";
}

BoolVector& BoolVector::operator |= (BoolVector& obj)
{
	if (len == obj.len)
	{
		for (int i = 0; i < size; bits[i] |= obj.bits[i], i++);

		return *this;
	}
	throw "different length";
}

BoolVector BoolVector::operator ^ (BoolVector& obj)
{
	if (len == obj.len)
	{
		BoolVector res(len);

		for (int i = 0; i < size; res.bits[i] = bits[i] ^ obj.bits[i], i++);

		return res;
	}
	throw "different length";
}

BoolVector& BoolVector::operator ^= (BoolVector& obj)
{
	if (len == obj.len)
	{
		for (int i = 0; i < size; bits[i] ^= obj.bits[i], i++);

		return *this;
	}
	throw "different length";
}

BoolVector BoolVector::operator ~ ()
{
	BoolVector res(len);
	for (int i = 0; i < size; res.bits[i] = ~bits[i], i++);

	int lastB = len % trailSize;
	if (lastB)
	{
		uc mask = (1 << lastB) - 1;
		res.bits[size - 1] &= mask;
	}

	return res;
}

BoolVector BoolVector::operator << (int N)
{
	if (N > 0)
	{
		BoolVector res(len);
		if (N >= len)
		{
			for (int i = 0; i < size; res.bits[i++] = 0);
		}
		else
		{
			int n1 = N / trailSize, n2 = N % trailSize, i;

			uc mask1 = (1 << (trailSize - n2)) - 1;
			uc mask2 = ~mask1;

			for (i = size - 1; i > n1; i--)
				res.bits[i] = ((bits[i - n1] & mask1) << n2) | ((bits[i - n1 - 1] & mask2) >> (trailSize - n2));

			res.bits[i] = bits[i] << n2;

			for (i--; i > 0; res.bits[i--] = 0);

			if (len % trailSize)
			{
				uc mask = (1 << (len % trailSize)) - 1;
				res.bits[size - 1] &= mask;
			}
		}
		return res;
	}
	return *this;
}

BoolVector BoolVector::operator >> (int N)
{
	if (N > 0)
	{
		BoolVector res(len);
		if (N >= len)
		{
			for (int i = 0; i < size; res.bits[i++] = 0);
		}
		else
		{
			int n1 = N / trailSize, n2 = N % trailSize, i;

			uc mask2 = (1 << n2) - 1;
			uc mask1 = ~mask2;

			for (i = 0; i < size - 1 - n1; i++)
				res.bits[i] = ((bits[i + n1] & mask1) >> n2) | ((bits[i + n1 + 1] & mask2) << (trailSize - n2));

			res.bits[i] = bits[i] >> n2;

			for (i++; i < size; res.bits[i++] = 0);
		}
		return res;
	}
	return *this;
}

BoolVector& BoolVector::operator >>= (int N)
{
	if (N > 0)
	{
		if (N >= len)
		{
			for (int i = 0; i < size; bits[i++] = 0);
		}
		else
		{
			int n1 = N / trailSize, n2 = N % trailSize, i;

			uc mask2 = (1 << n2) - 1;
			uc mask1 = ~mask2;

			for (i = 0; i < size - 1 - n1; i++)
				bits[i] = ((bits[i + n1] & mask1) >> n2) | ((bits[i + n1 + 1] & mask2) << (trailSize - n2));

			bits[i] >>= n2;

			for (i++; i < size; bits[i++] = 0);
		}
	}
	return *this;
}

BoolVector& BoolVector::operator <<= (int N)
{
	if (N > 0)
	{
		if (N >= len)
		{
			for (int i = 0; i < size; bits[i++] = 0);
		}
		else
		{
			int n1 = N / trailSize, n2 = N % trailSize, i;

			uc mask1 = (1 << (trailSize - n2)) - 1;
			uc mask2 = ~mask1;

			for (i = size - 1; i > n1; i--)
				bits[i] = ((bits[i - n1] & mask1) << n2) | ((bits[i - n1 - 1] & mask2) >> (trailSize - n2));

			bits[i] = bits[i] << n2;

			for (i--; i > 0; bits[i--] = 0);

			if (len % trailSize)
			{
				uc mask = (1 << (len % trailSize)) - 1;
				bits[size - 1] &= mask;
			}
		}
	}
	return *this;
}

BoolVector::bitK BoolVector::operator [] (int N)
{
	int n = N / trailSize;
	int k = N % trailSize;

	bitK res(bits + n, 1 << k);

	return res;
}

bool BoolVector::operator == (BoolVector& obj)
{
	if (len == obj.len)
	{
		int i;
		for (i = 0; i < size && bits[i] == obj.bits[i]; i++);

		if (i == size)
			return true;
	}
	return false;
}

bool BoolVector::operator != (BoolVector& obj)
{
	if (len == obj.len)
	{
		int i;
		for (i = 0; i < size && bits[i] == obj.bits[i]; i++);

		if (i == size)
			return false;
	}
	return true;
}

BoolVector::operator char* ()
{
	char* str = new char[len + 1];

	for (int i = 0, k = 0; i < len; k++)
	{
		uc mask = 1;
		for (int j = 0; j < trailSize; j++, i++, mask <<= 1)
			if (bits[k] & mask)
				str[i] = '1';
			else
				str[i] = '0';
	}

	str[len] = '\0';

	return str;
}

BoolVector::operator bool()
{
	for (int i = 0; i < size; i++)
	{
		if (bits[i])
			return true;
	}
	return false;
}

BoolVector::operator std::string()
{
	std::string str;
	str.resize(len + 1);

	for (int i = 0, k = 0; i < len; k++)
	{
		uc mask = 1;
		for (int j = 0; j < trailSize; j++, i++, mask <<= 1)
			str[i] = bits[k] & mask + 48;
	}

	str[len] = '\0';

	return str;
}

std::ostream& operator << (std::ostream& out, BoolVector& obj)
{
	int p = obj.len % obj.trailSize;
	int i = obj.size - 1;
	if (p)
	{
		uc mask = 1 << (p - 1);
		for (int j = 0; j < p; j++, mask >>= 1)
			if (obj.bits[i] & mask)
				std::cout << '1';
			else
				std::cout << '0';
		i--;
	}

	for (; i > -1; i--)
	{
		uc mask = ~((1 << (obj.trailSize - 1)) - 1);
		for (int j = 0; j < obj.trailSize; j++, mask >>= 1)
			if (obj.bits[i] & mask)
				std::cout << '1';
			else
				std::cout << '0';
	}

	return out;
}

std::istream& operator >> (std::istream& in, BoolVector& obj)
{
	if (obj.bits)
		delete[] obj.bits;

	std::string str;

	std::getline(in, str);
	obj.len = str.length();
	obj.size = obj.len / obj.trailSize;

	obj.bits = new uc[obj.size];

	for (int i = 0, k = 0; i < obj.len; k++)
	{
		uc mask = 1;
		for (int j = 0; j < obj.trailSize && i < obj.len; i++, j++, mask <<= 1)
		{
			if (str[i] - 48)
				obj.bits[k] |= mask;
			else
				obj.bits[k] &= ~mask;
		}
	}

	return in;
}