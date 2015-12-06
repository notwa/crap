#include <emmintrin.h>
typedef __m128d _v2df;
typedef __m64 _v2sf;
typedef __m128 _v4sf;

struct v2df {
	_v2df v;

	v2df()
	{}

	v2df(double d)
	{ v = _mm_set1_pd(d); }

	v2df(double d0, double d1)
	{ v = _mm_setr_pd(d0,d1); }

	friend inline v2df
	operator-(const v2df &a)
	{ v2df c; c.v =_mm_sub_pd(_mm_setzero_pd(),a.v); return c; }

	friend inline v2df
	operator+(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_add_pd(a.v,b.v); return c; }

	friend inline v2df
	operator-(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_sub_pd(a.v,b.v); return c; }

	friend inline v2df
	operator*(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_mul_pd(a.v,b.v); return c; }

	friend inline v2df
	operator/(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_div_pd(a.v,b.v); return c; }

	friend inline v2df
	sqrt(const v2df &a)
	{ v2df c; c.v = _mm_sqrt_pd(a.v); return c; }

	friend inline v2df
	operator+(double a, const v2df &b)
	{ v2df c; c.v = _mm_add_pd(_mm_set1_pd(a),b.v); return c; }

	friend inline v2df
	operator-(double a, const v2df &b)
	{ v2df c; c.v = _mm_sub_pd(_mm_set1_pd(a),b.v); return c; }

	friend inline v2df
	operator*(double a, const v2df &b)
	{ v2df c; c.v = _mm_mul_pd(_mm_set1_pd(a),b.v); return c; }

	friend inline v2df
	operator/(double a, const v2df &b)
	{ v2df c; c.v = _mm_div_pd(_mm_set1_pd(a),b.v); return c; }

	inline v2df &
	operator+=(const v2df &a)
	{ v = _mm_add_pd(v,a.v); return *this; }

	inline v2df &
	operator-=(const v2df &a)
	{ v = _mm_sub_pd(v,a.v); return *this; }

	inline v2df &
	operator*=(const v2df &a)
	{ v = _mm_mul_pd(v,a.v); return *this; }

	inline v2df &
	operator/=(const v2df &a)
	{ v = _mm_div_pd(v,a.v); return *this; }

	friend inline v2df
	operator&(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_and_pd(a.v,b.v); return c; }

	friend inline v2df
	operator|(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_or_pd(a.v,b.v); return c; }

	friend inline v2df
	operator^(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_xor_pd(a.v,b.v); return c; }

	friend inline v2df
	andnot(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_andnot_pd(a.v,b.v); return c; }

	friend inline v2df
	operator<(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_cmplt_pd(a.v,b.v); return c; }

	friend inline v2df
	operator>(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_cmpgt_pd(a.v,b.v); return c; }

	friend inline v2df
	operator==(const v2df &a, const v2df &b)
	{ v2df c; c.v = _mm_cmpeq_pd(a.v,b.v); return c; }

	friend inline v2df
	operator<(const v2df &a, double b)
	{ v2df c; c.v = _mm_cmplt_pd(a.v,_mm_set1_pd(b)); return c; }

	friend inline v2df
	operator>(const v2df &a, double b)
	{ v2df c; c.v = _mm_cmpgt_pd(a.v,_mm_set1_pd(b)); return c; }

	friend inline v2df
	max(const v2df &a, v2df &b)
	{ v2df c; c.v = _mm_max_pd(a.v,b.v); return c; }

	inline double &
	operator[](int index) {
		return ((double *)&v)[index];
	}

	inline const double &
	operator[](int index) const {
		return ((double *)&v)[index];
	}
};
