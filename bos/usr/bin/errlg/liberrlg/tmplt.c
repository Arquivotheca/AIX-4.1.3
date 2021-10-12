static char sccsid[] = "@(#)14  1.6  src/bos/usr/bin/errlg/liberrlg/tmplt.c, cmderrlg, bos411, 9428A410j 3/31/94 19:09:36";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: tmpltinit, tmpltinit_create, tmpltgetbycrcid, tmpltdump, tcmp,
 *            tmpltclose, tmpltwrite, settmpltfile, gettmpltfile, t_lookup,
 *            invalid_tmplt, val_class, val_type, val_det_len, val_det_enc,
 *            ann_bad_tmplt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Template file manipulation routines.
 */

#define _ILS_MACROS
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cmderrlg_msg.h>
#include <errlg.h>

static struct obj_errtmplt *t_lookup();
static tcmp();

static	int	tot_det_len;

struct obj_errtmplt T_errtmplt;

struct t {
	unsigned             t_crcid;
	struct obj_errtmplt *t_tmpltp;
};

char *Tmpltfile;
char *T_filename_dflt;
static Tfd = -1;
static T_size;			/* size of normalized obj_errtmplt */
static T_count;
static T_mtime;
static char *T_base;
static struct t *T_table;

static struct obj_errtmplt Etnull;
#define FF(x) memset(x,0xFF,sizeof(x))


/*
 * NAME:      settmpltfile()
 * FUNCTION:  Set the error template file to the supplied name.
 * RETURNS:   None.
 */

void
settmpltfile(filename)
char *filename;
{

	if(filename)
		T_filename_dflt = stracpy(filename);
	else
		T_filename_dflt = ERRTMPLT_DFLT;
}


/*
 * NAME:      tmpltinit()
 * FUNCTION:  Read the error templates and store them in a table.
 * RETURNS:   -1 - Error
 *             0 - Success
 */

tmpltinit(rwflag)
{
	int i,n;
	struct stat statbuf;

	Tmpltfile = T_filename_dflt ? T_filename_dflt : ERRTMPLT_DFLT;
	T_size = ALIGN(4,sizeof(struct obj_errtmplt));
	if(Tfd >= 0)
		close(Tfd);
	if(T_base)
		free(T_base);
	if(T_table)
		free(T_table);
	T_base  = 0;
	T_table = 0;
	T_count = 0;
	/*
	 * fill in null template
	 */
	Etnull.et_logflg    = 1;
	Etnull.et_alertflg  = 0;
	Etnull.et_reportflg = 1;
	Etnull.et_desc = 0xFFFF;
	FF(Etnull.et_probcauses);
	FF(Etnull.et_usercauses);
	FF(Etnull.et_useraction);
	FF(Etnull.et_instcauses);
	FF(Etnull.et_instaction);
	FF(Etnull.et_failcauses);
	FF(Etnull.et_failaction);
	Etnull.et_detail_length[0] = LE_DETAIL_MAX;
	Etnull.et_detail_encode[0] = 'H';
	Etnull.et_detail_descid[0] = 0xFFFF;

	if(rwflag) {
		if((Tfd = open(Tmpltfile,O_RDWR)) < 0 &&
		   (Tfd = open(Tmpltfile,O_RDWR|O_TRUNC|O_CREAT,0644)) < 0)  {
			T_errtmplt = Etnull;
			return(-1);
		}
		
	} else {
		if((Tfd = open(Tmpltfile,0)) < 0)  {
			T_errtmplt = Etnull;
			return(-1);
		}
		
	}
 	if(fstat(Tfd,&statbuf)) 
		return(-1);
	
	T_mtime = statbuf.st_mtime;
	n = statbuf.st_size - statbuf.st_size % T_size;
	T_count = n / T_size;
	if(T_count > 0) {
		T_base = jalloc(T_count * T_size);
		if(read(Tfd,T_base,n) != n) {
			perror("Tmpltfile: read");
			genexit(1);
		}
		T_table = MALLOC(T_count,struct t);
		for(i = 0; i < T_count; i++) {
			struct obj_errtmplt *tp;

			tp = (struct obj_errtmplt *)&T_base[i * T_size];
			T_table[i].t_crcid = tp->et_crcid;
			T_table[i].t_tmpltp = tp;
		}
		qsort(T_table,T_count,sizeof(T_table[0]),tcmp);
	}
	return(0);
}


/*
 * NAME:      tmpltinit_create()
 * FUNCTION:  Create an new error template repository.
 * RETURNS:   -1 - Error
 *             0 - Success
 */

tmpltinit_create()
{
	char *dir;
	char cmd[128];

	if(Tmpltfile == 0 || Tfd >= 0)
		return(-1);
	unlink(Tmpltfile );
	dir = ERRLG_DIR;
	sprintf(cmd,"errupdate -qf %s/%s",dir,ERRIDS_DESC);
	if(shell(cmd))
		return(-1);
	return(tmpltinit(0));
}


/*
 * NAME:      tmpltgetbycrcid()
 * FUNCTION:  Get the template associated with the supplied crcid.
 * RETURNS:   0 - Error
 *            1 - Success
 */

tmpltgetbycrcid(crcid)
unsigned crcid;
{
	struct obj_errtmplt *tmpltp;
	struct stat statbuf;

	if(Tfd < 0 || T_table == 0 || fstat(Tfd,&statbuf)) {
		T_errtmplt = Etnull;
		return(0);
	}
	if(statbuf.st_mtime > T_mtime) 
		tmpltinit(0);
	
	tmpltp = t_lookup(crcid);
	if(tmpltp == 0) {
		T_errtmplt = Etnull;
		return(0);
	}
	T_errtmplt = *tmpltp;
	return(1);
}


/*
 * NAME:      t_lookup()
 * FUNCTION:  Binary search of error template table T_table for the template
 *            associated with the supplied crcid.
 * RETURNS:   !0 - pointer to obj_errtmplt structure with that id.
 *             0 - template not found.
 */

static struct obj_errtmplt *t_lookup(crcid)
unsigned crcid;
{
	register n,ln,rn;
	register tcrcid;

	ln = 0;
	rn = T_count;
	for(;;) {
		n = (ln + rn)/2;
		tcrcid = T_table[n].t_crcid;
		if(crcid < tcrcid) {
			if(rn == n)
				return(0);
			rn = n;
			continue;
		}
		if(crcid > tcrcid) {
			if(ln == n)
				return(0);
			ln = n;
			continue;
		}
		return(T_table[n].t_tmpltp);
	}
}


/*
 * NAME:      tcmp()
 * FUNCTION:  Compare two crcids, used with qsort().
 * RETURNS:   -1 - crc1 < crc2
 *             0 - crc1 == crc2
 *             1 - crc2 > crc2
 */

static tcmp(tp1,tp2)
struct t *tp1,*tp2;
{

	if(tp1->t_crcid < tp2->t_crcid)
		return(-1);
	if(tp1->t_crcid > tp2->t_crcid)
		return(+1);
	return(0);
}


/*
 * NAME:      tmpltdump()
 * FUNCTION:  Optionally initialize the error template list. Copy
 *            the next template from the list.
 * RETURNS:    0 - No more templates in list.
 *             1 - Success
 */

tmpltdump(initflg)
{
	static tidx;

	if(initflg) {
		tidx = 0;
		if(Tfd < 0 && tmpltinit(0) < 0)
			return(0);
	}
	if(tidx >= T_count)
		return(0);
	memcpy(&T_errtmplt,T_table[tidx++].t_tmpltp,sizeof(T_errtmplt));
	return(1);
}


/*
 * NAME:      tmpltclose()
 * FUNCTION:  Close the error template file.
 * RETURNS:   None.
 */

tmpltclose()
{

	if(Tfd >= 0) {
		close(Tfd);
		Tfd = -1;
	}
}

/*
 * NAME:      gettmpltfile()
 * FUNCTION:  Get the error template file name.
 * RETURNS:   Pointer to the template file name.
 */

char *
gettmpltfile()
{
	return(Tmpltfile);
}


/*
 * NAME:      tmpltwrite()
 * FUNCTION:  Write an template to an error template file.  Optionally
 *            truncate an error template file.
 * RETURNS:   -1 - Failure
 *             0 - Success
 */

tmpltwrite(truncflg)
{
	static count;

	if(Tfd < 0)
		return(-1);
	if(truncflg) {
		count = 0;
		ftruncate(Tfd,0);
	}
	lseek(Tfd,count * T_size,0);
	if (write(Tfd,&T_errtmplt,sizeof(T_errtmplt)) < 0)
		{
		cat_print(CAT_BAD_TMPLT_WRITE,
"Cannot update the error template database %s.\n\
A write to the error template database failed.\n%s\n\
Possible Causes:\n\
1.  The filesystem containing the error template database\n\
    is full.\n\
Run the errupdate command with the '.undo' file(s) to clean up\n\
the error template database.  Increase the filesystem, and then\n\
rerun the errupdate command to add the templates.\n",gettmpltfile(),errstr());
		exit(1);
		}	
	count++;
	return(0);
}


/*
 * NAME:      invalid_tmplt()
 * FUNCTION:  Invalidate a given error template.  Check the label for emptiness
 *            and printability.  Check the class to be in {H,S,O,U}.  Check the
 *            type to be in {PERM,TEMP,PERF,PEND,UNKN,INFO}.  Check the detail
 *            length for valid range.  Check the detail encode to be in {A,H,D}.
 *            Check the log, alert, and report flags to be in	{0,1}.
 * RETURNS:   
 *            0 ------- No members INVALID.
 *            1 ------- INVALID label.
 *            1 << 1 -- INVALID class.
 *            1 << 2 -- INVALID type.
 *            1 << 3 -- INVALID log flag.
 *            1 << 4 -- INVALID alert flag.
 *            1 << 5 -- INVALID report flag.
 *            1 << 6 -- INVALID detail length 1
 *            1 << 7 -- INVALID detail length 2
 *            1 << 8 -- INVALID detail length 3
 *            1 << 9 -- INVALID detail length 4
 *            1 << 10 - INVALID detail length 5
 *            1 << 11 - INVALID detail length 6
 *            1 << 12 - INVALID detail length 7
 *            1 << 13 - INVALID detail length 8
 *            1 << 14 - INVALID detail encode 1
 *            1 << 15 - INVALID detail encode 2
 *            1 << 16 - INVALID detail encode 3
 *            1 << 17 - INVALID detail encode 4
 *            1 << 18 - INVALID detail encode 5
 *            1 << 19 - INVALID detail encode 6
 *            1 << 20 - INVALID detail encode 7
 *            1 << 21 - INVALID detail encode 8
 */

 int
 invalid_tmplt(tp)
 struct obj_errtmplt *tp;
 {
	int	rc = 0;

	tot_det_len = 0;

	if (!val_name(tp->et_label,LE_LABEL_MAX))
		rc = 1;
	if (!val_class(tp->et_class))
		rc |= (1 << 1);
	if (!val_type(tp->et_type))
		rc |= (1 << 2);
	if (!(tp->et_logflg == 0 || tp->et_logflg == 1))
		rc |= (1 << 3);
	if (!(tp->et_alertflg == 0 || tp->et_alertflg == 1))
		rc |= (1 << 4);
	if (!(tp->et_reportflg == 0 || tp->et_reportflg == 1))
		rc |= (1 << 5);
	if (!val_det_len(tp->et_detail_length[0]))
		rc |= (1 << 6);
	if (!val_det_len(tp->et_detail_length[1]))
		rc |= (1 << 7);
	if (!val_det_len(tp->et_detail_length[2]))
		rc |= (1 << 8);
	if (!val_det_len(tp->et_detail_length[3]))
		rc |= (1 << 9);
	if (!val_det_len(tp->et_detail_length[4]))
		rc |= (1 << 10);
	if (!val_det_len(tp->et_detail_length[5]))
		rc |= (1 << 11);
	if (!val_det_len(tp->et_detail_length[6]))
		rc |= (1 << 12);
	if (!val_det_len(tp->et_detail_length[7]))
		rc |= (1 << 13);
	if (!val_det_enc(tp->et_detail_encode[0]))
		rc |= (1 << 14);
	if (!val_det_enc(tp->et_detail_encode[1]))
		rc |= (1 << 15);
	if (!val_det_enc(tp->et_detail_encode[2]))
		rc |= (1 << 16);
	if (!val_det_enc(tp->et_detail_encode[3]))
		rc |= (1 << 17);
	if (!val_det_enc(tp->et_detail_encode[4]))
		rc |= (1 << 18);
	if (!val_det_enc(tp->et_detail_encode[5]))
		rc |= (1 << 19);
	if (!val_det_enc(tp->et_detail_encode[6]))
		rc |= (1 << 20);
	if (!val_det_enc(tp->et_detail_encode[7]))
		rc |= (1 << 21);
	
	return(rc);
}


/*
 * NAME:      val_class()
 * FUNCTION:  Validate an error class name.
 * RETURNS:   1 - valid
 *            0 - invalid
 */

int
val_class(class)
char class[];
{
	char buf[LE_CLASS_MAX];
	int	rc;
	int	i;

	for(i = 0; i < LE_CLASS_MAX; i++) {
		buf[i] = islower(class[i]) ? _toupper(class[i]) : class[i];
		if(class[i] == '\0'){
			buf[i] = '\0';
			break;
		}
	}

	if(!strcmp(buf,"H") || !strcmp(buf,"O") ||
	   !strcmp(buf,"S") || !strcmp(buf,"U"))
		rc = 1;
	else
		rc = 0;
	return(rc);
}


/*
 * NAME:      val_type()
 * FUNCTION:  Validate an error type name.
 * RETURNS:   1 - valid
 *            0 - invalid
 */

int
val_type(type)
char type[];
{
	char buf[LE_TYPE_MAX];
	int	rc;
	int	i;

	for(i = 0; i < LE_TYPE_MAX; i++) {
		buf[i] = islower(type[i]) ? _toupper(type[i]) : type[i];
		if(type[i] == '\0'){
			buf[i] = '\0';
			break;
		}
	}
	if(!strcmp(buf,"PERM") || !strcmp(buf,"TEMP") ||
	   !strcmp(buf,"PERF") || !strcmp(buf,"PEND") ||
	   !strcmp(buf,"INFO") || !strcmp(buf,"UNKN"))
		rc = 1;
	else
		rc = 0;
	return(rc);
}


/*
 * NAME:      val_det_len()
 * FUNCTION:  Validate that the maximum detail length has not been exceeded.
 * RETURNS:   1 - valid
 *            0 - invalid
 */

int
val_det_len(len)
unsigned short len;
{
	int	rc;

	if(len == 0)
		rc = 1;
	else {
		tot_det_len += len;
		if(tot_det_len > LE_DETAIL_MAX)
			rc = 0;
		else
			rc = 1;
	}
	return(rc);
}


/*
 * NAME:      val_det_enc()
 * FUNCTION:  Validate the supplied detail data encode code.
 * RETURNS:   1 - valid
 *            0 - invalid
 */

int
val_det_enc(code)
unsigned short code;
{
	int	rc;

	if(code == 'H' || code == 'A' || code == 'D' || code == 0)
		rc = 1;
	else
		rc = 0;
	return(rc);
}

/*
 * NAME:      ann_bad_tmplt()
 * FUNCTION:  Annunciate a bad error template.
 * RETURNS:   None
 */
void
ann_bad_tmplt(code,tp)
int	code;
struct obj_errtmplt *tp;
{
	if(code & 1 ) 			/* bad label */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"error label");

	if(code & (1 << 1)) 	/* bad class */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"error class");
	
	if(code & (1 << 2))		/* bad type */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"error type");
	
	if(code & (1 << 3)) 	/* bad logflg */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"log flag");
	
	if(code & (1 << 4))		/* bad alertflg */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"alert flag");
	
	if(code & (1 << 5))		/* bad reportflg */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"report flag");
	
	if(code & (1 << 6))		/* bad detail length 1 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 1");
	
	if(code & (1 << 7))		/* bad detail length 2 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 2");
	
	if(code & (1 << 8))		/* bad detail length 3 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 3");
	
	if(code & (1 << 9))		/* bad detail length 4 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 4");
	
	if(code & (1 << 10))	/* bad detail length 5 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 5");
	
	if(code & (1 << 11))	/* bad detail length 6 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 6");
	
	if(code & (1 << 12))	/* bad detail length 7 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 7");
	
	if(code & (1 << 13))	/* bad detail length 8 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail length 8");
	
	if(code & (1 << 14))	/* bad detail encode 1 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 1");
	
	if(code & (1 << 15))	/* bad detail encode 2 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 2");
	
	if(code & (1 << 16))	/* bad detail encode 3 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 3");
	
	if(code & (1 << 17))	/* bad detail encode 4 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 4");
	
	if(code & (1 << 18))	/* bad detail encode 5 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 5");
	
	if(code & (1 << 19))	/* bad detail encode 6 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 6");
	
	if(code & (1 << 20))	/* bad detail encode 7 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 7");
	
	if(code & (1 << 21))	/* bad detail encode 8 */
		cat_eprint(CAT_BAD_TMPLT_ENTRY,"\
The error template for error id %08X contains an invalid %s.\n", tp->et_crcid,"detail encode 8");
	
}
