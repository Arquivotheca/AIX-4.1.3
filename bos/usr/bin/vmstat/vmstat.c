static char sccsid[] = "@(#)71	1.20.1.17  src/bos/usr/bin/vmstat/vmstat.c, cmdstat, bos41J, 9509A_all 2/27/95 13:27:28";
/*
 * COMPONENT_NAME: (CMDSTAT) Reports Virtual Memory Statistics
 *
 * FUNCTIONS: vmstat
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#define		_ILS_MACROS
#include 	<sys/types.h>
#include 	<sys/sysinfo.h>
#include 	<sys/vminfo.h>
#include 	<sys/vmker.h>
#include 	<sys/iostat.h>
#include 	<sys/param.h>
#include	<sys/priv.h>
#include	<fcntl.h>
#include 	<stdlib.h>
#include	<ctype.h>
#include 	<errno.h>
#include	<stdio.h>
#include	<a.out.h>
#include	<math.h>
#include	<locale.h>

#include "vmstat_msg.h"
static nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_VMSTAT, Num, Str)

#define NUM_OF_SYMS	(sizeof(symnames)/sizeof(symnames[0]))
#define	NLINES	20			/* number of rate lines to print before new header */
#define	NUMDISKS	4		/* maximum numer for drives displayed */

static char	sbuf[BUFSIZ];			/* for setbuf() */
static int	mem;				/* file descriptor for /dev/kmem */
static char	*namelist = "/dev/kmem";	/* kernel memory */
static char    *errname;

/* stat structure names */
static char 	*symnames[] = {"vmminfo","sysinfo",
			"vmker","lbolt","iostat"};

static struct  nlist  nltbl[NUM_OF_SYMS];	/* nlist table 	*/

/* nlist pointers */
static struct  nlist  *Vminfo, *Sysinfo, *Vmker, *Lbolt, *Iostat;

#define	NDEV	32

struct 	diskstuff
	{
 	struct dkstat *diskptr;
	char  dev[NDEV];
	};

struct 	diskxfer
	{
	ulong  xfercnt[NUMDISKS];
	};

static struct	diskstuff disks[NUMDISKS];
static struct	dkstat	dkstatus[NUMDISKS];

static struct	sysinfo	sysinfo;		/* buffer for sysinfo structures */
static struct	sysinfo	sysinfo2;		/* buffer for sysinfo structures */
static struct	vminfo	vminfo;			/* buffer for vminfo structures  */
static struct	vminfo	vminfo2;		/* buffer for vminfo structures  */
static struct	vmkerdata vmkerdata;		/* buffer for vmker structure    */
static struct	diskxfer diskxfer;
static struct  diskxfer diskxfer2;

static int	lines = 1;			/* rate lines printed */
static int	ndisks;				/* number of disks specified */
ulong	udiff(ulong, ulong);

/*
 * NAME: vmstat [ -fsi ] [drives] [interval] [count]
 *
 * FUNCTION: 	report virtual memory and system statistics.
 *
 *	vmstat -f    	reports fork statistics.
 *	vmstat -i    	reports interrupt statistics
 *	vmstat -s    	reports sum statistics.
 *	vmstat 	     	reports sum statistics divided by number of seconds
 *			since last boot.
 *	vmstat [drives]
 *			reports tranfer rates for up to four specified disk
 *			drives.
 *	vmstat [interval]
 *			reports one second rate statistics at the specified 
 *			interval (seconds) with the first interval reporting
 *			sum statistics divided by the number of seconds since
 *			since last boot.
 *	vmstat [interval][count]
 *			reports one second rate statistics at the specfied
 *			specified interval (seconds) for count number of times
 *			with the first interval reporting sum statistics
 *			divided by the number of seconds since last boot.
 *
 */

main(argc, argv)
int argc;
char **argv;
{
	char **cp;
	int cnt, intv;
	int iflag = 0;

	(void) setlocale (LC_ALL,"");
	privilege (PRIV_LAPSE);
	init();

	argc--, argv++;
	while (argc>0 && argv[0][0]=='-') 
	{
		char *cp = *argv++;
		argc--;
	 	while (*++cp) 
		{
	 		switch (*cp)
			{
			case 's':
				dosum();
				exit(0);

			case 'f':
				doforkst();
				exit(0);

			case 'i':
				iflag = 1;
				break;

			default:
				prusage();
			} 
		}
	}

	ndisks = 0;
	while (argc > 0 && !isdigit(argv[0][0]))
	{
		if (iflag)
			prusage ();
		if (ndisks < NUMDISKS)
		{
			strncpy(&disks[ndisks].dev[0],argv[0],NDEV);
			ndisks++;
		}
		argc--; 
		argv++;
	}

	cnt = 1;
	if (argc)
	{
		if ((intv = atoi(argv[0])) <= 0)
		{
			fprintf(stderr,MSGSTR(MSGE01,
			"vmstat: invalid interval value\n"));
			prusage();
		}
		cnt = -1;
		argc--;
	}
	if (argc)
	{
		if ((cnt = atoi(argv[1])) <= 0)
		{
			fprintf(stderr,MSGSTR(MSGE02,
			"vmstat: invalid count value\n"));
			prusage();
		}
	}
	if (iflag)
	{
		initintr();
		dointr(intv, cnt);
	}
	else
		dorate(intv,cnt);
	exit(0);
}

/*
 * NAME: dosum()
 *
 * FUNCTION: 	get current and report current sum statistics.
 *
 */

static dosum()
{
	unsigned long	hardintrs;
	unsigned long	softintrs;

	/*
	 * get hardware and software interrups
	 */
	gethwswintr (&hardintrs, &softintrs);

	/*
	 *  read vminfo data.
	 */ 
	if (readmem(&vminfo,Vminfo,sizeof(struct vminfo)) != 0)
		fatal(MSGSTR(MSGE02,
			"vmstat: read error on vmminfo structure"));

	/*
	 *  read sysinfo data.
	 */ 
	if (readmem(&sysinfo,Sysinfo,sizeof(struct sysinfo)) != 0)
		fatal(MSGSTR(MSGE03,
			"vmstat: read error on sysinfo structure"));

	/*
	 *  report the relevant sum statistics
	 */ 
	printf(MSGSTR(MSGS01,"%9d total address trans. faults\n"), vminfo.pgexct);
	printf(MSGSTR(MSGS02,"%9d page ins\n"), vminfo.pageins);
	printf(MSGSTR(MSGS03,"%9d page outs\n"), vminfo.pageouts);
	printf(MSGSTR(MSGS04,"%9d paging space page ins\n"), vminfo.pgspgins);
	printf(MSGSTR(MSGS05,"%9d paging space page outs\n"), vminfo.pgspgouts);
	printf(MSGSTR(MSGS06,"%9d total reclaims\n"), vminfo.pgrclm);
	printf(MSGSTR(MSGS07,"%9d zero filled pages faults\n"), vminfo.zerofills);
	printf(MSGSTR(MSGS08,"%9d executable filled pages faults\n"), vminfo.exfills);
	printf(MSGSTR(MSGS09,"%9d pages examined by clock\n"), vminfo.scans);
	printf(MSGSTR(MSGS10,"%9d revolutions of the clock hand\n"), vminfo.cycles);
	printf(MSGSTR(MSGS11,"%9d pages freed by the clock\n"), vminfo.pgsteals);
	printf(MSGSTR(MSGS12,"%9d backtracks\n"), vminfo.backtrks);
	printf(MSGSTR(MSGS13,"%9d lock misses\n"), vminfo.lockexct);
	printf(MSGSTR(MSGS14,"%9d free frame waits\n"), vminfo.freewts);
	printf(MSGSTR(MSGS15,"%9d extend XPT waits\n"), vminfo.extendwts);
	printf(MSGSTR(MSGS16,"%9d pending I/O waits\n"), vminfo.pendiowts);
	printf(MSGSTR(MSGS17,"%9d start I/Os\n"), vminfo.numsios);
	printf(MSGSTR(MSGS18,"%9d iodones\n"), vminfo.numiodone);
	printf(MSGSTR(MSGS19,"%9d cpu context switches\n"), sysinfo.pswitch);
	printf(MSGSTR(MSGS20,"%9lu device interrupts\n"), hardintrs);
	printf(MSGSTR(MSGS21,"%9lu software interrupts\n"), softintrs);
	printf(MSGSTR(MSGS22,"%9d traps\n"), sysinfo.traps);
	printf(MSGSTR(MSGS23,"%9lu syscalls\n"), (ulong_t)sysinfo.syscall);

	return(0);
}

/*
 * NAME: doforkst()
 *
 * FUNCTION: 	get current and report current fork statistics.
 *
 */

static doforkst()
{


	/*
	 *  read sysinfo data.
	 */ 
	if (readmem(&sysinfo,Sysinfo,sizeof(struct sysinfo)) != 0)
		fatal(MSGSTR(MSGE04,
			"vmstat: read error on sysinfo structure"));

	/*
	 *  report the fork statistics.
	 */ 
	printf(MSGSTR(MSGF01,"%d forks\n"),sysinfo.sysfork);

	return(0);
}

/*
 * NAME: dorate(intv,cnt)
 *
 * FUNCTION: 	get and report rate statistics every intv seconds, cnt
 * 		number of times.  The first set of rate statistics    
 *		reported will consists of the current sum statistics
 *		divided by the number of seconds since the last bootup.
 *
 */

static dorate(intv,cnt)
int intv;
int cnt;
{
	int		i, now, boottime;                
	time_t		kernel_lbolt;
	double          local_lbolt;
	struct sysinfo	*si = &sysinfo, *osi = &sysinfo2, *tsi;
	struct vminfo	*vi = &vminfo, *ovi = &vminfo2, *tvi;
	struct diskxfer *di = &diskxfer, *odi = &diskxfer2, *tdi;
	unsigned long	dummy;
	static unsigned long hwintrs[2];
	int		intrx = 0;
	ulong_t		rinv;
	ulong_t		sumcpu, cpulog[4];  /* cpulog[0] - user cpu	*/
					    /* cpulog[1] - system cpu	*/
					    /* cpulog[2] - idle cpu	*/
					    /* cpulog[3] - wait cpu	*/

	/*
	 * Read last boot time
	 */
	if ((readmem(&kernel_lbolt,Lbolt,sizeof(kernel_lbolt)) != 0))
		fatal(MSGSTR(MSGE05,"vmstat: read error on lbolt\n"));


	/* Treat kernel_lbolt as if it is unsigned.     */
        local_lbolt = (double)((unsigned long)kernel_lbolt);
        if (local_lbolt < HZ)
                fatal(MSGSTR(MSGE05,"vmstat: read error on lbolt\n"));

	prthdr();

	rinv = (ulong_t)(local_lbolt / HZ);
	while(cnt)
	{
		/*
	  	 *  get disk infomation.
		 */
		getdiskinfo();
		/*
		 * Get vminfo and sysinfo kernel data values.
		 * The first time around the statistics reported
		 * correspond to the sum since boot time, because
		 * osi->... fields are zero.
		 */
		if (readmem(vi,Vminfo,sizeof(struct vminfo)) != 0)
			fatal(MSGSTR(MSGE03,
				"vmstat: read error on vmminfo structure"));
		if (readmem(si,Sysinfo,sizeof(struct sysinfo)) != 0)
			fatal(MSGSTR(MSGE04,
				"vmstat: read error on sysinfo structure"));

		gethwswintr (hwintrs + intrx, &dummy);
 
		/*
	 	 * Get vmm kernel data values.
	 	 */
		if (readmem(&vmkerdata,Vmker,sizeof(struct vmkerdata)) != 0)
			fatal(MSGSTR(MSGE08,
				"vmstat: read error on vmker structure"));

		/*
	 	 * Report rates.
	 	 */

		printf("%2d", udiff(si->runque, osi->runque) / rinv);
		printf(" %2d", udiff(si->swpque, osi->swpque) / rinv);
		printf(" %5d", vmkerdata.numpsblks - vmkerdata.psfreeblks);
		printf(" %5d", vmkerdata.numfrb);
		printf(" %3d", udiff(vi->pgrclm, ovi->pgrclm) / rinv);
		printf(" %3d", udiff(vi->pgspgins, ovi->pgspgins) / rinv);
		printf(" %3d", udiff(vi->pgspgouts, ovi->pgspgouts) / rinv);
		printf(" %3d", udiff(vi->pgsteals, ovi->pgsteals) / rinv);
		printf(" %4d", udiff(vi->scans, ovi->scans) / rinv);
		printf(" %3d", udiff(vi->cycles, ovi->cycles) / rinv);
		printf(" %3lu", udiff(hwintrs[intrx], hwintrs[1-intrx]) / rinv);
		printf(" %4d", udiff((ulong_t)si->syscall, (ulong_t)osi->syscall) / rinv);
	 	printf(" %3d", udiff(si->pswitch, osi->pswitch) / rinv);

		/*
		 * The following is a hack to allow the following fields to be displayed
		 * in 3 columns a piece.  Since we are doing integer arithmetic and since
		 * that alone can account for as much as a 2 percentage points error
		 * (i.e. all four fields having ".5" in the decimal) I have rounded 100
		 * down to 99 enabling us to fit in 3 columns and only creating an error
		 * of 1 percentage point.
		 */

		cpulog[0] = udiff(si->cpu[CPU_USER], osi->cpu[CPU_USER]);
		cpulog[1] = udiff(si->cpu[CPU_KERNEL], osi->cpu[CPU_KERNEL]);
		cpulog[2] = udiff(si->cpu[CPU_IDLE], osi->cpu[CPU_IDLE]);
		cpulog[3] = udiff(si->cpu[CPU_WAIT], osi->cpu[CPU_WAIT]);

		if ( rinv == intv )
		{
		    sumcpu = cpulog[0] + cpulog[1] + cpulog[2] + cpulog[3] ;
		    for ( i = 0; i < 4 ; i++ )
		    {
			cpulog[i] = nearest (100 * ((float) cpulog[i] / 
						    (float) sumcpu));
			printf(" %2d", (cpulog[i] >= 100) ? 99 : cpulog[i] );
		    }
		}
		else
		    for ( i = 0; i < 4 ; i++ )
		    {
			cpulog[i] = nearest (100 * ((float) cpulog[i] /
						    (float) local_lbolt));
			printf(" %2d", (cpulog[i] >= 100) ? 99 : cpulog[i] );
		    }

		for (i = 0; i < ndisks; i++)
		{
			di->xfercnt[i] = disks[i].diskptr->dk_xfers;
	 		printf(" %2d", udiff(di->xfercnt[i], odi->xfercnt[i]) / rinv);
		}

	 	printf("\n");
		fflush(stdout);		/* force output, even to a pipe */

		cnt--;
		lines--;

		if (cnt)
		{
			if (lines == 0)
				prthdr();
			rinv = intv;
			sleep(intv);
			tsi = si; si = osi; osi = tsi;
			tvi = vi; vi = ovi; ovi = tvi;
			tdi = di; di = odi; odi = tdi;
			intrx = 1 - intrx;
		}
	}

	return(0);
}

/*
 * NAME: udiff(a,b)
 *
 * FUNCTION:	Find difference between two (possibly overflowed) unsigned
 *		longs.
 *
 */

static ulong udiff(a,b)
ulong a,b;
{
	if (a >= b)
		return (a - b);
	
	return ((0xFFFFFFFF - b) + a + 1);
}

/*
 * NAME: prthdr()
 *
 * FUNCTION: 	print rate header.
 *
 */

static prthdr()
{

	printf(MSGSTR(MSGH01,"kthr     memory             page           "));
	printf(MSGSTR(MSGH02,"   faults        cpu     "));

	if (ndisks)
		printf(MSGSTR(MSGH05," disk xfer"));

	printf("\n");
	printf("----- ----------- ------------------------");
	printf(" ------------ -----------");
	if (ndisks)
		printf(" -----------");
	printf("\n");
	printf(MSGSTR(MSGH03," r  b   avm   fre  re  pi  po  fr   sr  cy "));
	printf(MSGSTR(MSGH04," in   sy  cs us sy id wa "));

	if (ndisks)
		printf(MSGSTR(MSGH06," 1  2  3  4"));

	printf("\n");
	lines = NLINES;

	return(0);
}

/*
 * NAME: getdiskinfo()
 *
 * FUNCTION: 	get current disk transfer rates.
 *
 */

static getdiskinfo()
{
	int	i;
	struct	iostat iostat;
	struct	dkstat *dkptr;

	/*
	 * if no drives were specified return.
	 */
	if (ndisks == 0)
		return(0);

	/*
	 *  read iostat data.
	 */ 
	if (readmem(&iostat,Iostat,sizeof(struct iostat)) != 0)
		fatal(MSGSTR(MSGE11,"vmstat: read error on iostat structure"));

	for (i = 0; i < ndisks; i++)
	{
		disks[i].diskptr = 0;
		for (dkptr = iostat.dkstatp; dkptr; dkptr = dkstatus[i].dknextp)
		{
			if (readdkstat(&dkstatus[i],dkptr) != 0)
				fatal(MSGSTR(MSGE11,
				"vmstat: read error on iostat structure"));

			if (!strcmp(&disks[i].dev[0],&dkstatus[i].diskname[0]))
			{
				disks[i].diskptr = &dkstatus[i];
				break; /* stop looking */  
			}
		}
		if (disks[i].diskptr == 0)
		{
			fprintf(stderr,MSGSTR(MSGE12,
				"vmstat: drive %s not found\n"), disks[i].dev);
			exit(1);
		}
	}

	return(0);
}


/*
 * NAME: init()
 *
 * FUNCTION: 	perform initialization activities:  call setbuf(), open 
 *		/dev/kmem, setup symbol table.
 *
 */

static init()
{
	struct nlist * symsrch();

	catd = catopen(MF_VMSTAT, NL_CAT_LOCALE);

	setbuf(stdout,sbuf);

	privilege (PRIV_ACQUIRE);
	if((mem = open(namelist, O_RDONLY)) < 0) {
		privilege (PRIV_DROP);
		if (errno == EACCES)
			fatal(MSGSTR(MSGE14, "vmstat: Permission Denied.\n"));
		else
			fatal(MSGSTR(MSGE09,"vmstat: cannot open name list"));
	}

	intrsymbs ();
	rdsymtab();
	privilege (PRIV_DROP);

	Vminfo = symsrch("vmminfo");
	Sysinfo = symsrch("sysinfo");
	Vmker = symsrch("vmker");
	Lbolt = symsrch("lbolt");
	Iostat = symsrch("iostat");

	fflush(stdout);

	return(0);
}

/*
 * NAME: rdsymtab()
 *
 * FUNCTION: 	initialize the symbol array with the symbols for
 * 		the statistics structures and call knlist() to
 *		fill it in with addresses. 
 *
 */

static rdsymtab()
{
	int i;

	for(i=0;i<NUM_OF_SYMS;i++)
		nltbl[i].n_name = symnames[i];

	if ( knlist(nltbl,NUM_OF_SYMS,sizeof (struct nlist)) == -1) {
		privilege(PRIV_DROP);
		if (errno == EPERM)
			fatal(MSGSTR(MSGE14, "vmstat: Permission Denied.\n"));
		else
			fatal(MSGSTR(MSGE13,"vmstat: knlist failed"));
	}
}

/*
 * NAME: symsrch(s)
 *
 * FUNCTION: 	searchs the symbol table for the a given symbol
 * 		string and returns a pointer to the corresponding
 *		table entry.
 *
 */

static struct nlist *
symsrch(s)
register char *s;
{
	register struct nlist  *found;
	register int i;

	found = 0;
	for(i=0; i < NUM_OF_SYMS; i++)
	{
		if(strcmp(nltbl[i].n_name, s) == 0)
		{
			found = &nltbl[i];
			break;
		}
	}

	return(found); 
}

/*
 * NAME: readmem(buf,ptr,len)
 *
 * FUNCTION:  	read len number of bytes beginning at a ptr within 
 *		within /dev/kmem into buf.  Return a zero if the read 
 *		was successfull and a -1 otherwise.
 *
 */

static readmem(buf, ptr, len)
char *buf;
struct nlist *ptr;
int len;
{

	if (lseek(mem, (long)ptr->n_value, 0) == -1)
	{
		return(-1);
	}

	if (read(mem, &buf[0], len) != len)
	{
		return(-1);
	}

	return(0);
}

/*
 * NAME: readdkstat(buf,ptr,len)
 *
 * FUNCTION:  	read len number of bytes beginning at a ptr within 
 *		within /dev/kmem into buf.  Return a zero if the read 
 *		was successfull and a -1 otherwise.
 *
 */

static readdkstat(dkstatp, ptr)
struct dkstat *dkstatp;
long ptr;
{

	if (lseek(mem, ptr, 0) == -1)
	{
		return(-1);
	}

	if (read(mem, dkstatp, sizeof(struct dkstat)) != sizeof(struct dkstat))
	{
		return(-1);
	}

	return(0);
}


/*
 * NAME: fatal(str)
 *
 * FUNCTION: 	prints a string to STDERR and exits,
 *		returning 1.
 *
 */

static fatal(str)
char *str;

{
	fprintf(stderr, str);
	exit(1);
}

/*
 * NAME: prusage()
 *
 * FUNCTION: 	print vmstat usage statement to STDERR and exits,
 *		returning 1.
 *
 */

static prusage()

{
	fprintf(stderr,MSGSTR(MSGUSG,"usage: vmstat [ -fs ] [drives] [interval] [count]\n"));
	exit(1);
}

/*
 * The following code added to sample interrupt counts.
 */

#include <nlist.h>
#include <sys/intr.h>
#include <sys/m_intr.h>
#include <sys/ios/interrupt.h>

#define BUGVDEF(x,y)		/* for loader headers */
#include <sys/malloc.h>
#include <sys/ldr.h>
#include <a.out.h>
#include <sys/ldr/ld_data.h>

#ifdef	malloc			/* because of loader headers */
#undef	malloc
#endif
#ifdef	free
#undef	free
#endif

#define	RATE	5
#define	NOFFL	(INTOFFL3-INTOFFL0+1)

struct ext {
	char	*addr;
	char	*name;
};

void		 *getmem ();
static struct nlist	  nlst[] = {{"i_data"}, {"kernel_anchor"}, {"misc_intrs"}, {0}};
#define	I_DATA	(&nlst[0])
#define	KANCHOR	(&nlst[1])
#define MISC_INTRS	(&nlst[2])
static char		  kmem[] = {"/dev/kmem"};
static int		  nintr;
static int		  oldnintr;
static int		  freenintr;
static ulong		  misc_intrs;
static struct intr	**oldtab;
static struct intr	**freetab;
static struct intr	  zerointr;
static struct i_data	  i_data;
static struct ext	 *symtab;	/* use the sparse addr space */
static int		  symct = 0;	/* how many we've allocated for symtab	*/
static struct ext	 *exttab;
static int		  symnum;
static char		 *prog;

struct intr1 {
	struct intr	intr;
	caddr_t		*intr_addr;  /* kernel address for this intr struct */
};

/*
 * get hardware and software interrupts
 */
static gethwswintr (hw, sw)
unsigned long	*hw;
unsigned long	*sw;
{
	struct i_poll	*poll;
	struct intr	*ip;
	struct intr	*n;
	unsigned long	h = 0, s = 0;

	getintr ();

	for (poll = i_data.i_poll;
	     poll < &i_data.i_poll[NUM_INTR_SOURCE];
	     poll++) {
		ip = poll->poll;
		while (ip) {
			if (ip->priority >= INTOFFL0 &&
			    ip->priority <= INTOFFL3)
				s += ip->i_count;
			else
				h += ip->i_count;
			/*
			 * free was we go.  be sure to yank out next
			 * pointer before we free, since we never, ever
			 * assume that free doesn't touch the data
			 */
			n = ip->next;
			free((void *) ip);
			ip = n;
		}
	}
	*hw = h + misc_intrs;
	*sw = s;
}

/*
 * initialize symbol tables
 */
static initintr ()
{
	getunix();
	getextensions();
}

/*
 * get symbol table from /unix
 */
static getunix ()
{
	struct ext	*sym;

	sym = symtab;

	act_like_nm(&sym);

	exttab = sym;
}

/*
 * sample interrupt every cnt seconds
 * showing the first time the interrupts
 * since boot, next iterations will
 * show count since the previous iteration
 */
static dointr (intv,cnt)
int	intv;
int	cnt;
{
	while (cnt) {
		getintr ();
		puthead ();
		listintr ();
		putintr ();
		if (--cnt) {
			sleep (intv);
		}
	}
}

/*
 * get the i_data and kernel_anchor addresses
 */
static intrsymbs ()
{
	if (nlist ("/unix", nlst) != 0) {
		if (errno == EPERM)
			fatal(MSGSTR(MSGE14, "vmstat: Permission Denied.\n"));
		else
			fatal(MSGSTR(MSGE13,"vmstat: knlist failed"));
		exit (1);
	}
}

/*
 * allocate a struct intr 
 */
static struct intr	*intralloc ()
{
	struct intr	*ip;

	ip = (struct intr *) getmem (sizeof (struct intr1));
	return ip;
}

/*
 * free a struct intr list.
 */
static freeintr(ip)
struct intr	*ip;
{
	struct intr	*nextip;
	while(ip) {
		nextip = ip->next;
		free(ip);
		ip = nextip;
	}
	return;
}

#define RETRY_LIMIT1	300
#define RETRY_LIMIT2	5
/*
 * get the intr structures for the second level interrupt handlers
 * (a similar loop would get the intr structures for the i_sched
 * service)
 */
static getintr ()
{
	int i, j, res, gotit=0, itries, retries;
	struct i_poll	*poll;
	struct intr	*ip;

	nintr = 0;
	for(itries=0; !gotit && (itries < RETRY_LIMIT1); itries++) {
		if (kread (&i_data, sizeof i_data, I_DATA->n_value) < 0) {
			perror(errname);
			exit(1);
		}
		for (i=0; i<NUM_INTR_SOURCE; i++) {
			ip = NULL;
			retries = 0;
			do {
				freeintr(ip);
				ip = i_data.i_poll[i].poll;
				res = readintr(&ip);
			} while((res < 0) && (retries++ < RETRY_LIMIT2));

			if (retries < RETRY_LIMIT2) {
				i_data.i_poll[i].poll = ip;
				gotit = 1;
			} else {
				for (j=0; j<i; j++)
					freeintr(i_data.i_poll[j].poll);
				freeintr(ip);
				gotit = 0;
				break; /* for */
			}
		}
	}
	if (!gotit) {
		errno = ENXIO;
		perror(errname);
		exit(1);
	}
	if (kread (&misc_intrs, sizeof misc_intrs, MISC_INTRS->n_value) < 0) {
		perror(errname);
		exit(1);
	}
}

/*
 * read a list of intr structs from kernel memory to user memory
 */
static readintr (ipp)
struct intr	**ipp;
{
	int sav_nintr = nintr;
	struct intr	*ip, **curipp = NULL, tmpip;

	/* Read dummy header from kernel. */
	if (*ipp) {
		if (kread (&tmpip, sizeof (struct intr), *ipp) < 0) {
			if (errno == ENXIO) {
				*ipp = NULL;
				return -1;
			} else {
				perror(errname);
				exit(1);
			}
		}
		curipp = &(tmpip.next);
	}
	*ipp = NULL;

	/* Read remainder of list into user space. */
	while (*curipp) {
		ip = intralloc ();
		++nintr;
		if (kread (ip, sizeof (struct intr), *curipp) < 0) {
			if (errno == ENXIO) {
				nintr = sav_nintr;
				free(ip);
				*curipp = NULL;
				return -1;
			} else {
				perror(errname);
				exit(1);
			}
		}

		/* save the kernel address for the intr struct */
		((struct intr1 *)ip)->intr_addr = (caddr_t *)*ipp;

		if (!*ipp)
			*ipp = ip;
		*curipp = ip;
		curipp = &ip->next;
	}
	return 0;
}

/*
 * release the memory used by a list of intr structs
 */
static putintr ()
{
	struct intr	**tp;
	struct intr	**endtab;

	endtab = freetab + freenintr;
	tp = freetab;
	if (freetab) {
		while (tp < endtab)
			free (*tp++);
		free (freetab);
	}
}

/*
 * compare two intr structs,
 * this gives the order of the output,
 * see qsort below
 */
static intrcmp (a, b)
struct intr	**a;
struct intr	**b;
{
	return ((*a)->priority == (*b)->priority) ?
		(*a)->level - (*b)->level :
		(*a)->priority - (*b)->priority;
}

/*
 * find an intr struct in the table of structs
 * found in the previous iteration, return
 * a pointer to a zeroed out intr if it is not found
 * so that the interrupt count since boot are not
 * handled as a special case.
 */
static struct intr	  *findintr (ip, oldtp, oldendtab)
struct intr	  *ip;
struct intr	***oldtp;
struct intr	 **oldendtab;
{
	struct intr	**tp = *oldtp;

	while (tp < oldendtab) {
		if ((*tp)->level == ip->level &&
		    (*tp)->priority == ip->priority &&
		    (*tp)->handler == ip->handler &&
		    ( ((struct intr1 *)(*tp))->intr_addr == ((struct intr1 *)ip)->intr_addr )) {
			*oldtp = tp;
			return *tp;
		}
		tp++;
	}
	return &zerointr;
}

/*
 * list the interrupts
 */
static listintr ()
{
	struct i_poll		 *poll;
	struct intr		 *ip;
	struct intr		**tab;
	struct intr		**endtab;
	struct intr		**tp;
	struct intr		**oldtp;
	struct intr		**oldendtab;

	tab = (struct intr **) getmem (nintr * sizeof (struct intr *));
	tp = tab;
	endtab = tab + nintr;
	for (poll = i_data.i_poll;
	     poll < &i_data.i_poll[NUM_INTR_SOURCE];
	     poll++) {
		ip = poll->poll;
		while (ip) {
			*tp++ = ip;
			ip = ip->next;
		}
	}
	qsort (tab, nintr, sizeof (struct intr *), intrcmp);
	oldtp = oldtab;
	oldendtab = oldtab + oldnintr;
	for (tp = tab; tp < endtab; tp++) {
		ip = *tp;
		lsintr (ip, ip->i_count - 
			findintr (ip, &oldtp, oldendtab)->i_count);
	}
	freenintr = oldnintr;
	oldnintr = nintr;
	freetab = oldtab;
	oldtab = tab;
}

static puthead ()
{
	printf (MSGSTR (MSGH07,
		"priority level    type   count module(handler)\n"));
}

/*
 * list the fields of an intr struct
 * looking up the address of the handler in the symbol table
 */
static lsintr (ip, count)
struct intr	*ip;
unsigned long	 count;
{
	char	*findmodule ();

	printf (MSGSTR (MSGH08, "%5d %7d   %s %5d %s(%x)\n"),
		ip->priority, 
		ip->level,
		ip->priority <= INTOFFL3 &&
		ip->priority >= INTOFFL0 ?
		"offlevel" : "hardware",
		count,
		findmodule (ip->handler),
		ip->handler);
}

/*
 * read from kernel memory to user memory,
 * kaddr is the kaddr, uaddr the user's
 * and count the byte count
 */
static kread (uaddr, n, kaddr)
char	*uaddr;
int	 n;
unsigned long	 kaddr;
{
	if (llseek (mem, (offset_t) kaddr, SEEK_SET) == -1) {
		errname = "llseek";
		return (-1);
	}
	if (read (mem, uaddr, n) != n) {
		errname = "read";
		return (-1);
	}
	return 0;
}

/*
 * allocate core and take care of errors here
 */
static void	*getmem (size)
size_t	 size;
{
	void	*p;

	if (!(p = malloc (size))) {
		perror ("malloc");
		exit (1);
	}
	return p;
}

/*
 * allocate core and copy a string to it
 */
static char	*cpstr (s)
char	*s;
{
	char	*d;

	d = getmem (strlen (s) + 1);
	strcpy (d, s);
	return d;
}

/*
 * allocate core and copy a possibly non null terminated string to it
 */
static char	*cpstrl (s, len)
char	*s;
int	 len;
{
	char	*d;

	/* figure length */
	for (d = s; d < s + len && *d; d++)
		;
	len = d - s;

	d = getmem (len + 1);
	strncpy (d, s, len);
	*(d + len) = 0;

	return d;
}

/*
 * compare two struct ext tables used in the qsort below
 */
static extcmp (a, b)
struct ext	*a;
struct ext	*b;
{
	/*
	 * used to be:
	return a->addr - b->addr;
	 *
	 * but this doesn't work since we're comparing unsigned (char *)
	 * and then converting back to signed
	 */

	return (a->addr > b->addr ? 1 :
		a->addr < b->addr ? -1 : 0);
}

/*
 * get the kernel extensions load addresses
 * and sort the /unix+extension sysmbol table
 */
static getextensions ()
{
	struct ext		*ext;
	struct loader_entry	*le;
	struct loader_entry	 lentry;
	struct loader_anchor     kanchor;
	char			 buf[BUFSIZ];

	if (kread(&kanchor, sizeof (struct loader_anchor), KANCHOR->n_value) < 0) {
		perror(errname);
		exit(1);
	}
	ext = exttab;
	le = kanchor.la_loadlist;
	while (le) {
		if (ext == symtab + symct)	/* need to expand symtab? */
			expand_symtab(&ext);
		if (kread (&lentry, sizeof (struct loader_entry), le) < 0) {
			perror(errname);
			exit(1);
		}
		if (kread (buf, BUFSIZ, lentry.le_filename) < 0) {
			perror(errname);
			exit(1);
		}
		ext->addr = lentry.le_file;
		ext->name = cpstr (buf+1);
		++ext;
		le = lentry.le_next;
	}
	symnum = ext - symtab;
	qsort (symtab, symnum, sizeof (struct ext), extcmp);
}

/*
 * return a name for the given kernel address
 */
static char	*findmodule (addr)
char	*addr;
{
	struct ext	*ext;
	if (addr < symtab->addr)
		return "???";
	ext = symtab + symnum;
	do
		--ext;
	while (addr < ext->addr && ext > symtab);
	return ext->name;
}

/*
 *	act_like_nm()
 *
 *	- read in the symbol table ala nm.
 *
 *	- pretty much snarfed from nlist with a bit or two of info from nm
 *
 *	- flames to /dev/null.  the original code was a popen of nm
 *	  which, besides being slower than heck, was also a humungous (sp?)
 *	  security hole.
 */

#include <xcoff.h>

/*
 * object file types we handle...
 */
#define	BADMAG(x)	\
	((x.f_magic) != U800TOCMAGIC && (x.f_magic) != U802TOCMAGIC)

/*
 * macros to help us look around the file...
 */
#define	A_SYMPOS(x)	x.f_symptr
#define	A_NSYMS(x)	x.f_nsyms
#define	A_NAMEPOS(x)	(A_SYMPOS(x) + A_NSYMS(x) * SYMESZ)

static act_like_nm(sym)
struct ext **sym;
{
	register FILE *fp;
	register char   *np, *sp;
	register int    numaux;
	register long   nsyms;
		 struct syment cursym;
 	register struct syment *symp = &cursym;
		 long   begstr = 0, endstr = 0;
		 struct filehdr    filehdr;
		 char   strbuf[BUFSIZ];		/* for lack of a better size */
	struct ext *e;

	e = *sym;	/* init e to point to the front of the symbol tbl */

	/* attempt to open /unix */
	if( (fp = fopen("/unix", "r")) == NULL ) {
		perror("/unix");
		exit(1);
		}

	/* read file header and check file type */
	(void) fread((void *) &filehdr, (size_t)sizeof filehdr, 1ul, fp);
	if( BADMAG(filehdr) )
	{   fclose(fp);
	    fatal("vmstat: /unix is not an xcoff file\n");
	}

	(void) fseek(fp, A_SYMPOS(filehdr), SEEK_SET);  /* seek symbol table */
	numaux = 0;                     /* first entry is not auxiliary */

	/*
	 * scan through the symbols, read only SYMESZ bytes at
	 * a time (even though that may be < sizeof(struct syment),
	 * that's actually all that the link editor left us...)
	 */
	for (nsyms = A_NSYMS(filehdr); nsyms > 0 &&
	     fread((void *) symp, (size_t)SYMESZ, 1ul, fp) == 1; nsyms--) {
		if( numaux )
		{   /* skip auxiliary entry */
		    --numaux;
		    continue;
		}

		numaux = symp->n_numaux;

		/*
		 * we only want external symbols (ala "nm | grep extern")
		 */
		if (symp->n_sclass != C_EXT)
			continue;

		if (e == symtab + symct)	/* need to expand symtab? */
			expand_symtab(&e);

		if( symp->n_zeroes == 0 )
		{   /* long name; use string table */
		    if( symp->n_offset < begstr || symp->n_offset >= endstr )
		    {   /* offset is not within strbuf, need to read */

				 int    len;
				 long   home;

			/* check for absurd offset; offset includes the
			 * (long) string table length which precedes the table
			 */
			if( symp->n_offset < sizeof(long) ) continue;

			/* save our place */
			home = ftell(fp);

			/* get a chunk of string table */
			(void) fseek(fp, A_NAMEPOS(filehdr)+symp->n_offset,
					SEEK_SET);
			if ((len = fread((void *) strbuf, 1ul, sizeof(strbuf), fp)) <= 0)
			{   /* file malformed, or I/O error */
			    fclose(fp);
			    fatal("vmstat: fread of /unix symbol failed\n");
			}
			/* back to where we were */
			(void) fseek(fp, home, SEEK_SET);

			/* don't include partial strings in the buffer */
			while( strbuf[len-1] != '\0' )
			    if( --len <= 0 )
			    /* strings longer than sizeof(strbuf) not allowed */
			    {
				*strbuf = '\0';
				break;
			    }

			/* indicate the limits of the buffer */
			begstr = symp->n_offset;
			endstr = begstr + len;
		    }
		    /* locate string within strbuf */
		    sp = strbuf + (int)(symp->n_offset-begstr);
		    e->name = cpstr (sp);
		} else {
		    /* short name - the name may not
		     * be null terminated if it's SYMNMLEN long... */
		    sp = symp->_n._n_name;
		    e->name = cpstrl (sp, SYMNMLEN);
		    }

		e++->addr = (char *)symp->n_value;
		}

	fclose(fp);

	*sym = e;	/* now have *sym point to the last symbol we read */
}

/*
 * expand symtab.  point *spot back to the end of it
 */
static expand_symtab(spot)
struct ext **spot;
{
	unsigned newsize;
	int oldsymct = symct;

	/*
	 * grow it 32 at a time (for lack of a better number)
	 */
	newsize = (unsigned) (sizeof(*symtab) * (symct += 32));

	symtab = (oldsymct == 0) ? (struct ext *) malloc(newsize) :
			(struct ext *) realloc((void *) symtab, newsize);

	if (symtab == NULL)
		fatal("vmstat: expand_symtab: out of mem\n");

	*spot = symtab + oldsymct;
}
