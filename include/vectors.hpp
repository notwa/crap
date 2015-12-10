typedef Eigen::Vector2d _v2df;
typedef Eigen::Vector2f _v2sf;
typedef Eigen::Vector4f _v4sf;

template<typename T, typename Base>
struct Vector : public Aligned {
	T v;

	Vector()
	{}

	Vector(const Base &d)
	{ v.setConstant(d); }

	Vector(const Base &d0, const Base &d1)
	{ v[0] = d0; v[1] = d1; }

	Vector(const Base &d0, const Base &d1, const Base &d2, const Base &d3)
	{ v[0] = d0; v[1] = d1; v[2] = d2; v[3] = d3; }

	template<typename Derived>
	Vector(const Eigen::ArrayBase<Derived> &v2)
	//{ v = v2.cast<Base>(); } // FIXME
	{ v = v2; }

	template<typename Derived>
	Vector(const Eigen::MatrixBase<Derived> &v2)
	{ v = v2; }

	friend inline Vector
	operator-(const Vector &a)
	{ return -a.v.array(); }

	friend inline Vector
	operator+(const Vector &a, const Vector &b)
	{ return a.v.array() + b.v.array(); }

	friend inline Vector
	operator-(const Vector &a, const Vector &b)
	{ return a.v.array() - b.v.array(); }

	friend inline Vector
	operator*(const Vector &a, const Vector &b)
	{ return a.v.array() * b.v.array(); }

	friend inline Vector
	operator/(const Vector &a, const Vector &b)
	{ return a.v.array() / b.v.array(); }

	friend inline Vector
	sqrt(const Vector &a)
	{ return a.v.cwiseSqrt(); }

	inline Vector &
	operator+=(const Vector &a)
	{ v.array() += a.v.array(); return *this; }

	inline Vector &
	operator-=(const Vector &a)
	{ v.array() -= a.v.array(); return *this; }

	inline Vector &
	operator*=(const Vector &a)
	{ v.array() *= a.v.array(); return *this; }

	inline Vector &
	operator/=(const Vector &a)
	{ v.array() /= a.v.array(); return *this; }

	/* unimplemented for now
	friend inline Vector
	operator&(const Vector &a, const Vector &b)
	{}

	friend inline Vector
	operator|(const Vector &a, const Vector &b)
	{}

	friend inline Vector
	operator^(const Vector &a, const Vector &b)
	{}

	friend inline Vector
	andnot(const Vector &a, const Vector &b)
	{}
	*/

	/*
	friend inline Vector
	operator<(const Vector &a, const Vector &b)
	{ return a.v.array() < b.v.array(); }

	friend inline Vector
	operator>(const Vector &a, const Vector &b)
	{ return a.v.array() > b.v.array(); }

	friend inline Vector
	operator==(const Vector &a, const Vector &b)
	{ return a.v.array() == b.v.array(); }
	*/

	friend inline Vector
	max(const Vector &a, const Vector &b)
	{ return a.v.cwiseMax(b.v); }

	inline Base &
	operator[](int index) {
		return v[index];
	}

	inline const Base &
	operator[](int index) const {
		return v[index];
	}
};

typedef Vector<_v2df,double> v2df;
typedef Vector<_v2sf,float> v2sf;
typedef Vector<_v4sf,float> v4sf;
