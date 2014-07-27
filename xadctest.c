/*
Copyright (c) 2014, Adapteva, Inc.
Contributed by Fred Huettig <Fred@Adapteva.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*

  xadctest.c

  Basic test of the para_xadc system.  This code was used 
    for debug during development of para_xadc.

  Build:
  gcc -o xadctest xadctest.c para_xadc.c -Wall -lrt

*/


#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "para_xadc.h"

#ifndef countof
  #define countof(x)  (sizeof(x)/sizeof(x[0]))
#endif

void Usage() {

  printf("\nUsage:  xadctest -h  (show this help)\n");
  printf("        xadctest\n");
  printf("\n");
  printf("\n");

}

int main(int argc, char *argv[]) {
  int	n, i, c, rc, nDeltas;
  struct timespec  tsStart, tsEnd;
  float  fVal, fArray[100], fLast;
  double  dTimeElapsed;
  const char **aNameList;

  printf("XADCTEST - Basic test of para_xadc\n\n");

  while ((c = getopt (argc, argv, "h")) != -1) {
    switch (c) {

    case 'h':
      Usage();
      exit(0);

    case '?':
      if (optopt == 'w')
	fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint (optopt))
	fprintf (stderr, "Unknown option `-%c'.\n", optopt);
      else
	fprintf (stderr,
		 "Unknown option character `\\x%x'.\n",
		 optopt);
      exit(1);

    default:
      fprintf(stderr, "Unexpected result from getopt?? (%d:%c)\n", c, c);
      exit(1);
    }
  }

  printf("Getting Info...\n");

  if((rc = para_infoxadc(&n, &aNameList)) != parax_ok) {
    printf("ERROR %d from para_infoxadc()\n", rc);
    exit(2);
  }

  printf("Found %d channels:\n", n);

  for(i=0; i<n; i++) {

    if((rc = para_getxadc(i, &fVal)) != parax_ok) {
      printf("ERROR %d from para_getxadc()\n", rc);
      exit(3);
    }

    printf("  %2d: %s = %.3f\n", i, aNameList[i], fVal);

  }

  rc = para_getidxadc("BRAM", &n);
  if(rc)
    printf("\nError %d looking up 'BRAM'!\n\n", rc);
  else
    printf("\nLook-up 'BRAM': ID=%d\n", n);

  c = 1;
  n = countof(fArray);
  printf("\nReading channel %d %d times...\n", c, n);

  for(i=0; i<n; i++) {
    if((rc = para_getxadc(c, fArray+i)) != parax_ok) {
      printf("ERROR %d from para_getxadc()\n", rc);
      break;
    }
  }

  for(i=0; i<n; i++)
    printf("%8.4f", fArray[i]);

  printf("\n\n");

  c = 3;
  n = 20000;
  nDeltas = 0;
  fLast = 0.0;
  printf("\nReading channel %d %d times...\n", c, n);

  clock_gettime(CLOCK_REALTIME, &tsStart);
  for(i=0; i<n; i++) {
    if((rc = para_getxadc(c, &fVal)) != parax_ok) {
      printf("ERROR %d from para_getxadc()\n", rc);
      break;
    }

    if(fVal != fLast) {
      nDeltas++;
      fLast = fVal;
    }
  }
  clock_gettime(CLOCK_REALTIME, &tsEnd);

  if(rc == parax_ok) {
    dTimeElapsed = (double)(tsEnd.tv_sec - tsStart.tv_sec);
    dTimeElapsed += (double)(tsEnd.tv_nsec - tsStart.tv_nsec) / 1.0e9;
    printf("Took %.3lf seconds, %.0lf updates/sec\n", dTimeElapsed, n / dTimeElapsed);
    printf("Found %d unique values\n", nDeltas);
  }

  return 0;
}
