static char sccsid[] = "@(#)48  1.1  src/bos/diag/util/umlc/umlcglobal.c, dsaumlc, bos411, 9428A410j 3/14/94 17:19:41";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - umlcglbl.c
 *
 * FUNCTIONS:   main(argc,argv)
 *              int_handler(signal)
 *              genexit(exitcode)
 *              usage_exit(exitcode)
 *              disp_and_sleep()
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

#include "mlc_defs.h"
#include <nl_types.h>

/* GLOBAL VARIABLES     */

nl_catd         fdes;           /* catalog file descriptor               */

char *keywrds[] = {     "*VC", /* value = 1 */
                        "*SE", /* value = 2 */
                        "*TM", /* value = 3 */
                        "*L1", /* value = 4 */
                        "*L2", /* value = 5 */
                        "*L3", /* value = 6 */
                        "*L4", /* value = 7 */
                        "*L5", /* value = 8 */
                        "*L6", /* value = 9 */
                        "*SY", /* value = 10 */
                        "*CN", /* value = 11 */
                        "*EA", /* value = 12 */
                        "*PI", /* value = 13 */
                        "*SQ", /* value = 14 */
                        "*FC", /* value = 15 */
                        "*T1", /* value = 16 */
                        "*S1", /* value = 17 */
                        "*MS", /* value = 18 */
                        "*FB", /* value = 19 */
                        "*FD", /* value = 20 */
                        "*PN", /* value = 21 */
                        "*PL", /* value = 22 */
                        "*AX", /* value = 23 */
                        "*EC", /* value = 24 */
                        "*SN", /* value = 25 */
                        "*BC", /* value = 26 */
                        "*SI", /* value = 27 */
                        "*CD", /* value = 28 */
                        "*LO", /* value = 29 */
                        "*RL", /* value = 30 */
                        "*LL", /* value = 31 */
                        "*DD", /* value = 32 */
                        "*DG", /* value = 33 */
                        "*FN", /* value = 34 */
                        "*DC", /* value = 35 */
                        "*DS", /* value = 36 */
                        "00"
                };

char *lvl_kwds[] = { "*SE","*FC","*FB","*PN" };

VPD_REC         vprec;

struct PN_PTR   *pn_ptr;
struct FB_PTR   *fb_ptr;
struct FC_PTR   *fc_ptr;

MACH_REC        machine = {
                "\0", "\0", "\0", "\0", "\0",
                "\0", "\0", "\0", "\0", "\0",
                "\0", "\0", "\0", "\0", "\0",
                "\0", "\0", "\0", "\0", "\0",
                "\0", "\0", "\0", "\0", "\0",
                "\0", "\0", "\0", "\0", "\0",
                "\0" };

int     fccnt;
int     fbcnt;
int     pncnt;
int     vpcnt;

int     diskette;               /* TRUE = ipl from diskette */
int     install_yes=FALSE;      /* TRUE = System Installation run */
int     db_loaded=FALSE;
int     output = FALSE;         /* TRUE = output diskettes were written */
int     fdd_gone;               /* TRUE = Floppy Disk Drive missing */
int     fdd_not_supported=FALSE;/* TRUE = don't write files to floppy disk */
char    mfg_sno[FIELD_SIZE];
char    mfg_typ[FIELD_SIZE];
char    diagdir[80] = "/etc/lpp/diagnostics/data";      /* default directory */
int     no_menus=FALSE;         /* TRUE = -e flag was specified */
char    outputfile[255] = "\0";
int     cstop_found=FALSE;      /* TRUE = a checkstop file was found */
int     output_file_open=FALSE; /* TRUE = The PT_SYS file is open for writing */
