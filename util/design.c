#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include "util.h"

void
usage()
{
	fputs("usage: design SAMPLERATE TYPE CENTERFREQ GAIN BANDWIDTH\n", stderr);
	exit(1);
}

int
main(int argc, char **argv)
{
	if (argc != 6) usage();
	double tv[5];
	for (int i = 1; i <= 5; i++) {
		tv[i - 1]=strtold(argv[i], NULL);
		if (errno) {
			fprintf(stderr, "arg #%i failed to convert to double\n", i);
			exit(1);
		}
	}
	biquad bq = biquad_gen((int) tv[1], tv[2], tv[3], tv[4], tv[0]);
	printf("b0 %+.18f\nb1 %+.18f\nb2 %+.18f\na1 %+.18f\na2 %+.18f\n",bq.b0,bq.b1,bq.b2,bq.a1,bq.a2);
	return 0;
}
