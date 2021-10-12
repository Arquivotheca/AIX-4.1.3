/* @(#)60       1.18.1.1  src/bos/kernel/sys/audit.h, syssaud, bos411, 9428A410j 10/15/93 12:49:07 */
/*
 * COMPONENT_NAME: (SYSAUDIT) Audit Management
 *
 * FUNCTIONS: audit.h support for system auditing
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_AUDIT
#define _H_AUDIT

#include <sys/types.h>
#include <sys/user.h>
#include <sys/tcb.h>
/*
 * this file provides definitions for the
 * audit system call interfaces
 */

/*
 * commands for audit() system call
 */
#define	AUDIT_OFF	0
#define	AUDIT_ON	1
#define	AUDIT_QUERY	2
#define	AUDIT_RESET	4
#define AUDIT_NO_PANIC	8
#define AUDIT_PANIC	16

/*
 * commands for auditbin() system call
 */
#define AUDIT_WAIT	0x01
#define AUDIT_EXCL	0x02

/*
 * commands for auditproc() system call
 */
#define AUDIT_QEVENTS	1
#define AUDIT_EVENTS	2
#define AUDIT_QSTATUS	3
#define	AUDIT_STATUS	4
#define AUDIT_RESUME	0
#define AUDIT_SUSPEND	1

/*
 * comands for auditevents() system call
 */
#define AUDIT_GET	0
#define AUDIT_SET	1
#define AUDIT_LOCK	3

/*
 * structure for auditevents() system call
 */
struct	audit_class
{
	char	*ae_name;	/* name of this administrative event */
	char	*ae_list;	/* ptr to list of null terminated base */
				/* event names, terminated by null string */
	int	ae_len;		/* length of names in ae_list (including */
				/* all nulls) */
};


/*
 * an audit trail consists of a sequence of bins.
 * each bin starts with a bin head, and must be terminated by
 * a bin tail before other bins can be appended to the trail
 */
struct aud_bin
{
	ushort_t bin_magic;
#define	AUDIT_MAGIC	0xf0f0
	u_char	 bin_version;
#define	AUDIT_VERSION	1
	u_char	 bin_tail;
#define	AUDIT_HEAD	0
#define	AUDIT_BIN_END	1
#define	AUDIT_TRAIL	2
 	ulong_t	 bin_len;	/* unpacked length of bin's records, if this */
				/* is non-zero, the bin has a tail record */
	ulong_t	 bin_plen;	/* current length of bin's records (may be */
				/* packed) */
	ulong_t	 bin_time;	/* timestamp at which head/tail was written */
	ulong_t	 bin_reserved1;
	ulong_t	 bin_reserved2;
};


/* auditobj parameters */

struct o_event
{
	int	o_type;		/* type of object (AUDIT_FILE,..) */
#define AUDIT_FILE 	0x01
	char	*o_name;	/* object to be audited */
	char	*o_event[16];   /* event names indexed by access */
#define AUDIT_READ 	0
#define AUDIT_WRITE 	1
#define AUDIT_EXEC 	2
}; 

/* auditpack parameters */
#define AUDIT_PACK	0
#define AUDIT_UNPACK	1

/* audit device ioctl */
#define AIO_EVENTS	0x1

/* 
 * aud_rec ah_status values 
 */
#define AUDIT_OK		0
#define AUDIT_TCB_MOD		0x99
#define AUDIT_FAIL		0x01
#define AUDIT_FAIL_AUTH		0x03
#define AUDIT_FAIL_PRIV		0x05
#define AUDIT_FAIL_ACCESS	0x09
#define AUDIT_FAIL_DAC		0x19

struct aud_rec { 
	ushort_t ah_magic;	/* magic value new format = AUD_REC_MAGIC */
#define	AUDIT_HDR0	0
#define	AUDIT_HDR1	1
	ushort_t ah_length;	/* length of tail of this record */ 
	char	ah_event[16];	/* event name with null terminator */ 
	ulong_t	ah_result;	/* the audit status - see auditlog for values */
	uid_t	ah_ruid;	/* real user id */
	uid_t	ah_luid;	/* login user id */
	char	ah_name[MAXCOMLEN];	/* program name with null terminator */
	pid_t	ah_pid;		/* process id */
	pid_t	ah_ppid;	/* process id of parent */
        tid_t   ah_tid;         /* thread id */
	time_t	ah_time;	/* time in secs */
	long	ah_ntime;	/* nanosecs offset from ah_time */
	/* record tail follows */
};

struct auddata {		/* audit relevant data */
	ushort	svcnum;		/* name index from audit_klookup */
	ushort	argcnt;		/* number of arguments stored */
	int	args[10];	/* Parameters for this call */
	char	*audbuf;	/* buffer for misc audit record */
	int	bufsiz;		/* allocated size of pathname buffer */
	int	buflen;		/* actual length of pathname(s) */
	ushort	bufcnt;		/* number of pathnames stored */
	ulong	status;		/* audit status bitmap */
};

#endif /* _H_AUDIT */
