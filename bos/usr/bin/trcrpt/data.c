static char sccsid[] = "@(#)77  1.23  src/bos/usr/bin/trcrpt/data.c, cmdtrace, bos41J, 9513A_all 2/21/95 10:02:40";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: global data for trcrpt
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information   
 */

#include <sys/types.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <time.h>
#include "rpt.h"
#include "td.h"

unsigned Hookword;
/* Time conversion factor, see rpt.h */
double cnvtfact = CNVTFACT_DEF;
/* time wrap, see rpt.h */
double time_wrap = RISC_WRAP;
/* end ms defect 134388 */
int Timestamp;
int Logtraceid;
struct version Version;
int Tmpltidx;
int Toffset;
struct tindex Tindexes[NHOOKIDS];
short Traceids[MAXTEMPLATES];	/* index into Tindexes */
int Ntemplates;

char Numbuf1[NUMBUFSIZE];
char Numbuf2[NUMBUFSIZE];

union td *Tdp0;

unsigned char Condhookids[NHOOKIDS/8];

FILE *Rptfp;
FILE *Logfp;
int   Logidx0;
int   Logidx;
FILE *Tmpltfp;
FILE *Listfp;

int Starttime;
int Endtime;

int Listflg;
int jflag;
int quietflag;
int rawflag;
int nohdrflag;
int checkflag;		/* scan trcfmt for syntax errors only */
int semaflg;		/* set with the routine  trcupdate (semaphore get) */
int compressflg;	/* set when the file /etc/trcfmt is uncompressed in
			 * the routine updtmain (trace package is not installed)
 			*/
int sid;                /* semaphore id */
int Tid;		/* thread id */
int Cpuid;             /* cpu id */
int Pri;            
int Threadflg;		/* set if the trcfile was done with a threaded system */
char *Execname_ref;

			/* static initialization in case of no-existing errno.h */
char *Errorstr[NERRNO] = {
/*   0 */ NULL, "EPERM", "ENOENT", "ESRCH", "EINTR",
/*   5 */ "EIO", "ENXIO", "E2BIG", "ENOEXEC", "EBADF",
/*  10 */ "ECHILD", "EAGAIN", "ENOMEM", "EACCES", "EFAULT",
/*  15 */ "ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV",
/*  20 */ "ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE",
/*  25 */ "ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE",
/*  30 */ "EROFS", "EMLINK", "EPIPE", "EDOM", "ERANGE",
/*  35 */ "ENOMSG", "EIDRM", "ECHRNG", "EL2NSYNC", "EL3HLT",
/*  40 */ "EL3RST", "ELNRNG", "EUNATCH", "ENOCSI", "EL2HLT",
/*  45 */ "EDEADLK", "ENOTREADY", "EWRPROTECT", "EFORMAT", "ENOLCK",
/*  50 */ "ENOCONNECT", NULL, "ESTALE", "EDIST", "EWOULDBLOCK",
/*  55 */ "EINPROGRESS", "EALREADY", "ENOTSOCK", "EDESTADDRREQ", "EMSGSIZE",
/*  60 */ "EPROTOTYPE", "ENOPROTOOPT", "EPROTONOSUPPORT", "ESOCKTNOSUPPORT", "EOPNOTSUPP",
/*  65 */ "EPFNOSUPPORT", "EAFNOSUPPORT", "EADDRINUSE", "EADDRNOTAVAIL", "ENETDOWN",
/*  70 */ "ENETUNREACH", "ENETRESET", "ECONNABORTED", "ECONNRESET", "ENOBUFS",
/*  75 */ "EISCONN", "ENOTCONN", "ESHUTDOWN", "ETIMEDOUT", "ECONNREFUSED",
/*  80 */ "EHOSTDOWN", "EHOSTUNREACH", "ERESTART", "EPROCLIM", "EUSERS",
/*  85 */ "ELOOP", "ENAMETOOLONG", "ENOTEMPTY", "EDQUOT", NULL,
/*  90 */ NULL, NULL, NULL,"EREMOTE", NULL,
/*  95 */ NULL, NULL, NULL, NULL, NULL,
/* 100 */ NULL, NULL, NULL, NULL, NULL,
/* 105 */ NULL, NULL, NULL, NULL, "ENOSYS",
/* 110 */ "EMEDIA", "ESOFT", "ENOATTR", "ESAD", "ENOTRUST",
/* 115 */ "ETOOMANYREFS", "EILSEQ", "ECANCELED", "ENOSR", "ETIME",
/* 120 */ "EBADMSG", "EPROTO", "ENODATA", "ENOSTR", "ENOTSUP",
/* 125 */ "EMULTIHOP", "ENOLINK", "EOVERFLOW"
};

int Nerrorstr;

int Desclevel;

struct cf Cf;

struct lblock Lb;
