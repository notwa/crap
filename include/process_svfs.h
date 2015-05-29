{
	disable_denormals();

	assert(count % 2 == 0);

	v4sf buf_L[BLOCK_SIZE/2];
	v4sf buf_R[BLOCK_SIZE/2];

	svf_matrix *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
			buf_L[j][0] = in_L[i+0];
			buf_L[j][1] = in_L[i+1];
			buf_R[j][0] = in_R[i+0];
			buf_R[j][1] = in_R[i+1];
		}

		f0 = data->filters[0];
		for (ulong i = 0; i < BANDS; i++) {
			svf_run_block_mat(f0, buf_L, rem);
			f0++;
		}
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			svf_run_block_mat(f1, buf_R, rem);
			f1++;
		}

		for (ulong i = 0, j = 0; i < rem; i += 2, j++) {
			out_L[i+0] = buf_L[j][0];
			out_L[i+1] = buf_L[j][1];
			out_R[i+0] = buf_R[j][0];
			out_R[i+1] = buf_R[j][1];
		}

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}
