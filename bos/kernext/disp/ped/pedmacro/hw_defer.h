/* @(#)80       1.2  src/bos/kernext/disp/ped/pedmacro/hw_defer.h, pedmacro, bos411, 9428A410j 3/17/93 19:26:58 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************/
/*                                                              */
/*      PEDERNALES HW DEFER PROGRAMMING INTERFACE               */
/*                                                              */
/****************************************************************/


#ifndef _H_MID_HW_DEFER
#define _H_MID_HW_DEFER

/************************************************************************
*************************************************************************
A general definition of abbreviations:

        ACT     =       active
        BG      =       background
        CURS    =       cursor
        E       =       evaluate
        FB      =       frame buffer
        FG      =       foreground
        K       =       kernel
        L       =       length
        OL      =       overlay buffer
        PIX     =       pixel
        PLINE   =       polyline
        PMRKR   =       polymarker
        PNT     =       point
        RD      =       read
        S       =       "set" or "store"
        SE      =       structure element (FIFO command element)
        U       =       user
        UL      =       underlay buffer
        WR      =       write
        ZB      =       Z buffer
        PCB     =       priority command buffer path
        IND     =       indirect address path
        FIFO    =       circular buffer fifo path
        HOST    =       host register path
        CARD    =       card register path
        ADR     =       address
        DATA    =       data
        COMO    =       communication
        CTL     =       control
        FREE    =       free space
        INTR    =       interrupt mask
        STAT    =       status register
        REL     =       relative


Some terms that are common to the macros:

"evaluate"      means this macro returns a value or an expression.

"flush"         means this macro moves a buffer into a FIFO

"purge"         means this macro deletes the contents of a buffer by
                resetting the pointer to the head of the malloc-ed area

"write"         means this is a "basic function" macro
"read"          means the same



/*************************************************************************
**************************************************************************
** Things which define the standard operation of the deferral and the   **
** build buffers.                                                       **
**************************************************************************
*************************************************************************/




/****************************************************************
* For each of the 16 possible sizes of the array of data, the   *
* macros will do a structure to structure copy of the data      *
* from the array given by pData into the pBuf.  The typecasting *
* causes the structure to be declared volatile, which makes the *
* xlc compiler recognize a store string operation               *
*****************************************************************/

/*
 * Define macro to set the Buffer pointer to the proper type
 */
#define MID_SET_BUFFER_PTR(pBUFFER, pTYPE, pDest )              \
        pTYPE  pBUFFER = (pTYPE) pDest ;

/*-------------------------------------------------------------
 * Stuff one data item into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_1(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER1, pMID_1param , pDest)    \
        *(pMIDBUFFER1) = *( (pMID_1param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff two data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_2(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER2, pMID_2param , pDest)    \
        *(pMIDBUFFER2) = *( (pMID_2param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff three data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_3(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER3, pMID_3param , pDest)    \
        *(pMIDBUFFER3) = *( (pMID_3param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff four data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_4(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER4, pMID_4param , pDest)    \
        *(pMIDBUFFER4) = *( (pMID_4param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff five data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_5(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER5, pMID_5param , pDest)    \
        *(pMIDBUFFER5) = *( (pMID_5param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff six data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_6(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER6, pMID_6param , pDest)    \
        *(pMIDBUFFER6) = *( (pMID_6param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff seven data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_7(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER7, pMID_7param , pDest)    \
        *(pMIDBUFFER7) = *( (pMID_7param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff eight data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_8(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER8, pMID_8param , pDest)    \
        *(pMIDBUFFER8) = *( (pMID_8param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff nine data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_9(pDest , pSrc)                           \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER9, pMID_9param , pDest)    \
        *(pMIDBUFFER9) = *( (pMID_9param)pSrc );                \
}


/*-------------------------------------------------------------
 * Stuff ten data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_10(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER10, pMID_10param , pDest)  \
        *(pMIDBUFFER10) = *( (pMID_10param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff eleven data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_11(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER11, pMID_11param , pDest)  \
        *(pMIDBUFFER11) = *( (pMID_11param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff twelve data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_12(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER12, pMID_12param , pDest)  \
        *(pMIDBUFFER12) = *( (pMID_12param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff thirteen data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_13(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER13, pMID_13param , pDest)  \
        *(pMIDBUFFER13) = *( (pMID_13param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff fourteen data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_14(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER14, pMID_14param , pDest)  \
        *(pMIDBUFFER14) = *( (pMID_14param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff fifteen data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_15(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER15, pMID_15param , pDest)  \
        *(pMIDBUFFER15) = *( (pMID_15param)pSrc );              \
}


/*-------------------------------------------------------------
 * Stuff sixteen data items into the user BUFFER using store string.
--------------------------------------------------------------- */
#define MID_WR_BUFFER_16(pDest , pSrc)                          \
{                                                               \
        MID_SET_BUFFER_PTR(pMIDBUFFER16, pMID_16param , pDest)  \
        *(pMIDBUFFER16) = *( (pMID_16param)pSrc );              \
}


/*----------------------------------------------------------------------
 * This is the generic BUFFER stuffer.  It takes as input an array of data
 * items up to 16000 words (64K bytes) long and breaks
 * it down to stuff into the BUFFER
 *
 *   Input :
 *      unsigned long * data;    (16000 words (64K bytes))
 *      num_data_items = number of words actually have data (<= 16000)
 *
 *   Process :
 *      1) stuff as many 8 word chunks into the BUFFER as possible
 *      2) stuff the remaining 1-7 words into the BUFFER (one operation)
 ----------------------------------------------------------------------*/


#define MID_WR_BUFFER(pBuffer, pSrc, num_data_items)            \
{                                                               \
        int             num_8, residue;                         \
        int             i;                                      \
        ulong *         ptr = (ulong * )pSrc;                   \
        ulong *         pDest = (ulong *)pBuffer;               \
                                                                \
        num_8 = num_data_items >> 3;                            \
        for(i=0; i<num_8; i++)                                  \
        {                                                       \
                MID_WR_BUFFER_8(pDest , ptr);                   \
                pDest += 8;                                     \
                ptr += 8;                                       \
        }                                                       \
                                                                \
        residue = num_data_items & 7;                           \
                                                                \
        switch( residue )                                       \
        {                                                       \
                case 1:                                         \
                        MID_WR_BUFFER_1(pDest , ptr)            \
                        pDest += 1;                             \
                        break;                                  \
                case 2:                                         \
                        MID_WR_BUFFER_2(pDest , ptr)            \
                        pDest += 2;                             \
                        break;                                  \
                case 3:                                         \
                        MID_WR_BUFFER_3(pDest , ptr)            \
                        pDest += 3;                             \
                        break;                                  \
                case 4:                                         \
                        MID_WR_BUFFER_4(pDest , ptr)            \
                        pDest += 4;                             \
                        break;                                  \
                case 5:                                         \
                        MID_WR_BUFFER_5(pDest , ptr)            \
                        pDest += 5;                             \
                        break;                                  \
                case 6:                                         \
                        MID_WR_BUFFER_6(pDest , ptr)            \
                        pDest += 6;                             \
                        break;                                  \
                case 7:                                         \
                        MID_WR_BUFFER_7(pDest , ptr)            \
                        pDest += 7;                             \
                        break;                                  \
        }                                                       \
}

/************************************************************************
*       The manipulation of the deferral buffer is based on some        *
*       pointer maintenance macros defined below and on the generic     *
*       buffer write macro defined above.                               *
************************************************************************/

        /*----------------------------------------------*
         *                                              *
         *      DEFER BUFFER PURGE                      *
         *                                              *
         *----------------------------------------------*/

#ifdef  MID_DD
#define MID_DEFERBUF_PURGE

#define MID_DEFERBUF_FLUSH

#define MID_WR_DEFERBUF( pData , num_words )                            \
        MID_WR_FIFO( pData , num_words )

#define MID_WRITE_TO_FIFO( pData , num_words )                          \
        MID_WR_FIFO( pData , num_words )

#else   /* MID_DD */

#define MID_DEFERBUF_PURGE                                      \
        {                                                       \
        MID_DEFERBUF_LEN = 0;                                   \
        MID_DEFERBUF = MID_DEFERBUF_HD;                         \
        }

        /*----------------------------------------------*
         *                                              *
         *      DEFER BUFFER FLUSH                      *
         *                                              *
         *----------------------------------------------*/

#define MID_DEFERBUF_FLUSH                                              \
{                                                                       \
                MID_WR_FIFO( MID_DEFERBUF_HD, MID_DEFERBUF_LEN )        \
                MID_DEFERBUF_PURGE                                      \
}

        /*----------------------------------------------*
         *                                              *
         *      DEFER BUFFER WRITE                      *
         *                                              *
         *----------------------------------------------*/

#define MID_WR_DEFERBUF( pData , num_words )                            \
{                                                                       \
        if ( (MID_DEFERBUF_LEN + num_words) > MID_DEFER_CAPACITY )      \
        {                                                               \
                MID_DEFERBUF_FLUSH;                                     \
        }                                                               \
        MID_WR_BUFFER( MID_DEFERBUF , pData , num_words );              \
        MID_DEFERBUF_LEN += num_words;                                  \
        MID_DEFERBUF     += num_words;                                  \
}


#define MID_WRITE_TO_FIFO( pData , num_words )                          \
{                                                                       \
        if (MID_DEFERBUF_LEN > 0)                                       \
          MID_DEFERBUF_FLUSH;                                           \
        MID_WR_FIFO( pData , num_words );                               \
}
#endif

#endif  /* _H_MID_HW_DEFER */
