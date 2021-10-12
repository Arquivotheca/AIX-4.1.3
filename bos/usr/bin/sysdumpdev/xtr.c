static char sccsid[] = "@(#)00	1.10.1.3  src/bos/usr/bin/sysdumpdev/xtr.c, cmddump, bos411, 9428A410j 3/30/94 09:33:12";

/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: dsp_compinit, dsp_compoffsets, dsp_compall, scan, numchk
 *
 * ORIGINS: 27
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
 * Extract and format the data from a dump file and provide 
 * and interactive sub-command line interface.
 */

#define _ILS_MACROS

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/dump.h>
#include "dmpfmt.h"

#define CAT_PRINT \
	binaryflg != 0 ? 0 : cat_print	/* no formatted headers in binary mode */

extern struct cdt0 Cdt0;		/* current component dump table */
extern struct da_table *kern_dt;	/* kernel da_table	    	   */

static dsp_help();
static dsp_complist();
static dsp_dalist();
static dsp_comp();
static dsp_da();
static ic_tableinit();
static ida_tableinit();
static da_lookup();

/*
 * This 2 tables contain the name, size, and starting offset into the
 * dumpfile for each data area (component).
 * Each time a new component is selected, the Da_table is recreated
 * for that component.
 * This allows random access by data area of component name into the
 * dumpfile.
 */
struct da_table *Da_table;	/* list of data areas for curr. component */
static Da_count;					/* number of entries in Da_table */

static struct c_table *C_table;		/* list of components in 'dumpfile' */
static C_count;						/* number of entries in C_table */

static Offset;						/* starting offset of current component */
int	comp_is_lvm;					/* component is lvm component */

/*
 * NAME:     dsp_compinit
 * FUNCTION: init the C_table list of components
 * INPUTS:   none
 * RETURNS:  return -i if initialization failed.
 *
 * This routine is just the extenal name of the ic_tableinit routine.
 */
dsp_compinit()
{

	Offset = jtell();			/* set start of this component */
	return(ic_tableinit());		/* call ic_tableinit() */
}

/*
 * NAME:     dsp_compoffsets
 * FUNCTION: display list of components and their starting offsets.
 * INPUTS:   none
 * RETURNS:  none
 *
 * This routine is called by "dmpfmt -O"
 */
dsp_compoffsets()
{
	int i;

	if(C_table == 0)						/* empty dumpfile */
		return;
	for(i = 0; i < C_count; i++)			/* display name and offset */
		printf("%-16.16s 0x%06X\n",	
			C_table[i].c_name,C_table[i].c_offset); /* for each component */
}

/*
 * NAME:     dsp_compall
 * FUNCTION: display all data areas of all components
 * INPUTS:   none
 * RETURNS:  none
 *
 * This routine is called by "dmpfmt -B" and by the
 * "comp all" subcommand.
 */
dsp_compall()
{
	int i;

	if(C_table == 0)						/* empty dumpfile */
		return;
	for(i = 0; i < C_count; i++)			/* call dsp_comp */
		dsp_comp(i,1);						/* for each component */
}

/*
 * NAME:     scan
 * FUNCTION: subcommand routine
 * INPUTS:   none (input from Cmdfp)
 * RETURNS:  none
 *
 * 1. Save the start of this component in Offset.
 * 2. Intialize Da_table for this component.
 * 3. If in batch mode (-b), call dsp_da for each data area and return.
 *    Otherwise, if data areas are selected by the -n option, call dsp_da
 *    to display those.
 * 4. Then take subcommand input from Cmdfp.
 *    If the subcommand starts with '\',
 *      treat it as the name of a data area. Look it up through da_lookup()
 *      and if it exists, display it with dsp_da.
 *    If the subcommand is a number, it is the number of a data area within
 *      the current component. Display with dsp_da.
 *    If the subcommand is not a keyword (such as list, comp, quit, etc)
 *      treat as the name of a data area. Look it up through da_lookup()
 *      and if it exists, display it with dsp_da. Otherwise, display a help
 *      message.
 *    Valid keywords are:
 *      list             List data areas in current component.
 *      all              Display all data areas in current component.
 *      ?, help          Display help screen.
 *      ! [shell cmd]    Escape to shell command.
 *      quit, EOF        Quit.
 *      set display_opt  Set display option. (See opts.c)
 *      comp             List components.
 *      comp list        List components.
 *      comp compname    Set current component to 'compname'.
 *      comp all         Display all data areas of all components.
 *
 * Note: keyword matching is case-independent. Also, a match occurs if
 *       the scanned keyword matches the first n characters of a keyword,
 *       so that 'qui' matches 'quit'.
 */
scan()
{
	int i;
	int n;
	char *cp;
	char *line;
	char linebuf[128];

	Offset = jtell();	/* start of this component */
	Debug("jtell returns %x\n",Offset);
	if(ida_tableinit() < 0)
		return(-1);
	if(batchflg || allflg) {
		for(i = 0; i < Da_count; i++)
			dsp_da(i);
		if(batchflg) {
			jseek(Offset,0);
			return(0);
		}
	} else if(da_idx) {
		for(i = 0; i < da_idx; i++)
			dsp_da(da_num[i]);
	}
	if(listflg)
		dsp_dalist();
	for(;;) {
		prompt();
		line = linebuf;
		if(jgets(line,128) == 0) 
		{
			if(!binaryflg)
				printf("\n");
			break;
		}
		if(*line == '\\') {			/* leading '\' overrides keywords */
			line++;
			if(*line != '\\') {
				if((cp = strtok(line," \t\n")) && (n = da_lookup(cp)) >= 0)
					dsp_da(n);
				continue;
			}
		}
		if((cp = strtok(line," \t\n")) == 0)
			continue;
		if(numchk(cp)) {
			n = atoi(cp);
			dsp_da(n == 0 ? 0 : n-1);
			continue;
		}
		/* LVM component: map an lbuf address to pbuf address(es) */
		if (comp_is_lvm && streq_cn(cp, "lbuf")) {
			if ((cp = strtok(0," \t\n"))) {
				if (numchk(cp) || *cp == '0') {
					if (n = (int) strtol(cp,NULL,0)) {
						dsplookuplbuf(n);
						continue;
					}
				}
			}
		}
		if(streq_cn(cp,"comp")) {
			if(C_table == 0 && ic_tableinit() < 0)
				return(-1);
			if((cp = strtok(0," \t\n")) == 0) {
				dsp_complist();
				continue;
			}
			if(numchk(cp)) {
				if(n = atoi(cp))
					n--;
			} else {
 				if((n = c_lookup(cp)) < 0) {
					if(streq_cn(cp,"all")) {
						dsp_compall();
						continue;
					}
					if(streq_cn(cp,"list")) {
						dsp_complist();
						continue;
					}
					CAT_PRINT(CAT_COMP_INVALID,"%s not a component\n",cp);
					continue;
				}
			}
			dsp_comp(n,0);		/* just set component */
			continue;
		}
		if(streq_cn(cp,"list")) {
			dsp_dalist();
			continue;
		}
		if(streq_cn(cp,"all")) {
			for(i = 0; i < Da_count; i++)
				dsp_da(i);
			continue;
		}
		if(streq_cn(cp,"?") || streq_cn(cp,"help")) {
			dsp_help();
			continue;
		}
		if(streq_cn(cp,"!")) {
			cp = cp + strlen(cp) + 1;
			while(*cp && isspace(*cp))
				cp++;
			shell(*cp ? cp : 0);
			continue;
		}
		if(streq_cn(cp,"set")) {
			char *optname,*optvalue;

			optname  = strtok(0," \t\n");
			optvalue = strtok(0," \t\n");
			opts(optname,optvalue);
			continue;
		}
		if(streq_cn(cp,"quit"))
			break;
		if((n = da_lookup(cp)) >= 0) {
			dsp_da(n);
			continue;
		}
		dsp_help();
	}
	jseek(Offset,0);
	return(0);
}

/*
 * NAME:     dsp_help
 * FUNCTION: display help screen
 * INPUTS:   none
 * RETURNS:  none
 */
static dsp_help()
{
	if (comp_is_lvm) {
		CAT_PRINT(CAT_LVM_HELP,"\
lbuf <address>   show pbuf(s) corresponding to lbuf\n");
	}

	CAT_PRINT(CAT_HELP,"\
<number>         display selected data area (number is 1-based)\n\
list             list all data areas\n\
all              display all data areas for current component\n\
comp <number>    display all data areas for selected component\n\
comp list        list all components\n\
comp all         display all data areas for all components\n\
comp <number>    set current component to <number>\n\
set              show current display settings\n\
set ?            show help screen for display settings\n\
quit             exit\n\
?                show this help screen\n\
!                escape to shell\n");
}

/*
 * NAME:     dsp_complist
 * FUNCTION: display the list of components (table of contents)
 * INPUTS:   none
 * RETURNS:  none
 */
static dsp_complist()
{
	int i;

	for(i = 0; i < C_count; i++)
		printf("%3d %-16.16s\n",i+1,C_table[i].c_name);
}

/*
 * NAME:     dsp_dalist
 * FUNCTION: display the list of data areas for the current component
 * INPUTS:   none
 * RETURNS:  none
 */
static dsp_dalist()
{
	int i;

	CAT_PRINT(CAT_COMPNAME,"Component Name   %-16.16s\n\n",Cdt0.cdt_name);
	for(i = 0; i < Da_count; i++)
		printf("%3d %-8.8s\n",i+1,Da_table[i].da_name);
}

/*
 * NAME:     dsp_comp
 * FUNCTION: display the n-th component
 * INPUTS:   n           (0-based) number of the component to display.
 *           printflg    if false, do not display anything.
 * RETURNS:  none
 *
 * dsp_comp will call ida_tableinit to generate a new Da_table.
 * If printflg is false, do not display the component. This is used to
 *   change components.
 */
static dsp_comp(n,printflg)
{
	int i;
	struct c_table *ctp;

	if((unsigned)n+1 > C_count) {
		CAT_PRINT(CAT_RNGCOMP,
			"component number %d out of range ( > %d)\n",n+1,Da_count);
		return;
	}
	ctp = &C_table[n];
	jseek(ctp->c_offset,0);
	if(ida_tableinit() < 0)
		return;
	if(printflg == 0)
		return;
	CAT_PRINT(CAT_COMPNAME,"Component Name   %-16.16s\n\n",ctp->c_name);
	for(i = 0; i < Da_count; i++)
		dsp_da(i);
}

/*
 * NAME:     dsp_da
 * FUNCTION: display the n-th data area under control of the bitmap for
 *           this date area.
 * INPUTS:   n           (0-based) number of the data area to display.
 * RETURNS:  none
 *
 * If the number is out of range, print out an error message and
 *   print a list of data areas for this component.
 * If in binary mode, print data without a descriptive header and expand
 *   not-in-memory data as 0's.
 *
 * The algorithm is to display a page at a time. If the page is in memory,
 * call jdump to display it. Otherwise, call jdump_paged to in any case
 * maintain the address pointer.
 * BITMAPSIZE(ptr,len)   is the size in bytes of the bitmap.
 * NPAGES(ptr,len)       is the number of pages spanned by the virtual address
 *                       range of the data area.
 * ISBITMAP(n)  is true if relative page 'n' in in the bitmap.
 * These macros are also used in the wr_cdt() routine of the dmp_do() kernel
 *   dump routine, and are defined in sys/dump.h .
 */

static bitmap_t bm[DMP_MAXPAGES / sizeof(bitmap_t)];

static dsp_da(n)
{
	int i;
	int count;
	int tcount;
	int addr;
	int npages;
	int offset;
	int bitmapsize;
	struct da_table *tp;
	char buf[PAGESIZE];

	if((unsigned)n+1 > Da_count) {
		CAT_PRINT(CAT_RNGDATAAREA,
			"data_area number %d out of range ( > %d)\n",n+1,Da_count);
		dsp_dalist();
		return;
	}
	tp = &Da_table[n];
	jseek(tp->da_bmoffset,0);
	/*
	 * This portion is very similar to wr_cdt() in dumpdd.c
	 */
	if (comp_is_lvm == TRUE)	/* 1 is dmpbuf */
		bitmapsize = BITMAPSIZE(kern_dt->da_ptr,kern_dt->da_len);
	else
		bitmapsize = BITMAPSIZE(tp->da_ptr,tp->da_len);
	npages     = NPAGES(tp->da_ptr,tp->da_len);
	jread(bm,bitmapsize);
	if (comp_is_lvm == TRUE)	/* 1 is dmpbuf */
		jseek((off_t)dump_addr(kern_dt, tp->da_ptr),0);
	if(binaryflg) {
		addr  = tp->da_ptr;
		count = tp->da_len;
		for(i = 0; i < npages; i++) {
			offset = (unsigned)addr % PAGESIZE;
			tcount = MIN(count,PAGESIZE-offset);
			if(ISBITMAP(bm,i))
				jread(buf,tcount);
			else
				memset(buf,0,tcount);
			fwrite(buf,tcount,1,stdout);
			count -= tcount;
		}
		fflush(stdout);
		return;
	}
	CAT_PRINT(CAT_DATAAREA,"\
Data Area   %-8.8s\n\
Address     %08x\n\
Length      %04x\n\n",
		tp->da_name,tp->da_ptr,tp->da_len);
	addr  = tp->da_ptr;
	count = tp->da_len;
	jdumpinit(tp->da_ptr,ISBITMAP(bm,0));
	for(i = 0; i < npages; i++) {
		offset = (unsigned)addr % PAGESIZE;
		tcount = MIN(count,PAGESIZE-offset);
		if(ISBITMAP(bm,i)) {
			jread(buf,tcount);
			jdump(buf,tcount);
		} else {
			jdump_paged(tcount);
		}
		addr  += tcount;
		count -= tcount;
	}
	jdumpterm();
	printf("\n");
}

/*
 * NAME:     ic_tableinit
 * FUNCTION: initialize the C_table
 * INPUTS:   none
 * RETURNS:  -1 if could not initialize
 *
 * This routine calls c_tableinit() in tbl.c to allocate a new
 * C_table array.
 * It then counts the number of entries and places it it 'C_count'
 */
static ic_tableinit()
{
	struct c_table *ctp;
	int offsetsv;

	C_count = 0;
	offsetsv = jtell();
	jseek(Offset,0);
	if((C_table = c_tableinit()) == 0) {
		jseek(offsetsv,0);
		return(-1);
	}
	jseek(offsetsv,0);
	ctp = C_table;
	while(ctp->c_offset != -1) {
		C_count++;
		ctp++;
	}
	return(1);
}

/*
 * NAME:     ida_tableinit
 * FUNCTION: initialize the Da_table
 * INPUTS:   none
 * RETURNS:  -1 if could not initialize
 *
 * This routine calls da_tableinit() in tbl.c to allocate a new
 * Da_table array.
 * It then counts the number of entries and places it it 'Da_count'
 */
static ida_tableinit()
{
	struct da_table *tp;

	Da_count = 0;
	if((Da_table = da_tableinit()) == 0)
		return(-1);
	tp = Da_table;
	while(tp->da_offset != -1) {
		Da_count++;
		tp++;
	}
	return(1);
}

/*
 * NAME:     c_lookup
 * FUNCTION: Find the slot in the C_table whose component name matches
 *           the supplied name.
 * INPUTS:   name           name of component
 * RETURNS:  index into C_table[] of component 'name'
 *           -1 if not found
 */
c_lookup(name)
char *name;
{
	int i;

	for(i = 0; i < C_count; i++)
		if(streq(C_table[i].c_name,name))
			return(i);
	return(-1);
}

/*
 * NAME:     da_lookup
 * FUNCTION: Find the slot in the Da_table whose data area name matches
 *           the supplied name.
 * INPUTS:   name           name of data area
 * RETURNS:  index into Da_table[] of data area 'name'
 *           -1 if not found
 */
static
da_lookup(name)
char *name;
{
	int i;

	for(i = 0; i < Da_count; i++)
		if(streq(Da_table[i].da_name,name))
			return(i);
	return(-1);
}

/*
 * NAME:     numchk
 * FUNCTION: return true if the string is a valid decimal number
 * INPUTS:   str     character strings to check
 * RETURNS:  0 if not a number. non-zero otherwise.
 */
numchk(str)
char *str;
{
	char *cp;
	int c;

	cp = str;
	if(*cp == '-') {		/* leading -  as in   -35 */
		cp++;
		if(*cp == '\0')		/* filter out a '-' by itself */
			return(0);
	}
	while(c = *cp++)
		if(!isdigit(c))
			return(0);
	return(1);
}

