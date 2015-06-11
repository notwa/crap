/* via http://nbviewer.ipython.org/urls/music-synthesizer-for-android.googlecode.com/git/lab/Second%20order%20sections%20in%20matrix%20form.ipynb */

typedef struct {
	float A0[2], A1[2], B[2], C[3], memory[2];
} svf;

typedef struct {
	double A0[2], A1[2], B[2], C[3];
} svf_interim;

TEMPLATE
struct svf_matrix {
	T a, b, c, d, memory;
};

static svf_interim
svf_design(double w0, double Q, double c0, double c1, double c2, double gc)
{
	svf_interim svfi;
	double g = tan(0.5*w0)*gc;
	double a1 = 1/(1 + g*(g + 1/Q));
	double a2 = g*a1;
	double a3 = g*a2;
	svfi.A0[0] = 2*a1 - 1;
	svfi.A0[1] = -2*a2;
	svfi.A1[0] = 2*a2;
	svfi.A1[1] = 1 - 2*a3;
	svfi.B[0] = 2*a2;
	svfi.B[1] = 2*a3;
	double v0[3] = {1, 0, 0};
	double v1[3] = {a2, a1, -a2};
	double v2[3] = {a3, a2, 1 - a3};
	svfi.C[0] = v0[0]*c0 + v1[0]*c1 + v2[0]*c2;
	svfi.C[1] = v0[1]*c0 + v1[1]*c1 + v2[1]*c2;
	svfi.C[2] = v0[2]*c0 + v1[2]*c1 + v2[2]*c2;
	return svfi;
}

static svf
svf_gen_raw(filter_t type, double w0, double A, double Q)
{
	double As = sqrt(A);

	svf_interim svfi;

	#define d(Q,c0,c1,c2,gc) svfi = svf_design(w0,Q,c0,c1,c2,gc)
	switch (type) {
	case FILT_PEAKING:    d(Q*A, 1, (A*A - 1)/A/Q, 0, 1);        break;
	case FILT_LOWSHELF:   d(Q, 1,     (A - 1)/Q, A*A - 1, 1/As); break;
	case FILT_HIGHSHELF:  d(Q, A*A, (1 - A)*A/Q, 1 - A*A,   As); break;
	case FILT_LOWPASS:    d(Q, 0, 0, 1, 1);                      break;
	case FILT_HIGHPASS:   d(Q, 1, -1/Q, -1, 1);                  break;
	case FILT_ALLPASS:    d(Q, 0, 0, 0, 1);                      break; // TODO: implement
	case FILT_BANDPASS:   d(Q, 0, 1, 0, 1);                      break; // FIXME: no gain
	case FILT_BANDPASS_2: d(Q, 0, 0, 0, 1);                      break; // TODO: implement
	case FILT_NOTCH:      d(Q, 1, -1/Q, 0, 1);                   break;
	case FILT_GAIN:       d(Q, 0, 0, 0, 1);                      break; // TODO: implement
	}
	#undef d

	svf s;
	s.A0[0] = svfi.A0[0];
	s.A0[1] = svfi.A0[1];
	s.A1[0] = svfi.A1[0];
	s.A1[1] = svfi.A1[1];
	s.B[0] = svfi.B[0];
	s.B[1] = svfi.B[1];
	s.C[0] = svfi.C[0];
	s.C[1] = svfi.C[1];
	s.C[2] = svfi.C[2];
	s.memory[0] = 0;
	s.memory[1] = 0;
	return s;
}

static svf
svf_gen(filter_t type, double fc, double gain, double bw, double fs)
{
	double w0 = ANGULAR_LIM(fc, fs);
	double A = DB2LIN(gain/2);
	//double Q = M_SQRT1_2/bw;
	double Q = M_SQRT1_2*(1 - (w0/M_PI)*(w0/M_PI))/bw;
	return svf_gen_raw(type, w0, A, Q);
}

INNER float
svf_run(svf *s, float x)
{
	float y = s->C[0]*x + s->C[1]*s->memory[0] + s->C[2]*s->memory[1];
	float temp = s->memory[0];
	s->memory[0] = s->B[0]*x + s->A0[0]*temp + s->A0[1]*s->memory[1];
	s->memory[1] = s->B[1]*x + s->A1[0]*temp + s->A1[1]*s->memory[1];
	return y;
}

TEMPLATE static svf_matrix<T>
svf_gen_matrix(svf s)
{
	float AA0[2], AA1[2], AB[2], CA[2], cb;
	AA0[0] = s.A0[0]*s.A0[0] + s.A0[1]*s.A1[0];
	AA1[0] = s.A1[0]*s.A0[0] + s.A1[1]*s.A1[0];
	AA0[1] = s.A0[0]*s.A0[1] + s.A0[1]*s.A1[1];
	AA1[1] = s.A1[0]*s.A0[1] + s.A1[1]*s.A1[1];
	AB[0] = s.A0[0]*s.B[0] + s.A0[1]*s.B[1];
	AB[1] = s.A1[0]*s.B[0] + s.A1[1]*s.B[1];
	CA[0] = s.A0[0]*s.C[1] + s.A1[0]*s.C[2];
	CA[1] = s.A0[1]*s.C[1] + s.A1[1]*s.C[2];

	cb = s.C[1]*s.B[0] + s.C[2]*s.B[1];

	svf_matrix<T> mat;
	mat.memory = T(0, 0, 0, 0);
	mat.a = T(s.C[0],      0, s.C[1], s.C[2]);
	mat.b = T(    cb, s.C[0],  CA[0],  CA[1]);
	mat.c = T( AB[0], s.B[0], AA0[0], AA0[1]);
	mat.d = T( AB[1], s.B[1], AA1[0], AA1[1]);
	return mat;
}

TEMPLATE INNER void
svf_run_block_mat(svf_matrix<T> *RESTRICT mat, T *RESTRICT buf, ulong count)
{
	T t1, t2, t3, t4;
	T memory = mat->memory;
	for (ulong i = 0; i < count/2; i++) {
		memory[0] = buf[i][0];
		memory[1] = buf[i][1];
		t1 = mat->a*memory;
		t2 = mat->b*memory;
		t3 = mat->c*memory;
		t4 = mat->d*memory;
		memory[0] = t1[0] + t1[1] + t1[2] + t1[3];
		memory[1] = t2[0] + t2[1] + t2[2] + t2[3];
		memory[2] = t3[0] + t3[1] + t3[2] + t3[3];
		memory[3] = t4[0] + t4[1] + t4[2] + t4[3];
		buf[i][0] = memory[0];
		buf[i][1] = memory[1];
	}
	mat->memory = memory;
}
