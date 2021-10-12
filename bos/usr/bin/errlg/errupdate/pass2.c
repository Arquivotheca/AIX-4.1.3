static char sccsid[] = "@(#)66	1.7  src/bos/usr/bin/errlg/errupdate/pass2.c, cmderrlg, bos411, 9428A410j 3/31/94 17:06:52";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pass2
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

/*
 * Second pass scan of parsed template description file.
 * The input to the second pass is a linked lists of symbol structures.
 * Each of these lists represents an entry in the template description file.
 * The input is free of syntax errors and duplicate values.
 * The second pass does not check for any errors.
 */

#include <stdio.h>
#include <errupdate.h>
#include <parse.h>

pass2()
{

	Pass = 2;
	readmajor2_init();
	while(readmajor2())
		pass2a();
}

static pass2a()
{
	unsigned crcid;
	struct obj_errtmplt etnew;

	crcid = Major->s_number;
	switch(Major->s_type) {
	case IPLUS:
		majortoet();
		T_errtmplt.et_crcid = crcid;
		if(udb_tmpltinsert() == 0)
			break;
		Addcount++;
		break;
	case IMINUS:
		if(udb_tmpltdelete(crcid) == 0)
			break;
		Deletecount++;
		break;
	case IEQUAL:
		majortoet();
		T_errtmplt.et_crcid = crcid;
		etnew = T_errtmplt;
		if(udb_tmpltget(crcid) == 0) {
			break;
		}
		T_errtmplt.et_logflg    = etnew.et_logflg;
		T_errtmplt.et_alertflg  = etnew.et_alertflg;
		T_errtmplt.et_reportflg = etnew.et_reportflg;
		if(udb_tmpltupdate(T_errtmplt.et_crcid) == 0)
			break;
		Updatecount++;
		break;
	case IUPDATE:
		majortoet();
		T_errtmplt.et_crcid = crcid;
		if(udb_tmpltupdate(T_errtmplt.et_crcid) == 0)
			break;
		Updatecount++;
		break;
	default:
		genexit_nostats(1);
	}
}

