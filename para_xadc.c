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

  para_xadc.c

  Parallella library to read temperature & other XADC info from Zynq.
  See header file for info & function entries.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glob.h>
#include "para_xadc.h"

  // This is supposed to be in string.h, no?
  char *strcasestr(const char *haystack, const char *needle);

const char *strBase = "/sys/bus/iio/devices/iio:device0/in_";

int      nChannels = -1;
int      nOffsets[PARAX_MAXCHANS];
float    fScales[PARAX_MAXCHANS];
char     *pstrNames[PARAX_MAXCHANS];

static int para_initxadc() {
  glob_t  stGlob;
  char  strRead[PARAX_MAXSTRLEN], strName[PARAX_MAXSTRLEN], *strTemp;
  FILE *sysfile;
  int     rc, n, l;

  nChannels = 0;
  memset(nOffsets, 0, sizeof(nOffsets));
  memset(fScales, 0, sizeof(fScales));
  memset(pstrNames, 0, sizeof(pstrNames));

  strcpy(strName, strBase);
  strcat(strName, "*");

  rc = glob(strName, 0, NULL, &stGlob);
  if(rc == GLOB_NOMATCH) {
    fprintf(stderr, "WARNING: No XADC objects found!\n");
    globfree(&stGlob);
    return parax_ok;
  } else if(rc) {
    fprintf(stderr, "ERROR: glob() returned %d\n", rc);
    globfree(&stGlob);
    return parax_fileerr;
  }

  for(n=0; n<stGlob.gl_pathc; n++) {

    if(!strcmp("_raw", stGlob.gl_pathv[n] + strlen(stGlob.gl_pathv[n]) - 4)) {

      if(nChannels == PARAX_MAXCHANS) {
	fprintf(stderr, "WARNING: Max. number of channels exceeded!\n");
	break;
      }

      strTemp = rindex(stGlob.gl_pathv[n], '/');
      if(strTemp == NULL  || (l = strlen(strTemp)) < 5) {
	fprintf(stderr, "ERROR: Internal error in para_initxadc() [%p: %s]\n",
		strTemp, stGlob.gl_pathv[n]);
	globfree(&stGlob);
	nChannels = 0;
	return parax_fileerr;
      }

      strTemp += 4;  // /in_xxx_raw, cut off "/in_"

      pstrNames[nChannels] = malloc(l);  // will never be free'd ?
      if(pstrNames[nChannels] == NULL) {
	fprintf(stderr, "ERROR: Out of memory\n");
	globfree(&stGlob);
	return parax_outofmemory;
      }

      strcpy(pstrNames[nChannels], strTemp);
      pstrNames[nChannels][l - 8] = 0;  // cut off "_raw"

      nChannels++;
    }
  }

  globfree(&stGlob);

  for(n=0; n<nChannels; n++) {

    strcpy(strName, strBase);
    strcat(strName, pstrNames[n]);
    strcat(strName, "_offset");

    if((sysfile = fopen(strName, "ra")) != NULL) {

      fgets(strRead, PARAX_MAXSTRLEN-1, sysfile);
      fclose(sysfile);
      nOffsets[n] = atoi(strRead);

    } else {

      nOffsets[n] = 0;  // Not all entries have offsets

    }

    strcpy(strName, strBase);
    strcat(strName, pstrNames[n]);
    strcat(strName, "_scale");
      
    if((sysfile = fopen(strName, "ra")) == NULL) {
      fprintf(stderr, "ERROR: Can't open scale file for %s\n",
	      pstrNames[n]);
      return parax_fileerr;
    }

    fgets(strRead, PARAX_MAXSTRLEN-1, sysfile);
    fclose(sysfile);
    fScales[n] = atof(strRead);

//    printf("%d>> %s (%d, %.4f)\n", n, pstrNames[n], nOffsets[n], fScales[n]);

  }

  return 0;
}

int   para_infoxadc(int *pnChannels, const char ***pppStrNames) {
  int   rc = parax_ok;
 
  if(pnChannels == NULL)
    return parax_badarg;

  if(nChannels == -1)
    rc = para_initxadc();
    
  if(pppStrNames != NULL)
    *pppStrNames = (const char **)pstrNames;

  *pnChannels = nChannels;

  return rc;
}

int   para_getidxadc(const char *pStrNamePat, int *pnIdNum) {
  int  rc, n;

  if(pnIdNum == NULL || pStrNamePat == NULL)
    return parax_badarg;

  if(nChannels == -1) {
    rc = para_initxadc();
    if(rc) return rc;
  }

  for(n=0; n<nChannels; n++) {
    if(strcasestr(pstrNames[n], pStrNamePat)) {
      *pnIdNum = n;
      return parax_ok;
    }
  }

  return parax_notfound;
}

int   para_getxadc(int nId, float *pfValue) {
  FILE *sysfile;
  char  strRead[PARAX_MAXSTRLEN], strName[PARAX_MAXSTRLEN];
  int   rc = parax_ok, nRaw;

  if(nChannels == -1) {
    rc = para_initxadc();
    if(rc) return rc;
  }

  if(nId < 0 || nId >= nChannels)
    return parax_badarg;

  strcpy(strName, strBase);
  strcat(strName, pstrNames[nId]);
  strcat(strName, "_raw");

  if((sysfile = fopen(strName, "ra")) == NULL) {
    fprintf(stderr, "ERROR: Unable to open raw data file %s\n",
	    strName);
    return parax_fileerr;
  }

  fgets(strRead, PARAX_MAXSTRLEN-1, sysfile);
  fclose(sysfile);

  nRaw = atoi(strRead);
  *pfValue = (nRaw + nOffsets[nId]) * fScales[nId] / 1000.;

  return rc;
}
