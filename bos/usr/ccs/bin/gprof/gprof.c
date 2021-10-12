static char sccsid[] = "@(#)54	1.20.1.9  src/bos/usr/ccs/bin/gprof/gprof.c, cmdstat, bos41B, 9504A 12/21/94 13:42:09";
/*
* COMPONENT_NAME: (CMDSTAT) Displays Call Graph Profile Data
*
* FUNCTIONS: gprof
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Copyright (c) 1980 Regents of the University of California.
* All rights reserved.  The Berkeley software License Agreement
* specifies the terms and conditions for redistribution.
*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint
*/



#include "gprof.h"
#include <stdio.h> 
#include <locale.h>
#include <filehdr.h>
#include <ldfcn.h>
#include <ar.h>
#include <sys/access.h>
#include <stdlib.h>
#include <string.h>

#include <demangle.h>
#include "gprof_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 


#define LOC(a) if (debug) fprintf (stderr,"location is: %x\n",ftell(a));

#define Do_READ(a,b,c,d,e) if (fread((void *) a, (size_t) b, (size_t) c, d) !=c) \
{ \
		eofon(a, mon_fn,e); \
		}
#define Do_WRITE(a,b,c,d,e) if (fwrite((void *) a, (size_t) b, (size_t) c, d) !=c) \
{ \
		eofon(a, sfile,e); \
		}


static int	rflag=0;		/* The rflag is used to dump out 
				 * the raw data.  Just the ticks and the
				 * symbol address.  This is useful if a 
				 * person want to get fine grain profiling
				 * on a per line basis....
				 */
static int 	multi_monfiles=0;	/* incremented for each mon.out file used */
static int	scalechanged=0;		/* flag set if scale factor changes    */
static int total_symbols=0;    /* Number of text symbols (of proper type)     */
static int vn_cc, n_cc;        /* Number of cnt structures in profile
			 * data file (later # ones used).
			 */
static char	*cmdname="gprof";
static unsigned short text_section_number;
static double          maxtime;                /* maximum time of any routine  */
static unsigned lowpc=0xFFFFFFFF, highpc=0;	/* range profiled, in UNIT's */

/* Used for "unsigned fixed-point fraction with binary scale at the left". */

#define BIAS    ((uint)0200000L)	/* This is 0x10000 */
 
#define CCADIFF (0x38+(4*5))
/*	time in gprof is stored in ticks.... */
#define SEC(ticks) ((double)(ticks))         /* Convert clock ticks to second
s. */

#define	PROC	

PROC static void charge_ticks_to_routines();
PROC static void getnames ();


	/* Number of chars in a symbol. For subsequent sizes & counts. */
#define N_NAME	255	/* 255 is max name len supported by loader */
#define MAXSTRING       2000



off_t	fsize();

	/* Memory allocation. Like malloc(), but no return if error. */
char	*Malloc();

	/* Scan past path part (if any) in the ... */
char	*basename();


/* Structure of subroutine call counters (cnt) is defined in mon.h. */
/* Structure for header of mon.out (hdr) is defined in mon.h. */

static struct outhdr head,oldhead;	/* Profile file (MON_OUT) header. */

static HISTCOUNTER *pcounts;		/* Pointer to allocated area for pcounts: 
			 	 * PC clock hit counts 
			 	 */
static HISTCOUNTER **pcounts_total;	/* Pointer to allocated area for accumulated
				 * pcounts. Used with -s option.
				 */
static struct goutcnt *ccounts;	/* Pointer to allocated area for cnt
				 *	structures: subr PC-call counts. 
				 */

char	*whoami = "gprof";

    /*
     *	things which get -E excluded by default.
     */
static char	*defaultEs[] = { "mcount" , "__mcleanup" , 0 };
static FILE    *mon_iop;               /* For profile (MON_OUT) file.  */
static char    *mon_fn = "gmon.out";   /* Default profile file name.   */
static char    *newpath;               /* redirecting load object module   */
static int     Lflg=0,         /* Different shared object pathname */
	lflg=0;         /* reroute objects as well...       */
static int 	once=0;		/* first occurrence of file         */
static float   last_sf=999;	/* last Scale for index into pcounts:
			    i(pc) = ((pc - pc_l) * sf)/sf.
			   initialized to 999 for ID           */


main(argc, argv)
    int argc;
    char **argv;
{
    char	**sp;
    nltype	**timesortnlp;

    (void) setlocale(LC_ALL, "");

    catd = catopen(MF_GPROF,NL_CAT_LOCALE);

    --argc;
    argv++;
    debug = 0;
    bflag = TRUE;
    while ( *argv != 0 && **argv == '-' ) {
	(*argv)++;
	switch ( **argv ) {
	case 'b':
	    bflag = FALSE;
	    break;
	case 'c':
	    cflag = TRUE;
	    break;
#ifdef DEBUG
	case 'd':
	    dflag = TRUE;
	    (*argv)++;
	    debug |= atoi( *argv );
	    debug |= ANYDEBUG;
	    fprintf(stderr, "[main] debug = %d\n", debug);
	    break;
#endif DEBUG
	case 'L':		/* different default directory */
	    Lflg++;
	    (*argv)++;
	    if (**argv == '\0')
		    argv++;
	    newpath= *argv;
	    if (debug)
		    fprintf(stderr, "[main] newpath = %s\n", newpath);
	    break;
	case 'E':
	    ++argv;
	    addlist( Elist , *argv );
	    Eflag = TRUE;
	    addlist( elist , *argv );
	    eflag = TRUE;
	    break;
	case 'e':
	    addlist( elist , *++argv );
	    eflag = TRUE;
	    break;
	case 'F':
	    ++argv;
	    addlist( Flist , *argv );
	    Fflag = TRUE;
	    addlist( flist , *argv );
	    fflag = TRUE;
	    break;
	case 'f':
	    addlist( flist , *++argv );
	    fflag = TRUE;
	    break;
	case 'r':
	    rflag = TRUE; 	/* The rflag is used to dump out 
				 * the raw data.  Just the ticks and the
				 * symbol address.  This is useful if a 
				 * person want to get fine grain profiling
				 * on a per line basis....
				 */
	    break;
	case 's':
	    sflag = TRUE;
	    break;
	case 'z':
	    zflag = TRUE;
	    break;
	default:
	    fprintf (stderr,MSGSTR(INVOPT,"invalid option\n"));
	    fprintf (stderr,MSGSTR(OPT1,"usage: gprof [-b][-c][-s][-z][-e name]"));
	    fprintf (stderr,MSGSTR(OPT2,"[-E name][-f name][-F name][-L Pathname] [gmon.out ...] \n"));
	    exit(-1);
	}
	argv++;
    }
    if ( *argv != 0 ) {
	a_outname  = *argv;	/* We will use this a.out file instead of */
	argv++;			/* one specified in the gmon.out file.    */
	lflg++;
    } 
    if ( *argv != 0 ) {
	gmonname = *argv;	/* Use a alternative gmon.out file.       */
	argv++;
    } else {
	gmonname = GMONNAME;	/* Default is gmon.out                    */
    }
	/*
	 *	turn off default functions
	 */
    for ( sp = &defaultEs[0] ; *sp ; sp++ ) {
	Eflag = TRUE;
	addlist( Elist , *sp );
	eflag = TRUE;
	addlist( elist , *sp );
    }
	/*
	 *	how many ticks per second?
	 *	if we can't tell, report time in ticks.
	 */
    hz = hertz();
    if (hz == 0) {
	hz = 1;
	fprintf(stderr, MSGSTR(TICKS, "time is in ticks, not seconds\n")); /*MSG*/
    }
	/*
	 *	get information about gmon.out file(s).
	 */
    do	{
	getpfile( gmonname );
	if ( *argv != 0 ) {	/* Call once for each gmon.out file on the */
	    gmonname = *argv;	/* command line.                           */
	}
    } while ( *argv++ != 0 );
	/*
	 *	dump out a gmon.sum file if requested
	 */
    if ( sflag ) {
     dumpsum( GMONSUM );
    }
	/*
	 *	assemble the dynamic profile
	 */
    timesortnlp = doarcs();
	/*
	 *	print the dynamic profile
	 */
    printgprof( timesortnlp );	
	/*
	 *	print the flat profile
	 */
    printprof();	
	/*
	 *	print the index
	 */
    printindex();	
    done();
}


    /*
     *	information from a gmon.out file is in five parts:
     *	a header, load table, range table, an array of sampling
     *  hits within pc ranges, and the arcs.
     */

static getpfile(filename)
    char *filename;
{
    FILE		*pfile;
    FILE		*openpfile();
    struct rawarc	arc;

    mon_fn = filename;
    mon_proc(filename);
    multi_monfiles++;
}


static tally( rawp )
    struct rawarc	*rawp;
{
    nltype		*parentp;
    nltype		*childp;

#   ifdef DEBUG
    if ( debug & TALLYDEBUG )
	fprintf(stderr, "[tally]: frompc: %x  selfpc: %x   rawcount: %x\n",
		rawp->raw_frompc,rawp->raw_selfpc,rawp->raw_count);
#   endif DEBUG

    if (( parentp = nllookup( rawp -> raw_frompc )) == 0) {
	fprintf (stderr,MSGSTR(BADDATA,"lookup failed!! Data corrupt?\n"));
	return;
    }
    if ((childp = nllookup( rawp -> raw_selfpc )) == 0) {
	fprintf (stderr,MSGSTR(BADDATA,"lookup failed!! Data corrupt?\n"));
	return;
    }
    childp -> ncall += rawp -> raw_count;
#   ifdef DEBUG
	if ( debug & TALLYDEBUG ) {
	    fprintf(stderr,  "[tally] arc from %s to %s traversed %d times\n" ,
		    parentp -> name , childp -> name , rawp -> raw_count );
	}
#   endif DEBUG
    addarc( parentp , childp , rawp -> raw_count );
}

/*
 * dump out the gmon.sum file
 */
static dumpsum( sumfile )
    char *sumfile;
{
    register nltype *nlp;
    register arctype *arcp;
    struct rawarc arc;
    FILE *sfile;
    char *buffer;
    struct outrng *ranges;
    int n;

    if ( ( sfile = fopen ( sumfile , "w" ) ) == NULL ) {
	perror( sumfile );
	done();
    }
	if ((mon_iop = fopen(mon_fn, "r")) == NULL)
		perror(mon_fn);

	/* Get size of file containing profiling data. Read header part. */

	Do_READ (&head, sizeof(struct outhdr), 1, mon_iop,"Header")
	Do_WRITE(&head, sizeof(struct outhdr), 1, sfile,  "Header")
	buffer = Malloc (head.lbs,1);
	Do_READ  (buffer, head.lbs,1,mon_iop,"loader stuff");
	Do_WRITE (buffer, head.lbs,1,sfile,"out loader stuff");
	ranges = (struct outrng *) Malloc (sizeof(struct outrng),head.nrngs);
	Do_READ  (ranges, sizeof(struct outrng),head.nrngs,mon_iop,"range stuff");
	Do_WRITE (ranges, sizeof(struct outrng),head.nrngs,sfile,"out range stuff");
	for (n=0;n<head.nrngs;n++) {
		Do_WRITE (pcounts_total[n], ranges[n].nhcnt * sizeof(HISTCOUNTER),1,sfile,"out history stuff");
	}

    /*
     * dump the normalized raw arc information
     */
    for ( nlp = nl ; nlp < npe ; nlp++ ) {
	for ( arcp = nlp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
	    arc.raw_frompc = arcp -> arc_parentp -> value;
	    arc.raw_selfpc = arcp -> arc_childp -> value;
	    arc.raw_count = arcp -> arc_count;
	    if ( fwrite ((void *)&arc , (size_t)sizeof (struct rawarc) , (size_t)1 , sfile ) != 1 ) {
		perror( sumfile );
		done();
	    }
#	    ifdef DEBUG
		if ( debug & SAMPLEDEBUG ) {
		    fprintf(stderr,  "[dumpsum] frompc 0x%x selfpc 0x%x count %d\n" ,
			    arc.raw_frompc , arc.raw_selfpc , arc.raw_count );
		}
#	    endif DEBUG
	}
    }
    fclose( sfile );
}

static valcmp(p1, p2)
    nltype *p1, *p2;
{
    if ( p1 -> value < p2 -> value ) {
	return LESSTHAN;
    }
    if ( p1 -> value > p2 -> value ) {
	return GREATERTHAN;
    }
    return EQUALTO;
}


done()
{
    exit(0);
}

static PROC
mon_proc(name)
char *name;
{
	int	i,n;
	HISTCOUNTER	sample;
	long	begin_of_loadinfo;
	HISTCOUNTER *pcounts;	/* Pointer to allocated area for pcounts:
				 * PC clock hit counts */
	struct outrng *range;
	int rng;
	char    *pc_l;          /* From header. */
	char    *pc_h;          /*   "  header. */
	int n_pc;       /* Number of pcounts in profile data file. */
	struct goutcnt *ccp;    /* For scanning ccounts. */



		/* Open monitor data file */

	if ((mon_iop = fopen(mon_fn, "r")) == NULL) {
		perror(mon_fn);
		exit (-1);
	}

	/* Get size of file containing profiling data. Read header part. */

	Do_READ(&head, sizeof(struct outhdr), 1, mon_iop,"Header")

	if (multi_monfiles && oldhead.nrngs != head.nrngs 
				&& oldhead.lbs != head.lbs) {
		fprintf (stderr,MSGSTR(BADMON,"mon.out %d file doesn't match\n"),multi_monfiles);
		exit(1);
	}
	oldhead = head;

	begin_of_loadinfo = ftell (mon_iop);

		/* Count the symbols...but only once! */

	if (!multi_monfiles)
		nname = total_symbols= count_symbols(begin_of_loadinfo,mon_iop);

	LOC(mon_iop);
	fseek (mon_iop, head.lbs + sizeof (struct outhdr),0);
	LOC(mon_iop)
	range = (struct outrng *) Malloc (head.nrngs, sizeof (struct outrng));

	Do_READ(range, sizeof(struct outrng), head.nrngs, mon_iop,"ranges")

		/*
		 * save tick counts to write them out again if
		 * the -s option is given.
		 */
	if (sflag && !pcounts_total) 
		pcounts_total = (HISTCOUNTER **) Malloc (head.nrngs,sizeof(HISTCOUNTER *));

	for (rng = 0; rng < head.nrngs; rng++) {
		pc_h = range[rng].hpc;
		pc_l = range[rng].lpc;

		pcounts = (HISTCOUNTER *) Malloc (range[rng].nhcnt, sizeof (HISTCOUNTER));
		if (sflag && !pcounts_total[rng]) 
			pcounts_total[rng] = (HISTCOUNTER *) Malloc (range[rng].nhcnt, sizeof (HISTCOUNTER));
		
		n_pc = range[rng].nhcnt;
		if (rflag)
			fprintf(stderr,"Range high: %x  low: %x counters: %x\n",pc_h,pc_l,n_pc);
		for ( i = 0 ;i<n_pc ; i++ ) {
			Do_READ(&sample, sizeof(HISTCOUNTER), 1, mon_iop,"samples")
			pcounts[i] = sample;
			if (sflag)
				pcounts_total[rng][i] += sample;
			if ( debug )
			    if ( pcounts[i] != 0 )
				fprintf(stderr,"pcounts[%d] is %o\n",i,pcounts[i]);
			if (rflag && (pcounts[i] != 0))
				fprintf(stderr,"tick bucket: %x is: %x\n",i,pcounts[i]);
		}
		charge_ticks_to_routines(pc_h,pc_l,n_pc,pcounts);
		if ((unsigned)pc_l < lowpc)
			lowpc = (unsigned)pc_l / sizeof(UNIT);
		if ((unsigned)pc_h > highpc)
			highpc = (unsigned)pc_h / sizeof(UNIT);

	}

	/* 
	 * The remaining file is a list of call counts.  To find how
	 * many of them, why take the size and divide by the size of
	 * one element.
	 */
	n = fsize(fileno(mon_iop));
	n_cc = (int) ((n - ftell (mon_iop)) / sizeof (struct goutcnt));

        if (n_cc == 0) {
                fprintf (stderr,MSGSTR(NOCNTS1,"Warning: mon.out file has no call counts.  Program\n"));
                fprintf (stderr,MSGSTR(NOCNTS2,"    possibly not compiled with profiled libraries.\n"));
        }
	else
	   ccounts = (struct goutcnt *)Malloc(n_cc, sizeof(struct goutcnt));

	if ( debug )
	    fprintf (stderr,"number of functions: %x\n",n_cc);

		/* Read the call addr-count pairs. */

	LOC(mon_iop)

	Do_READ(ccounts, sizeof(struct goutcnt), n_cc, mon_iop,"call counts")

	LOC(mon_iop)

	for (i=0;i<n_cc;i++) {
		if (debug) 
			fprintf (stderr,"from: %x self: %x count: %x\n",
				ccounts[i].raw_frompc, ccounts[i].raw_selfpc, ccounts[i].raw_count);
		tally( &ccounts[i] );
	}
	fclose (mon_iop);
}

/*
 * NAME: count_symbols
 *                                                                    
 * FUNCTION: 
 * 	Given a mon.out file, we build a nl structure and initialize:
 *
 *		-the names of the routines in program, 
 *		-the number of seconds spent in each routine
 *		-the address of the routine.
 *
 *	Once this is built, it will be passed the call count data
 *	will be read in and charged to each routine.
 *
 *	For each object loaded, there is an entry in the load table.
 *	This file is opened up, the run-time load address is added 
 *	to each symbol, and it is appended to the nl array structure.
 *	Since the array is dynamically built, we make one pass thru just
 *	counting the symbols.  Then we malloc the space and read thru
 *	one last time, filling the structure.
 *                                                                    
 * RETURNS: 
 *	The total number of symbols returned in the nl structure.
 */  

PROC int 
static count_symbols (ptr,mon_iop)
FILE *mon_iop;
long ptr;
{
int 	finished = 0;			/* set when last load module found  */
int	tot_symbols=0;			/* count total symbols              */
char	tmpbuf[2 * MAXSTRING+1];	/* buffer to read load module names */
char	tmpname[MAXSTRING+1];		/* tmp object file load name        */
char	filename[MAXSTRING+1];		/* object file load name            */
char	member[MAXSTRING+1];		/* if load module is a archive      */
char	chtmpname[MAXSTRING+1];		/* temporary holding place for name */
int	loc;				/* where are we in file (debugging) */
uint	next;				/* pointer to next load module      */
caddr_t	textorg;			/* run-time load address of module  */
int 	dups=0;				/* There maybe duplicate entries    */
int	i,n;
struct	nl *slp;
int load_modules=0;
char *p;

	if (fseek (mon_iop,ptr,0) != 0) {
		perror ("gprof. can't seek");
		exit (-1);
	}


		/*
		 * We aren't specifically told how many modules were loaded
		 * so we must go until the next field is NULL
		 */

	while (!finished) {

	   LOC(mon_iop)

	   Do_READ (&next, sizeof(uint),1, mon_iop, "next offset read failed")

	   LOC(mon_iop)

	   Do_READ (&textorg, sizeof(caddr_t), 1, mon_iop,"cant read textorg")

	   LOC(mon_iop)

		if (next == 0) {
			finished++;
			(void) fgets (filename, MAXSTRING, mon_iop);
			 strncpy (member,(strchr(filename,'\0')+1),MAXSTRING);
		}
		else if (next > 2 * MAXSTRING) {
			printf (MSGSTR(STR2BIG,"Filenames/paths too big in loader table\n"));
			exit(-1);
		}
		else {
			Do_READ (tmpbuf, (next - (uint) sizeof(uint) 
				- (uint) sizeof(void *)),
				1 ,mon_iop,"string read failed")

			getnames (filename,member,tmpbuf,MAXSTRING);

		}

		if (lflg && !once) {
			strncpy (filename, a_outname,MAXSTRING);
			once++;
		}
		else if (Lflg) {
			if ((p = strrchr(filename,'/')) != NULL) {
				strcpy (tmpname,p);
				if ( debug & AOUTDEBUG )
				    fprintf(stderr, "tmpname: %s filename: %s\n",tmpname,filename);
				sprintf(chtmpname,"%s/%s",newpath,tmpname);
                                if (!access(chtmpname,E_ACC))
                                {           
				 sprintf(filename,"%s/%s",newpath,tmpname);
                                }
				if ( debug & AOUTDEBUG )
				    fprintf(stderr, "newpath: %s \n",newpath);
			}
		}

		if ( debug & AOUTDEBUG ) {
			fprintf(stderr, "filename: %s  ",filename);
			fprintf(stderr, "member:   %s\n",member);
		}
				/* NULL means not to save it */
		tot_symbols += digest_object (filename,member,NULL,NULL);

	   if ( debug & AOUTDEBUG )
		fprintf(stderr, "curr local is: %x syms: %x\n",ftell(mon_iop),tot_symbols);
	}

	/* 
	 * Now we have the size of the array, we can malloc the space.
	 * We could have coded this routine fancily by making a loop or
	 * saving all the load modules, but for the sake of clear logic
	 * flow (and my hung over brain), we'll just dup the code and do
	 * it again.
	 */

	finished =0;	/* doit a second time */
	nl = slp = (struct nl *)Malloc(tot_symbols, sizeof(struct nl));
	tot_symbols = 0;	    /* now fill and count how many qualify */
	once=0;

	if (fseek (mon_iop,ptr,0) != 0) {	/* reset pointer */
		perror ("can't seek2");
		exit (-1);
	}

					/* Ah! doesn't this look familiar? */
	while (!finished) {

	   Do_READ (&next, sizeof(uint),1, mon_iop,"next offset rd failed")
	   Do_READ (&textorg, sizeof(void *),1, mon_iop, "text org RD failed")

	   if (next == 0) {
		finished++;
		(void) fgets (filename, MAXSTRING, mon_iop);
		 strncpy (member,(strchr(filename,'\0')+1),MAXSTRING);
	   }
	   else if (next > 2 * MAXSTRING) {
		printf (MSGSTR(STR2BIG,"Filenames/paths too big in loader table\n"));
		exit(-1);
	   }
	   else {
		Do_READ (tmpbuf, (next - (uint) sizeof(uint) 
			- (uint) sizeof(void *)),1,mon_iop,"string read failed")

		getnames (filename,member,tmpbuf,MAXSTRING);
	   }

		if (lflg && !once) {
			strncpy (filename, a_outname,MAXSTRING);
			once++;
		}
		else if (Lflg) {
			if ((p = strrchr(filename,'/')) != '\0') {
				strcpy (tmpname,p);
				sprintf(chtmpname,"%s/%s",newpath,tmpname);
                                if (!access(chtmpname,E_ACC))
                                {           
				 sprintf(filename,"%s/%s",newpath,tmpname);
                                }
			}
		}


	   if ( debug & AOUTDEBUG )
		fprintf(stderr, "filename: %s  member:%s\n",filename,member);
	   
		/* 
		 * call routine again, but this time give it a place
		 * to fill symbols
		 */
	   tot_symbols += digest_object (filename,member,&slp,textorg);
	   load_modules++;

	   if ( debug & AOUTDEBUG )
		fprintf (stderr,"total_sym: %d  slp: %x\n",tot_symbols,slp);
	}

	LOC(mon_iop)

		/* NOTE: may have to align the mon_iop */

		/* Sort symbols by increasing address. */
	slp = nl;
	qsort(slp, tot_symbols, sizeof(struct nl), valcmp);

	/*
	 *      Get rid of duplicate addresses.
	 */
	for (n = 1, i = 0; n < tot_symbols; n++) {
		if (nl[i].value != nl[n].value)
			i++;
		else
			dups++;
		if (i < n)
			nl[i] = nl[n];
	}
	tot_symbols -=  dups;
	if (rflag || ( debug & AOUTDEBUG )) 
		for (i=0;i<tot_symbols;i++)

			fprintf(stderr,"Symbol name: %s \t %x\n", nl[i].name,nl[i].value);
	return (tot_symbols);
}


/*
 * NAME: digest_object
 *                                                                    
 * FUNCTION: 
 *
 *	Open up the file given (search to its the appropriate member,
 *	if it is an archive), if no storage pointer is given, just
 *	return the number of symbols.  If a storage pointer is given
 *	read each symbol and add it to the nl structure.  The text
 *	run-time load address is also add to the function address since
 *	counts and the histigram deal with runtime addresses.
 *
 *	Note: This routine uses the loader library routines which
 *	      are located in libld.a  Also to get the correct offsets,
 *	      we need to define -DAIXV3AR if it is not the default.
 *
 * RETURNS: 
 *	The number of symbols in this object is returned.
 */  


static PROC
digest_object (filename,member,ptr,textbase)
char 		*filename;
char 		*member;
struct nl	**ptr;
caddr_t		textbase;
{

  FILHDR	filehead;
  archdr	arhead;
  LDFILE	*ldptr;
  SYMENT	symbol;
  AUXENT	auxsym;
  SCNHDR 	sexpoiter;
  AOUTHDR	opt_hdr;

  int		glu_code;		/* set if identified symbol glu code */
  long i,ii;
  struct nl	*slp;
  int		filled;
  static char		*sym_name;
  unsigned short sec;
  struct stat stat_buf;

	if (stat (filename,&stat_buf) != 0){
		perror (filename);
		exit(-1);
	}

	filled = 0;
	ldptr = NULL;
	do {
		if ((ldptr = ldopen(filename,ldptr)) != NULL) {
		
			if (ldfhread (ldptr,&filehead) == FAILURE) {
				fprintf (stderr,MSGSTR(FILEHEAD,"couldn't read fileheader\n"));
				exit (-1);
			}
			if (debug & AOUTDEBUG)
				fprintf (stderr,"num syms is: %d\n",filehead.f_nsyms);
			if (*member != '\0') {
				if (ldahread (ldptr,&arhead) == FAILURE) {
					fprintf (stderr,MSGSTR(NOACHIVE,"ldahread failed. Not an archive.\n"));
					exit(-1);
				}
				else {
					if (debug & AOUTDEBUG)
						fprintf (stderr,"arch object is: %s\n",arhead.ar_name);
					if (strcmp(member,arhead.ar_name) != 0)
						continue;
				}
			}
			if ( ldohseek (ldptr) == FAILURE) {
				fprintf(stderr,"No optional header\n");
				exit(-1);
			}
			Do_READ (&opt_hdr, sizeof(AOUTHDR), 1, IOPTR(ldptr),"No Optional Header")
			textbase -= opt_hdr.text_start;
			if (debug & AOUTDEBUG)
			    fprintf(stderr, "textbase: %x, text_start: %x\n",
				textbase,opt_hdr.text_start);
			/* 
			 * If the object that is being process has been 
			 * stripped, then we will attribute all of the time to
			 * this object instead of individual routines inside the
			 * object.
			 * In a future enhancement we could get the symbols of
			 * the external routines from the loader table instead
			 * of the symbol table.
			 */
			if (ldtbseek (ldptr) == FAILURE) {
				if (ptr == NULL) 	/* first passed, */
					return (1); 	/* only count    */
				fprintf (stderr,MSGSTR(SEEKER,"%s(%s) is stripped.\n"),filename,member);
				slp = *ptr;
				slp->value = (ulong) textbase;
				slp->name = Malloc (1,strlen(member)+strlen(filename)+5);
				sprintf (slp->name,"%s(%s) {stripped object}",basename(filename),member);
				slp->time = 0.0;
				npe = ++slp;
				*ptr = slp;
				ldaclose(ldptr);
				return(1);	/* only one symbol used */
			}

			if (ptr == NULL) {    /* Just interested in the count */
				ldaclose (ldptr);
				return (filehead.f_nsyms);
			}
			for (sec = (unsigned short) 1; sec <= filehead.f_nscns;sec++) {
				if (ldshread (ldptr,sec,&sexpoiter) == FAILURE) {
					fprintf (stderr,MSGSTR(SCN,"Ldnshread failed\n"));
					exit (-1);
				}
				if (strcmp(_TEXT,sexpoiter.s_name) == 0) {
					if (debug & AOUTDEBUG)
						fprintf(stderr, "text section is: %x name: %s\n",sec,sexpoiter.s_name);
					text_section_number = sec;
					break;
				}
			}

			slp = *ptr;

			for (i= 0; i<filehead.f_nsyms; i++) {
				if ( ldtbread (ldptr,i,&symbol) == FAILURE) {
					printf(MSGSTR(NOSYM,"Couldn't read symbol: %d\n"),i);
					exit(-1);
				}

				symbol.n_value += (unsigned long) textbase;
				sym_name = dem_conv(ldgetname(ldptr, &symbol));
				if (sym_name == INVAL)	
					sym_name = ldgetname(ldptr, &symbol);

				if (debug & AOUTDEBUG) {
					fprintf (stderr,"Symbol: %d is: %s\t",i,sym_name);
					fprintf (stderr," val: %x  scnum: %x  type: %x  class: %x numaux: %x\n",
						(int) symbol.n_value,
						(int) symbol.n_scnum,
						(int) symbol.n_type,
						(int) symbol.n_sclass,
						(int) symbol.n_numaux);
					}
				if (isvalidsymbol (&symbol, sym_name)) {

					glu_code = 0;
					for (ii=1;ii <= symbol.n_numaux;ii++) {
						if ( ldtbread (ldptr,i+ii,&auxsym) == FAILURE) {
							fprintf (stderr,"couldn't read auxillary entry\n");
							exit(-1);
						}
if (debug & AOUTDEBUG)
    fprintf (stderr,"Aux Symbol: %d is: %s smclas: %x\n",
	i+ii, ldgetname(ldptr,&auxsym), (int) auxsym.x_csect.x_smclas);
						if (auxsym.x_csect.x_smclas == XMC_GL)
							glu_code++;
					}

					if (glu_code)
						slp->name = Malloc (1,N_NAME+5);
					else
						slp->name = Malloc (1,N_NAME+1);

					/*  fill in the nl struct */
 					(void)mbsncpy(slp->name, sym_name, N_NAME);
					slp->name[N_NAME] = '\0';  

					if (glu_code)
						strcat (slp->name,".GL");

					slp->value = (ulong)symbol.n_value;

					   /* set other nl fields to zero. */

					slp->time = 0.0;
					if (debug & AOUTDEBUG)
					    fprintf(stderr, "Add: %-10.10s: %x\n",
						slp->name, slp->value);
					slp++;
					filled++;
					slp->value=-1;
				}

				i += (long) symbol.n_numaux;
			}
			npe = slp;
			*ptr = slp;
			ldaclose(ldptr);
			return (filled);
		}
	} while (ldclose(ldptr) == FAILURE);
}


/*
 * NAME: charge_ticks_to_routines
 *                                                                    
 * FUNCTION: 
 *
 *	Time is charged to routines in the nl structure.  Ticks
 *	are found in a histigram buffer pointed to by "pcounts".  The
 *	nearest routine is charged for each tic in the buffer.
 *
 * RETURNS: 
 *	none
 */  


PROC
static void
charge_ticks_to_routines(pc_h,pc_l,n_pc,pcounts)
char	*pc_l;		
char	*pc_h;	
int	n_pc;
HISTCOUNTER *pcounts;	/* Pointer to allocated area for pcounts: PC clock hit counts */
{
	uint sf;	/* Scale for index into pcounts:
					i(pc) = ((pc - pc_l) * sf)/sf. */

	uint s_inv;	/* Inverse: i_inv(i) =
					{pc00, pc00+1, ... pc00+s_inv-1}. */

	unsigned pc_m;	/* Range of PCs profiled: pc_m = pc_h - pc_l */
	register HISTCOUNTER *pcp;	/* For scanning pcounts. */
	float t, t0;
	int i,n;
	struct nl *slp;

	/*
	Having gotten preliminaries out of the way, get down to business.
	The range pc_m of addresses over which profiling was done is
	computed from the low (pc_l) and high (pc_h) addresses, gotten
	from the MON_OUT header.  From this and the number of clock
	tick counters, n_pc, is computed the so-called "scale", sf, used
	in the mapping of addresses to indices, as follows:

			(pc - pc_l) * sf
		i(pc) = ----------------
			 0200000

	Also, the N-to-one value, s_inv, such that

		i(pc_l + K * s_inv + d) = K, for 0 <= d < s_inv

	Following this, the symbol table is scanned, and those symbols
	that qualify are counted.  These  are T-type symbols, excluding
	local (nonglobal) unless the "-g" option was given. Having thus
	determined the space requirements, space for symbols/times etc.
	is allocated, and the symbol table re-read, this time keeping
	qualified symbols.
	*/


		/* Range of profiled addresses. */

	pc_m = (unsigned int) pc_h - (unsigned int) pc_l;	

	if ( debug & CHARGEDEBUG ) {
	    fprintf(stderr, "low pc = %x, high pc = %x, range = %x = %u\n",
		pc_l, pc_h, pc_m, pc_m);
	    fprintf(stderr, "call counts: %u, %u used; pc counters: %u\n",
		total_symbols, n_cc, n_pc);
	}

	scale = (double) (pc_h - pc_l) / (double) n_pc;
	sf = (float) ((float)BIAS * (float)n_pc)/(float)pc_m;  /* The "scale" used to map PCs to indices. */

	if (last_sf != 999 && sf != last_sf)
		scalechanged++;
	else
		last_sf = sf;
	s_inv = pc_m/n_pc;	  /* Range of PCs mapped into one index.     */
	if (s_inv == 0)
		s_inv = 1;

	if ( debug & CHARGEDEBUG )
	    fprintf(stderr, "sf = %x, s_inv = %ld\n", (long)sf, s_inv);

	/*
	The distribution of times to addresses is done on a proportional
	basis as follows: The t counts in pcounts[i] correspond to clock
	ticks for values of pc in the range pc, pc+1, ..., pc+s_inv-1
	(odd addresses excluded for PDP11s).  Without more detailed
	information, it must be assumed that there is no greater probability
	of the clock ticking for any particular pc in this range than for
	any other.  Thus the t counts are considered to be equally distributed
	over the addresses in the range, and that the time for any given
	address in the range is pcounts[i]/s_inv.

	The values of the symbols that qualify, bounded below and above
	by pc_l and pc_h, respectively, partition the profiling range into
	regions to which are assigned the total times associated with the
	addresses they contain in the following way:

	The sum of all pcounts[i] for which the corresponding addresses are
	wholly within the partition are charged to the partition (the
	subroutine whose address is the lower bound of the partition).

	If the range of addresses corresponding to a given t = pcounts[i]
	lies astraddle the boundary of a partition, e.g., for some k such
	that 0 < k < s_inv-1, the addresses pc, pc+1, ..., pc+k-1 are in
	the lower partition, and the addresses pc+k, pc+k+1, ..., pc+s_inv-1
	are in the next partition, then k*pcounts[i]/s_inv time is charged
	to the lower partition, and (s_inv-k) * pcounts[i]/s_inv time to the
	upper.  It is conceivable, in cases of large granularity or small
	subroutines, for a range corresponding to a given pcounts[i] to
	overlap three regions, completely containing the (small) middle one.
	The algorithm is adjusted appropriately in this case.
	*/


	pcp = pcounts;				/* Reset to base. */
	slp = nl;				/* Ditto. */
	t0 = 0.0;				/* Time accumulator. */
	for (n = 0; n < total_symbols; n++) {		/* Loop on symbols. */
			/* Start addr of region, low addr of overlap. */
		char *pc0, *pc00;
			/* Start addr of next region, low addr of overlap. */
		char *pc1, *pc10;
		 /* First index into pcounts for this region and next region. */
		register int i0, i1;
		long ticks;

			/* Address of symbol (subroutine). */
		pc0 = (char *)slp[n].value;

			/* 
			 * Skipped this symbol if not within the range...
			 * i.e. it's from another object load module.
			 */

		if ((pc0 < pc_l) || (pc0 > pc_h))
			continue;

			/* Address of next symbol, if any or top of profile
								range, if not */
		if (n < total_symbols - 1) 
			pc1 = (char *)slp[n+1].value;
		else
			pc1 = pc_h;

			/* Lower bound of indices into pcounts for this range */

		i0 = ((pc0 - pc_l) * (float)sf)/(float)BIAS;

			/* Upper bound (least or least + 1) of indices. */

		i1 = ((pc1 - pc_l) * (float)sf)/(float)BIAS;

		if ((uint) i1 >= (uint) n_pc)				/* If past top, */
			i1 = n_pc - 1;			/*      adjust. */

			/* Lowest addr for which count maps to pcounts[i0]; */

		pc00 = pc_l + (long)(((float)BIAS * i0)/(float)sf);

			/* Lowest addr for which count maps to pcounts[i1]. */

		pc10 = pc_l + (long)(((float)BIAS * i1)/(float)sf);

		if ( debug & CHARGEDEBUG ) {
		    fprintf(stderr, "%-8.8s\ti0 = %4d, pc00 = %#6o, pc0 = %#6o\n",
			    slp[n].name, i0, pc00, pc0);
		    fprintf(stderr, "\t\ti1 = %4d, pc10 = %#6o, pc1 = %#6o\n\t\t",
			    i1, pc10, pc1);
		}
		t = 0.0;		/* Init time for this symbol. */
		if (i0 == i1) {

			/* 
			 * Counter overlaps two areas? (unlikely 
			 * unless large granularity). 
			 */

			ticks = pcp[i0];	/* # Times (clock ticks). */

			    /* Time less that which overlaps adjacent areas */

			t += (pc1 - pc0) * SEC(ticks)/s_inv;

			if ( debug & CHARGEDEBUG )
			    fprintf(stderr, "%ld/%ld", (pc1 - pc0) * ticks, s_inv);
		} else {
				/* Overlap with previous region? */
			if (pc00 < pc0) {
				ticks = pcp[i0];

				/* Get time of overlapping area and subtract
						proportion for lower region. */
				t += SEC(pcp[i0]) - (pc0 - pc00) * SEC(ticks)/s_inv;

				/* Do not count this time when summing times
						wholly within the region. */
				i0++;
				if ( debug & CHARGEDEBUG )
				    fprintf(stderr, "%ld/%ld + ",
					(pc0 - pc00) * ticks, s_inv);
			}

			/* Init sum of counts for PCs not shared w/other
								routines. */
			ticks = 0;

			/* Stop at first count that overlaps following
								routine. */
			for (i = i0; i < i1; i++)
				ticks += pcp[i];

			t += SEC(ticks);  /* Convert to secs & add to total. */
			if ( debug & CHARGEDEBUG )
			    fprintf(stderr, "%ld", ticks);
			/* Some overlap with low addresses of next routine? */
			if (pc10 < pc1) {
					/* Yes. Get total count ... */
				ticks = pcp[i1];

				/* and accumulate proportion for addresses in
							range of this routine */
				t += (pc1 - pc10) * SEC(ticks)/s_inv;
				if ( debug & CHARGEDEBUG )
				    fprintf(stderr, " + %ld/%ld",
					(pc1 - pc10) * ticks, s_inv);
			}
		}		/* End if (i0 == i1) ... else ... */

		slp[n].time += t;	/* Store time for this routine. */
		t0 += t;		/* Accumulate total time. */
		if ( debug & CHARGEDEBUG )
		    fprintf(stderr, " ticks = %.2f msec\n", t);
	}	/* End for (n = 0; n < total_symbols; n++) */

	/* Final pass to total up time. */

	for (n = n_pc; --n >= 0; )	{
		if (SEC(*pcp) > maxtime)
			maxtime = SEC(*pcp);
		totime += SEC(*pcp++);
	}
}


/*
 * NAME: getnames
 *                                                                    
 * FUNCTION: 
 *
 *	Fill in the filename and member name found in the buffer
 *	pointed to by "tmpbuf".  These are two null terminated strings.
 *
 * RETURNS: 
 *	none
 */  


PROC
static void
getnames (filename,member,tmpbuf,maxlen)
char *filename,*member,*tmpbuf;
int maxlen;
{
	char *p1=filename,*p2=tmpbuf;
	int max = maxlen;
	do {
		*p1++ = *p2;
		if (!max--) {
			fprintf (stderr,MSGSTR(STR2LONG,"filename string too long\n"));
			exit(-1);
		}
	} while (*p2++);
	max = maxlen;
	p1 = member;
	do {
		*p1++ = *p2;
		if (!max--) {
			fprintf (stderr,MSGSTR(MEMBER,"member string too long\n"));
			exit(-1);
		}
	} while (*p2++);
}


/*
 * NAME: isvalidsymbol
 *                                                                    
 * FUNCTION: 
 *
 * RETURNS: 
 *	Returnes true if the given symbol is within the appropriate range
 *	and is a text symbol.
 */  


static PROC
isvalidsymbol(nl, name)
  SYMENT	*nl;
  char		*name;
{

	if (nl->n_scnum != text_section_number)
	    return 0;
	if (name == NULL)
	    return 0;
	if (*name == '\0')
	    return 0;
	switch(nl->n_sclass) {
	    case C_EXT:
	    case C_HIDDEN:
	    case C_HIDEXT:
	    case C_STAT:
		return 1;
	}
	return 0;
}


/*
 * NAME: Malloc
 *                                                                    
 * FUNCTION: 
 * 	Error-checking memory allocator. 
 *	Guarentees good return (else none at all).
 *                                                                    
 * RETURNS: 
 *	pointer to allocated memory
 */  

static PROC char *
Malloc(item_count, item_size)
{
	register char *p;

	if ((p = calloc((size_t)item_count, (size_t)item_size)) == NULL) {
		perror("Out of space");
		exit (-1);
	}

#ifdef DBG
	if ( debug )
	    fprintf (stderr,
		"Callocing: cnt: %x size: %x total: %x  addr: %x\n",
		item_count,item_size,item_count*item_size,p);
#endif DBG
	return (p);
}


/*
 * NAME: eofon
 *
 * FUNTCTION:
 *      Come here if unexpected read problem.
 * RETURNS:
 *      exits
 */

static PROC
eofon(iop, fn,msg)
register FILE *iop;
register char *fn;
char *msg;
{
       if (ferror(iop))                /* Real error? */
	       perror(fn);             /* Yes. */
       fprintf(stderr, MSGSTR(EARLYEND,"%s: %s: %s: Premature EOF\n"), cmdname,
fn,msg);
       exit(1);
}


/*
 * NAME: fsize
 *
 * FUNCTION:
 *      Return size of file associated with file descriptor fd.
 *
 * RETURNS: off_t
 */

static PROC off_t
fsize(fd)
{
        struct stat sbuf;

        if (fstat(fd, &sbuf) < 0) {              /* Status of open file. */
                perror("stat");
		exit(-1);
	}
        return (sbuf.st_size);                  /* This is a long. */
}

/*
 * NAME: dem_conv
 *
 * FUNTCTION:
 *      Conversion routine for demangled name
 * RETURNS:
 *      demangled name or invalid (-1) 
 */
char *
dem_conv( char *str )
{
    static char sym_save_size = 0;
    static char *sym_save_area = NULL;
    char	**rest, *sym_name;
    struct	Name	*demname, *demangle();
    int		needDot, len;

    if (*str == '.') {
	needDot = 1;
	str++;
    } else
	needDot = 0;

    demname = demangle (str, &rest, RegularNames);

    /* did we do anything? */
    if (demname == NULL)
	return INVAL;

    /* get the real name */
    sym_name = text (demname);

    /* ensure we have enough room to play with */
    len = strlen(sym_name) + needDot + 1;
    if (len >= sym_save_size) {
	if (sym_save_area)
	    free (sym_save_area);
	/* get some extra, just for fun */
	len *= 2;
	sym_save_area = malloc (len);
	if (sym_save_area)
	    sym_save_size = len;
	else {
	    perror ("malloc");
	    exit (-1);
	}
    }

    /* save the name (with leading dot if necessary) */
    if (needDot) {
	sym_save_area[0] = '.';
	strcpy (&sym_save_area[1], sym_name);
    } else
	strcpy (sym_save_area, sym_name);

    /* return the storage */
#ifndef F_PLOT
    /* demangle erase conflicts with plot erase! */
    erase (demname);
#endif
    return sym_save_area;
}
