/* @(#)08 1.1 src/bos/usr/fvcioem/ddt/fddi/ddt_com.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:45 */

/*--------------------------------------------------------------------------
*
*             DDT_COM.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool Common Definitions.
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

/*
 * For FDDI there is no distinction between stderr and stdout. The
 * testcases that use this driver tester want to see things as
 * happen and if you use stderr and stdout then the ordering is lost.
 * To distinguish stderr from stdout change the following define.
 */
#define STDERR stdout

# define KID   "kid"       /* Kernel Interface Driver */

# define STATUS_TIMEOUT     2000 /* 2 seconds */
# define TX_ACK_TIMEOUT     1000    /* 1 seconds */
# define READ_TIMEOUT       3000 /* 3 seconds */

# define ERROR_LIMIT     10      /* give up after 10 errors */

typedef struct
{
   char     use_tx_ext;
   char     use_rcv_ext;
   char     free_mbuf;
   char     ack_tx_done;
   char     funky_mbuf;
   char     poll_reads;
   char     mbuf_thresh[10];
   char     mbuf_wdto[10];
   char     default_dest[20];
   char     default_src[20];
   char     default_netid[8];
   char     default_netid_size[4];
   char     default_size[8];
   char     default_writes[8];
   char     default_reads[8];
   char     statusrate[8];
   char     default_trf[8];
   int      default_frames_req;
} PROFILE;

/*---------------------------------------------------------------------------*/
/*                                  MACROS                                   */
/*---------------------------------------------------------------------------*/

/* General Purpose: */

#ifndef MAX
# define MAX(a, b) (((b) < (a)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

# define ODD( val )     ((val) & 1)    /* TRUE if odd # */
# define EVEN( val )    (!((val) & 1))    /* TRUE if even # */

# define LO_BYTE(x)  ((unsigned char)(x))
# define HI_BYTE(x)  ((x) >> 8)

/* This makes driver calls easier to find: */

# define WRITE(fd, bufptr, len, ext)   writex(fd, bufptr, len, ext)
# define READ(fd, bufptr, len)      read(fd, bufptr, len)
# define READX(fd, bufptr, len, ext)   readx(fd, bufptr, len, ext)
# define IOCTL(fd, cmd, arg)     ioctl(fd, cmd, arg)


/* Test pattern types: */

# define USER_INPUT     0
# define ONES_PATTERN      1
# define ZEROES_PATTERN    2
# define MODULO_128_PATTERN   3
# define MODULO_256_PATTERN   4
# define RANDOM_PATTERN    5
