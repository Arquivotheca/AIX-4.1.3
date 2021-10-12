/* @(#)26	1.1  src/bos/usr/bin/virscan/vsdefs.h, cmdsvir, bos411, 9428A410j 4/3/91 19:24:21 */
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: NONE
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Various constants.
 */

/*
 * Program's return values.
 */
#define NO_ERR    0    			/* No error. Normal case. */
#define NORM_ERR  1    			/* Error. Not virus related. */
#define SIG_FOUND 2    			/* A viral signature was found. */

#define SIZE_TMP_LINE_BUF   256          /* Length of working buffer used */
                                         /*   reading signature file and */
                                         /*   filelist file */

#define MAX_SIZE_SIGN       (SIZE_TMP_LINE_BUF/4)
#define MAX_SIZE_MSG        (SIZE_TMP_LINE_BUF/2)
#ifndef TRUE
#define TRUE                (1)
#endif
#ifndef FALSE
#define FALSE               (!TRUE)
#endif
#ifndef MIN
#define MIN(A,B)            ((A) < (B) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A,B)            ((A) > (B) ? (A) : (B))
#endif
#define NUMFRAGS 4
#define SIZEFRAG 11
#define SIZE_WORK_BUF       0xC000       /* The buffer into which executable */
                                         /*   file data is read */
#define DEBUGGERY           0            /* Turn on for debugging messages */
#define ASM_SCANLOOP        FALSE        /* Turn on for (fast) 80X86 assembler*/
                                         /*   version of the scan loop */

#if 0
#define SELF_TEST           TRUE         /* If this is TRUE, code to perform */
                                         /*   an integrity test on VIRSCAN */
                                         /*   is included. This has only been */
                                         /*   tested for the PC versions and */
                                         /*   so the macro is defined in the */
                                         /*   PC make file */

#define SEARCH_DRIVE_ENABLE FALSE        /* If this is TRUE, OS/2 */
                                         /*   Family API support for */
                                         /*   searching an entire specified */
                                         /*   drive will be included. */
                                         /*   Could be a define on compiler */
                                         /*   command line */
#define BOOTS               FALSE        /* If this is TRUE, enable boot */
                                         /*   sector scan code. Some of this */
                                         /*   code is Family API, some will */
                                         /*   only function under DOS or the */
                                         /*   DOS box. */
                                         /*   Could be a define on compiler */
                                         /*   command line */
#endif
#define FREQ_COUNTS         0            /* Used in development only. If this */
                                         /*   is set to true, virscan will */
                                         /*   count characters, and display */
                                         /*   counts at program termination. */


#ifndef NULL
#define NULL                ((void *)0)  /* Make sure NULL is defined */
#endif

#define T_FILE                 1
#define T_MASTER_BOOT_RECORD   2
#define T_SYSTEM_BOOT_SECTOR   3
#define T_SYSTEM_MEMORY        4

#ifdef _AIX
#define ADDENDA_PATH 	"/etc/security/scan/addenda.lst"
#define VIRSIG_PATH	"/etc/security/scan/virsig.lst"
#define strupr(A)	A
#define SCAN_EXE	"__SCAN_EXE"
#define SCAN_ALL 	"__SCAN_ALL"
#endif
