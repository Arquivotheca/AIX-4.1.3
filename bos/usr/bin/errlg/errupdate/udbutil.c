static char sccsid[] = "@(#)53  1.12  src/bos/usr/bin/errlg/errupdate/udbutil.c, cmderrlg, bos411, 9428A410j 9/16/93 17:49:56";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: udb_init, udb_tmpltinsert, udb_islabel, udb_iscrcid
 *            udb_tmpltupdate, udb_tmpltdelete, udb_tmpltget, udb_close
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
 * High level ODM interface routines.
 *
 * NAME:      udb_islabel(label)
 * FUNCTION:  Return true if errtmplt contains an object
 *            whose et_label field equals 'label'
 * RETURN:    1  if true
 *            0  otherwise
 *
 *
 * NAME:      udb_iscrcid(crcid)
 * FUNCTION:  Return true if errtmplt contains an object
 *            whose et_crcid field equals 'crcid'
 * RETURN:    1  if true
 *            0  otherwise
 *
 *
 * NAME:      udb_tmpltinsert()
 * FUNCTION:  Insert the template of the global ODM C structure T_errtmplt
 *            into the ODM object class errtmplt.
 * RETURN:    -1 if error
 *
 *
 * NAME:      udb_tmpltupdate(crcid)
 * FUNCTION:  Update the template of the ODM object class errtmplt with
 *            the values of the global ODM C structure T_errtmplt
 *            whose et_crcid field equals 'crcid'
 * RETURN:    -1 if error
 *
 * NAME:      udb_tmpltdelete(crcid)
 * FUNCTION:  Delete the template from the ODM object class errtmplt
 *            whose et_crcid field equals 'crcid'
 * RETURN:    -1 if error
 *
 *
 * NAME:      udb_tmpltget(crcid)
 * FUNCTION:  Read the template into the global ODM C structure T_errtmplt
 *            whose et_crcid field equals 'crcid'
 * RETURN:    1  if 1 or more
 *            0  if 0
 *            -1 if error
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errupdate.h>

extern char *T_filename_dflt;
extern char *Tmpltfile;

struct t {
	struct t           *t_next;
	struct obj_errtmplt t_tmplt;
};
static struct t *Thead;
static Tchangeflg;
static int compressflg = 0;

static struct t *gett();

udb_init()
{
	int firstflg;

	Tmpltfile = T_filename_dflt ? T_filename_dflt : ERRTMPLT_DFLT;
	if (iscompressed(Tmpltfile)) {
		if (uncompress(Tmpltfile))
			cat_fatal(CAT_TMPLT_UNCOMPRESS,"\
Cannot uncompress template file %s.\n\
This could be because you don't have permissions, that the\n\
filesystem containing the template file is full, or a fork or\n\
exec system call failed.\n",Tmpltfile);
		else	
			compressflg++;
	}
	if(tmpltinit(1) < 0)
		return(-1);
	for(firstflg = 1; ; firstflg = 0) {
		if(tmpltdump(firstflg) <= 0)
			break;
		udb_tmpltinsert();
	}
	return(0);
}

udb_islabel(label)
char *label;
{
	struct t *tp;

	for(tp = Thead; tp; tp = tp->t_next)
		if(STREQ(tp->t_tmplt.et_label,label))
			break;
	return(tp ? 1 : 0);
}

udb_iscrcid(crcid)
unsigned crcid;
{
	struct t *tp;

	for(tp = Thead; tp; tp = tp->t_next)
		if(tp->t_tmplt.et_crcid == crcid)
			break;
	return(tp ? 1 : 0);
}

udb_tmpltinsert()
{
	struct t *tp;

	tp = gett();
	tp->t_next = Thead;
	Thead = tp;
	tp->t_tmplt = T_errtmplt;
	Tchangeflg++;
	return(1);
}

static struct t *gett()
{

	return((struct t *)jalloc(sizeof(struct t)));
}

udb_tmpltupdate()
{
	struct t *tp;

	for(tp = Thead; tp; tp = tp->t_next)
		if(tp->t_tmplt.et_crcid == T_errtmplt.et_crcid)
			break;
	if(tp == 0)
		udb_tmpltinsert();
	else
		tp->t_tmplt = T_errtmplt;
	Tchangeflg++;
	return(1);
}

udb_tmpltdelete(crcid)
{
	struct t *tp;
	struct t *tp2;

	tp2 = 0;
	for(tp = Thead; tp; tp = tp->t_next) {
		if(tp->t_tmplt.et_crcid == crcid)
			break;
		tp2 = tp;
	}

	if(tp != 0) {
		Tchangeflg++;

		if(tp2 == 0)
			Thead = tp->t_next;
		else
			tp2->t_next = tp->t_next;
	}

	return((int)tp);
}

udb_tmpltget(crcid)
{
	struct t *tp;

	for(tp = Thead; tp; tp = tp->t_next)
		if(tp->t_tmplt.et_crcid == crcid)
			break;
	if(tp != 0)
		T_errtmplt = tp->t_tmplt;

	return((int)tp);
}

udb_close()
{
	struct t *tp;
	int flg;

	if(Tchangeflg) {
		for(tp = Thead, flg = 1; tp; tp = tp->t_next, flg = 0) {
			T_errtmplt = tp->t_tmplt;
			tmpltwrite(flg);
		}
	}
	tmpltclose();
	if ( (compressflg) && (compress(Tmpltfile)) )
		return (-1);
	return (0);
}

