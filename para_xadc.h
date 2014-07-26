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

  para_xadc.h

  Header file for the para_xadc library, providing access to the
  internal Analog-to-Digital converter of the Zynq FPGA.

  Available ADC Channels:

    On the Parallella there are 9 or 10 adc channels depending on the
    settings in the devicetree:

        temp0             - Zynq Temperature in deg. C
	voltage0_vccint   - PL Internal core voltage
	voltage1_vccaux   - PL VccAux voltage
	voltage2_vccbram  - PL Internal BRAM voltage
	voltage3_vccpint  - PS Internal core voltage
	voltage4_vccpaux  - PS VccAux voltage
	voltage5_vcc0_ddr - DDR I/O voltage
	voltage6_vrefp    - XADC positive reference voltage
	voltage7_vrefn    - XADC negative reference voltage
	voltage8          - Dedicated VP/VN input voltage

    The last entry "voltage8" is not available when using early 
    devicetree files.

  Basic C functions:
    Unless otherwise stated, all functions return a result code, 0 always
      indicates success.  The codes are enumerated as e_para_xadcres.

    para_infoxadc(int *pnChannels, const char ***pppStrNames) -
      Returns with the number of channels available and a list of names
      of each channel filled-in to the pointers in the arguments.
      pppStrNames may be NULL, in which case only the number of
      channels is provided.

    para_getidxadc(const char *pStrNamePat, int *pnIdNum) -
      Call with pStrNamePat pointing to a null-terminated string with
      a portion of the name of the desired channel, e.g. "temp".  Sets
      *pnIdNum to the ID number of the first ADC channel contining that
      string.  If no match is found returns with *pnChanNum = -1.
      This is a simple case-insensitive match, not a regex.

    para_getxadc(int nId, float *pfValue) -
      Call with nId indicating the desired channel, pfValue will be
      filled-in with the current reading, offset and gain corrected.

  Caveats:

    The first call to any one of these functions will scan for the
      sysfs entries and initialize the internal records for all
      channels present, including the scale (and offset in the case
      of temperature measurements).

    Reading values in a tight loop does not guarantee new values
      each time.

    I think the scale values reported for VREFP/N are incorrect, 
      VREFP should be 1.25V.

*/

#ifndef PARA_XADC_H
#define PARA_XADC_H

#define PARAX_MAXCHANS  64
#define PARAX_MAXSTRLEN 256

// Function return values
enum e_para_xadcres {
    parax_ok = 0,
    parax_outofrange = 1,
    parax_outofmemory = 2,
    parax_fileerr = 5,
    parax_badarg = 10,
    parax_notfound = 12,
  };

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

  int   para_infoxadc(int *pnChannels, const char ***pppStrNames);
  int   para_getidxadc(const char *pStrNamePat, int *pnIdNum);
  int   para_getxadc(int nId, float *pfValue);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // !PARA_XADC_H
