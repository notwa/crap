/* IIR filter
     Fp = 20000.62
     os = 6; N = 8*2; Gp = 0.0135; Gs = 120;
   overall delay: ~5 samples
*/

#define BQSHIFT(i) \
	h[i*2 + 1] = h[i*2 + 0]; \
	h[i*2 + 0] = x;

// b0 and b2 are equivalent and factored out
// b0 and a0 are factored out as overall gain
#define LOWPASS(i, b1, a1, a2) \
	y = x + b1*h[i*2 + 0] +    h[i*2 + 1] \
	      - a1*h[i*2 + 2] - a2*h[i*2 + 3]; \
	BQSHIFT(i); \
	x = y;

static double
oversample(double *h, double x)
{
	// h[] should be at least 18 doubles long
	double y;
	LOWPASS(0,-1.7310964991540,-1.7686201550064,+0.9924894080401);
	LOWPASS(1,-1.7228987922703,-1.7591070571963,+0.9756664940293);
	LOWPASS(2,-1.7030928294094,-1.7508784553758,+0.9530819894740);
	LOWPASS(3,-1.6623628189757,-1.7420481292930,+0.9206849371458);
	LOWPASS(4,-1.5767803723821,-1.7314029454409,+0.8755898544307);
	LOWPASS(5,-1.3754954625889,-1.7189289035735,+0.8193999703446);
	LOWPASS(6,-0.7934216412437,-1.7066722500921,+0.7627918423024);
	LOWPASS(7,+1.1176227011686,-1.6987049565355,+0.7256338727337);
	BQSHIFT(8);
	return y*0.000005162322938;
}

#undef BQSHIFT
#undef LOWPASS
