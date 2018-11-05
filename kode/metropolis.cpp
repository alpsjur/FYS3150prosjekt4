//#include "mpi.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <armadillo>

#include "metropolis.hpp"

void solveGivenT(int L, int mcs, double T, double k, double J, double *values, long &idum, bool ordered){
  double beta = 1/(k*T);
  double w[17];
  imat spinMatrix;
  double nSpins = L*L;
  //regner ut mulige w-verdier
  for (int i=0; i<5; ++i){
    w[i*4] = exp(-beta*J*(i*4-8));
  }
  double E = 0; double M = 0;
  initialize(L, spinMatrix, E, M, J, ordered);
  //går gjennom gitt antall monte carlo sykluser (mcs)
  for (int i = 0; i < mcs; ++i){
    metropolis(L, spinMatrix, idum, E, M, w, J);
    values[0] += E;
    values[1] += E*E;
    values[2] += fabs(M);
    values[3] += M*M;
    values[4] += M;
  }
  for (int i = 0; i <= 4; ++i){
    values[i] /= (double) mcs;    //beregner gjennomsnittet av alle mc-syklusene
  }
  values[1] = (values[1]-values[0]*values[0])/(k*T*T*nSpins);      //beregner varmekapasiteten
  values[3] = (values[3]-values[2]*values[2])/(k*T*nSpins);        //beregner susceptibilit
  values[0] /= nSpins; values[2] /= nSpins; values[4] /= nSpins;   //forventning per spinn
}
void metropolis(int L, imat &spinMatrix, long &idum, double &E, double &M,double *w, double J){
  //itererer over alle spinnene
  int deltaE;
  int nSpins = L*L;
  for (int i=0; i < nSpins; ++i){
    //velger et tilfeldig spinn
    int k = (int) (ran2(&idum)*(double)L);
    int l = (int) (ran2(&idum)*(double)L);
    //beregner endring i energi
    deltaE = (spinMatrix(k,periodic(l+1,L))+
              spinMatrix(k,periodic(l-1,L))+
              spinMatrix(periodic(k+1,L),l)+
              spinMatrix(periodic(k-1,L),l))*
              spinMatrix(k,l)*2;
    //Utfører metropolis-testen
    if (ran2(&idum) <= w[deltaE+8]){
      spinMatrix(k,l) *= -1;
      E += (double) deltaE*J;
      M += (double) 2*spinMatrix(k,l);
    }
  }
  return;
}

void initialize(int L, imat &spinMatrix, double &E, double &M, double J, bool ordered){
  //initsialiserer total magnetisering
  M = L*L;

  if (ordered){
    //lager uniform initsialtilstand
    spinMatrix.ones(L,L);
  }
  else{
    //lager tilfeldig initsialtilstan
    spinMatrix = randi<imat>(L, L, distr_param(0, 1));   //setter matriseelementene til 0 eller 1
    for (int k = 0; k < L; ++k){
      for (int l = 0; l< L; ++l){
        if (spinMatrix(k,l) == 0){                       //elementene som er 0 endres til -1
          spinMatrix(k,l) = -1;
          M -= 2;                                        //oppdaterer magnetiseringen
        }
      }
    }
  }
  E = calculateTotalEnergy(L, spinMatrix, J);
  return;
}

double calculateTotalEnergy(int L, imat spinMatrix, double J){
  //kalkulerer total energi for hele systemet ved å iterere over alle elementene
  double E = 0;
  int nSpins = L*L;
  for (int k=0; k < L; ++k){
    for (int l=0; l < L; ++l){
      E -= (double)(spinMatrix(k,periodic(l-1,L))+
            spinMatrix(periodic(k-1,L),l))*
            spinMatrix(k,l)*J;
    }
  }
  return E;
}


// tilfeldig-tall-generator kopiert fra Morten sin kode
/*
** The function
**         ran2()
** is a long periode (> 2 x 10^18) random number generator of
** L'Ecuyer and Bays-Durham shuffle and added safeguards.
** Call with idum a negative integer to initialize; thereafter,
** do not alter idum between sucessive deviates in a
** sequence. RNMX should approximate the largest floating point value
** that is less than 1.
** The function returns a uniform deviate between 0.0 and 1.0
** (exclusive of end-point values).
*/

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

double ran2(long *idum)
{
  int            j;
  long           k;
  static long    idum2 = 123456789;
  static long    iy=0;
  static long    iv[NTAB];
  double         temp;

  if(*idum <= 0) {
    if(-(*idum) < 1) *idum = 1;
    else             *idum = -(*idum);
    idum2 = (*idum);
    for(j = NTAB + 7; j >= 0; j--) {
      k     = (*idum)/IQ1;
      *idum = IA1*(*idum - k*IQ1) - k*IR1;
      if(*idum < 0) *idum +=  IM1;
      if(j < NTAB)  iv[j]  = *idum;
    }
    iy=iv[0];
  }
  k     = (*idum)/IQ1;
  *idum = IA1*(*idum - k*IQ1) - k*IR1;
  if(*idum < 0) *idum += IM1;
  k     = idum2/IQ2;
  idum2 = IA2*(idum2 - k*IQ2) - k*IR2;
  if(idum2 < 0) idum2 += IM2;
  j     = iy/NDIV;
  iy    = iv[j] - idum2;
  iv[j] = *idum;
  if(iy < 1) iy += IMM1;
  if((temp = AM*iy) > RNMX) return RNMX;
  else return temp;
}
#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX

// End: function ran2()
