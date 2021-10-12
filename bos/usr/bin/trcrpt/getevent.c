static char sccsid[] = "@(#)98        1.27  src/bos/usr/bin/trcrpt/getevent.c, cmdtrace, bos411, 9428A410j 2/3/94 08:01:24";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: getevent, ereadinit, trcrpt_test
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
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
 *  
 */

/*
 * Read the variable-length event from the logfile into the buffer Eventbuf[].
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/trchkid.h>
#include <sys/trchdr.h>
#include "rpt.h"
#include "td.h"
#include "parse.h"

static void lstat_trcrpt();


#ifdef TRCRPT

extern FILE *Logfp;
extern verboseflg;

int Eventcount;
int Eventindex;
int Eventsize;

#endif

unsigned char Eventbuf[GENBUFSIZE+4+4+4+4];	/* max. length of event */
				    /* size max  + HW + d1 + timestamp + tid */
int offset_0;


#ifdef TRCRPT

static badtypeflg;
static struct trc_log *Logp;

extern struct lblock Lb;


extern Pid;
/*
 * NAME:     getevent
 * FUNCTION: Read the next complete event into the global buffer Eventbuf[].
 * INPUTS:   none   input is from the Logfile through eread
 * RETURNS:  RV_EOF  if EOF encountered
 *           RV_GOOD otherwise
 *           Events with bad types (synchronization loss) are turned into
 *           traceid 000, with the hope that the next event will be valid.
 *
 * The complete event into the global buffer                 Eventbuf[].
 * The number of bytes of the event is placed in             Eventsize.
 * The logfile offset of the start of the event is placed in Logidx0.
 * The traceid of the event is placed in                     Logtraceid.
 */
/*               
 * in this routine if the flag Threadflg is set :
 * 	- the size of the event is increased 4
 *	- we take the thread id in the event
 * The Threadflg flag is set in ereadinit() when :
 *	- the magic is TRC_NEW_LMAGIC
 */

getevent()
{
	unsigned type;

	if(Logidx >= Lb.lb_endoffset && next_lentry() < 0) {
		Debug("eof 1 \n ");
		return(RV_EOF);
	}
	badtypeflg = 0;
	Logidx0 = Logidx;
	if(eread(Eventbuf,4) < 0) {	/* get hookword */
		Debug("eof 2 \n ");
		return(RV_EOF);
	}
	Hookword = ((int *)Eventbuf)[0];
	type = HKWDTOTYPE(Hookword);
	Debug("hw=%x Eventbuf=%x %x %x %x type=%x\n",
		Hookword,
		Eventbuf[0],
		Eventbuf[1],
		Eventbuf[2],
		Eventbuf[3],
		type);

	if(type > HKWDTOTYPE(HKTY_LAST)) {
		cat_eprint(CAT_TPT_BADTYPE,
"Undefined event type detected on event %d, file offset 0x%x.\n\
This means that the trace file %s is not a properly formatted trace file.\n",
			Eventcount,Logidx0,Logfile);
		Eventcount++;
		Logtraceid = 0;
		badtypeflg = 1;
		return(RV_BADFORMAT);
	}
	Eventcount++;
	Logtraceid = HKWDTOHKID(Hookword);
	if((Eventsize = HKWDTOLEN(Hookword) * 4) == 0) {				/* calc. for trcgen */
		Eventsize = 4 + 4 + HKWDTOWLEN(Hookword) * 4;	/* HW + d1 + buffer */
	}
	/* + timestamp */
	if(ISTIMESTAMPED(Hookword))
		Eventsize += 4;
	Debug("size = %x et id = %d \n",Eventsize,Logtraceid);
	if (Threadflg)  {
		Eventsize += 4;
	}
	Debug("size2 = %x et id2 = %d \n",Eventsize,Logtraceid);
	if(Eventsize > sizeof(Eventbuf)) {
		int n;
		char cc;
		int event_buf_ovrflw;

		event_buf_ovrflw = Eventsize - sizeof(Eventbuf);
		cat_lwarn(CAT_TPT_BUFOVRFLW,
			"Event Buffer size of 4K exceeded,\nlast %d bytes in event may be incorrect.\n",event_buf_ovrflw);
		if(eread(Eventbuf+4,sizeof(Eventbuf)-4) < 0) {
			Debug("eof 3 \n ");
			return(RV_EOF);
		}
		n = sizeof(Eventbuf) - Eventsize;
		while(--n >= 0)
			if(eread(&cc,1) < 0) {
				Debug("eof 4 \n ");
				return(RV_EOF);
			}
	} else if(Eventsize > 4) {
		if(eread(Eventbuf+4,Eventsize-4) < 0) {
			Debug("eof 5 \n ");
			return(RV_EOF);
		}
	 }
	Timestamp = ISTIMESTAMPED(Hookword) ? *(int *)&Eventbuf[Eventsize-4] : 0;
	 Debug("Eventsize=%x Timestamp=%x %x \n", 
		Eventsize,Timestamp,ISTIMESTAMPED(Hookword));
	if (Threadflg)
		Tid = ISTIMESTAMPED(Hookword) ? *(int *)&Eventbuf[Eventsize-8] :
					*(int *)&Eventbuf[Eventsize-4];
	else
		Tid = Pid;
	Debug("Tid = %x \n",Tid);
	return(RV_GOOD);
}

/*
 * NAME:     eread
 * FUNCTION: Low overhead buffered read.
 * INPUTS:   buf    read data into the buffer 'buf'
 *           count  read 'count' bytes.
 * RETURNS:  >0  read completed successfully
 *           -1  EOF encountered during read
 *
 * This routine is similar to fread/getc
 */
static eread(buf,count)
char *buf;
{
	int n;
	char *cp;
	int c;

	cp = buf;
	n  = count;
	if(Logidx+count > Lb.lb_endoffset) {
		Debug ("idx = %x , count = %x , off =%x \n",
			Logidx, count, Lb.lb_endoffset);
		return(-1);
	}
	while(--n >= 0) {
		if((c = getc(Logfp)) == EOF) {
			Debug("n = %d \n", n);
			return(-1);
		}
		*cp++ = c;
	}
	Logidx += count;
	return(count);
}

eseek(offset)
{

	Debug("eseek(%x)\n",offset);
	if(fseek(Logfp,offset,0)) {
		perror("fseek");
		genexit(1);
	}
	Logidx = Logidx0 = offset;
}

/*
 * NAME:     trcrpt_test
 * FUNCTION: Invoked when the the program name (argv[0]) is 'trcrpt_test'
 * INPUTS:   argc,argv
 * RETURNS:  none (exits)
 *
 * trcrpt_test [filename]
 *
 * Read the logfile to look for sync. loss by getting a bad event.
 */
trcrpt_test(argc,argv)
char *argv[];
{
	int c;
	extern optind;

	while((c = getopt(argc,argv,"D")) != EOF) {
		switch(c) {
		case 'D':
			debuginit("Btrace");
			break;
		}
	}
	if(optind == argc) {
		strcpy(Logfile,"stdin");
		Logfp = stdin;
	} else {
		strcpy(Logfile,argv[optind]);
		if((Logfp = fopen(Logfile,"r")) == 0) {
			perror(Logfile);
			genexit(1);
		}
	}
	ereadinit();
	while(getevent() != RV_EOF) {
		if(badtypeflg) {
			printf("index=%x\n",Logidx0);
			genexit(1);
		}
	}
	genexit(0);
}

ereadinit()
{
	int magic;
	int size;
	int rv;
	struct trc_log_hdr log_hdr;
	struct stat statbuf;

	Lb.lb_endoffset = 0x7FFFFFFF;	/* override EOF checking in eread() */
	eseek(0);
	rv = eread(&magic,sizeof(magic));
	if(rv < 0 || (magic != TRC_LMAGIC && magic != TRC_NEW_LMAGIC))
		goto unwrapped;
	if (magic == TRC_NEW_LMAGIC)
		Threadflg++;
	eseek(0);
	if(eread(&log_hdr,sizeof(log_hdr)) < 0)
		goto unwrapped;
	size = sizeof(struct trc_log_hdr) +
		log_hdr.lh_nentries * sizeof(struct trc_log_entry);
	if(Logp)
		free(Logp);
	Logp = (struct trc_log *)jalloc(size);
	eseek(0);
	if(eread(Logp,size) < 0)
		goto unwrapped;
	/*
	 * Calculate stating lblock parameters
	 */
	if(Logp->l_wrapcount > 0) {
		/* MGS - if log wrapped around, firstentry=currentry */
		Lb.lb_firstentry = Logp->l_currentry;
		Lb.lb_lastentry = (Lb.lb_firstentry==0 ?
				   Logp->l_nentries - 1:
				   Lb.lb_firstentry - 1);
	} else {
		Lb.lb_firstentry = 0;
		Lb.lb_lastentry  = Logp->l_nentries - 1;
	}
	Lb.lb_currentry   = Lb.lb_firstentry;
	Lb.lb_startoffset = Logp->l_ic.le_offset;
	Lb.lb_endoffset   = Logp->l_ic.le_offset + Logp->l_ic.le_size;
	offset_0 = Logp->l_data[0].le_offset;
	lstat_trcrpt();
	return;
unwrapped:
	eseek(0);
	if(fstat(fileno(Logfp),&statbuf)) {
		Debug("fstat %s. %s\n",Logfile,errstr());
		genexit(1);
	}
	Threadflg++;
	Logp = (struct trc_log *)jalloc(sizeof(struct trc_log));
	Logp->l_nentries   = 0;
	Logp->l_ic.le_size = statbuf.st_size;
	Lb.lb_icflg       = 1;
	Lb.lb_firstentry  = 0;
	Lb.lb_lastentry   = 0;
	Lb.lb_currentry   = 0;
	Lb.lb_startoffset = 0;
	Lb.lb_endoffset   = statbuf.st_size;
	lstat_trcrpt();
}

next_lentry()
{
	struct trc_log_entry *lep;

	if(!Lb.lb_icflg) {
		Lb.lb_icflg++;
	} else {
		if(Lb.lb_currentry == Lb.lb_lastentry)
			return(-1);
		if(++Lb.lb_currentry >= Logp->l_nentries)
			Lb.lb_currentry = 0;
	}
	lep = &Logp->l_data[Lb.lb_currentry];
	Lb.lb_startoffset = lep->le_offset;
	Lb.lb_endoffset   = lep->le_offset + lep->le_size;
	eseek(Lb.lb_startoffset);
}

static void 
lstat_trcrpt()
{
	int i;

	vprint("threadflg = %d \n",Threadflg);
	vprint("nentries  = %d\n",Logp->l_nentries);
	vprint("currentry = %d\n",Logp->l_currentry);
	vprint("mode      = %x\n",Logp->l_mode);
	vprint("wrapcount = %d\n",Logp->l_wrapcount);
	vprint("fd        = %d\n",Logp->l_fd);
	vprint("fp        = %d\n",Logp->l_fp);
	vprint("ic: offset=%06x size=%04x\n",
		Logp->l_ic.le_offset,Logp->l_ic.le_size);
	for(i = 0; i < Logp->l_nentries; i++)
		vprint("%-2d: offset=%06x size=%04x\n",
			i,Logp->l_data[i].le_offset,Logp->l_data[i].le_size);
	vprint("firstentry =%x\n",Lb.lb_firstentry);
	vprint("lastentry  =%x\n",Lb.lb_lastentry);
	vprint("currentry  =%x\n",Lb.lb_currentry);
	vprint("startoffset=%x\n",Lb.lb_startoffset);
	vprint("endoffset  =%x\n",Lb.lb_endoffset);
}


#endif

