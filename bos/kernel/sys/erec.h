/*  @(#)28        1.19  src/bos/kernel/sys/erec.h, syserrlg, bos411, 9428A410j 9/28/93 10:59:05 */ 

/*
 * COMPONENT_NAME:            include/sys/erec.h
 *
 * FUNCTIONS:  header file for system error log buffer formats
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_EREC
#define _H_EREC

#include <sys/err_rec.h>

struct erec0 {
	int		erec_len;
	int		erec_timestamp;
	int		erec_rec_len;	/* Length of error data */
	int		erec_symp_len;	/* Symptom data's length */
};

/*
 * This is the format of the symptom data in err_data (i.e.),
 * the format of err_data.data.
 */
struct sympt_data0 {
	unsigned long probe_magic;	/* Magic # */
	int flags;			/* Symptom string flags */
	int nsskwd;			/* # of keywords */
};
struct sympt_data {
	unsigned long probe_magic;	/* Magic # */
	int flags;			/* Symptom string flags */
	int nsskwd;			/* # of keywords */
	char sympdata[1];		/* The keywords and data */
};

/*
 * See probe.h for questions about this discussion.
 *
 * MAX_SYMPTOM_DATA is the maximum data that can be passed.
 * It applies only to the sskwd data, not to probe_rec0.
 * It is calculated by adding the maximum string lengths and the 
 * keywords themselves (1 word per keyword).
 * The string lengths are calculated keeping in mind that there must
 * be a terminating NULL byte, and that we want to start each keyword
 * on a word boundary.
 * The string lengths for the 5 keywords not involved in the symptom data
 * are longname 32, owner 20, applid 12, desc 84, sev 4, yields 152.
 * There are up to 8 symptom string keywords, the maximum string length
 * is 11 (rounds to 12), therefore the max here is 96.
 * Finally, we must add in 13 times 4 for the 13 keywords, or 52.
 * The total is then 152+96+52 or 300.
 */
#define SSDATA_MAX 300		/* Maximum data in symptom string. */
#define SSREC_MAX (SSDATA_MAX + sizeof(struct sympt_data0))/* Max symptom record */

struct erec {
	int		erec_len;
	int		erec_timestamp;
	int		erec_rec_len;	/* Length of error data */
	int		erec_symp_len;	/* Symptom data's length */
	struct err_rec	erec_rec;	/* Error data (always present) */
	struct sympt_data erec_symp;	/* Symptom data */
};

/* Current maximum size of an individual buffer entry.
 * Includes the header, 1 error log record and 1 symptom string record.
 */
#define EREC_MAX (ERR_REC_MAX_SIZE + SSREC_MAX + sizeof(struct erec0))

#define	ERRIOC      ('E'<<8)
#define ERRIOC_STOP (ERRIOC|0x01)	/* return EOF to error demon */
#define ERRIOC_SYNC (ERRIOC|0x02)	/* sleep till buf. drained by errdemon */
#define ERRIOC_STAT (ERRIOC|0x03)	/* /dev/error status */
#define ERRIOC_BUFSET  (ERRIOC|0x04)	/* set the buffer size */
#define ERRIOC_BUFSTAT (ERRIOC|0x05)	/* get the buffer size */

#define MDEV_ERROR    0				/* /dev/error */
#define MDEV_ERRORCTL 1				/* /dev/errorctl */

/*
 * Errlog buffer control structure in errdd.
 * Also used by errdead to extract errlog entries from a dump.
 */
#define ERRBUFSIZE 8192  /* make buffer size multiple of page size */ 

/*
 * This structure is used be errdead to detect wraparound in the
 * error log buffer. It is separated from errc so that errc can change
 * without affecting errc_io and errdead.
 */
struct errc_io {
	char     *e_start;			/* base of the buffer */
	char     *e_end;			/* end of the buffer */
	char     *e_inptr;			/* errsave will write here */
	char     *e_outptr;			/* errread will read here */
	char *   e_stale_data_ptr;	/* any data following this pointer is invalid */
	unsigned e_over_write_count;	/* number data recs overwritten since boot */
	unsigned e_err_count;		/* number entries in buffer */
	unsigned e_discard_count;	 /* number of records discarded during overflow */
	unsigned e_errid_1st_discarded; /* error id of first event discarded */
	unsigned e_errid_last_discarded; /* error id of last event discarded */

};

/*
 * trace "sub-hooks"
 * The lower lower 16 bits of the hookword contain the sub-hook in the
 *   upper byte and an 8 bit error code (u_error) in the lower byte.
 */
#define ERRTRC_ERROPEN     0x01
#define ERRTRC_ERRCLOSE    0x02
#define ERRTRC_ERRIOCTL    0x03
#define ERRTRC_ERRREAD2    0x04
#define ERRTRC_ERRREAD     0x05
#define ERRTRC_ERRWRITE    0x06
#define ERRTRC_ERRPUT      0x07
#define ERRTRC_ERRPUT_OVF  0x08
#define ERRTRC_LOCKL       0x09
#define ERRTRC_UNLOCKL     0x0A
#define ERRTRC_ERRDEMON    0x10

/*
 * tracehook interface macros
 */

#ifdef TRCHK
#define ERR_TRCHK(name,dw) \
	TRCHK(HKWD_RAS_ERRLG  | (ERRTRC_##name << 8) | ((dw) & 0x0FF))

#define ERR_TRCHKL(name,dw,value2) \
	TRCHKL(HKWD_RAS_ERRLG | (ERRTRC_##name << 8) | ((dw) & 0x0FF),value2)

#define ERR_TRCGEN(name,dw,len,buf) \
	TRCGEN(0,HKWD_RAS_ERRLG,(ERRTRC_##name << 8) | ((dw) & 0x0FF),len,buf)
#endif

/*
 * error device driver status info.
 * Used by errdemon to know when it is first invoked.
 */

struct errstat {
	int es_opencount;		/* number of times /dev/error has been opened */
	int es_errcount;		/* number of errsave()/errlog() calls */
	int es_ovfcount;		/* number of errsave overflows */
	int es_readerr;			/* number of errread errors */
	int es_fill[13];
};

#endif /* _H_EREC */

