#ifndef __pic32mk_h__
#define __pic32mk_h__

#include "device.h"
#include <math.h>
#include <stdbool.h>
#include <sys/kmem.h>

// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************


typedef enum
{
    DMAC_CHANNEL_0 = 0x0,

    DMAC_CHANNEL_1 = 0x1,

    DMAC_CHANNEL_2 = 0x2,

    DMAC_CHANNEL_3 = 0x3,

    DMAC_CHANNEL_4 = 0x4,

    DMAC_CHANNEL_5 = 0x5,

    DMAC_CHANNEL_6 = 0x6,

    DMAC_CHANNEL_7 = 0x7,

    DMAC_NUMBER_OF_CHANNELS = 0x8

} DMAC_CHANNEL;

typedef enum
{
    /* No events yet. */
    DMAC_TRANSFER_EVENT_NONE = 0,

    /* Data was transferred successfully. */
    DMAC_TRANSFER_EVENT_COMPLETE = 1,

    /* Error while processing the request */
    DMAC_TRANSFER_EVENT_ERROR = 2,

    /* Half Data is transferred */
    DMAC_TRANSFER_EVENT_HALF_COMPLETE = 4

} DMAC_TRANSFER_EVENT;

typedef enum
{
    /* Data was transferred successfully. */
    DMAC_ERROR_NONE /*DOM-IGNORE-BEGIN*/ = 1, /* DOM-IGNORE-END*/

    /* DMA address error. */
    DMAC_ERROR_ADDRESS_ERROR /*DOM-IGNORE-BEGIN*/ = 2 /* DOM-IGNORE-END*/

} DMAC_ERROR;

void MY_CLK_Initialize( void );
void MY_GPIO_Initialize ( void );
void MY_UART1_Initialize( void );
void MY_EVIC_Initialize( void );
void MY_I2S3_Initialize( uint32_t srate );
void MY_DMAC_Initialize( void );
bool MY_DMAC_ChannelTransfer( DMAC_CHANNEL channel, const void *srcAddr, size_t srcSize, const void *destAddr, size_t destSize, size_t cellSize);

#endif