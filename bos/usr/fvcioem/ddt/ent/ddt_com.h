/* @(#)98   1.1  src/bos/usr/fvcioem/ddt/ent/ddt_com.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:21 */

/*-------------------------------------------------------------------------
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

# define KID   "kid"       /* Kernel Interface Driver */

# define STATUS_TIMEOUT     140000  /* 140 seconds */
# define READ_TIMEOUT       10000   /* 10 seconds */

# define ERROR_LIMIT     10      /* give up after 10 errors */

typedef struct {        /* default parameters */
   char     free_mbuf;
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

/* Random number generator: */

# define RANDNUM(max)   ((unsigned int)(rand() % (unsigned int)max))

/* Test pattern types: */

# define USER_INPUT     0
# define ONES_PATTERN      1
# define ZEROES_PATTERN    2
# define MODULO_128_PATTERN   3
# define MODULO_256_PATTERN   4
# define RANDOM_PATTERN    5
