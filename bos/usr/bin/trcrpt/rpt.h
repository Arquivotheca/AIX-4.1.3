/* @(#)21       1.32  src/bos/usr/bin/trcrpt/rpt.h, cmdtrace, bos41J, 9513A_all 2/21/95 10:02:44 */

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: header file for trcrpt
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
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

extern char *hexstr();
extern char *argstr();
extern char *formatstr();

/*
 * completion codes
 */
#define RV_EOF       1
#define RV_GOOD      2
#define RV_UNDEFINED 3
#define RV_BADFORMAT 4

#define NUMBUFSIZE 32
extern char Numbuf1[NUMBUFSIZE];
extern char Numbuf2[NUMBUFSIZE];

#define CTOK(x) ((x) - 0x80)

extern jflag;
extern checkflag;
extern Aflag;
extern nohdrflag;
extern quietflag;
extern rawflag;
extern Errflg;
extern Listflg;
extern semaflg;
extern compressflg;
extern Tid;
extern Cpuid;
extern Pri;
extern Threadflg;
extern char *Execname_ref;

extern Starttime;
extern Endtime;

/*
 * conditional hookid bitmap
 */
#define NHOOKIDS 4096
#define MAXTEMPLATES 4096
#define SETHOOKID(cp,i) (cp[(i)/8] |=  (1 << (i)%8))
#define CLRHOOKID(cp,i) (cp[(i)/8] &= ~(1 << (i)%8))
#define ISHOOKID(cp,i)  (cp[(i)/8] &   (1 << (i)%8))
#define HOOKTOID(hw)    (((hw) >> 20) & 0xFFF)
extern unsigned char Condhookids[NHOOKIDS/8];

extern char *Listfile;
extern char *Logfile;
extern char *Errfile;
extern char *Tmpltfile;

extern Logfd;
extern Logidx0;
extern Logidx;

#define NERRNO 200
extern Nerrorstr;
extern char *Errorstr[NERRNO];

extern Tmpltidx;
extern Lineno;
extern Toffset;
struct tindex {
	char      t_state;
	short     t_version;
	short	  t_release;
	int       t_size;
	int       t_offset;
	int       t_lineno;
	union td *t_tdp;
};
#define T_DEFINED 0x01
#define T_ERRMSG  0x02
extern struct tindex Tindexes[NHOOKIDS];
extern short Traceids[MAXTEMPLATES];	/* index into Tindexes */
extern Ntemplates;

struct version {
	short v_version;
	short v_release;
};
extern struct version Version;
extern Logtraceid;
extern unsigned Hookword;

/* Time conversion factor for Power PCs.  This converts the time base
 * to nanoseconds, see calculation in prevent.c.
 */
extern double cnvtfact;
#define CNVTFACT_DEF 1
/* On the standard RISC machines the low-order timer register rolls
 * over every second (1 billion nanoseconds).  However, on the time base
 * machines (such as 603s) it just wraps, i.e., rolls over every 
 * 1 and 8 0s, 0xffffffff+1, timer tics.
 */
extern double time_wrap;
#define RISC_WRAP (double)1000000000	/* 1 billion nanoseconds default */
#define TIMEBASE_WRAP ((double)0xffffffff+(double)1) /* 4294967296 */

extern Timestamp;

extern int Eventcount;
extern int Eventindex;
extern int Eventsize;
extern unsigned char Eventbuf[];

#define LINESIZE 80000

struct conflist {
	char *cf_stanza;
	char *cf_entry;
	char *cf_value;
};

/*
 * Format code numbers of special registers
 * NOT register numbers
 */
#define RES_ERROR       1
#define RES_LOGFILE     2
#define RES_INDEX       3
#define RES_TRACEID     4
#define RES_DEFAULT     5
#define RES_STOP        6
#define RES_INDEX0      7
#define RES_BREAK       8
#define RES_SKIP        9
#define RES_NOPRINT     10

#define IND_APPL    1
#define IND_SVC     2
#define IND_KERN    3
#define IND_INT     4
#define IND_0       5
#define IND_NOPRINT 6

#define RREG_BASEPOINTER  0
#define RREG_DATAPOINTER  1
#define RREG_EXECPATH     2
#define RREG_PID          3
#define RREG_TIME         4
#define RREG_SYMBOL_VALUE 5
#define RREG_DATA1        6
#define RREG_DATA2        7
#define RREG_DATA3        8
#define RREG_DATA4        9
#define RREG_DATA5        10
#define RREG_HDATA        11
#define RREG_SVC          12
#define RREG_SYMBOL_RANGE 13
#define RREG_SYMBOL_NAME  14
#define RREG_LINENO       15
#define RREG_RELLINENO    16
#define RREG_CURRFILE     17
#define RREG_HDATAU       18
#define RREG_HDATAL       19
#define RREG_HTYPE        20
#define RREG_INADDR       21
#define RREG_TID          22
#define RREG_CPUID        23
#define RREG_PRI          24

#define NREGISTERS 255

struct cf {
	char *cf_tmpltfile;
	char *cf_logfile;
};
extern struct cf Cf;

#define LISTFILE       "trc.list"
#define NAMELIST_DFLT  "/unix"
#define TMPLT_DFLT     "/etc/trcfmt"
#define LOGFILE_DFLT   "/usr/adm/ras/trcfile"

struct displayopts {
	int opt_timestamp;
	int opt_pagesize;
	int opt_microsecflg;
	int opt_idflg;
	int opt_execflg;
	int opt_pidflg;
	int opt_svcflg;
	int opt_compactflg;
	int opt_fileflg;
	int opt_2lineflg;
	int *opt_startp;
	int *opt_endp;
	int opt_histflg;
	int opt_tidflg;
	int opt_cpuidflg;
};
extern struct displayopts Opts;

struct builtins {
	char     *bl_name;
	int     (*bl_func)();
	union td *bl_desc;
	int       bl_regcount;
};
extern struct builtins *fntobuiltin();
extern struct builtins *finstall();

#define MCS_CATALOG "cmdtrace.cat"
#include "cmdtrace_msg.h"

#include <ras.h>

typedef struct lblock {
	int lb_currentry;
	int lb_firstentry;
	int lb_lastentry;
	int lb_startoffset;
	int lb_endoffset;
	int lb_icflg;
};
