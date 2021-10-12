/* @(#)35	1.7  src/bos/usr/ccs/lib/libc/sysconf.h, libcenv, bos411, 9428A410j 3/4/94 11:40:13 */
/*
 * COMPONENT_NAME: LIBCENV
 *
 * FUNCTIONS: sysconf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SYSCONF
#define _H_SYSCONF

/*
 * Internal functions used by sysconf()
 */

static long num_cpus();
static long num_cpus_cfg();
static long child_max();
static long def_chk();
static long nodef_chk();

/*
 * Structure of entry to store the corresponding limit values of the
 * defined symbols. The func field is a pointer to the function to be 
 * activated for determining the return value in some case. If it is 0,
 * no function call is needed and the return value is in the limit field.
 */

typedef struct entry {
        int     limit;
        long    (*func)();
} entry_t;

/* 
 * Every entry in the values array corresponds to each defined parameter that
 * sysconf() accepts. This ordered list "values" should always be kept 
 * in-sync with the #define values of the symbolic parameters of sysconf() 
 * listed in the header file unistd.h. These symbolic parameters are used 
 * as direct array index to access the values array. 
 */

static entry_t values[]={
#ifdef 	ARG_MAX				/*     0   */
	ARG_MAX,		0,
#else
	-1,			0,
#endif
#ifdef 	CHILD_MAX			/*     1   */
	CHILD_MAX,		child_max,
#else
	-1,			0,
#endif
#ifdef	CLK_TCK				/*     2   */
	CLK_TCK,		0,
#else
	-1,			0,
#endif
#ifdef	NGROUPS_MAX			/*     3   */
	NGROUPS_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	OPEN_MAX			/*     4   */
	OPEN_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	STREAM_MAX			/*     5   */
	STREAM_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	TZNAME_MAX			/*     6   */
	TZNAME_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_JOB_CONTROL		/*     7   */
	TRUE,			0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_SAVED_IDS		/*     8   */
	TRUE,			0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_VERSION			/*     9   */
	_POSIX_VERSION,		0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_ARG_MAX			/*    10   */
	_POSIX_ARG_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_CHILD_MAX		/*    11   */
	_POSIX_CHILD_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_LINK_MAX			/*    12   */
	_POSIX_LINK_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_MAX_CANON		/*    13   */
	_POSIX_MAX_CANON,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_MAX_INPUT		/*    14   */
	_POSIX_MAX_INPUT,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_NAME_MAX			/*    15   */
	_POSIX_NAME_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_NGROUPS_MAX		/*    16   */
	_POSIX_NGROUPS_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_OPEN_MAX			/*    17   */
	_POSIX_OPEN_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_PATH_MAX			/*    18   */
	_POSIX_PATH_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_PIPE_BUF			/*    19   */
	_POSIX_PIPE_BUF,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_SSIZE_MAX		/*    20   */
	_POSIX_SSIZE_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_STREAM_MAX		/*    21   */
	_POSIX_STREAM_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX_TZNAME_MAX		/*    22   */
	_POSIX_TZNAME_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	BC_BASE_MAX			/*    23   */
	BC_BASE_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	BC_DIM_MAX			/*    24   */
	BC_DIM_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	BC_SCALE_MAX			/*    25   */
	BC_SCALE_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	BC_STRING_MAX			/*    26   */
	BC_STRING_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	EQUIV_CLASS_MAX			/*    27   */
	EQUIV_CLASS_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	EXPR_NEST_MAX			/*    28   */
	EXPR_NEST_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	LINE_MAX			/*    29   */
	LINE_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	RE_DUP_MAX			/*    30   */
	RE_DUP_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	_POSIX2_VERSION			/*    31   */
	_POSIX2_VERSION,	0,
#else
	-1,			0,
#endif
#ifdef	_POSIX2_C_DEV			/*    32   */
	_POSIX2_C_DEV,		def_chk,
#else
	0,			nodef_chk,
#endif
#ifdef	_POSIX2_FORT_DEV		/*    33   */
	_POSIX2_FORT_DEV,	def_chk,
#else
	0,			nodef_chk,
#endif
#ifdef	POSIX2_FORT_RUN			/*    34   */
	POSIX2_FORT_RUN,	def_chk,
#else
	0,			nodef_chk,
#endif
#ifdef	_POSIX2_LOCALEDEF		/*    35   */
	_POSIX2_LOCALEDEF,	def_chk,
#else
	0,			nodef_chk,
#endif
#ifdef	_POSIX2_SW_DEV			/*    36   */
	_POSIX2_SW_DEV,		def_chk,
#else
	0,			nodef_chk,
#endif
#ifdef	POSIX2_BC_BASE_MAX		/*    37   */
	POSIX2_BC_BASE_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_BC_DIM_MAX		/*    38   */
	POSIX2_BC_DIM_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_BC_SCALE_MAX		/*    39   */
	POSIX2_BC_SCALE_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_BC_STRING_MAX		/*    40   */
	POSIX2_BC_STRING_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_EQUIV_CLASS_MAX		/*    41   */
	POSIX2_EQUIV_CLASS_MAX,0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_EXPR_NEST_MAX		/*    42   */
	POSIX2_EXPR_NEST_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_LINE_MAX			/*    43   */
	POSIX2_LINE_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	POSIX2_RE_DUP_MAX		/*    44   */
	POSIX2_RE_DUP_MAX,	0,
#else
	-1,			0,
#endif
#ifdef	PASS_MAX			/*    45   */
	PASS_MAX,		0,
#else
	MAX_PASS,		0,
#endif
#ifdef	_XOPEN_VERSION			/*    46   */
	_XOPEN_VERSION,		0,
#else
	-1,			0,
#endif
#ifdef	ATEXIT_MAX			/*    47   */
	ATEXIT_MAX,		0,
#else
	-1,			0,
#endif
#ifdef	PAGE_SIZE			/*    48   */
	PAGE_SIZE,		0,
#else
	-1,			0,
#endif
#ifdef	_AES_OS_VERSION			/*    49   */
	_AES_OS_VERSION,	0,
#else
	-1,			0,
#endif
#ifdef COLL_WEIGHTS_MAX			/*    50   */
       COLL_WEIGHTS_MAX, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX2_C_BIND			/*    51   */
       _POSIX2_C_BIND,	 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX2_C_VERSION		/*    52   */
       _POSIX2_C_VERSION, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX2_UPE			/*    53   */
       _POSIX2_UPE,	 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX2_CHAR_TERM		/*    54   */
       _POSIX2_CHAR_TERM, 	0,
#else
	-1,			0,
#endif
#ifdef _XOPEN_SHM			/*    55   */
       _XOPEN_SHM, 	0,
#else
	-1,			0,
#endif
#ifdef _XOPEN_CRYPT			/*    56   */
       _XOPEN_CRYPT, 	0,
#else
	-1,			0,
#endif
#ifdef _XOPEN_ENH_I18N			/*    57   */
       _XOPEN_ENH_I18N, 	0,
#else
	-1,			0,
#endif
#ifdef	IOV_MAX				/*    58   */
	IOV_MAX,		0,
#else
	-1,			0,
#endif
#ifdef _POSIX_REENTRANT_FUNCTIONS	/*    59   */
       _POSIX_REENTRANT_FUNCTIONS, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREADS			/*    60   */
       _POSIX_THREADS, 		0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_ATTR_STACKADDR	/*    61   */
       _POSIX_THREAD_ATTR_STACKADDR, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_ATTR_STACKSIZE	/*    62   */
       _POSIX_THREAD_ATTR_STACKSIZE, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_FORKALL		/*    63   */
       _POSIX_THREAD_FORKALL, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING	/*    64   */
       _POSIX_THREAD_PRIORITY_SCHEDULING, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_PRIO_INHERIT	/*    65   */
       _POSIX_THREAD_PRIO_INHERIT, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_PRIO_PROTECT	/*    66   */
       _POSIX_THREAD_PRIO_PROTECT, 	0,
#else
	-1,			0,
#endif
#ifdef _POSIX_THREAD_PROCESS_SHARED	/*    67   */
       _POSIX_THREAD_PROCESS_SHARED, 	0,
#else
	-1,			0,
#endif
#ifdef PTHREAD_DATAKEYS_MAX		/*    68   */
       PTHREAD_DATAKEYS_MAX, 	0,
#else
	-1,			0,
#endif
#ifdef PTHREAD_STACK_MIN		/*    69   */
       PTHREAD_STACK_MIN, 	0,
#else
	-1,			0,
#endif
#ifdef PTHREAD_THREADS_MAX		/*    70   */
       PTHREAD_THREADS_MAX, 	0,
#else
	-1,			0,
#endif
/*     NPROCESSORS_CONF   */		/*    71   */
       0, 			num_cpus_cfg,
/*     NPROCESSORS_ONLN   */		/*    72   */
       0, 			num_cpus
};

#endif		/* _H_SYSCONF */
