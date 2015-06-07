struct Crap {
	virtual inline
	~Crap() {}

	virtual void
	pause() = 0;

	virtual void
	resume() = 0;

	virtual void
	process(
	    double *in_L, double *in_R,
	    double *out_L, double *out_R,
	    ulong count) = 0;

	virtual void
	process(
	    float *in_L, float *in_R,
	    float *out_L, float *out_R,
	    ulong count) = 0;

	//void
	//construct_params(param *params) = 0;

	virtual void
	adjust(param *params, ulong fs) = 0;

	virtual void
	adjust_one(param *params, int i) = 0;
};

template<class Mixin>
struct AdjustAll : public virtual Mixin {
	ulong fs;

	virtual void
	adjust_all(param *params) = 0;

	inline void
	adjust(param *params, ulong fs_new)
	{
		fs = fs_new;
		adjust_all(params);
	}

	inline void
	adjust_one(param *params, int i)
	{
		adjust_all(params);
	}
};

template<class Mixin>
struct NoParams : public virtual Mixin {
	// etc

	//void
	//construct_params(param *params) = 0;

	void
	adjust(param *params, ulong fs)
	{}

	void
	adjust_one(param *params, int i)
	{}
};
