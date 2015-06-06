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

	inline DumberBase&
	operator=(const DumberBase& v2) {
		v = v2.v;
		return *this;
	}

	inline DumberBase
	operator+(const DumberBase &v2)
	{ return DumberBase(v + v2.v); }

	inline DumberBase
	operator-(const DumberBase &v2)
	{ return DumberBase(v - v2.v); }

	inline DumberBase
	operator-()
	{ return DumberBase(-v); }

	inline DumberBase
	operator*(const DumberBase &v2)
	{ return DumberBase(v * v2.v); }

	inline DumberBase
	operator/(const DumberBase &v2)
	{ return DumberBase(v / v2.v); }

	inline DumberBase&
	operator+=(const DumberBase &v2)
	{
		v = v + v2.v;
		return *this;
	}

	inline DumberBase&
	operator-=(const DumberBase &v2)
	{
		v = v - v2.v;
		return *this;
	}

	inline DumberBase&
	operator*=(const DumberBase &v2)
	{
		v = v * v2.v;
		return *this;
	}

	inline DumberBase&
	operator/=(const DumberBase &v2)
	{
		v = v / v2.v;
		return *this;
	}
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

