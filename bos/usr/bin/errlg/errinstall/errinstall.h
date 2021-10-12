/* @(#)57       1.8  src/bos/usr/bin/errlg/errinstall/errinstall.h, cmderrlg, bos411, 9428A410j 4/16/93 16:23:47 */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: header file for errinstall
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern char *Infile;
extern Lineno;
extern Warnflg;
extern Errflg;
extern verboseflg;
extern promptflg;
extern Dupflg;
extern checkflg;
extern linitflg;
extern hexflg;
extern quietflg;
extern forceflg;
extern Nomsgidflg;

extern char *msgcatstr();
extern unsigned ettocrc();
extern char *Codeptfile;

struct md {
	struct md *md_next;
	int        md_alertid;
	int        md_set;
	char      *md_text;
};

#include <cmderrlg_msg.h>

#include <errlg.h>

/*
 * conversion between command line SET representation (E,P,U,etc) and
 * message 'set' numbers. These numbers are defined in err_rec.h.
 */
struct codetoset {
        char *nc_code;
        int   nc_set;
};

struct codetoset codetoset[NERRSETS] = {
        "E", ERRSET_DESCRIPTION,        /* Error Description message set */
        "P", ERRSET_PROBCAUSES,         /* Probable Cause message set */
        "U", ERRSET_USERCAUSES,         /* User Cause message set */
        "I", ERRSET_INSTCAUSES,         /* Install Cause message set */
        "F", ERRSET_FAILCAUSES,         /* Failure Cause message set */
        "R", ERRSET_RECACTIONS,         /* Recommended Action message set */
        "D", ERRSET_DETAILDATA,         /* Detailed Data Id message set */
};

#define CODEALL "E,P,U,I,F,R,D" /* all 7 types */
