#define _USE_MATH_DEFINES 
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// PROTOTYPES
//float SQR(float x);
//float Ranf(unsigned int* seedp, float low, float high);
//int Ranf(unsigned int* seedp, int ilow, int ihigh);
void InitBarrier(int);
void WaitBarrier();
void Deer();
void Grain();
void Watcher();
void Weed();


//The "state" of the system consists of the following global variables

int	NowYear;		// 2022 - 2027
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
float NowWeedHeight;	// week height in inches
int	NowNumDeer;		// number of deer in the current population
float NowWeed;       // weed height in inches

const float GRAIN_GROWS_PER_MONTH = 9.0;
const float ONE_DEER_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 7.0;	// average
const float AMP_PRECIP_PER_MONTH = 6.0;	// plus or minus
const float RANDOM_PRECIP = 2.0;	// plus or minus noise

const float AVG_TEMP = 60.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise

const float WEED_GROWS_PER_MONTH = 0.8;

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

unsigned int seed = 0;

//for barriers:
omp_lock_t	Lock;
volatile int	NumInThreadTeam;
volatile int	NumAtBarrier;
volatile int	NumGone;


int main(int argc, char* argv[])
{
#ifndef _OPENMP
	fprintf(stderr, "No OpenMP support!\n");
	return 1;
#endif
	// starting date and time:
	NowMonth = 0;
	NowYear = 2022;

	// starting state (feel free to change this if you want):
	NowNumDeer = 1;
	NowHeight = 1.;
	NowHeight = 0.1;

	InitBarrier(4);

	omp_set_num_threads(4);	// same as # of sections


#pragma omp parallel sections
	{
#pragma omp section
		{
			Deer();
		}

#pragma omp section
		{
			Grain();
		}

#pragma omp section
		{
			Watcher();
		}

#pragma omp section
		{
			Weed();	// your own
		}
	}       // implied barrier -- all functions must return in order
		// to allow any of them to get past here
}

// specify how many threads will be in the barrier:
//	(also init's the Lock)

void
InitBarrier(int n)
{
	NumInThreadTeam = n;
	NumAtBarrier = 0;
	omp_init_lock(&Lock);
}

// have the calling thread wait here until all the other threads catch up:

void
WaitBarrier()
{
	omp_set_lock(&Lock);
	{
		NumAtBarrier++;
		if (NumAtBarrier == NumInThreadTeam)
		{
			NumGone = 0;
			NumAtBarrier = 0;
			// let all other threads get back to what they were doing
// before this one unlocks, knowing that they might immediately
// call WaitBarrier( ) again:
			while (NumGone != NumInThreadTeam - 1);
			omp_unset_lock(&Lock);
			return;
		}
	}
	omp_unset_lock(&Lock);

	while (NumAtBarrier != 0);	// this waits for the nth thread to arrive

#pragma omp atomic
	NumGone++;			// this flags how many threads have returned
}

float
SQR(float x)
{
	return x * x;
}

float
Ranf(unsigned int* seedp, float low, float high)
{
	//srand(seedp)
	float r = (float)rand();              // 0 - RAND_MAX

	return(low + r * (high - low) / (float)RAND_MAX);
}


int
Ranf(unsigned int* seedp, int ilow, int ihigh)
{
	float low = (float)ilow;
	float high = (float)ihigh + 0.9999f;

	return (int)(Ranf(seedp, low, high));
}

void Deer() {
	while (NowYear <= 2027) {
		//Compute the next amount of deer based on the values from current state
		int nextNumDeer = NowNumDeer;
		int carryingCapacity = (int)(NowHeight);
		if (nextNumDeer < carryingCapacity)
			nextNumDeer++;
		else
			if (nextNumDeer > carryingCapacity)
				nextNumDeer--;

		if (nextNumDeer < 0)
			nextNumDeer = 0;


		// DoneComputing barrier:
		WaitBarrier();
		NowNumDeer = nextNumDeer;


		// DoneAssigning barrier:
		WaitBarrier();


		// DonePrinting barrier:
		WaitBarrier();

	}
}

void Grain() {
	while (NowYear <= 2027) {
		//Compute the next height of grain based on the values from current state
		float nextHeight = NowHeight;

		float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
		float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));

		nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
		nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
		//Weed prohabits the growth of grain.
		nextHeight -= 0.5 * NowWeedHeight;

		//Be sure to clamp nextHeight against zero, that is :
		if (nextHeight < 0.) nextHeight = 0.;


		// DoneComputing barrier:
		WaitBarrier();
		NowHeight = nextHeight;


		// DoneAssigning barrier:
		WaitBarrier();


		// DonePrinting barrier:
		WaitBarrier();
	}
}

void Weed() {
	while (NowYear <= 2027) {
		//Compute the next height of grain based on the values from current state
		float nextWeedHeight = NowWeedHeight;

		float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
		float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));

		nextWeedHeight += tempFactor * precipFactor * WEED_GROWS_PER_MONTH;
		
		nextWeedHeight = nextWeedHeight - 0.01 * NowHeight;

		//Be sure to clamp nextHeight against zero, that is :
		if (nextWeedHeight < 0.) nextWeedHeight = 0.;


		// DoneComputing barrier:
		WaitBarrier();
		NowWeedHeight = nextWeedHeight;


		// DoneAssigning barrier:
		WaitBarrier();


		// DonePrinting barrier:
		WaitBarrier();
	}
}

void Watcher() {
	NowYear = 2022;
	while (NowYear <= 2027) {
		NowMonth = 0;
		while (NowMonth < 12) {

			// DoneComputing barrier:
			WaitBarrier();


			// DoneAssigning barrier:
			WaitBarrier();

			//printf("%d/%d Temp: %f Precip: %f NumOfDeer: %d HeightOfGrain: %f Weed: %f\n", NowMonth, NowYear, (NowTemp - 32)* (5. / 9.), NowPrecip * 2.54, NowNumDeer, NowHeight * 2.54, NowWeedHeight * 2.54);
			printf("%d/%d, %f, %f, %d , %f ,%f\n", NowMonth, NowYear, (NowTemp - 32) * (5. / 9.), NowPrecip * 2.54, NowNumDeer, NowHeight * 2.54, NowWeedHeight * 2.54);
			
			//Computer and set the new state parametaers
			float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

			float temp = AVG_TEMP - AMP_TEMP * cos(ang);
			NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

			float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
			NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
			if (NowPrecip < 0.)
				NowPrecip = 0.;

			// DonePrinting barrier:
			WaitBarrier();
			NowMonth++;
		}
		NowYear++;
	}
	
}

