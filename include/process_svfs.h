{
	disable_denormals();

	float buf_L[BLOCK_SIZE];
	float buf_R[BLOCK_SIZE];

	svf *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem; i++) {
			buf_L[i] = in_L[i];
			buf_R[i] = in_R[i];
		}

		f0 = data->filters[0];
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			for (ulong j = 0; j < rem; j++)
				buf_L[j] = svf_run(f0, buf_L[j]);
			for (ulong j = 0; j < rem; j++)
				buf_R[j] = svf_run(f1, buf_R[j]);
			f0++;
			f1++;
		}

		for (ulong i = 0; i < rem; i++) {
			out_L[i] = buf_L[i];
			out_R[i] = buf_R[i];
		}

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}
