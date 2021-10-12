static char sccsid[] = "@(#)32  1.5  src/bos/diag/tu/sun/pc5xxini.c, tu_sunrise, bos411, 9437A411a 8/1/94 16:07:50";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: CODECSetup
 *              CheckHuffmanTables
 *              DumpHTable
 *              DumpQTable
 *              InitCL5xx
 *              InitPC550
 *              LoadTable
 *              ReadCL5xxRegister
 *              TestTable
 *              WriteCL5xxRegister
 *              calculateConfig
 *              calculateHActive
 *              calculateHControl
 *              calculateHDelay
 *              calculateHPeriod
 *              calculateHSync
 *              calculateVControl
 *              calculateVDelay
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**************************************************************************/
/*  File:  pc5xxini.c                                                     */
/**************************************************************************/
/*
        History -
                Written by L.S.         May-June, 1992
                Modififed for CL5xx     February, 1993

        Description -
        This file contains the functions for configuring the
        CL5xx develompent board and the CL5xx processor to perform
        wonders with your image.

        For you debuggers, there is a register tracing feature
        built into this program. The tracing selections are made here with
        #ifdef's, and you enable tracing code in PC5xx.h at compile-time.
*/

#include <stdio.h>
#include "pc5xx.h"      /* PC5xx PC board definitions file */
#include "cl5xx.h"      /* CL5xx initialization tables */
#include "suntu.h"
#include "error.h"

/**** Static Prototypes *********************************************/

int InitCL5xx( void );
int LoadTable
(
        unsigned short  Base,       /* CL5xx table base */
        short           Count,      /* number of table entries to load */
        unsigned short  *Table,     /* Source Table pointer */
        unsigned short  Mask        /* Bitmask used for read-back test */
);
static unsigned short calculateHPeriod( short HPeriod );
static unsigned short calculateHSync( short HSync, short HPeriod );
static unsigned short calculateHDelay(  short HDelay );
static unsigned short calculateHActive( short HActive );
static unsigned short calculateVDelay(
        short VDelay,
        short HActive,
        short HPeriod);
static unsigned short calculateConfig(void);
static unsigned short calculateHControl( short HActive);
static unsigned short calculateVControl(
        short HActive,
        short VActive,
        short HPeriod);
static short TestTable(
        FILE *outfile,
        unsigned short DeviceBase,
        unsigned short *HuffmanTable,
        short count,
        unsigned short mask);

/***** Globals ******************************************************/

short DeviceType;                   /* Init Variables */
short Direction;
short PixelMode;
short MasterSlave;
short ToggleOn;

short CL5xxImageWidth, CL5xxImageHeight;

short NoLoad;    /* inhibits huffman loading for speed */

/*********************************************************************/

/*      InitPC550()
        This function will initialize the PC550 card, giving a reset
        followed by programming the PCcontrol port. Next, the CL5xx
        is programmed using InitCL5xx().
*/

int InitPC550(void)
{
        short  temp;
        int rc=0;
        /*************************************************/
        /* Issue soft reset and set config register bits */
        /*************************************************/

        if (rc = WriteCL5xxRegister(_S_Reset, 0xff, 0)) return(rc);

        if (rc = InitCL5xx()) return(rc);  /* Initialize the CL5xx device. */
}

/*******************************************************************/

/*  InitCL5xx()

        This function performs all register and table initialization of the
        CL5xx for compression or decompression operations. This code is
        very generic and can be used in most CL5xx applications as-is. There
        are three main sections to this function.

        1)  Load the CL5xx registers. Some register values are caluculated
                based on H/V parameters, others are loaded from static arrays
                depending on device type, direction and pixel type.
        2)  Load Quantizer tables with data stored in arrays. The data arrays
                are initialized by MakeQ.c
        3)  Load Huffman Tables with data stored in arrays. The data arrays
                are initialized by MakeHuff.c
*/

int InitCL5xx(void)
{
        short LeftBlanking, RightBlanking;
        short TopBlanking, BottomBlanking;
        short HPeriod, HSync, HDelay, HActive;
        short VPeriod, VSync, VDelay, VActive;
        unsigned short temp;
        int rc;

/*    printf("Loading CL5xx Registers\n");   */


/*  Part 1. Registers Common to all CL5xx devices */
/*
        Calculate and load the Horizontal/Vertical image parameters
        into the CL5xx. The first part of this process involves computing some
        mode-independent image parameters that specify the CL5xx image frame
        relative to the HSYNC and VSYNC reference signals. These formulas are
        given as examples only. See the CL5xx Databook for specific programming
        documentation.
*/

        LeftBlanking   = 2;             /* arbitrary */   /* pixels */
        RightBlanking  = 2;             /* arbitrary */   /* pixels */
        TopBlanking    = 16;             /* minimum 9 */   /* lines */
        BottomBlanking = 16;             /* minimum 9 */   /* lines */

        HPeriod = CL5xxImageWidth + LeftBlanking + RightBlanking;
        HDelay  = LeftBlanking;                            /* pixels */
        HSync   = 4;                      /* arbitrary */  /* pixels */
        HActive = (CL5xxImageWidth / 8);                   /* blocks */

        VPeriod = CL5xxImageHeight + TopBlanking + BottomBlanking;
        VDelay  = TopBlanking;                             /* lines */
        VSync   = 4;                     /* arbitrary */   /* lines */
        VActive = (CL5xxImageHeight / 8);                  /* lines */

/*
    Based on the values calculated above, mode-dependent values
    for the CL5xx are calculated and loaded into the CL5xx's
    Horizontal/Vertical control registers using the functions below.
*/

        if (rc = WriteCL5xxRegister(_HPeriod, calculateHPeriod(HPeriod), 0)) return(rc);
        if (rc = WriteCL5xxRegister(_HSync, calculateHSync(HSync, HPeriod), 0)) return(rc);
        if (rc = WriteCL5xxRegister(_HDelay, calculateHDelay(HDelay), 0)) return(rc);
        if (rc = WriteCL5xxRegister(_HActive, calculateHActive(HActive), 0)) return(rc);

        if(MasterSlave == MASTER) {
                if (rc = WriteCL5xxRegister(_VPeriod, VPeriod, 0)) return(rc);
        }
        else
                if (rc = WriteCL5xxRegister(_VPeriod, 0, 0)) return(rc);

        if(HDelay == 0)  /* Vsync reg is used only in MASTER mode */
                if (rc = WriteCL5xxRegister(_VSync, VSync+1, 0)) return(rc);   /* ADD 1 if HDelay = 0 */
        else
                if (rc = WriteCL5xxRegister(_VSync, VSync, 0)) return(rc);

        if (rc = WriteCL5xxRegister(_VDelay, calculateVDelay(VDelay,HActive,HPeriod), 0)) return(rc);
/*
        WriteCL5xxRegister(_VActive, 14, 0);
*/
        if (rc = WriteCL5xxRegister(_VActive, VActive, 0)) return(rc);

/*  Notes programming HControl and VControl -
        HControl and VControl are significant only during decompression.
        For all still image applications, load these registers with 0xffff.
        For video applications in which the CL550 FIFO is empty between
        frames, load these registers with 0xffff.
        For video applications in which JPEG data is supplied continuously
        (FIFO not empty between frames), use the functions
        CalculateHControl and CalculateVControl to compute these values.
*/

/*  Use these only for continuous video playback */

        if (rc = WriteCL5xxRegister(_HControl, calculateHControl(HActive), 0)) return(rc);
        if (rc = WriteCL5xxRegister(_VControl, calculateVControl(HActive,
                             VActive, HPeriod), 0)) return(rc);
/*  Use these for still image or play-reset-play video frames */

/*
        if (rc = WriteCL5xxRegister(_HControl, 0xffff, 0)) return(rc);
        if (rc = WriteCL5xxRegister(_VControl, 0xffff, 0)) return(rc);
*/

/*  Load machine-specific DCT coefficients (constants) */

        if (rc = WriteCL5xxRegister(_DCT, 0x5A82, 0)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x7FFF, 1)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x30FC, 2)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x7642, 3)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x5A82, 4)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x7FFF, 5)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x30FC, 6)) return(rc);
        if (rc = WriteCL5xxRegister(_DCT, 0x7642, 7)) return(rc);

/*  Load RGB-YUV conversion matrix with conversion coefficients */

/*  Notes on Programming the Color-Space Conversion Matrix -
        The CL5xx has an on-chip color-space converter that is used to
        support the RGB-to-YUV422 pixel mode. The matrix converts
        pixel components according to the model below:

        (PD0-7)       | C1 |       | Matrix00  Matrix01  Matrix02 |     | Y |
        (PD8-15)      | C2 |   X   | Matrix10  Matrix11  Matrix12 |  =  | U |
        (PD16-23)     | C3 |       | Matrix20  Matrix21  Matrix22 |     | V |

        If the pixels are in the order R, G, B on the pixel data bus, the
        matrices are loaded as follows:

        (PD0-7)   -->  | R |     | R2Y  G2Y  B2Y |     | Y |  To compression
        (PD8-15)  -->  | G |  X  | R2U  G2U  B2U |  =  | U |  -------->
        (PD16-23) -->  | B |     | R2V  G2V  B2V |     | V |

        From decompress  | Y |     | Y2R  U2R  V2R |     | R | --> (PD0-7)
                 --------->  | U |  X  | Y2G  U2G  V2G |  =  | G | --> (PD8-15)
                                         | V |     | Y2B  U2B  V2B |     | B | --> (PD16-23)

        Numerical values for R2Y, G2Y, etc. appear in cl5xx.h

        Note that this program initializes for R,G,B pixel arrangement, as
        shown above. Other arrangements ( GBR, RBG,...) are possible as long
        as the conversion matrix is changed to reflect the new ordering.
*/

        if(Direction == COMPRESS)
        {     /* load compression RGB->YUV coefficients */

                if (rc = WriteCL5xxRegister(_Matrix00, R2Y, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix01, G2Y, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix02, B2Y, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix10, R2U, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix11, G2U, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix12, B2U, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix20, R2V, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix21, G2V, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix22, B2V, 0)) return(rc);
        }
        else
        {     /* load decompression YUV->RGB coefficients */

                if (rc = WriteCL5xxRegister(_Matrix00, Y2R, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix01, U2R, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix02, V2R, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix10, Y2G, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix11, U2G, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix12, V2G, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix20, Y2B, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix21, U2B, 0)) return(rc);
                if (rc = WriteCL5xxRegister(_Matrix22, V2B, 0)) return(rc);
        }

/*
        This code segment loads CL5xx registers from static arrays in the
        file "CL5xx.h" these values are dependent on Direction
    (Direction = 0 for compression) and PixelMode.
*/
if (rc = WriteCL5xxRegister(_Config,    calculateConfig(), 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_1,    Init_1_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_2,    Init_2_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_3,    Init_3_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_4,    Init_4_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_5,    Init_5_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_6,    Init_6_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_Init_7,    Init_7_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_QuantSync, QuantSync_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_QuantYCSequence,
           QuantYCSequence_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_QuantABSequence,
           QuantABSequence_Values[Direction][PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_VideoLatency, VideoLatency_Values[Direction][PixelMode],0))
           return(rc);

    /* these values are direction-independent */
if (rc = WriteCL5xxRegister(_HuffTableSequence, HuffTableSequence_Values[PixelMode], 0)) return(rc);
if (rc = WriteCL5xxRegister(_DPCM_SeqHigh, DPCM_SeqHigh_Values[PixelMode],  0)) return(rc);
if (rc = WriteCL5xxRegister(_DPCM_SeqLow, DPCM_SeqLow_Values[PixelMode],  0)) return(rc);

if (rc = WriteCL5xxRegister(_CodingIntH,   CodingIntH_Values[PixelMode],  0)) return(rc);
if (rc = WriteCL5xxRegister(_CodingIntL,   CodingIntL_Values[PixelMode],  0)) return(rc);
if (rc = WriteCL5xxRegister(_DecoderLength, DecLength_Values[PixelMode],   0)) return(rc);
if (rc = WriteCL5xxRegister(_DecoderCodeOrder, DecCodeOrder_Values[PixelMode],   0)) return(rc);

/*  Part II. Device-Specific Register Initialization */

        switch(DeviceType)
        {
                case CL550:
                        /* these registers are in the 550 only */
                        if (rc = WriteCL5xxRegister(_CoderAttr,
                                        _550CoderAttr_Values[PixelMode],   0)) return(rc);
                        break;
                case CL560:
                case CL570:
                        /* these registers apply only to 560/570 */
                        if (rc = WriteCL5xxRegister(_CoderAttr,
                                        _560CoderAttr_Values[PixelMode],   0)) return(rc);
                        if (rc = WriteCL5xxRegister(_CoderSync,
                                        _560CoderSync_Values[PixelMode], 0)) return(rc);
                        if (rc = WriteCL5xxRegister(_FRMEND_Enable, 0, 0)) return(rc);
                        if (rc = WriteCL5xxRegister(_CoderRCEnable, 0, 0)) return(rc);
                        if (rc = WriteCL5xxRegister(_CoderPadding, 0, 0)) return(rc);
                        break;
        }

/*  Part III. Loading of Q Tables */

/*
        NOTE:  This code segment is written for 4-table mode, and will load
        all 4 CL5xx Q tables with data. Following reset, the CL5xx is in
        two-table, or table-swap mode. Four-table mode is entered by programming
        bit 10 of the QuantSync register (address BE00) to 1. The code that
        precedes this has already set this bit (see also CL5xx.h).

        In general, programmers should initialize the QuantSync value before
        loading the Q Tables. For most applications, programmers should use
        four-table mode. Two-table mode is reserved for video applications only,
        where frame-by-frame bank-switching of Q Tables is desired. When
        in two-table mode, only tables 1 and 2 can be loaded prior to starting
        the compress/decompress. When VSync occurs, the tables will switch,
        allowing tables 3 and 4 to be loaded. A two-table example is given
        below:

        1. Write QuantSync with bit 10 = 0   (set up two-table mode)
        2. Write Quant A/B Select = 0        (A tables - reset state)
        3. Load A Tables at addresses B800-B9FC
        4. Write Quant A/B Select = 1   (B Tables - will switch at VYSNC)
        5. Start the compression/decompression
        6. When the VSync bit in Flags register is set,
                Load B tables to addresses B800-B9FC
        7. Repeat this process every frame to load new tables or leave the
           same two tables and toggle A/B select as desired.
*/

/*    printf("Loading Q Tables\n"); */

        WriteCL5xxRegister(_QuantABSelect, 00, 0);

        if (rc = LoadTable(_QTable1, 64, CL5xx_Q_Table1, 0xffff)) return(rc);
        if (rc = LoadTable(_QTable2, 64, CL5xx_Q_Table2, 0xffff)) return(rc);
        if (rc = LoadTable(_QTable3, 64, CL5xx_Q_Table3, 0xffff)) return(rc);
        if (rc = LoadTable(_QTable4, 64, CL5xx_Q_Table4, 0xffff)) return(rc);

        if(Error)
                return;

/** Part IV. Loading of the Huffman Tables  */

        if( !NoLoad )
        {

        if (rc = WriteCL5xxRegister(_Huff_Enable, 0x1, 0)) return(rc);

        if(DeviceType == CL550)
        {
                if (rc = LoadTable(_HuffYAC_550, 704, CL5xxHuffmanYAC, 0x01ff)) return(rc);
                if (rc = LoadTable(_HuffYDC_550, 24, CL5xxHuffmanYDC, 0x01ff )) return(rc);
                if (rc = LoadTable(_HuffCAC_550, 704, CL5xxHuffmanCAC, 0x01ff)) return(rc);
                if (rc = LoadTable(_HuffCDC_550, 24, CL5xxHuffmanCDC, 0x01ff )) return(rc);
        }
        else  /* load 560/570 tables */
        {
                if (rc = LoadTable(_HuffYAC_560, 384, CL5xxHuffmanYAC, 0x03ff)) return(rc);
                if (rc = LoadTable(_HuffYDC_560, 24, CL5xxHuffmanYDC, 0x03ff )) return(rc);
                if (rc = LoadTable(_HuffCAC_560, 384, CL5xxHuffmanCAC, 0x03ff)) return(rc);
                if (rc = LoadTable(_HuffCDC_560, 24, CL5xxHuffmanCDC, 0x03ff )) return(rc);
        }

        if (rc = WriteCL5xxRegister(_Huff_Enable, 0x00, 0 )) return(rc);

        }
        return(0);
}

/*  LoadTable()
        This is a generic table-loader function for the CL5xx with a read-back
        verification step. The readback is enabled at compile-time.
*/
int LoadTable
(
        unsigned short  Base,       /* CL5xx table base */
        short           Count,      /* number of table entries to load */
        unsigned short  *Table,     /* Source Table pointer */
        unsigned short  Mask        /* Bitmask used for read-back test */
)
{
        unsigned short LoadValue, ReadBack, i;
        int rc;

        for( i = 0; i < Count; i++ )
        {
                LoadValue = Table[i];
                if (rc = WriteCL5xxRegister(Base, LoadValue, i)) return(rc);

        }
        return(0);
}

static unsigned short calculateHPeriod(short HPeriod)
{
        /* this function computes the value for the HPeriod register */

        unsigned short MyHPeriod;

        if(MasterSlave == MASTER)
        {   /* it's master mode */
                switch(PixelMode)
                {
                        case _MONO:
                                MyHPeriod = HPeriod / 2 - 1;
                                break;
                        case _444:
                        case _4444:
                                MyHPeriod = HPeriod * 2 - 1;
                                break;
                        case _RGB_422:
                        case _444_422:
                        case _422:
                                MyHPeriod = HPeriod - 1;
                                break;
                }
        }
        else
        {   /* it's slave mode */

                MyHPeriod =  HPeriod * 9 / 10;

        }
        return(MyHPeriod);
}

static unsigned short calculateHSync(short HSync, short HPeriod)
{
        unsigned short  MyHSync;

        if(MasterSlave == MASTER)
                return( HSync - 1 );        /* master mode. load HSync - 1 */
        else
                return( HPeriod >> 1 );     /* slave mode. load HPeriod / 2  */
}

static unsigned short calculateHDelay(short HDelay)
{
        /* This function computes the value of the HDelay register */

        unsigned short MyHDelay;

    switch(PixelMode)
    {
        case _MONO:
                MyHDelay = HDelay / 2;
            break;
        case _444:
        case _4444:
                MyHDelay = HDelay * 2;
            break;
        case _RGB_422:
        case _444_422:
        case _422:
                MyHDelay = HDelay;
            break;
    }
        return(MyHDelay);
}

static unsigned short calculateHActive(short HActive)
{
        /* This function returns the value for the HActive regster */

        unsigned short MyHActive;

    switch(PixelMode)
    {
        case _MONO:
                MyHActive = HActive - 1;
            break;
        case _444:
        case _4444:
                MyHActive = 4 * HActive - 1;
            break;
        case _RGB_422:
        case _444_422:
        case _422:
                MyHActive = 2 * HActive - 1;
            break;
    }
        return(MyHActive);
}

static unsigned short calculateVDelay
(
        short VDelay,
        short HActive,
        short HPeriod
)
{
        /* This function returns the value for the VDelay register */

        unsigned long MyHActive,MyHPeriod,MyVDelay;

    if(Direction)   /* if decompression mode */
    {
        MyHActive= 2 * HActive * 8;     /* Number of clock cycles*/
        MyHPeriod= 2 * HPeriod;         /* Number of clocks between HSyncs*/
        MyVDelay = VDelay
                - ((unsigned long)VideoLatency_Values[Direction][PixelMode]
                + MyHActive)/MyHPeriod;

        MyVDelay+=3;  /* wen's majic fix - will it work? or will it fry your box */
    }
    else           /* if compresssion mode */
        MyVDelay = VDelay;

        if (MyVDelay < 0)
                        MyVDelay = 0;

        return(MyVDelay);
}

static unsigned short calculateConfig(void)
{
        unsigned long MyConfig;

        MyConfig = ConfigMode_Values[PixelMode];    /* found in CL5xx.h */
        if(Direction == DECOMPRESS)
                MyConfig |= 0x0100;     /* set Dir=1 for decompression*/
        if(MasterSlave == MASTER)
                MyConfig |= 0x0008;     /* set MASTER bit */
        if(DeviceType == CL570)
                MyConfig |= 0x0200;     /* 570 Enable bit */

        MyConfig |= 0x0002;     /* Add in for End of Frame Enable - dv */

        return(MyConfig);
}

static unsigned short calculateHControl( short HActive )
{
        unsigned long MyHActive, EndX, PBI_Latency, MyHControl;

    switch(PixelMode)
        {   /* get Pixel Bus Interface Latency (mode dependent) */
        case _MONO:
        case _422:
        case _444:
        case _4444:
        case _444_422:
                PBI_Latency = 9;
            break;
        case _RGB_422:
                PBI_Latency = 12;
            break;
    }

    switch(PixelMode)
    {   /* compute mode-dependent HActive values (again) */
        case _MONO:
                MyHActive = 1 * HActive * 8;
            break;
        case _444:
        case _4444:
                MyHActive = 4 * HActive * 8;
            break;
        case _RGB_422:
        case _444_422:
        case _422:
                MyHActive = 2 * HActive * 8;
            break;
     }

     /* finally, put it all together and compute the value */
         EndX = MyHActive;
         MyHControl = EndX
                   - ((unsigned long)VideoLatency_Values[Direction][PixelMode] + 1 +
                          + PBI_Latency) % MyHActive;

        MyHControl += 3;    /* Majic fix for low-Q video operation */
                                                /* Don't ask - even I can't explain it */

        return((unsigned short)MyHControl);
}

static unsigned short calculateVControl
(
        short HActive,
        short VActive,
        short HPeriod
)
{
        unsigned long MyHActive, MyVActive, MyHPeriod;
        unsigned long EndY, PBI_Latency, MyVControl;

    switch(PixelMode)
    { /* get mode-dependent Pixel Bus Interface latency */
        case _MONO:
        case _422:
        case _444:
        case _4444:
        case _444_422:
                PBI_Latency = 9;
            break;
        case _RGB_422:
                PBI_Latency = 12;
            break;
    }

    switch(PixelMode)
    { /* get CL550 HActive value (again) */
        case _MONO:
                MyHActive = 1 * HActive * 8;
            break;
        case _444:
        case _4444:
                MyHActive = 4 * HActive * 8;
            break;
        case _RGB_422:
        case _444_422:
        case _422:
                MyHActive = 2 * HActive * 8;
            break;
    }

    switch(PixelMode)
        { /* get CL5xx HPeriod Value (again) */
        case _MONO:
                MyHPeriod = 1 * HPeriod;
            break;
        case _444:
        case _4444:
                MyHPeriod = 4 * HPeriod;
            break;
        case _RGB_422:
        case _444_422:
        case _422:
                MyHPeriod = 2 * HPeriod;
            break;
     }

     /* Finally, put it all together and compute the value */

         MyVActive = VActive * 8;   /* Number of lines       */

         EndY = MyVActive
           + ((unsigned long) VideoLatency_Values[Direction][PixelMode] + 1 +
          MyHActive)/MyHPeriod - 1;

         MyVControl = EndY
           - ((unsigned long) VideoLatency_Values[Direction][PixelMode] + 1 +
          PBI_Latency)/MyHActive;

        return( (unsigned short) MyVControl );
}

/********************************************************************
        Huffman Table Testing Functions
        This function reads back the Huffman Tables from the CL5xx device
        and compares them with what was loaded. Results are placed in a file.
*********************************************************************/

short CheckHuffmanTables(FILE *outfile)
{
        short ErrorCount;

        ErrorCount = 0;
        fprintf(outfile, "Checking Huffman Tables:\n");

        WriteCL5xxRegister(_Huff_Enable, 0xffff, 0);

        switch( DeviceType )
        {
                case    CL550:
                        ErrorCount+=TestTable(outfile,_HuffYDC_550,CL5xxHuffmanYDC,24,0x1ff);
                        ErrorCount+=TestTable(outfile,_HuffYAC_550,CL5xxHuffmanYAC,352,0x1ff);
                        ErrorCount+=TestTable(outfile,_HuffCDC_550,CL5xxHuffmanCDC,24,0x1ff);
                        ErrorCount+=TestTable(outfile,_HuffCAC_550,CL5xxHuffmanCAC,352,0x1ff);
                        break;
                case    CL560:
                        ErrorCount+=TestTable(outfile,_HuffYDC_560,CL5xxHuffmanYDC,24,0x3ff);
                        ErrorCount+=TestTable(outfile,_HuffYAC_560,CL5xxHuffmanYAC,352,0x3ff);
                        ErrorCount+=TestTable(outfile,_HuffCDC_560,CL5xxHuffmanCDC,24,0x3ff);
                        ErrorCount+=TestTable(outfile,_HuffCAC_560,CL5xxHuffmanCAC,352,0x3ff);
                        break;
                default:
                        break;
        }

        WriteCL5xxRegister(_Huff_Enable, 0, 0);

        fprintf(outfile, "==> %d errors found.\n", ErrorCount);
        return(ErrorCount);
}

static short TestTable(
        FILE *outfile,
        unsigned short DeviceBase,
        unsigned short *HuffmanTable,
        short count,
        unsigned short mask
)
{
        short i;
        short MyErrors;
        unsigned short MyIndex;
        unsigned short ReadValue;
        unsigned short ExpectedValue;

        MyIndex = DeviceBase;
        MyErrors = 0;
        for(i = 0; i < count; i++)
        {
/*                outpw(PCindex, MyIndex); */
                MyIndex += 4;
/*                ReadValue = inpw(PCdata) & mask; */
                ExpectedValue = HuffmanTable[i] & mask;

                if(ReadValue != ExpectedValue)
                {
                        MyErrors++;
                        fprintf(outfile, "%4x: Read %x, Expected %x\n",
                                MyIndex, ReadValue, ExpectedValue);
                }
        }

        if(MyErrors == 0)
                fprintf(outfile, "%4x: ok\n", DeviceBase);

        return(MyErrors);
}

/**** End of File ****/

int DumpQTable()
{
  FILE *FILEOUT;
  int i,j;

  FILEOUT = fopen("/tmp/ccubedump", "w");

  fprintf (FILEOUT, "\n\nQ_Table1:\n");
  for (i=0;i<64; i++){
      if((i%8)==0)
        fprintf(FILEOUT,"\n\n");
      fprintf(FILEOUT, " %04x ", CL5xx_Q_Table1[i]);
  } /* endfor */
  fprintf (FILEOUT, "\n\nQ_Table2:\n");
  for (i=0;i<64; i++){
      if((i%8)==0)
        fprintf(FILEOUT,"\n\n");
      fprintf(FILEOUT, " %04x ", CL5xx_Q_Table2[i]);
  } /* endfor */
  fprintf (FILEOUT, "\n\nQ_Table3:\n");
  for (i=0;i<64; i++){
      if((i%8)==0)
        fprintf(FILEOUT,"\n\n");
      fprintf(FILEOUT, " %04x ", CL5xx_Q_Table3[i]);
  } /* endfor */
  fprintf (FILEOUT, "\n\nQ_Table4:\n");
  for (i=0;i<64; i++){
      if((i%8)==0)
        fprintf(FILEOUT,"\n\n");
      fprintf(FILEOUT, " %04x ", CL5xx_Q_Table4[i]);
  } /* endfor */

  fclose(FILEOUT);
  }

int DumpHTable()
{
  FILE *FILEOUT;
  int i,j;

  FILEOUT = fopen("/tmp/huffdump", "w");

  fprintf (FILEOUT, "\n\nHuffman YDC:\n");
  for (i=0;i<24; i++){
      if ((i%8 == 0) && ( i != 0))
        fprintf(FILEOUT,"\n");
      fprintf(FILEOUT, " %04x ", CL5xxHuffmanYDC[i]);
  } /* endfor */

  fprintf (FILEOUT, "\n\nHuffman CDC:\n");
  for (i=0;i<24; i++){
      if ((i%8 == 0) && ( i != 0))
        fprintf(FILEOUT,"\n");
      fprintf(FILEOUT, " %04x ", CL5xxHuffmanCDC[i]);
  } /* endfor */

  fprintf (FILEOUT, "\n\nHuffman YAC:\n");
  for (i=0;i<384; i++){
      if ((i%8 == 0) && ( i != 0))
        fprintf(FILEOUT,"\n");
      fprintf(FILEOUT, " %04x ", CL5xxHuffmanYAC[i]);
  } /* endfor */

  fprintf (FILEOUT, "\n\nHuffman CAC:\n");
  for (i=0;i<384; i++){
      if ((i%8 == 0) && ( i != 0))
        fprintf(FILEOUT,"\n");
      fprintf(FILEOUT, " %04x ", CL5xxHuffmanCAC[i]);
  } /* endfor */


  fclose(FILEOUT);
  }

int WriteCL5xxRegister(unsigned int INDEX, unsigned int DATA, unsigned int OFFSET)
{
        int rc;
        if (rc = pio_write(CDbaseaddr+(INDEX)+((OFFSET)<<2), (DATA) ,1))
          return(rc);
        return(OK);
}

int ReadCL5xxRegister(unsigned int INDEX, unsigned int *DATA, unsigned int OFFSET)
{
        int rc;
        if (rc = pio_read(CDbaseaddr+(INDEX)+((OFFSET)<<2), DATA, 1))
          return(rc);
        return(OK);
}

/*****************************************************************************/
/*  CODECSetup()                                                             */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will setup the CL550 for either Compression OR Decompresion  */
/*                                                                           */
/*  INPUT:    ImageWidth = Width of the raw image                            */
/*            ImageHeight= Height of the raw image                           */
/*            QFactor    = Compress Q factor                                 */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int CODECSetup(short ImageWidth, short ImageHeight, short QFactor)
{
  int rc;
  unsigned int StatusVal, Dummy;
  unsigned int Flags;
  unsigned int CodecData1, CodecData2;

/* Prepare the hardware */

/*  Direction = COMPRESS;        */
  DeviceType = CL550;
  MasterSlave = MASTER;
  PixelMode = _422;      /* set up color mode for the driver */

  CL5xxImageWidth = ImageWidth;
  CL5xxImageHeight = ImageHeight;

  /* Build some Q tables */
  switch( PixelMode )
  {   /*  Notice that I build 4 tables for all cases. It's not always
                  necessary to do this, but this way I won't load garbage
                  into the CL550 - just playing it safe. */
          case _MONO:
          case _444:
          case _4444:   /* load with default-y tables only */

  MakeJPEG_Q_Table( QFactor, Default_Y_Visi_Table, JPEG_Q_Table1 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table1 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table2 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table3 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table4 );
             break;

          case _422:
          case _RGB_422:
          case _444_422:  /* Build Y and C Tables */

  MakeJPEG_Q_Table( QFactor, Default_Y_Visi_Table, JPEG_Q_Table1 );
  MakeJPEG_Q_Table( QFactor, Default_C_Visi_Table, JPEG_Q_Table2 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table1 );
  MakeCL5xx_Q_Table( JPEG_Q_Table2, CL5xx_Q_Table2 );
  MakeCL5xx_Q_Table( JPEG_Q_Table1, CL5xx_Q_Table3 );
  MakeCL5xx_Q_Table( JPEG_Q_Table2, CL5xx_Q_Table4 );
             break;
  }

  /* Now build some Huffman Tables. */
#ifdef  HUFF_TABLES_THE_HARD_WAY
  /* compute tables from BitsNValues lists - this simulates a
  case in which you would parse out tables from the JFIF file header */
  ClearTables();
  MakeCL5xxHuffmanTable(DefaultBitsNVals_YDC, CL5xxHuffmanYDC, Direction, DC);
  MakeCL5xxHuffmanTable(DefaultBitsNVals_CDC, CL5xxHuffmanCDC, Direction, DC);
  MakeCL5xxHuffmanTable(DefaultBitsNVals_YAC, CL5xxHuffmanYAC, Direction, AC);
  MakeCL5xxHuffmanTable(DefaultBitsNVals_CAC, CL5xxHuffmanCAC, Direction, AC);
#else
  DropDefaultHuffmanTables();     /* drop in from pre-calculated tables */
#endif

  /* Finally, call the board init function. It does all the work. */
  if (rc = InitPC550()) return(rc);

  /* Fix the 64th eoefficient to zero - chip bug fix */
#ifdef CL560B
  if (rc = WriteCL5xxRegister(0xb8fc,0,0)) return(rc);
  if (rc = WriteCL5xxRegister(0xb9fc,0,0)) return(rc);
#endif
  return(OK);

} /* End of CODECSetup() */
