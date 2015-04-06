// provides a generic wrapper around process_double

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 256
#endif

static void
process(personal *data,
    float *in_L, float *in_R,
    float *out_L, float *out_R,
    ulong count)
{
	double  in_L2[BLOCK_SIZE],  in_R2[BLOCK_SIZE];
	double out_L2[BLOCK_SIZE], out_R2[BLOCK_SIZE];

	for (ulong pos = 0; pos < count; pos += BLOCK_SIZE) {
		ulong rem = BLOCK_SIZE;
		if (pos + BLOCK_SIZE > count)
			rem = count - pos;

		for (ulong i = 0; i < rem; i++)
			in_L2[i] = in_L[i];
		for (ulong i = 0; i < rem; i++)
			in_R2[i] = in_R[i];

		process_double(data, in_L2, in_R2, out_L2, out_R2, rem);

		for (ulong i = 0; i < rem; i++)
			out_L[i] = out_L2[i];
		for (ulong i = 0; i < rem; i++)
			out_R[i] = out_R2[i];

		in_L += BLOCK_SIZE;
		in_R += BLOCK_SIZE;
		out_L += BLOCK_SIZE;
		out_R += BLOCK_SIZE;
	}
}
