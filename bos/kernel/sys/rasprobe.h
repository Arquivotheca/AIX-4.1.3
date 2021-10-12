#ifndef _h_PROBE
/* @(#)32	1.3  src/bos/kernel/sys/rasprobe.h, syserrlg, bos412, 9443A412c 10/24/94 13:42:01 */

/*
 * COMPONENT_NAME:            (syserrlg)
 *
 * FUNCTIONS:  header file for system error log symptom data
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
#define _h_PROBE

#include <ctype.h>
#include <sys/err_rec.h>

/*
 * Keyword data, one per keyword.
 */
struct sskwd {
	int sskwd_id;			/* Keyword identifier */
	union {
		unsigned long sskwd_v;	/*   Value */
		void * sskwd_p;		/*   Data pointer. */
	} sskwd_u;
};
#define	SSKWD_VAL sskwd_u.sskwd_v
#define SSKWD_PTR sskwd_u.sskwd_p

/*
 * Probe structure.
 */
struct probe_rec0 {
	unsigned long probe_magic;	/* Magic # */
	struct err_rec *erecp;		/* Error record ptr. */
	size_t erecl;			/*   length */
	int flags;			/* Symptom string flags */
	int nsskwd;			/* # of keywords */
};
struct probe_rec {
	unsigned long probe_magic;	/* Magic # */
	struct err_rec *erecp;		/* Error record ptr. */
	size_t erecl;			/*   length */
	int flags;			/* Symptom string flags */
	int nsskwd;			/* # of keywords */
	struct sskwd sskwds[1];		/* at least 1 keyword */
};
typedef struct probe_rec probe_t;

/* For allocation of a probe_rec */
#define PROBE_DATA(nam,n) struct {\
	unsigned long probe_magic;\
	struct err_rec *erecp;\
	size_t erecl;\
	int flags;\
	int nsskwd;\
	struct sskwd sskwds[(n)];\
} nam = {PROBE_MAGIC, 0, 0, 0, (n)}

/* This must be in the probe_magic field. */
#define SYSPROBE_MAGIC	0xffdc0001		/* Magic number */
#define CUSTPROBE_MAGIC 0xfadc0001		/* Magic number */

/*
 * Flag definitions for probe() 
 * All bit positions not defined should be set to zero.
 */
/*
 * The SSNOSEND flag governs the value of "Reportable" under the
 * symptom data in the error report.  This flag may be used to
 * indicate that a problem is not to be sent to an external collector,
 * because, perhaps, it's just for debugging.
 */
#define SSNOSEND	0x80000000	/* Don't send outside */
/* SSFLAGS_MASK Selects the valid, specifyable flags. */
#define SSFLAGS_MASK	(SSNOSEND)

/*
 * Probe Interface
 */
/*
 * rc = probe(probe_rec *)
 */
#ifdef _NO_PROTO
#ifdef _KERNEL
extern int kprobe();			/* Kernel, non-prototyped */
#else /* ! _KERNEL */
extern int probe();			/* Application, non-prototyped */
#endif /* _KERNEL */
#else /* ! _NO_PROTO */
#ifdef _KERNEL
extern int kprobe(struct probe_rec *probe_p);	/* Kernel, prototyped */
#else /* ! _KERNEL */
extern int probe(struct probe_rec *probe_p);	/* Application, prototyped */
#endif /* _KERNEL */
#endif /* _NO_PROTO */


/* Maximum number of keywords allowed. */
#define SSKWD_MAX 13		/* overall maximum */
#define MAX_SYMPTOM_KWDS 8	/* Maximum symptom keywords */

/* Keyword identifiers (sskwd_id) */
#define SSKWD_LONGNAME	  1 	/* Product name up to 30 char */
#define SSKWD_OWNER	  2	/* Owner up to 16 char */
#define SSKWD_PIDS	  3	/* Product id, 9 chars */
#define SSKWD_LVLS	  4	/* level (format VRM..) */
#define SSKWD_APPLID	  5	/* Application id, 8 chars */
#define SSKWD_REPORTABLE  6	/* Compliment of SSNOSEND flag */
#define SSKWD_INTERNAL    7	/* 1 if magic no = SYSPROBE_MAGIC */
#define SSKWD_PCSS	 16	/* Probe id, 8 chars */
#define SSKWD_DESC	 17	/* Probe description, 80 chars */
#define SSKWD_SEV	 18	/* Severity, 1-4 */
#define SYMP_CODEPT	 20	/* Reserved (Code point 0x14) */
#define SSKWD_AB	 32	/* Abend code, unsigned long */
#define SSKWD_ADRS	 33	/* Address, void * */
#define SSKWD_DEVS	 34	/* Device type, 6 chars */
#define SSKWD_FLDS	 35	/* Field, 9 chars */
#define SSKWD_MS	 36	/* Msg. no., 11 chars */
#define SSKWD_OPCS	 37	/* op code, 8 chars */
#define SSKWD_OVS	 38	/* Overlaid storage, 9 chars */
#define SSKWD_PRCS	 39	/* Return code, int */
#define SSKWD_REGS	 40	/* Reg name, 4 chars */
#define SSKWD_VALU	 41	/* Value (usually of reg.), unsigned long */
#define SSKWD_RIDS	 42	/* Resource or module id, 8 chars */
#define SSKWD_SIG	 43	/* Signal, int */
#define SSKWD_SN	 44	/* Serial Number, 7 chars */
#define SSKWD_SRN	 45	/* Service Req. Number, 9 chars */
#define SSKWD_WS	 46	/* Coded wait, 10 chars */

/*
 * These lengths are for the string data for their corresponding
 * keywords.  For example, SSKWD_PIDS take up to 9 characters,
 * PIDS_LEN.  Note these lengths do NOT include a trailing 0x0.
 */
#define LONGNAME_LEN 30
#define OWNER_LEN 16
#define PIDS_LEN 9
#define LVLS_LEN 3
#define APPLID_LEN 8
#define PCSS_LEN 8
#define DESC_LEN 80
#define DEVS_LEN 6
#define FLDS_LEN 9
#define MS_LEN 11
#define OPCS_LEN 8
#define OVS_LEN 9
#define REGS_LEN 4
#define RIDS_LEN 8
#define SRN_LEN 9
#define WS_LEN 10
#define SN_LEN 7

#endif /* NOT _h_PROBE */
