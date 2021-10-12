/* @(#)58	1.1  src/bos/diag/tu/sun/sun_slih.h, tu_sunrise, bos411, 9437A411a 3/28/94 17:48:41 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: io_read
 *		io_write
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
/****************************************************************************
        File:   sun_slih.h
*****************************************************************************/

#define         INTR_MASK       0xFF
#define         ALL_INTR        0x1
#define         WAIT4INTR       0x2
#define         WAIT4PIXELINTR  0x3
#define         INTR_MODE       0x0
#define         CODEC_INTR      0x1
#define         FRAMEEND_INTR   0x2
#define         SCALAR_INTR     0x3
#define         MIAMI_INTR      0x4
#define         INTRBUFFSZ      10
#define         ILLEGAL_VALUE   -1

/* MACRO DEFINE */
#define io_write(ADDR,DATA)                                             \
   if (rc=diag_io_write(handle,IOLONG,_HSBR,(ADDR),NULL,INTRKMEM))        \
         return(INTR_SUCC);                                             \
   if (rc=diag_io_write(handle,IOLONG,_MDATA, (DATA), NULL, INTRKMEM))    \
         return(INTR_SUCC)

#define io_read(ADDR,DATA)                                             \
   if (rc=diag_io_write(handle,IOLONG,_HSBR,(ADDR),NULL,INTRKMEM))        \
         return(INTR_SUCC);                                             \
   if (rc=diag_io_read(handle,IOLONG,_MDATA,(&DATA), NULL, INTRKMEM))    \
         return(INTR_SUCC)

typedef struct sun_intr_data  {
   char   filler;
   unsigned short  ScalarIntrStatus;
   unsigned short  MiamiIntrStatus;
   unsigned short  CodecIntrStatus;
   unsigned short  dataarea[INTRBUFFSZ];
} ;
