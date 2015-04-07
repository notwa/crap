{
	disable_denormals();

	v2df buf[BLOCK_SIZE];

	biquad *f0, *f1;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem; i++) {
			buf[i][0] = in_L[i];
			buf[i][1] = in_R[i];
		}

		f0 = data->filters[0];
		f1 = data->filters[1];
		for (ulong i = 0; i < BANDS; i++) {
			biquad_run_block_stereo(f0, f1, buf, rem);
			f0++;
			f1++;
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
