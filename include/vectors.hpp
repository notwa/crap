// all of this is just so i can:
// v2df(0.5)

typedef double _v2df __attribute__((vector_size(16), aligned(16)));
typedef float _v2sf __attribute__((vector_size(8), aligned(8)));
typedef float _v4sf __attribute__((vector_size(16), aligned(16)));

template<>
struct Dumber<_v2df> : public DumberBase<_v2df> {
	inline Dumber() {}
	inline Dumber(DumberBase<_v2df> v2) : DumberBase<_v2df>(v2) {}

	TEMPLATE inline
	Dumber(T x, T y)
	{ v = (_v2df){double(x), double(y)}; }

	TEMPLATE inline
	Dumber(T x)
	{ v = (_v2df){double(x), double(x)}; }
};

template<>
struct Dumber<_v2sf> : public DumberBase<_v2sf> {
	inline Dumber() {}
	inline Dumber(DumberBase<_v2sf> v2) : DumberBase<_v2sf>(v2) {}

	TEMPLATE inline
	Dumber(T x, T y)
	{ v = (_v2sf){float(x), float(y)}; }

	TEMPLATE inline
	Dumber(T x)
	{ v = (_v2sf){float(x), float(x)}; }
};

template<>
struct Dumber<_v4sf> : public DumberBase<_v4sf> {
	inline Dumber() {}
	inline Dumber(DumberBase<_v4sf> v2) : DumberBase<_v4sf>(v2) {}

	TEMPLATE inline
	Dumber(T x, T y, T z, T w)
	{ v = (_v4sf){float(x), float(y), float(z), float(w)}; }

	TEMPLATE inline
	Dumber(T x, T y)
	{ v = (_v4sf){float(x), float(y), float(x), float(y)}; }

	TEMPLATE inline
	Dumber(T x)
	{ v = (_v4sf){float(x), float(x), float(x), float(x)}; }
};

typedef Dumber<_v2df> v2df;
typedef Dumber<_v2sf> v2sf;
typedef Dumber<_v4sf> v4sf;
