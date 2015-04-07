{
	disable_denormals();

	v2df drives[FULL_SIZE], wets[FULL_SIZE];
	v2df buf[BLOCK_SIZE];
	v2df over[FULL_SIZE];

	halfband_t *hb_up   = &data->hb_up;
	halfband_t *hb_down = &data->hb_down;

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		ulong rem2 = rem*OVERSAMPLING;

		for (ulong i = 0; i < rem2; i++) {
			double y = smooth(&data->drive);
			drives[i] = V(y);
		}
		for (ulong i = 0; i < rem2; i++) {
			double y = smooth(&data->wet);
			wets[i] = V(y);
		}

		for (ulong i = 0; i < rem; i++) {
			buf[i][0] = in_L[i];
			buf[i][1] = in_R[i];
		}

		for (ulong i = 0; i < rem; i++) {
			over[i*2+0] = interpolate_a(hb_up, buf[i]);
			over[i*2+1] = interpolate_b(hb_up, buf[i]);
		}

		for (ulong i = 0; i < rem2; i++) {
			over[i] = process_one(over[i], drives[i], wets[i]);
		}

		for (ulong i = 0; i < rem; i++) {
			         decimate_a(hb_down, over[i*2+0]);
			buf[i] = decimate_b(hb_down, over[i*2+1]);
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
