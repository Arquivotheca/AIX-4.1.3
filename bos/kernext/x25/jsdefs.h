/* @(#)87  1.2  src/bos/kernext/x25/jsdefs.h, sysxx25, bos411, 9428A410j 6/15/90 18:51:43 */
#ifndef _H_JSDEFS
#define _H_JSDEFS
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */


#if !defined(byte)
typedef unsigned char byte;         /* use for values between 0 and 255      */
#endif

#if !defined(bool)
typedef char bool;                  /* use for TRUE and FALSE                */
#endif

union local_max_align_t             /* used for pointer to anything          */
{
  int word;
  double dbl;
};
typedef union local_max_align_t max_align_t;


struct local_word_align_t           /* used to return pointers to control    */
{                                   /* blocks                                */
  int word;
};
typedef struct local_word_align_t word_align_t;

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifdef DOS
#define PC_DOS                      /* are we running on DOS ?               */
#else
#ifdef aiws                         /* are we running on 2.2 ?               */
#define AIX2_2
#else
#define AIX3_0                      /* or is it 3.0                          */
#endif
#endif

#ifdef AIX2_2
typedef word_align_t *generic_ptr_t;
typedef char *void_ptr_t;
#else
typedef void *generic_ptr_t;
typedef void *void_ptr_t;
#endif

#ifndef NULL
#define NULL (0)
#endif

#ifndef MAX
#define MAX(a,b) 		((a)<(b) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a,b) 		((a)>(b) ? (b) : (a))
#endif
#ifndef ABS
#define ABS(x)			((x)>=0 ? (x) : -(x))
#endif
/* cstd - */
#define OFFSETOF(member,type) ((unsigned)&(((type *)0)->member))
/* cstd +*/
#define ARRAYSIZEOF(array) (sizeof(array)/sizeof((array)[0]))

/* X25USER ON */

/*****************************************************************************/
/* X25_MAX_NUA_DIGITS           maximum number of digits in an NUA           */
/* X25_MAX_EXT_ADDR_DIGITS      maximum number of digits in an extended      */
/*                              address                                      */
/* X25_MAX_FACILITIES_LENGTH    maximum number of bytes of facilities        */
/* X25_MAX_CUD_LENGTH           maximum number of bytes of call user data    */
/*                              allowed in a packet.  This length can only   */
/*                              be achieved if fast select is set. Otherwise */
/*                              maximum is X25_MAX_NON_FASTSEL_CUD_LENGTH    */
/* X25_MAX_CARDS                number of supported adapters                 */
/* X25_MAX_NON_FASTSEL_CUD_LENGTH maximum number of bytes of call user data  */
/*                              without fast select                          */
/* X25_MAX_PACKET_SIZE          maximum number of bytes in a data packet     */
/* X25_MAX_ASCII_ADDRESS_LENGTH is the maximum length of the string used to  */
/*                              hold X.25 NUAs in call buffers               */
/*****************************************************************************/
#define X25_MAX_NUA_DIGITS             (15)
#define X25_MAX_EXT_ADDR_DIGITS        (40)
#define X25_MAX_FACILITIES_LENGTH      (109)
#define X25_MAX_CUD_LENGTH             (128)
#define X25_MAX_CARDS                  (4)
#define X25_MAX_NON_FASTSEL_CUD_LENGTH (16)
#define X25_MAX_PACKET_SIZE            (4096)
#define X25_MAX_ASCII_ADDRESS_LENGTH   (20)
/* X25USER OFF */

#endif
