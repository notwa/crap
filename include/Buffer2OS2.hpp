template<class Mixin>
struct Buffer2OS2 : public virtual Mixin {
	virtual inline void
	process2(v2df *buf, ulong rem) = 0;

	halfband_t<v2df> hb_up, hb_down;

	TEMPLATE void
	_process(T *in_L, T *in_R, T *out_L, T *out_R, ulong count)
	{
		disable_denormals();
		v2df buf[BLOCK_SIZE];
		v2df over[FULL_SIZE];

		for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
			ulong rem = BLOCK_SIZE;
			if (pos + BLOCK_SIZE > count)
				rem = count - pos;

			ulong rem2 = rem*OVERSAMPLING;

			for (ulong i = 0; i < rem; i++) {
				buf[i][0] = in_L[i];
				buf[i][1] = in_R[i];
			}

			for (ulong i = 0; i < rem; i++) {
				over[i*2+0] = interpolate_a(&hb_up, buf[i]);
				over[i*2+1] = interpolate_b(&hb_up, buf[i]);
			}

			process2(over, rem2);

			for (ulong i = 0; i < rem; i++) {
					 decimate_a(&hb_down, over[i*2+0]);
				buf[i] = decimate_b(&hb_down, over[i*2+1]);
			}

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
