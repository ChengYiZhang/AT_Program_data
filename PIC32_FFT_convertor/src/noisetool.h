#ifndef __noisetool_h__
#define __noisetool_h__

/*
 * Version 1.1
 * 
 * Created on: Mar 26, 2022
 * 
 * This source code is in the public domain. This source code can be copied,
 * modified, adapted, and distributed without permission, including commercial
 * uses. Any license disputes regarding this source code shall be resolved
 * under the terms of Creative Commons CC0 license.
 *  
 */

#include "kiss_fft.h"

#define SAMPLING_RATE 16000
#define FFT_SIZE 512
#define KISS_CFG_SIZE (FFT_SIZE-1)*8+272

#define DBA_WINDOW_SIZE 8

void setConst(float tuneVal); // dBa tuning constant
void init_fft(void *in_ptr, void *out_ptr);
void simple_fft(void);

void select_channel(uint8_t N); // 0 - left (empty), 1 - right (i2s microphone)
float get_noise(void);
float get_user_noise(uint32_t freqmin1, uint32_t freqmax1,
                     uint32_t freqmin2, uint32_t freqmax2,
                     uint32_t freqmin3, uint32_t freqmax3);
#endif