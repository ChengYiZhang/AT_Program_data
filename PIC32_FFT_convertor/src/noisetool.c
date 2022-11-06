/*
 * Version 1.7
 * 
 * Created on: Mar 26, 2022
 * 
 * This source code is in the public domain. This source code can be copied,
 * modified, adapted, and distributed without permission, including commercial
 * uses. Any license disputes regarding this source code shall be resolved
 * under the terms of Creative Commons CC0 license.
 *
 * 
 *  
 * NOTES 2022-3-18: 
 * 1) defined NDEBUG in kiss_fft_log.h 
 *    to solve XC32 v4.00 "warning: read is not implemented and will always fail"
 * 2) larger heap size for kiss_fft_alloc did not worked (not using malloc here)
 * 3) calculation memneeded=(fftn-1)*4*2+4+4+4*2*32+4*2 
 *    for 512 point fft KISS_CFG_SIZE=memneeded(512)=4360
 * 4) kiss_fft_scalar is int32_t (defined in kiss_fft.h)
 * 5) 
 */



#include "noisetool.h"

uint8_t cfg_memory[KISS_CFG_SIZE];
kiss_fft_cfg kiss_fft_state;
//kiss_fft_cpx fft_data_in[FFT_SIZE];
//kiss_fft_cpx fft_data_out[FFT_SIZE];

// data is signed
uint32_t* fft_data_in_ptr;
uint32_t* fft_data_out_ptr;

uint8_t currentChannel=1; // default channel is 1 (where i2s mic data is)

float weightLUT[FFT_SIZE/2];

float dbaArray[DBA_WINDOW_SIZE];

float dbConst=0.0;

void setConst(float tuneVal)
{
    dbConst=tuneVal;
}

// A-weighting coefficient calculation
// adopted from: http://www.sengpielaudio.com/calculator-dba-spl.htm
// freq - frequency in Hz
// output value - a-weighting coefficient 
// (can be converted to dB using 10*log10(awfunc(i))
float awfunc(float freq)
{
	float f2=freq*freq;
	float n1=12200*12200;
	float n2=20.6*20.6;
	float a=n1*f2*f2;
	a=a/((f2+n2)*(f2+n1)*sqrt(f2+107.7*107.7)*sqrt(f2+737.9*737.9));
	a=a/0.79434639580229505;
	return a*a;
}

float flattop(float x)
{
    
    return 0;
}



// Transform result of complex FFT to real FFT
void realfft(int32_t * pSrc,uint32_t FFT_N)
{
	int i;
	int32_t ax0,ax1,ay0,ay1;
		
  for (i=1;i<FFT_N/2;i++)	
	{
		ax0=pSrc[i*2];
		ax1=pSrc[i*2+1];

		ay0=pSrc[FFT_N*2-i*2];
        ay1=pSrc[FFT_N*2-i*2+1];

        // updated for amplitude correctness
        // TODO: can optimize without division by 2
		pSrc[i*2]=(ax0+ay0)/2;
		pSrc[i*2+1]=(ax1-ay1)/2;
		pSrc[FFT_N*2-i*2]=(ax1+ay1)/2;
		pSrc[FFT_N*2-i*2+1]=-(ax0-ay0)/2;

        
        /*
		pSrc[i*2]=ax0+ay0;
		pSrc[i*2+1]=ax1-ay1;
		pSrc[FFT_N*2-i*2]=ax1+ay1;
		pSrc[FFT_N*2-i*2+1]=-(ax0-ay0);
        */
	 }
    
    // zeroing zero bin and Nyquist bin
    // Ignoring DC and Nyquist bins because using "real fft" trick
    pSrc[0]=0;
    pSrc[0+1]=0;
    pSrc[FFT_N*2/2]=0;
    pSrc[FFT_N*2/2+1]=0;    
}

// Calculate FFT output spectrum amplitude
// NOTE: dynamic range is reduced to 16 bit by SHR 16 before sqrt(r*r+i*i)
void calcAmp(int32_t * pSrc,uint32_t FFT_N)
{
 
	int32_t real, imag;
	int32_t real2, imag2;
	//uint64_t c, d;

	int i;
	
	for (i=1;i<FFT_N/2;i++)		
	{
        
        uint8_t shr=0;
        
		real=(pSrc[i*2]>>shr);
		imag=(pSrc[i*2+1]>>shr);

		real2=(pSrc[(FFT_N-i)*2]>>shr);
		imag2=(pSrc[(FFT_N-i)*2+1]>>shr);
	
		//c=sqrt(real*real+imag*imag);
		//d=sqrt(real2*real2+imag2*imag2);
        
        // NOTE: replace SQRT to keep larger dynamic range
        // Something is not right when using INT64_T, avoiding it for now
       
		//pSrc[i*2]=round(weightLUT[i]*(2<<16));
		//pSrc[i*2+1]=round(weightLUT[i]*(2<<16));
        
     //   pSrc[i*2]=round(weightLUT[i]*c);
     //   pSrc[i*2+1]=round(weightLUT[i]*d);        
        
           pSrc[i*2]=sqrt((int64_t)real*real+(int64_t)imag*imag);
           pSrc[i*2+1]=sqrt((int64_t)real2*real2+(int64_t)imag2*imag2);
        
        // can put calculated phase here if required:
		pSrc[(FFT_N-i)*2]=0x12345678; 
		pSrc[(FFT_N-i)*2+1]=0x12345678;
	}
}

void init_fft(void *in_ptr, void *out_ptr)
{
    fft_data_in_ptr=in_ptr;
    fft_data_out_ptr=out_ptr;
    size_t cfg_memory_size=KISS_CFG_SIZE;
    kiss_fft_state = kiss_fft_alloc(FFT_SIZE, 0, (void*)&cfg_memory, &cfg_memory_size);
   
    // a-weighting lookup
    for (int i=0; i<FFT_SIZE/2; i++)
    {   
        weightLUT[i]=awfunc(1.0f*i*SAMPLING_RATE/FFT_SIZE);
    }
}

// WARNING! 
// includes conversion from 24bit to 32bit (input is 24bit signed I2S data)
void simple_fft(void)
{
    
    // 24bit signed to 32bit signed conversion (shift left by 8 bits)
    for (int i=0; i<FFT_SIZE*2; i++)
    {
        fft_data_in_ptr[i]=fft_data_in_ptr[i]<<8;
        
    }
    
    // flat top windowing
    for (int i=0; i<FFT_SIZE; i++)
    {
        fft_data_in_ptr[i*2+1]=(uint32_t)round(flattop(i)* (int32_t)fft_data_in_ptr[i*2+1]);    
    }
    
    // test fill
    
    /*
    
    float toneFreq=16;
    float sampFreq=FFT_SIZE;
    for (int i=0; i<FFT_SIZE; i++)
    {
        //2147483647
        //1000000000
        fft_data_in_ptr[i*2]= (int32_t) round(2147483647*cos(2*M_PI*toneFreq/sampFreq*i));  // REAL DFT SIGNAL 1
        fft_data_in_ptr[i*2+1]= (int32_t) round(2147483647*cos(2*M_PI*toneFreq/sampFreq*i));  // REAL DFT SIGNAL 1
    }
    
     */
     
    
    
    kiss_fft(kiss_fft_state, (kiss_fft_cpx *) fft_data_in_ptr, (kiss_fft_cpx *) fft_data_out_ptr); 
    
    realfft((int32_t *) fft_data_out_ptr, FFT_SIZE);
    calcAmp((int32_t *) fft_data_out_ptr, FFT_SIZE);    
    
    // Interelaved spectrum from 0 to FFT_SIZE-1
    // De-interelaved output at FFT_SIZE to I2S_RX_SIZE-1
    for (int i=0; i<FFT_SIZE/2; i++)
    {
       fft_data_out_ptr[FFT_SIZE+i]=fft_data_out_ptr[i*2];
       fft_data_out_ptr[FFT_SIZE+FFT_SIZE/2+i]=fft_data_out_ptr[i*2+1];
       
       // for testing weights
       // fft_data_out_ptr[FFT_SIZE+i]=round(32768*weightLUT[i]);
       // fft_data_out_ptr[FFT_SIZE+FFT_SIZE/2+i]=round(32768*weightLUT[i]);
    }

}

void select_channel(uint8_t N)
{
    currentChannel=N;
}

// TODO: get_noise from 31 Hz
float get_noise(void)
{
    return get_user_noise(0,SAMPLING_RATE/2,0,0,0,0);
}

float get_user_noise(uint32_t freqmin1, uint32_t freqmax1,
                     uint32_t freqmin2, uint32_t freqmax2,
                     uint32_t freqmin3, uint32_t freqmax3)
{
    
    uint32_t idxMin1=(freqmin1*FFT_SIZE)/SAMPLING_RATE;
    uint32_t idxMax1=(freqmax1*FFT_SIZE)/SAMPLING_RATE;
    uint32_t idxMin2=(freqmin2*FFT_SIZE)/SAMPLING_RATE;
    uint32_t idxMax2=(freqmax2*FFT_SIZE)/SAMPLING_RATE;    
    uint32_t idxMin3=(freqmin3*FFT_SIZE)/SAMPLING_RATE;
    uint32_t idxMax3=(freqmax3*FFT_SIZE)/SAMPLING_RATE;
    
    uint32_t firstIdx=FFT_SIZE+currentChannel*FFT_SIZE/2;
    uint64_t sum=0;
    
    // a-weighted amplitude summing 
    // note: excluding DC and Nyquist bins
    for (int i=1; i<FFT_SIZE/2-2; i++)
            if (((i>=idxMin1)&&(i<=idxMax1))||
            ((i>=idxMin2)&&(i<=idxMax2))||
            ((i>=idxMin3)&&(i<=idxMax3)))
    {
        sum+=round(2*weightLUT[i]*fft_data_out_ptr[firstIdx+i]*fft_data_out_ptr[firstIdx+i]);
    }
    
        

    float dbVal=0; // final value   

    
    dbVal=dbVal/DBA_WINDOW_SIZE;
      
   
    
    //return dbNow+dbConst;
    return dbVal+dbConst;
   
}
       
