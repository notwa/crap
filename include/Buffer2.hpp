template<class Mixin>
struct Buffer2 : public Mixin {
	virtual void
	process2(v2df *buf, ulong rem) = 0;

	TEMPLATE void
	_process(T *in_L, T *in_R, T *out_L, T *out_R, ulong count)
	{
		disable_denormals();

		v2df buf[BLOCK_SIZE];

		for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
			ulong rem = BLOCK_SIZE;
			if (pos + BLOCK_SIZE > count)
				rem = count - pos;

			for (ulong i = 0; i < rem; i++) {
				buf[i][0] = in_L[i];
				buf[i][1] = in_R[i];
			}

			process2(buf, rem);

			for (ulong i = 0; i < rem; i++) {
				out_L[i] = buf[i][0];
				out_R[i] = buf[i][1];
			}

			in_L += BLOCK_SIZE;
			in_R += BLOCK_SIZE;
			out_L += BLOCK_SIZE;
			out_R += BLOCK_SIZE;
		}
	}

	void
	process(
	    double *in_L, double *in_R,
	    double *out_L, double *out_R,
	    ulong count)
	{
		_process(in_L, in_R, out_L, out_R, count);
	}

	void
	process(
	    float *in_L, float *in_R,
	    float *out_L, float *out_R,
	    ulong count)
	{
		_process(in_L, in_R, out_L, out_R, count);
	}
};
