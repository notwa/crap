template<class Mixin>
struct Buffer4 : public Mixin {
	virtual void
	process2(v4sf *buf_L, v4sf *buf_R, ulong rem) = 0;

	TEMPLATE void
	_process(T *in_L, T *in_R, T *out_L, T *out_R, ulong count)
	{
		disable_denormals();

		assert(count % 2 == 0);

		v4sf buf_L[BLOCK_SIZE/2];
		v4sf buf_R[BLOCK_SIZE/2];

		for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
			ulong rem = BLOCK_SIZE;
			if (pos + BLOCK_SIZE > count)
				rem = count - pos;

			for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
				buf_L[j].v[0] = in_L[i+0];
				buf_L[j].v[1] = in_L[i+1];
				buf_R[j].v[0] = in_R[i+0];
				buf_R[j].v[1] = in_R[i+1];
			}

			process2(buf_L, buf_R, rem);

			for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
				out_L[i+0] = buf_L[j].v[0];
				out_L[i+1] = buf_L[j].v[1];
				out_R[i+0] = buf_R[j].v[0];
				out_R[i+1] = buf_R[j].v[1];
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
