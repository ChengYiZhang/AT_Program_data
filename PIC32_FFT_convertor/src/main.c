/*
 * Version 1.7
 * For more information about projects:
 * Author: libor.konecny@iqtronic.com
 * Created on: Mar 21, 2022
 * 
 * 
 * This source code is in the public domain. This source code can be copied,
 * modified, adapted, and distributed without permission, including commercial
 * uses. Any license disputes regarding this source code shall be resolved
 * under the terms of Creative Commons CC0 license.
 *  
 */


/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE

#include "device.h"
#include "peripheral/gpio/plib_gpio.h"
#include "peripheral/uart/plib_uart1.h"

#include "pic32mk.h"
#include "noisetool.h"



#define I2S_RX_SIZE 1024

// 512 point FFT uses 1024 point data (complex FFT)
// FFT_SIZE=512
// I2S_RX_SIZE=1024=FFT_SIZE*2 ( x 2 because have real, imag)
// Final real valued spectrum contains 2 x 256 spectrums
// 1) Reusing fft_spectrum "work" variable for final spectrum
// 2) kiss_fft library is not in-place FFT algorihtm,
//    so wasting some memory here
uint32_t fft_spectrum[I2S_RX_SIZE]; 

uint8_t txFlag;
uint8_t rxFlag;
uint32_t rxVal;

uint32_t i2s_rx_buf[I2S_RX_SIZE] __attribute__((coherent)); // TODO: check if coherent works (DMA)
uint32_t i2s_rx_safe[I2S_RX_SIZE]; 
volatile uint8_t i2s_copy=2;

// Mostly copy-pasted from harmony generated code,
// function prefix changed to "MY_"
void MY_SYS_Initialize (void)
{
    /* Start out with interrupts disabled before configuring any modules */
    __builtin_disable_interrupts();

    MY_CLK_Initialize();

    /* Configure CP0.K0 for optimal performance (cached instruction pre-fetch) */
    __builtin_mtc0(16, 0,(__builtin_mfc0(16, 0) | 0x3));

    /* Configure Wait States and Prefetch */
    CHECONbits.PFMWS = 3;
    CHECONbits.PREFEN = 0;

	MY_GPIO_Initialize();
    MY_UART1_Initialize();
    MY_EVIC_Initialize();

	/* Enable global interrupts */
    __builtin_enable_interrupts();
}

// All the handlers are defined here.  Each will call its PLIB-specific function.
void __ISR(_DMA0_VECTOR, ipl1SOFT) DMA0_Handler (void)
{

    /* Check whether the active DMA channel event has occurred */

    if((DCH0INTbits.CHSHIF == true) || (DCH0INTbits.CHDHIF == true))/* irq due to half complete */
    {
        if (i2s_copy==0)
        {
            i2s_copy=1;
            memmove(  (void *)&i2s_rx_safe[0],  (void *)&i2s_rx_buf[0],sizeof(i2s_rx_buf)/2); // dst src            
        }
    }
    
    if(DCH0INTbits.CHBCIF == true) /* irq due to transfer complete */
    {
        /* Channel is by default disabled on completion of a block transfer */
        /* Clear the Block transfer complete, half empty and half full interrupt flag */
        DCH0INTCLR = _DCH0INT_CHBCIF_MASK | _DCH0INT_CHSHIF_MASK | _DCH0INT_CHDHIF_MASK;

        if (i2s_copy==1)
        {       
            i2s_copy=2;
            memmove(  (void *)&i2s_rx_safe[I2S_RX_SIZE/2],  (void *)&i2s_rx_buf[I2S_RX_SIZE/2],sizeof(i2s_rx_buf)/2); // dst src            
        }               
        
        GPIO_RB0_Toggle();        
    }

    /* Clear the interrupt flag and call event handler */
    IFS2CLR = 0x100;    
}



// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{

    uint8_t ch; // variable for usart rx command
    
    MY_SYS_Initialize();
    MY_I2S3_Initialize( SAMPLING_RATE );
    
    /* DMA FOR SPI3(i2s) */
    MY_DMAC_Initialize();
    MY_DMAC_ChannelTransfer(DMAC_CHANNEL_0, (const void *)&SPI3BUF, 4, (const void *)&i2s_rx_buf, I2S_RX_SIZE*4, 4);    
    
    // TODO:
    // 1. Why SPIxBRG must be set to double sampling frequency? fmono=fstereo/2 ?
    // 2. Normalization?
    // 3. Add 20*log10 LUT from STM32 code
    // NJ 2022-3-22

    // init_fft set ponters to FFT input and output data, init kissfft
    init_fft(&i2s_rx_safe,&fft_spectrum);
    
    /*
     * 1) DMA constantly puts I2S data to i2s_rx_buf
     * 2) after i2s_copy is set to zero, i2s_rx_buf copied to i2s_rx_safe
     * 3) fft processes i2s_rx_safe and outputs fft_spectrum
     * 4) i2s_rx_safe data is interleaved LRLRLR... (left right channels), 
     *    FFT_SIZE real valued points for each channel
     * 5) microphone outputs data to R channel (why not L? check this later)
     * 6) fft_spectrum contains amplitude spectrum, starting from FFT_SIZE element,
     *    amplitude spectrum is in linear order (not interleaved)
     *    Left channel spectrum FFT_SIZE..FFT_SIZE+FFT_SIZE/2-1  
     *    Right channel spectrum FFT_SIZE+FFT_SIZE/2..FFT_SIZE*2
     * 7) If FFT size is 512 points, it requires 2x512 points data (complex valued FFT)
     *    I2S buffer size is set to 2x512=1024 points, each second point is zero (mono microphone)
     *    realfft() function allows to perform TWO 512 point real-FFTs using
     *    single 512 point complex FFT.
     
     */
    
    //GPIO_RB0_Clear();
    //GPIO_RB0_Toggle();
    //GPIO_RB0_Set();

    setConst(-53.0); // Tuning constant for dBa output
            
    while ( true )
    {       
        // Waiting for single byte command
        if (UART1_Read(&ch,1)) switch(ch)
        {
            case 0x21:
            {
                // set to zero, start copy from dma buffer to constant buffer
                // Note: microphone data had some interference from UART "0x21"
                // command on my board, probably because of long wires
                // filling array second time, when UART is "silent"
                i2s_copy=0; 
                while (i2s_copy!=2) {asm("NOP");}
                i2s_copy=0; // second time, uart is silent
                while (i2s_copy!=2) {asm("NOP");}                


                // NOTES:
                // 1) no complex signals, using realfft in noisetool.c
                //    to separate spectrums for REAL SIGNAL 1 & 2
                // 2) i2s_rx_safe[] is uint32_t, but data is signed!
                // 3) data arrangement in memory
                //    i2s_rx_safe[0] -> fft_data_in[i].r - REAL SIGNAL 1
                //    i2s_rx_safe[1] -> fft_data_in[i].i - REAL SIGNAL 2
                // 4) i2s data is stereo, microphone fills only ONE channel
                // 5) second channel may be used for other data

                
                for (int i=0; i<I2S_RX_SIZE/2; i++)
                {
                    //i2s_rx_safe[i*2+0]= can put real valued data here
                    //i2s_rx_safe[i*2+1]= filled by I2S microphone channel
                }
                 
                // fft+amplitude. Includes 24bit to 32bit conversion of input data
                simple_fft();
           

                if (0==1)
                for (int i=0; i<FFT_SIZE/2; i++)
                {
                    fft_spectrum[FFT_SIZE+i]=i2s_rx_safe[i];
                }
                
                // first element in spectrum 1 
                // = user noise in zero element for test purposes
                //fft_spectrum[FFT_SIZE]=round(10*get_user_noise(0,0,0,0,1000,2000));
                fft_spectrum[FFT_SIZE]=round(10*get_noise());
                
                //send two spectrums from FFT_SIZE to FFT_SIZE*2-1
                UART1_Write(&fft_spectrum[FFT_SIZE],FFT_SIZE*4);
                //UART1_Write(&i2s_rx_safe[0],FFT_SIZE*4);
                
           
                break;
            }
        
            default:
            {
                // set to zero, start copy from dma buffer to constant buffer
                i2s_copy=0; 
                while (i2s_copy!=2) {asm("NOP");}
                
                // fft+amplitude. Includes 24bit to 32bit conversion of input data
                simple_fft();
                    
                char tempChar[255];
                
                UART1_Write(&tempChar,strlen(tempChar));
                
                UART1_Write(&tempChar,strlen(tempChar));
                break; 
            }

        }
    }
    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

