/*
Copyright (c) 2017, StarBoard Design
Contributed by Fred Huettig <Fred@StarBoardDesign.com>
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

  gpiotest_irq.c

  Basic (i.e. incomplete!) test of the para_gpio IRQ system.

  Build:
  gcc -o gpiotest_irq gpiotest_irq.c para_gpio.c -lrt -Wall
*/


#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "para_gpio.h"

void Usage() {

  printf("\nUsage:  gpiotest -h  (show this help)\n");
  printf("        gpiotest [-g G]\n");
  printf("    -g G - Use GPIO pin G (default 7)\n");
  printf("\n");
  printf("Note: This application needs (probably root) access to /sys/class/gpio\n");
  printf("\n");

}

int main(int argc, char *argv[]) {
  int	n, i, c, rc;
  int	gpio = 7;
  para_gpio  *pGpio;
  struct timespec  tsStart, tsEnd;

  printf("GPIOTEST_IRQ - Basic test of para_gpio\n\n");

  while ((c = getopt (argc, argv, "hg:")) != -1) {
    switch (c) {

    case 'h':
      Usage();
      exit(0);

    case 'g':
      gpio = atoi(optarg);
      if(gpio < 0) {
        fprintf(stderr, "GPIO # must be > 0, exiting\n");
        exit(1);
      }
      break;

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

  printf("Initializing...\n");
  if((rc = para_initgpio(&pGpio, gpio)) != para_ok) {
    fprintf(stderr, "para_initgpio() failed with code %d, exiting\n", rc);
    exit(1);
  }
  printf("  Success\n");
  sleep(1);

  printf("Setting direction to 'input'.\n");
  if((rc = para_dirgpio(pGpio, para_dirin)) != para_ok) {
    fprintf(stderr, "para_dirpgio() failed with code %d, exiting\n", rc);
    exit(1);
  }

  printf("  Success\n");
  sleep(1);

  if((rc = para_getgpio(pGpio, &n)) != para_ok) {
    printf("ERROR %d from para_getgpio()\n", rc);
    exit(1);
  }
  clock_gettime(CLOCK_REALTIME, &tsStart);
  
  printf("Explicitly polling for changes, starting at %d...\n", n);
  do {
    if((rc = para_getgpio(pGpio, &c)) != para_ok) {
      printf("ERROR %d from para_getgpio()\n", rc);
      break;
    } else if(c != n) {
      printf("%d\n", c);
      n = c;
      clock_gettime(CLOCK_REALTIME, &tsStart);
    }
    clock_gettime(CLOCK_REALTIME, &tsEnd);
  } while((tsEnd.tv_sec - tsStart.tv_sec) < 10.0);

  printf("Efficiently waiting for rising edges...\n");
  for(i=1; i<=5; i++) {
    rc = para_waitedge(pGpio, para_edgerise, 10);
    if(rc == para_ok) {
      printf("Rise %d!\n", i);
    } else if(rc == para_timeout) {
      printf("Timeout!\n");
      break;
    } else {
      printf("ERROR %d from para_waitedge()\n", rc);
      break;
    } 
  }

  printf("Closing\n");
  para_closegpio_ex(pGpio, false);  // only un-export if we exported

  return 0;
}
