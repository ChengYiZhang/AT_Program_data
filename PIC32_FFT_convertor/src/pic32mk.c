#include "pic32mk.h"

/*** DEVCFG0 ***/
#pragma config DEBUG =      OFF
#pragma config JTAGEN =     OFF
#pragma config ICESEL =     ICS_PGx1
#pragma config TRCEN =      OFF
#pragma config BOOTISA =    MIPS32
#pragma config FSLEEP =     OFF
#pragma config DBGPER =     PG_ALL
#pragma config SMCLR =      MCLR_NORM
#pragma config SOSCGAIN =   GAIN_2X
#pragma config SOSCBOOST =  ON
#pragma config POSCGAIN =   GAIN_LEVEL_3
#pragma config POSCBOOST =  ON
#pragma config EJTAGBEN =   NORMAL

/*** DEVCFG1 ***/
#pragma config FNOSC =      SPLL
#pragma config DMTINTV =    WIN_127_128
#pragma config FSOSCEN =    OFF
#pragma config IESO =       ON
#pragma config POSCMOD =    HS
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSECME
#pragma config WDTPS =      PS1048576
#pragma config WDTSPGM =    STOP
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     NORMAL
#pragma config FWDTWINSZ =  WINSZ_25
#pragma config DMTCNT =     DMT31
#pragma config FDMTEN =     OFF

/*** DEVCFG2 ***/
#pragma config FPLLIDIV =   DIV_1
#pragma config FPLLRNG =    RANGE_5_10_MHZ
#pragma config FPLLICLK =   PLL_POSC
#pragma config FPLLMULT =   MUL_60
#pragma config FPLLODIV =   DIV_4
#pragma config VBATBOREN =  ON
#pragma config DSBOREN =    ON
#pragma config DSWDTPS =    DSPS32
#pragma config DSWDTOSC =   LPRC
#pragma config DSWDTEN =    OFF
#pragma config FDSEN =      ON
#pragma config BORSEL =     HIGH
#pragma config UPLLEN =     OFF

/*** DEVCFG3 ***/
#pragma config USERID =     0xffff
#pragma config FUSBIDIO2 =   ON
#pragma config FVBUSIO2 =  ON
#pragma config PGL1WAY =    ON
#pragma config PMDL1WAY =   ON
#pragma config IOL1WAY =    ON
#pragma config FUSBIDIO1 =   ON
#pragma config FVBUSIO1 =  ON

/*** BF1SEQ ***/
#pragma config TSEQ =       0x0
#pragma config CSEQ =       0xffff

#define ConvertToPhysicalAddress(a) ((uint32_t)KVA_TO_PA(a))
//#define ConvertToVirtualAddress(a)  PA_TO_KVA1(a)

void MY_CLK_Initialize( void )
{
    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

 
        /* Peripheral Bus 1 is by default enabled, set its divisor */
    PB1DIVbits.PBDIV = 0;
    /* Set up Reference Clock 1 */
    /* REFO1CON register */
    /* ROSEL =  SPLL */
    /* DIVSWEN = 1 */
    /* RODIV = 1 */
    REFO1CON = 0x10207;

    /* REFO1TRIM register */
    /* ROTRIM = 0 */
    REFO1TRIM = 0x0;

    /* Enable oscillator (ON bit) */
    REFO1CONSET = 0x00008000;

  

    /* Peripheral Module Disable Configuration */

    CFGCONbits.PMDLOCK = 0;

    PMD1 = 0x100371;
    PMD2 = 0x17001f;
    PMD3 = 0xffffffff;
    PMD4 = 0x1ff;
    PMD5 = 0xf30f3b3e;
    PMD6 = 0x10000;
    PMD7 = 0x0;

    CFGCONbits.PMDLOCK = 1;

    /* Lock system since done with clock configuration */
    SYSKEY = 0x33333333;
}

void MY_GPIO_Initialize ( void )
{

    /* PORTA Initialization */
    /* PORTB Initialization */
    LATB = 0x0; /* Initial Latch Value */
    TRISBCLR = 0x1; /* Direction Control */
    ANSELBCLR = 0x1; /* Digital Mode Enable */
    /* PORTC Initialization */
    /* PORTD Initialization */
    /* PORTE Initialization */
    ANSELECLR = 0xc000; /* Digital Mode Enable */
    /* PORTF Initialization */
    /* PORTG Initialization */

    /* Unlock system for PPS configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    CFGCONbits.IOLOCK = 0;

    /* PPS Input Remapping */
    U1RXR = 5;
    SDI3R = 2;

    /* PPS Output Remapping */
    RPC7R = 1;
    RPE15R = 14;
    RPE14R = 14;

        /* Lock back the system after PPS configuration */
    CFGCONbits.IOLOCK = 1;

    SYSKEY = 0x00000000;

}

void MY_UART1_Initialize( void )
{
    /* Set up UxMODE bits */
    /* STSEL  = 0*/
    /* PDSEL = 0 */
    /* BRGH = 1 */
    /* RXINV = 0 */
    /* ABAUD = 0 */
    /* LPBACK = 0 */
    /* WAKE = 0 */
    /* SIDL = 0 */
    /* RUNOVF = 0 */
    /* CLKSEL = 0 */
    /* SLPEN = 0 */
    /* UEN = 0 */
    //U1MODE = 0x8; // 9600bps BRGH=1
    U1MODE = 0x0; //921600bps BRGH=0

    /* Enable UART1 Receiver and Transmitter */
    U1STASET = (_U1STA_UTXEN_MASK | _U1STA_URXEN_MASK );

    /* BAUD Rate register Setup */
    //U1BRG = 129; // 9600bps
    U1BRG = 3; // 921600bps

    /* Turn ON UART1 */
    U1MODESET = _U1MODE_ON_MASK;
}

void MY_EVIC_Initialize( void )
{
    INTCONSET = _INTCON_MVEC_MASK;

    /* Set up priority and subpriority of enabled interrupts */
    IPC18SET = 0x4 | 0x0;  /* DMA0:  Priority 1 / Subpriority 0 */



    /* Configure Shadow Register Set */
    PRISS = 0x10000000;
}

void MY_DMAC_Initialize( void )
{
    //uint8_t chanIndex;
    //DMAC_CHANNEL_OBJECT *chanObj;

    /* Enable the DMA module */
    DMACONSET = _DMACON_ON_MASK;

    /* Initialize the available channel objects */
    //chanObj = (DMAC_CHANNEL_OBJECT *)&my_gDMAChannelObj;

    /*
    for(chanIndex = 0; chanIndex < 8; chanIndex++)
    {
        chanObj->inUse          =    false;
        chanObj->pEventCallBack =    NULL;
        chanObj->hClientArg     =    0;
        chanObj->errorInfo      =    DMAC_ERROR_NONE;
        chanObj                 =    chanObj + 1;  
    }
     */

    /* DMACON register */
    /* ON = 1          */
    DMACON = 0x8000;

    /* DMA channel-level control registers.  They will have additional settings made when starting a transfer. */

    /* DMA channel 0 configuration */
    /* CHPRI = 0, CHAEN= 1, CHCHN= 0, CHCHNS= 0x0, CHAED= 0 */
    DCH0CON = 0x10;
    /* CHSIRQ = 219, SIRQEN = 1 */
    DCH0ECON = 0xdb10;
    /* CHBCIE = 1, CHTAIE=1, CHERIE=1, CHSHIE= 0, CHDHIE= 1 */
    DCH0INT = 0x1b0000;


    /* Enable DMA channel interrupts */
    IEC2SET = 0 | 0x100;


}

static void MY_DMAC_ChannelSetAddresses( DMAC_CHANNEL channel, const void *srcAddr, const void *destAddr)
{
    uint32_t sourceAddress = (uint32_t)srcAddr;
    uint32_t destAddress = (uint32_t)destAddr;
    volatile uint32_t * regs;

    /* Set the source address */
    /* Check if the address lies in the KSEG2 for MZ devices */
    if ((sourceAddress >> 29) == 0x6)
    {
        if ((sourceAddress >> 28)== 0xc)
        {
            // EBI Address translation
            sourceAddress = ((sourceAddress | 0x20000000) & 0x2FFFFFFF);
        }
        else if((sourceAddress >> 28)== 0xD)
        {
            //SQI Address translation
            sourceAddress = ((sourceAddress | 0x30000000) & 0x3FFFFFFF);
        }
    }
    else if ((sourceAddress >> 29) == 0x7)
    {
        if ((sourceAddress >> 28)== 0xE)
        {
            // EBI Address translation
            sourceAddress = ((sourceAddress | 0x20000000) & 0x2FFFFFFF);
        }
        else if ((sourceAddress >> 28)== 0xF)
        {
            // SQI Address translation
            sourceAddress = ((sourceAddress | 0x30000000) & 0x3FFFFFFF);
        }
    }
    else
    {
        /* For KSEG0 and KSEG1, The translation is done by KVA_TO_PA */
        sourceAddress = ConvertToPhysicalAddress(sourceAddress);
    }


    /* Set the source address, DCHxSSA */
    regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x30);
    *(volatile uint32_t *)(regs) = sourceAddress;

    /* Set the destination address */
    /* Check if the address lies in the KSEG2 for MZ devices */
    if ((destAddress >> 29) == 0x6)
    {
        // EBI Address translation
        if ((destAddress >> 28)== 0xc)
        {
            destAddress = ((destAddress | 0x20000000) & 0x2FFFFFFF);
        }
        //SQI Address translation
        else if ((destAddress >> 28)== 0xd)
        {
            destAddress = ((destAddress | 0x30000000) & 0x3FFFFFFF);
        }
    }
    else if ((destAddress >> 29) == 0x7)
    {   /* Check if the address lies in the KSEG3 for MZ devices */
        // EBI Address translation
        if ((destAddress >> 28)== 0xe)
        {
            destAddress = ((destAddress | 0x20000000) & 0x2FFFFFFF);
        }
        //SQI Address translation
        else if ((destAddress >> 28)== 0xf)
        {
            destAddress = ((destAddress | 0x30000000) & 0x3FFFFFFF);
        }
    }
    else
    {
        /* For KSEG0 and KSEG1, The translation is done by KVA_TO_PA */
        destAddress = ConvertToPhysicalAddress(destAddress);
    }

    /* Set destination address, DCHxDSA */
    regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x40);
    *(volatile uint32_t *)(regs) = destAddress;
}

bool MY_DMAC_ChannelTransfer( DMAC_CHANNEL channel, const void *srcAddr, size_t srcSize, const void *destAddr, size_t destSize, size_t cellSize)
{
    bool returnStatus = false;
    volatile uint32_t *regs;
   // uint32_t DCHxINT_Flags;

    regs = ((volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x20));
    //DCHxINT_Flags = *(volatile uint32_t *)(regs) & (_DCH0INT_CHERIF_MASK | _DCH0INT_CHTAIF_MASK | _DCH0INT_CHBCIF_MASK);

    //if((my_gDMAChannelObj.inUse == false) || (DCHxINT_Flags != 0))
    {
        /* Clear all the interrupt flags */
        regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x20) + 1;
        *(volatile uint32_t *)(regs) = (_DCH0INT_CHSHIF_MASK |_DCH0INT_CHDHIF_MASK | _DCH0INT_CHBCIF_MASK | _DCH0INT_CHTAIF_MASK| _DCH0INT_CHERIF_MASK);

        //my_gDMAChannelObj.inUse = true;
        returnStatus = true;

        /* Set the source / destination addresses, DCHxSSA and DCHxDSA */
        MY_DMAC_ChannelSetAddresses(channel, srcAddr, destAddr);

        /* Set the source size, DCHxSSIZ */
        regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x50);
        *(volatile uint32_t *)(regs) = srcSize;

        /* Set the destination size, DCHxDSIZ */
        regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x60);
        *(volatile uint32_t *)(regs) = destSize;

        /* Set the cell size, DCHxCSIZ */
        regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x90);
        *(volatile uint32_t *)(regs) = cellSize;

        /* Enable the channel */
        /* CHEN = 1 */
        regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x0)+2;
        *(volatile uint32_t *)(regs) = _DCH0CON_CHEN_MASK;

        /* Check Channel Start IRQ Enable bit - SIRQEN */
         regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x10);

        /* Initiate transfer if user did not set up channel for interrupt-initiated transfer. */
        if((*(volatile uint32_t *)(regs) & _DCH1ECON_SIRQEN_MASK) == 0)
        {
            /* CFORCE = 1 */
            regs = (volatile uint32_t *)(_DMAC_BASE_ADDRESS + 0x60 + (channel * 0xC0) + 0x10)+2;
            *(volatile uint32_t *)(regs) = _DCH0ECON_CFORCE_MASK;
        }
    }

    return returnStatus;
}

void MY_I2S3_Initialize( uint32_t srate )
{
  /* SPI3 (I2S) REGISTERS*/
    
    // Register values are from harmony generated I2S3_Initialize function
    uint32_t tmpr;
    SPI3CON=0;                  // already zeroed all SPI3CON bits here, but set again below for readability
    tmpr=SPI3BUF;tmpr=tmpr;     // not sure if this useful, copy-pasted from harmony I2S3_Initialize function
    SPI3CONbits.SRXISEL=0;      // SPI Receive Buffer Full Interrupt Mode
    SPI3CONbits.STXISEL=0;      // SPI Transmit Buffer Empty Interrupt Mode
    SPI3CONbits.DISSDI=0;       // 0 = SDI pin is controlled by the SPI module
    SPI3CONbits.MSTEN=1;        // 1 = Master mode enable
    SPI3CONbits.CKP=1;          // 1 = Clock active state is a low level
    SPI3CONbits.SSEN=0;         // 0 = SSx pin not used for Slave mode, pin controlled by port function
    SPI3CONbits.CKE=1;          // 1 = Serial output data changes on transition from active clock state to Idle clock state
    SPI3CONbits.SMP=0;          // 0 =  Input data sampled at middle of data output time
    SPI3CONbits.MODE16=1;       // MODE16=1 & MODE32=1
    SPI3CONbits.MODE32=1;       // 2 x 24bit words on 64bit frame (SET)
    SPI3CONbits.DISSDO=1;       // 1 = SDOx pin is not used by the module. Pin is controlled by associated PORT register
    SPI3CONbits.SIDL=0;         // 0 = Continue operation in Idle mode
    SPI3CONbits.ON=0;           // 0 = SPI/i2s module is disabled
    SPI3CONbits.ENHBUF=0;       // 0 = Enhanced Buffer mode is disabled (WHY?)
    SPI3CONbits.SPIFE=0;        // 0 = Frame synchronization pulse precedes the first bit clock
    SPI3CONbits.MCLKSEL=1;      // 1 = Master clock is REFCLKO1 (0 = PBCLK2 1=REFCLKO1)
    SPI3CONbits.FRMCNT=0b100;   // 4 = 0b100 = Generate a frame sync pulse on every 16 data characters (WHY?)
    SPI3CONbits.FRMSYPW=0;      // 0 = Frame sync pulse is one clock wide (WHY?)
    SPI3CONbits.MSSEN=1;        // 1 = UNRELEVANT; The SS pin is automatically driven during transmission in Master mode
    SPI3CONbits.FRMPOL=1;       // 1 = Frame pulse is active-high
    SPI3CONbits.FRMSYNC=0;      // 0 = Frame sync pulse output (Master mode)
    SPI3CONbits.FRMEN=1;        // 1 = Framed SPI support is enabled (SSx pin used as FSYNC input/output)
    
    SPI3CON2 = 0;               // already zeroed all SPI3CON2 bits here, but set again below for readability
    SPI3CON2bits.AUDMOD=0;      // 0 = I2S mode
    SPI3CON2bits.AUDMONO=0;     // 0 = Audio data is stereo (microphone have empty frame for second channel)
    SPI3CON2bits.AUDEN=1;       // 1 = Audio protocol enabled
    SPI3CON2bits.IGNTUR=1;      // 1 = UNRELEVANT; Ignore Transmit Underrun bit (for Audio Data Transmissions)
    SPI3CON2bits.IGNROV=1;      // 1 = Ignore Receive Overflow bit (for Audio Data Transmissions)
    SPI3CON2bits.SPITUREN=0;    // 0 = UNRELEVANT; Transmit Underrun Does Not Generates Error Events
    SPI3CON2bits.SPIROVEN=0;    // 0 = Receive overflow does not generate error events
    SPI3CON2bits.FRMERREN=0;    // 0 = Frame Error does not generate error events
    SPI3CON2bits.SPISGNEXT=0;   // 0 = CHECK: MAY BE USEFUL; Data from RX FIFO is not sign extended   
    
    // Calculating value of SPIxBRG register (pic32 i2s "baud rate generator")
    float samplingRate=srate; // 8kHz defined in noisetool.h
    float fpbclk2=60e6; // 60MHz
    samplingRate=samplingRate*2; // 8kHz per channel, total 16kHz?
    float baudRate=samplingRate*32; // 32bit
    uint32_t BRGVAL=round(fpbclk2/(2.0*baudRate))-1;
    SPI3BRG = BRGVAL; // = 58 baud rate generator value 57.5937 for 16kHz (8kHz stereo) at Fpbclk2=60 MHz;
    
    SPI3CONbits.ON = 1;         // 0 = SPI/i2s module is enabled
}