// Presenting: Dumber, the dumb number
// this is just some C++ bloat-i-mean-boilerplate
// for inheritance in vector types.

TEMPLATE
struct DumberBase {
	T v;

	inline
	DumberBase()
	{}

	inline
	DumberBase(T v2)
	{ v = v2; }

	inline
	DumberBase(const DumberBase &x)
	{ v = x.v; }

	friend inline DumberBase
	operator-(const DumberBase &a)
	{ return -a.v; }

	friend inline DumberBase
	operator+(const DumberBase &a, const DumberBase &b)
	{ return a.v + b.v; }

	friend inline DumberBase
	operator-(const DumberBase &a, const DumberBase &b)
	{ return a.v - b.v; }

	friend inline DumberBase
	operator*(const DumberBase &a, const DumberBase &b)
	{ return a.v*b.v; }

	friend inline DumberBase
	operator/(const DumberBase &a, const DumberBase &b)
	{ return a.v/b.v; }

	inline DumberBase &
	operator+=(const DumberBase &a)
	{ v += a.v; return *this; }

	inline DumberBase &
	operator-=(const DumberBase &a)
	{ v -= a.v; return *this; }

	inline DumberBase &
	operator*=(const DumberBase &a)
	{ v *= a.v; return *this; }

	inline DumberBase &
	operator/=(const DumberBase &a)
	{ v /= a.v; return *this; }

	friend inline DumberBase
	operator&(const DumberBase &a, const DumberBase &b)
	{ return a.v & b.v; }

	friend inline DumberBase
	operator|(const DumberBase &a, const DumberBase &b)
	{ return a.v | b.v; }

	friend inline DumberBase
	operator^(const DumberBase &a, const DumberBase &b)
	{ return a.v ^ b.v; }

	friend inline DumberBase
	operator<(const DumberBase &a, const DumberBase &b)
	{ return a.v < b.v; }

	friend inline DumberBase
	operator>(const DumberBase &a, const DumberBase &b)
	{ return a.v > b.v; }

	friend inline DumberBase
	operator==(const DumberBase &a, const DumberBase &b)
	{ return a.v == b.v; }
};

TEMPLATE
struct Dumber : public DumberBase<T> {
	inline
	Dumber()
	{}

	inline
	Dumber(DumberBase<T> v2) : DumberBase<T>(v2)
	{}
};
