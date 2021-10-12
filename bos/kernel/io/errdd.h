/*  @(#)12        1.6  src/bos/kernel/io/errdd.h, syserrlg, bos411, 9428A410j 12/16/93 16:14:02 */ 

/*
 * COMPONENT_NAME: SYSERRLG   system error logging facility
 *
 * FUNCTIONS: common header file for the error logging device
 *			  driver		
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errids.h>

struct errc {
	int   e_state;              /* state variable using ST_ bits */
	int   e_lockword;           /* for lockl */
	int   e_sleepword_dd;       /* sleep on buffer non-empty */
	int   e_sleepword_sync;     /* sleep on buffer empty (slpsync()) */
	int   e_size;               /* size in bytes of the buffer */
	char *e_start;              /* base of the buffer */
	char *e_end;                /* end of the buffer */
	char *e_inptr;              /* errsave will write here */
	char *e_outptr;             /* errread will read here */
	char *e_stale_data_ptr;      /* any data following this pointer is invalid */
	unsigned e_over_write_count; /* number of records overwritten during overflow */  
	int e_err_count;        	/* number of entries in buffer */
	unsigned e_discard_count;	 /* number of records discarded during overflow */
	unsigned e_errid_1st_discarded; /* error id of first event discarded */
	unsigned e_errid_last_discarded; /* error id of last event discarded */
									
};

union errnv_buf {
   char buf[EREC_MAX];
   struct erec erec;
};

extern int dump_started;

#define ST_RDOPEN 0x01			/* state flags for e_state */
#define ST_SLEEP  0x02
#define ST_STOP   0x04
#define ST_SYNC   0x08
#define TIMESTAMP_LEN sizeof(int)  /* length of timestamp in error record */
#define ERECLEN_LEN   sizeof(int)  /* length of error record length field */
#define HEX_ERRID_STRLEN 8
#define DEC_DISCARD_COUNT_STRLEN 10
#define LOST_EVTS_ERRLEN sizeof(struct err_rec0) + \
			HEX_ERRID_STRLEN + \
			DEC_DISCARD_COUNT_STRLEN



/*
 * Address vector for passing data to errput().
 * This allows errput to handle discontiguous data (i.e.), to do a gather.
 */
struct	avec {
	void *p;			/* Data's address */
	ulong len;			/*   and length */
};
struct	errvec {
	int error_id;			/* Error number */
	ulong nel;			/* Number of elements */
	struct avec v[1];		/* Array of nel vectors */
};

/* Shift for bytes per word, used by xmalloc() */
#define BPWSHIFT 2


/* Macro to allocate n vectors */
#define ALOC_ERRVEC(n) struct {int error_id; ulong nel; struct avec v[(n)];}

