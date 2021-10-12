/* @(#)43	1.2  src/bos/diag/tu/siosl/kbddev/misc.h, tu_siosl, bos411, 9428A410j 12/17/93 12:25:42 */
/*
 * COMPONENT_NAME: TU_SIOSL (misc.h include for keyboard test units)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <nl_types.h>
/* Define error codes for Salmon system I/O and adapter TU's */

#define	IO_ERROR	(-1)	/* returned by ioctl()	*/

#define	KBD_RESET_LOGIC_ERROR   10
#define	KBD_LOGIC_ERROR         11
#define	KBD_TX_ERROR            12
#define	KBD_RX_ERROR            13
#define	KBD_DATA_COMPARED_ERROR 15
#define	KBD_CMD_NOT_ACK_ERROR   17
#define	KBD_EXTERNAL_BAT_ERROR  18
#define	KBD_RESET_CMD_ERROR     19
#define	SPK_LOGIC_ERROR         30

#define	WRONG_TU_NUMBER        256
#define	FUSE_BAD_ERROR          20

#define	FOREVER	while(1)

#define	BLANK	' '
#define	TAB		'\t'
#define EOS		'\0'

#define SUCCESS          0
#define FAIL             1


/* Define print macros */ 

#ifdef SUZTESTING
#define PRINT		printf
#else
#define PRINT
#endif

/* Set up structures for kbd testing */

typedef struct TUCB_SYS {
	nl_catd	catd;
	long	ad_mode;
	int	kbtype;
	int	mach_fd; /* Machine DD file descriptor */
};

typedef struct TUCB {
  struct tucb_t header;
  struct TUCB_SYS tuenv;
};

