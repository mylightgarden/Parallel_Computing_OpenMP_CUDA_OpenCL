// The main Program
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.
#define R		1.2
#define N		2.5

// print debugging messages?
#ifndef DEBUG
#define DEBUG	false
#endif

// setting the number of threads:
//#ifndef NUMT
//#define NUMT		    2
//#endif
int NUMT = 4;

// setting the number of trials in the monte carlo simulation:
//#ifndef NUMNODES
//#define NUMNODES	200, 500, 1000, 2000, 4000,  8000,  12000,  16000, 20000
//#endif
int NUMNODES = 100;


// how many tries to discover the maximum performance:
#ifndef NUMTIMES
#define NUMTIMES	20
#endif

float volume = 0;
double  maxPerformance = 0.;

float Height(int iu, int iv)
{
	float x = -1. + 2. * (float)iu / (float)(NUMNODES - 1);	// -1. to +1.
	float y = -1. + 2. * (float)iv / (float)(NUMNODES - 1);	// -1. to +1.

	float xn = pow(fabs(x), (double)N);
	float yn = pow(fabs(y), (double)N);
	float rn = pow(fabs(R), (double)N);
	float r = rn - xn - yn;
	if (r <= 0.)
		return 0.;
	float height = pow(r, 1. / (double)N);
	return height;
}

int main(int argc, char* argv[])
{		
	if (argc >= 2)
		NUMT = atoi(argv[1]);
	if (argc >= 3)
		NUMNODES = atoi(argv[2]);
	//int NUMTRIALS = NUMNODES * NUMNODES;

	#ifndef _OPENMP
	fprintf(stderr, "No OpenMP support!\n");
	return 1;
	#endif

	omp_set_num_threads(NUMT);    // set the number of threads to use in parallelizing the for-loop:`
	
	// the area of a single full-sized tile:
	// (not all tiles are full-sized, though)

	float fullTileArea = (((XMAX - XMIN) / (float)(NUMNODES - 1)) *
		((YMAX - YMIN) / (float)(NUMNODES - 1)));

	// sum up the weighted heights into the variable "volume"
	// using an OpenMP for-loop and a reduction:
	
	// looking for the maximum performance:
	for (int times = 0; times < NUMTIMES; times++)
	{
		//get start time
		double time0 = omp_get_wtime();
		volume = 0;

		#pragma omp parallel for collapse(2) default(none) shared(NUMNODES, fullTileArea)  reduction(+: volume)
		for (int iv = 0; iv < NUMNODES; iv++)
		{
			for (int iu = 0; iu < NUMNODES; iu++)
			{
				float z = Height(iu, iv);
				//corner cases:
				if ((iv == 0 && iu == 0) || (iv == 0 && iu == NUMNODES - 1) || (iv == NUMNODES - 1 && iu == 0) || (iv == NUMNODES - 1 && iu == NUMNODES - 1)) {
					float area = fullTileArea * 0.25;
					volume += area*z;
				}
				//edge cases:
				else if (iv == 0 || iu == 0 || iv == NUMNODES - 1 || iu == NUMNODES - 1) {
					float area = fullTileArea * 0.5;
					volume += area*z;
				}
				//center cases
				else {
					volume += fullTileArea*z;
				}
			}
		}
		//get finish time
		double time1 = omp_get_wtime();
		double megaComputePerSecond = (double)(NUMNODES* NUMNODES)/ (time1 - time0) / 1000000.;
		if (megaComputePerSecond > maxPerformance)
			maxPerformance = megaComputePerSecond;
	}

	//fianl volume = volue/20 * 2
	printf("%6.2lf\n", volume*2);
	printf("%2d, %8d, %6.2lf\n", NUMT, NUMNODES, maxPerformance);
}