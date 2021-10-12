static char sccsid[] = "@(#)07	1.63  src/bos/usr/bin/dd/dd.c, cmdarch, bos41J, 9521B_all 5/24/95 12:42:16";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: dd
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

/*
**      convert and copy
*/

#define _ILS_MACROS

#include    <stdio.h>
#include    <stdlib.h>
#include    <signal.h>
#include    <unistd.h>
#include    <limits.h>
#include    <fcntl.h>
#include    <errno.h>
#include    <locale.h>
#include    <nl_types.h>
#include    <sys/sysmacros.h>
#include    <sys/ipc.h>
#include    <sys/shm.h>
#include    <sys/devinfo.h>
#include    <sys/tape.h>
#include    <sys/wait.h>
#include    <sys/types.h>
#include    <sys/param.h>
#include    <sys/ioctl.h>
#include    <IN/standard.h>
#include    "dd_msg.h"
#include    <ctype.h>

#ifdef LCASE   
#undef LCASE
#endif


/*
 * If we are building the bldenv dd then it may be the
 * case that llseek in not available. This occurs when
 * dd is built on a 3.2* machine. To resolve this problem
 * the following macro is defined. This macro can only be 
 * removed when all possible build machines are running
 * 4.1.
 *
 */

#ifdef _BLD
#define  LLSEEK(fd, off, whence) 	(lseek(fd, (off_t)(off), whence))
#else
#define  LLSEEK(fd, off, whence) 	(llseek(fd, off, whence))
#endif


#define LCASE   01
#define UCASE   02
#define SWAB    04
#define NERR    010
#define SYNC    020
#define BLKI    040
#define BLKO    0100
#define NOTRUNC 0200
#define	NSHM_BUFS	3		/* number of shared memory buffers */
#define SYNC_GO -2
#define ERROR		(-1)
#define	MSGSTR(num,str)		catgets(catd,MS_DD,num,str)

#define inunits(bytes)  ((bytes)/iretrysz)
#define outunits(bytes) ((bytes)/oretrysz)

#define lowertoupper(c) if(cflag&UCASE && c>='a' && c<='z') \
							c += 'A'-'a';
#define uppertolower(c) if(cflag&LCASE && c>='A' && c<='Z') \
							c += 'a'-'A';


/*
 * If we are building the bldenv dd then it may be the
 * case that 64 bit is not available. This occurs when
 * dd is built on a 3.2* machine. To resolve this problem
 * the following variables are declared as int for the bldenv.
 * These dual declarations can be when all possible build
 * machines are running with 64 bit math libc.a.
 */
#ifdef _BLD

#define	FMT	"%u"	  /* format string for output statistics	*/

static off_t	inpos;    /* Used only for repositioning with BLKI */
static off_t	outpos;   /* Used only for repositioning with BLKO */
static off_t	newpos;   /* Used only for repositioning with errors (noerror) */ 
static long	skipi=0;
static long	skipo=0;
static long	count=0;
static long	skipf=0;
static int     nifr=0;         /* number of full input records(blocks) */
static int     nipr=0;         /* number of partial input records(blocks) */
static int     nofr=0;         /* number of full output records(blocks) */
static int     nopr=0;         /* number of partial output records(blocks) */
static int     ntrunc=0;       /* number of truncated records(blocks) */
static long    number(void);

#else	/* shipped dd	*/

#define	FMT	"%llu"	  /* format string for output statistics	*/

static offset_t	inpos;    /* Used only for repositioning with BLKI */
static offset_t	outpos;   /* Used only for repositioning with BLKO */
static offset_t	newpos;   /* Used only for repositioning with errors */ 
static long long	skipi=0;
static long long	skipo=0;
static long long	count=0;
static long long	skipf=0;
static long long	nifr=0;         /* number of full input records(blocks) */
static long long     	nipr=0;         /* number of partial input records(blocks) */
static long long     	nofr=0;         /* number of full output records(blocks) */
static long long     	nopr=0;         /* number of partial output records(blocks) */
static long long     	ntrunc=0;       /* number of truncated records(blocks) */
static long long    	number(void);

#endif	/* #ifdef _BLD */


static int	ascii(char *, int *),		block_mb(char *, int *),
	block_sb(char *, int *),	cnull_mb(char *, int *),
	cnull_sb(char *, int *),	ebcdic(char *, int *),
	ibm(char *, int *),		noconv(char *, int *),
	unblock(char *, int *);

static int	attach_shm(void),		close_ofile(void),
	do_child(void),			do_parent(void),
	flsh(void),			get_shm(int),
	match(char *),			mkpipes(void),
	null(int),			pad(int),
	prep_mbuf(void),		rpipe(int, char *, int),
	setobuf(void),			stats(void),
	wbuf(void),			wcbuf(char *, int),
	wpipe(int, char *, int);
static void	sig_hdl(int);
static void	chld_term(int);
static void	term(int);
static void	terminate(int);
static void	wpipe_err(int, int);

static unsigned ibs = UBSIZE;
static unsigned obs = UBSIZE;
static unsigned bs;
static unsigned cbs;
static unsigned ibc;
static unsigned obc;
static unsigned cbc;

static int     exitval=0;      /* exit value to indicate possible read errors */
static int     fflag=0;
static int     block_flag=0;   /* block or unblock was specified. */
static int     cflag=0;        /* for conv= options that are not conversion routines */
static int     ibf = -1;       /* input file descriptor */
static int     obf = -1;       /* output file descriptor */
static int     files = 1;
static int     nspace=0;
static int     iretrysz=0;     /* represents retry size when a read error occurs */
static int     oretrysz=0;     /* represents retry size when a write error occurs */
static int     irandom=0;      /* true if input is random access device */
static int     orandom=0;      /* true if output is random access device */
static int     singlebytecodeset;
static int	trunc_ofile;	/* true if obf needs to be truncated */
static int     add_nl=0;       /* add a newline to the end of the output */
static int     add_pad=0;      /* add padding to end of output */
static int     dd_signals (void);

static char    *string;
static char    *ifile;
static char    *ofile;
static char    *ibuf;
static char    *obuf;
static char    *op;

/* sprintf workplace to prepare perror() string */

static nl_catd	catd ;		/* message catalog descriptor	*/

/* double buffer vars */
static char *rbuf(void);		/* function to allocate an shm buffer */
static char *shmp[NSHM_BUFS];	/* shared memory pointers */
static int  shid[NSHM_BUFS];	/* shared memory ids */
static int  shm_ndx = 0;		/* index into shmp array */
static int  rparent_fd;		/* file descriptor for reading parent pipe */
static int  wparent_fd;		/* file descriptor for writing parent pipe */
static int  rchild_fd;			/* file descriptor for reading child pipe */
static int  wchild_fd;			/* file descriptor for writing child pipe */
static int  pid;				/* process id from fork() */
static struct shm_blk {		/* communication struct for parent & child */
	int	shm_ndx;
	int	cnt;
} shm_blk;

static char tempbuf[2* MB_LEN_MAX];    /* holds a split multibyte sequence */
static int split_mb=0;                 /* flag for the case of a split multibyte char */

int fildes[2];			/* pipe */
int wnofr = 0, wnopr = 0;	/* Write counts */

/* This is an EBCDIC to ASCII conversion table
*/
static unsigned char etoa [] =
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0325,0056,0074,0050,0053,0174,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0041,0044,0052,0051,0073,0176,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0313,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0136,0314,0315,0316,0317,0320,
	0321,0345,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0133,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0135,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};

/* This is an ASCII to EBCDIC conversion table
*/
static unsigned char atoe [] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0232,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0137,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0152,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0112,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0241,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

/* This is another (slightly different) ASCII to EBCDIC conversion table
*/
static unsigned char atoibm[] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};


main(int argc, char *argv[])
{
	register char *ip;
	register c;
	unsigned nunits;
	int     (*conv)(char *, int *);
	int     a;
	struct	devinfo devinfo;
	struct 	stat x;

	(void) setlocale(LC_ALL,"");
	catd = catopen (MF_DD , NL_CAT_LOCALE);

	singlebytecodeset = (MB_CUR_MAX == 1);

	conv = noconv;
	for(c=1; c<argc; c++) {
		string = argv[c];
		if(match("ibs=")) {
			ibs = (int) number();
			continue;
		}
		if(match("obs=")) {
			obs = (int) number();
			continue;
		}
		if(match("cbs=")) {
			cbs = (int) number();
			continue;
		}
		if(match("bs=")) {
			bs = (int) number();
			continue;
		}
		if(match("if=")) {
			ifile = string;
			continue;
		}
		if(match("of=")) {
			ofile = string;
			continue;
		}
		if(match("fskip=")) {
			skipf = number();
			continue;
		}
		if(match("seek=")) {
			skipo = number();
			continue;
		}
		if(match("skip=")) {
			skipi = number();
			continue;
		}
		if(match("count=")) {
			count = number();
			continue;
		}
		if(match("files=")) {
			files = number();
			continue;
		}
		if(match("conv=")) {
		cloop:
			if(match(","))
				goto cloop;
			if(*string == '\0')
				continue;
			if(match("ebcdic")) {
				conv = ebcdic;
		   		goto cloop;
			}
			if(match("ibm")) {
				conv = ibm;
				goto cloop;
			}
			if(match("ascii")) {
				conv = ascii;
				goto cloop;
			}
			if(match("block")) {
				block_flag++;
				if(singlebytecodeset)
					conv = block_sb;
				else
					conv = block_mb;
				goto cloop;
			}
			if(match("unblock")) {
				block_flag++;
				conv = unblock ;
				goto cloop;
			}
			if(match("iblock")) {
				cflag |= BLKI;
				goto cloop;
			}
			if(match("oblock")) {
				cflag |= BLKO;
				goto cloop;
			}
			if(match("lcase")) {
				cflag |= LCASE;
				goto cloop;
			}
			if(match("ucase")) {
				cflag |= UCASE;
				goto cloop;
			}
			if(match("swab")) {
				cflag |= SWAB;
				goto cloop;
			}
			if(match("noerror")) {
				cflag |= NERR;
				goto cloop;
			}
			if(match("sync")) {
				cflag |= SYNC;
				goto cloop;
			}
			if(match("notrunc")) {
				cflag |= NOTRUNC;
				goto cloop;
			}
		}
		fprintf(stderr, MSGSTR(EARG, "dd: bad arg %s\n"), string);
		usage();
	}
	if(conv == noconv && cflag&(LCASE|UCASE))
		if(singlebytecodeset)
			conv = cnull_sb;
		else
			conv = cnull_mb;

	if (ifile)
	{
		close(0);
		ibf = open(ifile, O_RDONLY);
		if(ibf < 0) {
			fputs("dd: ",stderr);
			perror(ifile);
			exit(2);
		}
	}
	else
		ibf = 0;

	/* make sure the input and output files are not the same */
	if (ifile && ofile) {
		struct stat in,out;

		if (stat(ifile, &in) == -1) {
			fputs("dd: ",stderr);
			perror(ifile);
			exit(2);
		}
		/* If ofile already exists AND it's not a special file AND      */ 
		/* it's dev/ino numbers are the same as ifile - don't SMASH it. */

		if (stat(ofile, &out) == 0 && !(in.st_mode & (S_IFCHR | S_IFBLK)) &&
				in.st_dev == out.st_dev && in.st_ino == out.st_ino) {
			fprintf(stderr, MSGSTR(ESTOMP, "dd: can't copy file onto itself\n"));
			exit(2);
		}
	}

	/* open the output file without truncating it if conv=notrunc was used */
	if (ofile) {
		close(1);
		if(skipo || cflag&NOTRUNC)
			obf = open(ofile, O_RDWR, (mode_t)0666);
		else
			obf = open(ofile, O_WRONLY|O_CREAT|O_TRUNC, (mode_t)0666);
		if(obf < 0) {
			fputs("dd: ",stderr);
			perror(ofile);
			exit(2);
		}
	}
	else
		obf = 1;

	trunc_ofile = ((ofile) && (stat(ofile, &x) == 0) &&
				(x.st_mode&S_IFREG) && !(cflag&NOTRUNC));

	if (bs)
		ibs = obs = bs;

	if(ibs == 0 || obs == 0) {
		fprintf(stderr, MSGSTR( EBLKSIZ, "dd: block sizes cannot be zero\n"));
		exit(2);
	}

	iretrysz = oretrysz = UBSIZE;
	irandom = FALSE;               /* bell code reads to skip on input  */
	orandom = TRUE;                /*      and  seeks to skip on output */

	if (cflag&BLKI)                /* for lack of contrary evidence,    */
		irandom = TRUE;        /* iblock option implies seekability */

    /* if input/output devices are random access */
	/* then set retry size to bytes per sector */
	if (ioctl(ibf, IOCINFO, &devinfo) >= 0) {
		irandom = ((devinfo.flags&DF_RAND) != 0);
		if (devinfo.devtype == DD_DISK)
			iretrysz = devinfo.un.dk.bytpsec;
	}

	if (ioctl(obf, IOCINFO, &devinfo) >= 0) {
		orandom = ((devinfo.flags&DF_RAND) != 0);
		if (devinfo.devtype == DD_DISK)
			oretrysz = devinfo.un.dk.bytpsec;
	}
	errno=0;

	if (((cflag&BLKI) && !irandom) || ((cflag&BLKO) && !orandom) ) {
		fprintf(stderr, MSGSTR( EDAC, "dd: conv=block requires direct access device\n"));
		exit(2);
	}

	if(((cflag&BLKI) && (ibs%iretrysz)) || ((cflag&BLKO) && (obs%oretrysz))) {
		fprintf(stderr,MSGSTR( ENOBSIZE, "dd: conv=block require block-sized records\n"));
		exit(2);
	}

	if (!orandom && skipo) {
		close(obf);
		obf = open(ofile, O_RDWR);
		if(obf < 0) {
			fprintf(stderr, MSGSTR( EPRMSKP,
			"dd: read permission needed to skip into file %s (on non-random device)\n"),
				 ofile);
			exit(2);
		}
	}

	/*
	 * POSIX 1003.2/D12 (paraphrased) states: "If bs is specified and
	 * no conversions OTHER than 'sync', 'noerror', or 'notrunc' are
	 * specified, then one block shall be written for each one read."
	 * This is accomplished through the 'fflag' variable.
	 */

	if ((conv == noconv) && (bs))
		if (!cflag || (cflag & (NERR | SYNC | NOTRUNC)))
			fflag++;

	prep_mbuf();	/* prepare for multiple buffering */

	ibc = 0;
	obc = 0;
	cbc = 0;
	op = obuf;

	dd_signals();

	if (skipf && (ioctl(ibf,IOCINFO,&devinfo)>=0) && (devinfo.devtype == DD_TAPE || devinfo.devtype == DD_SCTAPE)) {
		struct stop op;

		op.st_op = STFSF;
		op.st_count = skipf;
		if (ioctl(ibf, STIOCTOP, &op) < 0) {
			perror( MSGSTR( EREAD, "dd read error"));
			exit(2);
		}
	} else {
		while (skipf--) {
			int rc;

			while ((rc = read(ibf, ibuf, ibs)) > 0) ;
			if (rc < 0)
				exitval++;
		}
	}

	if (skipi)
		if (irandom)
			LLSEEK(ibf, inpos = ibs * skipi, 0);
		else
			while (skipi--) {
				if (read(ibf, ibuf, ibs) < 0)
					exitval++;
			}

	if (skipo)
		if (orandom)
			LLSEEK(obf, outpos = obs * skipo, 0);
		else
			while (skipo--) {
				if (read(obf, obuf, obs) < 0)
					exitval++;
			}

	/*
	 * Please do not change the way read is done.  This
	 * has caused major regression problems.  The standard
	 * clearly allows for reads of size less than a full
	 * block.  It is up to the user to realize that on a pipe
	 * they may get less than requested and therefore they
	 * need to specify either ibs and or obs and not use
	 * bs.  If bs is specified, the fflag (flush flag) will
	 * be set to write out the data in the size that was
	 * read in.  It will not loop to fill the input buffer.
	 */
	while (1) {
	if (ibc == 0) {
		if(count == 0 || nifr+nipr != count) {
			ibuf = rbuf();	/* allocate shm buffer */
			if(cflag&(NERR|SYNC)) {
				if (block_flag)
					memset(ibuf, ' ', ibs);	
				else
					bzero(ibuf, ibs);	
			}

			ibc = read(ibf, ibuf, ibs);
			/* set newpos for seeks during errors */
			newpos += (long) ibs;
		}
		if(ibc == (unsigned) -1) {
			perror( MSGSTR( EREAD, "dd read error"));
			exitval++;
			if (cflag&BLKI) {
				nunits = inunits(ibs);
				for (ip=ibuf, c=0; c<nunits; c++, ip+=iretrysz)  {
					LLSEEK(ibf, inpos+c*iretrysz, 0);
					switch (ibc = read(ibf,ip,(unsigned)iretrysz)) {
					   case -1:
						if ( iretrysz == UBSIZE )
							fprintf(stderr, MSGSTR( EBLKREAD, "dd: block " FMT ": read error\n"),
							inunits(inpos)+c);
						else
							fprintf(stderr, MSGSTR( ESCTREAD,
						"dd: sector " FMT ": read error\n"),
							inunits(inpos)+c);
						{
							register char *ip2;
							for (ip2=ip+iretrysz; ip2>ip;)
								*--ip2 = 0;
						}
						break;
					   case 0:
						goto reclen; /*double break*/
					   default:
						if (ibc != iretrysz)
						{       fprintf(stderr, MSGSTR( ERECSIZ,
								"dd: bad record size for conv=block\n"));
							exit(2);
						}
					} /* switch ibc */
				} /* for */
				/* Reposition in case last read failed */
				LLSEEK(ibf, inpos+c*iretrysz, 0);
reclen:         ibc = ip - ibuf;
			} /* if cflag&BLKI */
			else {   /* Error, not in blocki mode */
				if((cflag&NERR) == 0) {
					flsh();
					term(SIGINT);
				}
				else {
					/* seek to newpos on errors */
					LLSEEK(ibf, newpos, 0);
				}
				ibc = ibs;
			}
		} /* if ibc == -1 */
		if(ibc == 0 && --files <= 0) {
			if (add_nl)
				null('\n');
			if (add_pad)
				while(cbc < cbs) {
					null(' ');
					cbc++;
				}
			flsh();
			if (exitval > 0) {
				term(SIGINT);
			}
			else
				term(0);
		}
		if(ibc != ibs) {
			nipr++;
			if(cflag&SYNC)
				ibc = ibs;
		} else
			nifr++;
		inpos += ibc;
		ip = ibuf;
		if(cflag&SWAB) {
			if (ibc%2)
				ibc--;
			c = ibc >> 1;
			if (c) {
				do {
					a = *ip++;
					ip[-1] = *ip;
					*ip++ = a;
				} while(--c);
			}
		}
		ip = ibuf;
		if (fflag) {
			obc = ibc ;
			flsh();
			ibc = 0;
		}
		continue;
	} /* if ibc == 0 */
	(*conv)(ip, (int *) &ibc);
	} /* end while (1) */
} 

static
close_ofile(void)
{
	off_t filelen;

	/* In setting trunc_ofile, we asked is there an output file?, 
	 * if so, is it a disk file?, was the conv=notrunc not used?,
	 * If all of the above were true then we need to truncate the
	 * output file.
	 */
	if (trunc_ofile) {
		if ((filelen = lseek(obf, (off_t)0, SEEK_CUR)) == -1) {
			fputs("dd: ",stderr);
			perror(ofile);
			exit(2);
		}
		if (ftruncate(obf, filelen) == -1) {
			fputs("dd: ",stderr);
			perror(ofile);
			exit(2);
		}
	}
	close(ibf);
	if (close(obf) < 0) {
		perror(MSGSTR(EWRITE,"dd: write error"));
		exit(2);
	}
	return;
}

static
dd_signals (void)
{
	struct sigaction	act;
	sigaction (SIGINT, (struct sigaction *)0, &act);
	if (act.sa_handler != SIG_IGN) {
		act.sa_handler = sig_hdl;
		act.sa_flags &= ~SA_OLDSTYLE;
		sigaction (SIGINT, &act, (struct sigaction *)0);
	}
	sigaction (SIGTERM, (struct sigaction *)0, &act);
	act.sa_handler = sig_hdl;
	act.sa_flags &= ~SA_OLDSTYLE;
	sigaction (SIGTERM, &act, (struct sigaction *)0);
	sigaction (SIGQUIT, (struct sigaction *)0, &act);
	act.sa_handler = sig_hdl;
	act.sa_flags &= ~SA_OLDSTYLE;
	sigaction (SIGQUIT, &act, (struct sigaction *)0);
	sigaction (SIGPIPE, (struct sigaction *)0, &act);
	act.sa_handler = sig_hdl;
	act.sa_flags &= ~SA_OLDSTYLE;
	sigaction (SIGPIPE, &act, (struct sigaction *)0);
	sigaction (SIGCHLD, (struct sigaction *)0, &act);
	act.sa_handler = sig_hdl;
	act.sa_flags &= ~SA_OLDSTYLE;
	act.sa_flags |= SA_RESTART;
	act.sa_flags |= SA_NOCLDSTOP;
	sigaction (SIGCHLD, &act, (struct sigaction *)0);
}

static
flsh(void)
{
	if(obc) {
		wbuf();
		if(obc == obs)
			nofr++;
		else
			nopr++;
		outpos += obc;
		obc = 0;
	}
}

static
match(char *s)
{
	register char *cs;

	cs = string;
	while(*cs++ == *s)
		if(*s++ == '\0')
			goto true;
	if(*s != '\0')
		return(0);

true:
	cs--;
	string = cs;
	return(1);
}

#ifdef	_BLD
static long
number(void)
{
	long n;
#else
static long long
number(void)
{
	long long n;
#endif
	register char *cs;

	cs = string;
	n = 0;
	while(*cs >= '0' && *cs <= '9')
		n = n*10 + *cs++ - '0';
	for(;;)
	switch(*cs++) {

	case 'k':
		n *= 1024L;
		continue;

	case 'w':
		n *= 2L;
		continue;

	case 'b':
		n *= UBSIZE;
		continue;

	case '*':
	case 'x':
		string = cs;
		n *= number();

	case '\0':
		return(n);
	}
	/* never gets here */
}

static
noconv(char *p, int *cnt)
{
	while (*cnt > 0){
		*op = *p;
		p++;
		op++;
		*cnt -= 1;
		if(++obc >= obs) {
			flsh();
			setobuf();
			op = obuf;
		}
	}
}

static
pad(int c)
{
	do {
		*op = c;
		op++;
	} while (++obc < obs);
}

static
null(int c)
{
	*op = c;
	op++;
	if(++obc >= obs) {
		flsh();
		setobuf();
		op = obuf;
	}
}


/*
 * Single byte code set version of cnull() 
 */
static
cnull_sb(char *p, int *cnt)
{
	register c; 	/* c holds a character for singlebyte code esets */

	while (*cnt > 0){
		c = *p;
		p++;
		*cnt -= 1;
		if(cflag&UCASE && islower(c))
			c = toupper(c);
		if(cflag&LCASE && isupper(c))
			c = tolower(c);
		*op = c;
		op++;
		if(++obc >= obs) {
			flsh();
			setobuf();
			op = obuf;
		}
	}
}

/*
 * Multibyte code set version of cnull() 
 */
static
cnull_mb(char *p, int *cnt)
{
	wchar_t wc;		/* Holds a wide character code for multibyte code sets */
	char mbchar[MB_LEN_MAX];	/* Holds the upper/lower case of a multibyte char */
	int i;
	int rc;
	int wcnt;
	char *inchar;
	int  rem_cnt;	/* number of bytes that make the previous split char complete */
	/* 
	 * A full multibyte character = split_mb + rem_cnt 
	 * The split_mb acts as a flag as well as a count of how many bytes were 
	 * left in the previous buffer. 
	 * The rem_cnt is formed after converting the split multibyte character 
	 * with the latest read() buffer using mbtowc().
	 */
	while (*cnt > 0) {
		if(split_mb){
			/*
			 * copy MB_CUR_MAX - the current length into tempbuf, the
			 * current length is the number of split bytes previously saved.
			 */
			for(i=0;i<MB_CUR_MAX-split_mb; i++)
				tempbuf[i+split_mb] = p[i];
			/*
			 * tempbuf will now contain the full multibyte char
			 * that was previously split.  
			 * full multibyte char = split_mb + (max mbchar size - split_mb)
			 */
			inchar = tempbuf;
		}else	
			inchar = p; 	/* p is pointer to char to be converted */
		/*
		 * In the case of multibyte code sets we need to handle split multibyte
		 * characters across the read() system call. (ie split multibyte chars)
		 * The way we do this is by saving the bytes split in a tempbuf[].  We 
		 * will not flush this buffer or write the output bytes in this yet.  We
		 * fetch the next block from read() and see if it is a valid multibyte
		 * character.
		 */
		rc = mbtowc(&wc, inchar, MB_CUR_MAX);
        switch(rc){
            case -1:    /* case for an invalid multibyte char -
                         * If the current inchar wasn't truncated or the previous
                         * inchar wasn't either then this is a legitimate error.
                         */
                        if ((*cnt >= MB_CUR_MAX) || split_mb){
                            fprintf(stderr, MSGSTR(BADMBCH,
                                "dd: Invalid multibyte char found\n"));
                                /* ??? Should we pad the obf and flush this data */
				flsh();
                                term(SIGINT);
                        }else{
                            for (i=0; i< *cnt; i++)
                                tempbuf[i] = p[i];
                            split_mb = i; /* counter as well as a flag */
                            *cnt = 0;
                        }
                        break;
            case 0:     /* case for a null character */
                        null(0);
                        split_mb = 0;	/* reset it */
                        *cnt -= 1;
						p++;
                        break;

            default:    /* case for a valid multibyte character */
                        if(split_mb)
                            rem_cnt = rc - split_mb;
                        /*	
                         * rc is the the number of bytes that made the character.
                         * rem_cnt is the count of the bytes that participated in
                         * forming the multibyte character from the current read()
                         */
                        if (cflag & UCASE && iswlower(wc))
                           wc = towupper(wc);
                        if (cflag & LCASE && iswupper(wc))
                            wc = towlower(wc);

                        wcnt = wctomb(mbchar, wc);
                        for(i=0;i<wcnt;i++){
                            *op = mbchar[i];
                            op++;
                            if(++obc >= obs) {
                                flsh();
                                setobuf();
                                op = obuf;
                            }
                        }

                        /* 
                         * Need to move the input buffer pointer to point to the
                         * next multibyte char after handling the split character
                         * case.  So add the number of bytes after the split that
                         * made a full character, decrement the count by this same
                         * number.  And if we have just taken of a split multibyte
                         * character reset the split_mb flag.
                         */
                        if(split_mb){
                            split_mb = 0;	/* reset it */
                            p += rem_cnt;
                            *cnt -= rem_cnt;
                        }else{
                            p += rc;
                            *cnt -= rc;
                        }
        }/* switch */
	}/* while loop */
}

/*
 * Single byte code set version of ascii
 */
static
ascii(char *p, int *cnt)
{
	register c;

	while (*cnt > 0){
		c = *p;
		p++;
		*cnt -= 1;
		c = etoa[c];
		if(cbs == 0) {
			lowertoupper(c);
			uppertolower(c);
			null(c);
			continue;
		}
		if(c == ' ')
			nspace++;
		else {
			while(nspace > 0) {
				null(' ');
				nspace--;
			}
			lowertoupper(c);
			uppertolower(c);
			null(c);	/* Output the character */
		}

		if(++cbc >= cbs) {
			null('\n');
			cbc = 0;
			nspace = 0;
		}
	}/* while *cnt > 0 */	
}

/*
 * Single byte code set version for ebcdic
 */
static
ebcdic(char *p, int *cnt)
{
	register c,cc;

	while (*cnt > 0){
		c = cc = *p;
		p++;
		*cnt -= 1;
		lowertoupper(c);
		uppertolower(c);
		c = atoe[c];
		if(cbs == 0) {
			null(c);
			continue;
		}
		if(cc == '\n') {
			while(cbc < cbs) {
				null(atoe[' ']);
				cbc++;
			}
			cbc = 0;
			continue;
		}
		if(cbc == cbs)
			ntrunc++;
		cbc++;
		if(cbc <= cbs)
			null(c);
	}
}

/*
 * Single byte code version for ibm
 */
static
ibm(char *p, int *cnt)
{
	register c,cc;

	while (*cnt > 0){
		c = cc = *p;
		p++;
		*cnt -= 1;
		lowertoupper(c);
		uppertolower(c);
		c = atoibm[c];
		if(cbs == 0) {
			null(c);
			continue;
		}
		if(cc == '\n') {
			while(cbc < cbs) {
				null(atoibm[' ']);
				cbc++;
			}
			cbc = 0;
			continue;
		}
		if(cbc == cbs)
			ntrunc++;
		cbc++;
		if(cbc <= cbs)
			null(c);
	} /* while cnt */
}

 
/*
 * block_sb - single byte code set verion of block
 *
 * DESCRIPTION 
 *	Treat the input as a sequence of '\n' terminated variable 
 *	length records.  Each record shall be converted to a record 
 *	with a fixed length specified by the conversion block size
 *  (cbs).  The '\n' shall be removed from the input line; ' ''s
 *	appended to lines that are shorter than their conversion block
 *	to fill the block.  Lines that are longer than the conversion
 *	block size shall be truncated to the largest number of chars
 *	that will fit into that size; the number of truncated lines
 *	shall be reported.  
 *	
 * INPUT
 *	p - a character pointer to the input sequence to be converted
 *	cnt - the size of the input sequence
 *
 * OUTPUT
 *	The converted input shall be output when the conversion block
 *	size is reached.  
 */
static
block_sb(char *p, int *cnt)
{
	register c,cc;

	while (*cnt > 0){
		c = cc = *p;
		p++;
		*cnt -= 1;
		lowertoupper(c);
		uppertolower(c);
		c = c & 0xff;
		if(cbs == 0) {
			null(c);
			continue;
		}
		if(cc == '\n') {
			while(cbc < cbs) {
				null(' ');
				cbc++;
			}
			cbc = 0;
			continue;
		}
		if(cbc == cbs)
			ntrunc++;
		cbc++;
		if(cbc <= cbs)
			null(c);
	} /* while cnt */
	/*
	 * POSIX requires the last block to be padded even if it does not
	 * contain a newline.  The following check sets "add_pad" to true or
	 * false depending on whether a trailing newline has been processed in
	 * the current input block.  The last block converted will then set
	 * add_pad to the correct value, and another portion of code will add
	 * the padding.
	 */
	add_pad = ((cbs != 0) && (cc != '\n'));
}

/*
 * block_mb - multibyte code set verion of block
 *
 * DESCRIPTION 
 *	Treat the input as a sequence of '\n' terminated variable 
 *	length records.  Each record shall be converted to a record 
 *	with a fixed length specified by the conversion block size
 *  (cbs).  The '\n' shall be removed from the input line; ' ''s
 *	appended to lines that are shorter than their conversion block
 *	to fill the block.  Lines that are longer than the conversion
 *	block size shall be truncated to the largest number of chars
 *	that will fit into that size; the number of truncated lines
 *	shall be reported.  
 *	
 * INPUT
 *	p - a character pointer to the input sequence to be converted
 *	cnt - the size of the input sequence
 *
 * OUTPUT
 *	The converted input shall be output when the conversion block
 *	size is reached.  
 *	If an invalid multibyte character is found the converted input 
 *	up to the invalid character shall be padded with <space>'s and 
 *	flushed then exit with a non-zero value.
 */
static
block_mb(char *p, int *cnt)
{
	register c,cc;
	register char *mbc;
	size_t mbchlen;
	int i;
	static int split_ch=0;

	while (*cnt > 0){
		mbc = p;
		if (split_ch){
			for (i=0; i<MB_CUR_MAX-split_ch;i++)
				tempbuf[i+split_ch] = p[i];
			mbc = tempbuf;
		}
		mbchlen = mblen(mbc, MB_CUR_MAX);
		switch(mbchlen){
			case 1:		/* Single-Byte Case */
					p++;
					*cnt -= 1;
					c = cc = *mbc;
					lowertoupper(c);
					uppertolower(c);
					c = c & 0xff;
					if(cbs == 0) {
						null(c);
						continue;
					}
					if(cc == '\n') {
						while(cbc < cbs) {
							null(' ');
							cbc++;
						}
						cbc = 0;
						continue;
					}
					if(cbc == cbs)
						ntrunc++;
					cbc++;
					if(cbc <= cbs)
						null(c);
					break;

			case 0:		/* Null Characters */
					null(0);
					*cnt -= 1;;
					p++;
					cbc++;
					break;

			case -1:	/* Split/Invalid Multibyte Case */
					if (*cnt >= MB_CUR_MAX|| split_ch){
						fprintf(stderr, MSGSTR(BADMBCH,
							"dd: Invalid multibyte char found\n"));
							/* pad the obf and flush this data */
							pad(' ');
							flsh();
							term(SIGINT);
					}else{
						for(i=0;i < *cnt;i++)
							tempbuf[i] = mbc[i];
						split_ch = *cnt;
						*cnt = 0;
					}
					break;

			default:	/* Valid Multibyte Case */
					/* when blocking on multibyte characters we need to insure
					 * that we don't truncate long records in the middle of a
					 * multibyte character.  So if the current multibyte char
					 * would cause this we truncate at the previous character
					 * and pad up to the cbs size with blanks.
					 */
					if ((cbs == 0) || ((cbc + mbchlen) <= cbs))
						for (i=0;i < mbchlen;i++)
							null(mbc[i]);
					else
						for (i=0;i < cbs - cbc;i++)
							null(' ');
					cbc += i;
					*cnt -= (mbchlen - split_ch);
					p += (mbchlen - split_ch);
					split_ch = 0;
					break;
		} /* switch */
	} /* while cnt */
	/*
	 * POSIX requires the last block to be padded even if it does not
	 * contain a newline.  The following check sets "add_pad" to true or
	 * false depending on whether a trailing newline has been processed in
	 * the current input block.  The last block converted will then set
	 * add_pad to the correct value, and another portion of code will add
	 * the padding.
	 */
	add_pad = ((cbs != 0) && ((mbchlen!=1) || (cc != '\n')));
}

static
unblock(char *p, int *cnt)
{
	register c;

	while (*cnt > 0){
		c = *p;
		p++;
		*cnt -= 1;
		c = c & 0xff;
		if(cbs == 0) {
			lowertoupper(c);
			uppertolower(c);
			null(c);
			continue;
		}
		if(c == ' ')
			nspace++;
		else {
			while(nspace > 0) {
				null(' ');
				nspace--;
			}
			lowertoupper(c);
			uppertolower(c);
			null(c);
		}

		if(++cbc >= cbs) {
			null('\n');
			cbc = 0;
			nspace = 0;
		}
	}
	/*
	 * POSIX requires a newline after the last block even if it was a
	 * partial block.  The following check sets "add_nl" to true or false
	 * depending on whether a newline has not been added to the current
	 * block.  The last block converted will then set add_nl to the correct
	 * value, and another portion of code will add the final newline char.
	 */
	add_nl = ((cbs != 0) && (cbc != 0));
}

static int exit_value = 0;		/* Holder for parent's exit status */

static void
sig_hdl(int sig)
{
	int newpid, status;

	if ( sig != SIGCHLD ) {
		if (!exit_value)
			exit_value = sig;
		wpipe_err(nofr, nopr);
		kill(pid, SIGKILL);
		pause();		/* wait for SIGCHLD */
		return;			/* This shouldn't happen */
	}

	/*
	 * Cleanup child process by doing a wait().  If the child has already
	 * been waited for or does not exist, then an error condition exists.
	 */

	newpid = wait(&status);

	if (newpid < 0) {
		perror("wait");
		wpipe_err(nofr, nopr);
		kill (pid, SIGKILL);
		cleanup();
		exit (errno);
	}

	/*
	 * NOTE: Due do the way that ksh handles grave accent pipes,
	 * dd may have an 'illegitimate' child that caused us to enter
	 * this routine.  If that child is not ours, then just return
	 * from the signal handler.  e.g.:  FOO=`a|b|c|dd`
	 */
	if (newpid != pid)
		return;

	/*
	 * This should be the only call to terminate().  Now that the
	 * child process is dead, we can give it's exit status to the
	 * terminate() routine and exit gracefully.
	 */
	terminate(status);
}

static void
chld_term(int c)
{
	int i;
	/*
	 * child's cleanup
	 */
	for (i=0; i<NSHM_BUFS; i++)
		shmdt(shmp[i]);
	wpipe_err(wnofr, wnopr);
	close(wchild_fd);
	close(rchild_fd);
	exit(c);
}

static void
term(int c)
{
	/*
	 * If this is an error termination, kill the child process,
	 * otherwise, tell it that it should terminate when it is
	 * finished writing.
	 */
	if (c) {
		exit_value = c;
		wpipe_err(nofr, nopr);
		kill(pid, SIGKILL);
	} else {
		shm_blk.shm_ndx = EOF;
		wpipe(wparent_fd, (char *) &shm_blk, sizeof(shm_blk));
	}
	pause();			/* wait for SIGCHLD */
	exit(c); 			/* This shouldn't happen */
}

static void
terminate(int status)
{
	int i;

	for (i=0; i<NSHM_BUFS; i++)
		shmdt(shmp[i]);

	/*
	 * If exit_value is set then use it's value when exiting, else
	 * use the exit status or signal number from the child.
	 */
	if (!exit_value) {
		if (WIFEXITED(status))
			exit_value = WEXITSTATUS(status);
		else {
			exit_value = WTERMSIG(status);
			fprintf(stderr, MSGSTR(BYSIG,
				"dd: process killed by signal %d\n"), exit_value);
		}
	}

	cleanup();
	close_ofile();

	/* Per POSIX and XPG4 */
	if (exit_value == 0  ||  exit_value == SIGINT) {
		if (exit_value == SIGINT)
		{
			read(fildes[0], (char *) &wnofr, sizeof(wnofr));
			read(fildes[0], (char *) &wnopr, sizeof(wnopr));
			close(fildes[0]);
			close(fildes[1]);
			if(nofr != wnofr) {
				nofr = wnofr;
				nopr = wnopr;
			}
		}
		stats();
	}
	exit(exit_value);
}

static
stats(void)
{
	fprintf(stderr, MSGSTR( RECSIN, FMT "+" FMT " records in\n"),  nifr, nipr);
	fprintf(stderr, MSGSTR( RECSOUT, FMT "+" FMT " records out\n"), nofr, nopr);

	if (ntrunc == 1)
		fprintf(stderr, MSGSTR(TRUNREC, FMT " truncated record\n"), ntrunc);
	else if (ntrunc)
		fprintf(stderr, MSGSTR(TRUNRECS, FMT " truncated records\n"), ntrunc);
}

static
cleanup()
{
	int i;

	for (i=0; i<NSHM_BUFS; i++) {
		shmctl(shid[i], (int)IPC_RMID, (struct shmid_ds *)0);
	}
	close(wparent_fd);
	close(rparent_fd);
}

/*
 * added code for double buffering, using shm and pipes
 *
 * The basic algorithm is:
 *	a. dd creates two pipes:  reader->writer, writer->reader.
 *	b. dd also gets a shared mem id.
 *	c. after forking, the child becomes the writer, and the 
 *		parent the reader.
 *	d. the writer owns the output buffers, and tells the
 *		reader when he can put something in one.  To do this
 *		he sends the index of the shared memory segment address
 *		through the writer->reader pipe.
 *	e. when the reader has a buffer ready, he sends its index, and
 *		the number of bytes to write through the reader->writer pipe.
 *	f. all pipe reads are blocking.
 *	g. most of dd is still in the parent, i.e., all reads, conversions,
 *		etc., and the child is limited to writing the data.
 */

/*
 * prep_mbuf - get pipes and shm for the iobuffers, if the input and
 *	output buffers are not the same, grab an input buffer with
 *	sbrk.
 *	fork a child to handle the output buffers.
 */
static
prep_mbuf(void)
{
	mkpipes();
	get_shm(obs);

	if (!fflag) {
	    	int i;
		ibuf = sbrk(ibs);
		sbrk((unsigned)64);
		if(ibuf == (char *)-1) {
			fprintf(stderr, MSGSTR( ENOMRY, "dd: not enough memory\n"));
			for (i=0; i<NSHM_BUFS; i++) {
			    shmctl(shid[i], (int)IPC_RMID, (struct shmid_ds *)0);
			}
			exit(2);
		}
	}

	/* open a pipe for handling child write error before fork a child */
	pipe(fildes);

	if ((pid = fork()) == ERROR) {
		perror( MSGSTR( EFORK, "dd can't fork"));
		term(SIGQUIT);
	} else
	if (pid == 0) {				/* Child Code */
		signal(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		do_child();	/* never returns */
	} else {				/* Parent Code */
		do_parent();
		do_sync();
		setobuf();
		if (fflag) {
			ibuf = obuf;
		}
	}
}

/*
 * get_shm: get shared memory id's for each of two buffers
 */
static
get_shm(int bufsiz)
{
	register i;

	for (i=0; i<NSHM_BUFS; i++) {
		if ((shid[i] = shmget(IPC_PRIVATE, bufsiz, 0777)) == ERROR) {
			perror( MSGSTR( ESHMEM, "dd can't get shared memory"));
			exit(3);
		}
	}
}

/*
 * mkpipes:  make biway communication with two pipes
 */
static
mkpipes(void)
{
	int fd[2];

	if (pipe(fd) < 0) {
		perror( MSGSTR( ERRPIPE, "dd pipe error"));
		term(SIGQUIT);
	}
	rparent_fd = fd[0];
	wchild_fd = fd[1];

	if (pipe(fd) < 0) {
		perror( MSGSTR( ERRPIPE, "dd pipe error"));
		term(SIGQUIT);
	}
	rchild_fd = fd[0];
	wparent_fd = fd[1];
}

/*
 * do_parent: do parental duties
 *	a. close unused file descriptors
 *	b. attach to the shared memory
 */
static
do_parent(void)
{
	close(wchild_fd);	/* These 2 descriptors aren't needed here */
	close(rchild_fd);

	attach_shm();		/* Attach shared memory */
}

/*
 * do_child: stay here and write buffers
 *	a. close unused file descriptors
 *	b. attach to the shared memory
 *	c. tell reader side we're ready for some stuff
 *	d. write out buffers, and handshake over the pipes
 */
static
do_child(void)
{
	register i;
	int sync_now;

	
	close(wparent_fd);	/* These 2 descriptors aren't needed here */
	close(rparent_fd);

	attach_shm();		/* Attach shared memory */

	/* sync up with parent. */
	sync_now = SYNC_GO;
	wpipe(wchild_fd, (char *) &sync_now, sizeof(sync_now));

	/* read the go from parent. */
	rpipe(rchild_fd, (char *) &sync_now, sizeof(sync_now));
	if (sync_now != SYNC_GO)
		chld_term(0);
	for (i=0; i<NSHM_BUFS; i++) {
		shmctl(shid[i], (int)IPC_RMID, (struct shmid_ds *)0);
	}

	/*
	 * tell the reader all buffers are ready and waiting
	 */
	wpipe(wchild_fd, (char *) &shm_ndx, sizeof(shm_ndx));
	for (i=(fflag ? 0 : 1); i<NSHM_BUFS; i++) {
		shm_ndx = i;
		wpipe(wchild_fd, (char *) &shm_ndx, sizeof(shm_ndx));
	}

	shm_ndx = 0;
	while (1) {
		/*
		 * get a buffer index, and byte-cnt from the reader
		 * and write it out.
		 * parent will send us an EOF when all is done
		 */
		rpipe(rchild_fd, (char *) &shm_blk, sizeof(shm_blk));
		if (shm_blk.shm_ndx == EOF) {
			break;
		}
		wcbuf(shmp[shm_blk.shm_ndx], shm_blk.cnt);
		wpipe(wchild_fd, (char *) &shm_blk.shm_ndx, sizeof(shm_blk.shm_ndx));
	}
	chld_term(0);
}

/*
 * do_sync: synchronize the parent and child processes.
 * We now remove the share memory id so that it does not
 * stay around when dd is killed by SIGKILL when it can cause
 * a paging space problem.  (Happens when bs= is very large)
 */
static
do_sync()
{
        int sync_go, i;

        /* read the go from child. */
        rpipe(rparent_fd, (char *) &sync_go, sizeof(sync_go));
	if (sync_go != SYNC_GO)
		term(0);
        /* sync up with child. */
        sync_go = SYNC_GO;
        wpipe(wparent_fd, (char *) &sync_go, sizeof(sync_go));
        for (i=0; i<NSHM_BUFS; i++) {
		shmctl(shid[i], (int)IPC_RMID, (struct shmid_ds *)0);
        }
}

/*
 * attach_shm: attach to the shared memory. (used by both parent and child)
 */
static
attach_shm(void)
{
	register i;

	for (i=0; i<NSHM_BUFS; i++) {
		if ((shmp[i] = shmat(shid[i], (char *)NULL, (int)NULL)) == (char *)ERROR) {
			perror( MSGSTR( ESHMAT, "dd shmat error"));
			if (pid)
				term(SIGQUIT);
			else
				chld_term(SIGQUIT);
		}
	}
}

/*
 * setobuf: set output buffer address based on info from the writer
 */
static
setobuf(void)
{
	/*
	 * wait for next available output buffer
	 */
	rpipe(rparent_fd, (char *) &shm_ndx, sizeof(shm_ndx));
	obuf = shmp[shm_ndx];
}

/*
 * rpipe: read a pipe
 */
static
rpipe(int fd, char *bp, int cnt)
{

	if (read(fd, bp, (unsigned)cnt) == ERROR) {
		if (pid) {
			perror ( MSGSTR (EREADFP, "dd parent can't read pipe" ));
			term(SIGQUIT);
		} else {
			perror ( MSGSTR (EREADFC, "dd child can't read pipe" ));
			chld_term(SIGQUIT);
		}
	}
}

/*
 * wpipe: write a pipe
 */
static
wpipe(int fd, char *bp, int cnt)
{

	if (write(fd, bp, (unsigned)cnt) == ERROR) {
		if (pid) {
			perror ( MSGSTR (EWRITFP, "dd parent can't write pipe" ));
			term(SIGQUIT);
		} else {
			perror ( MSGSTR (EWRITFC, "dd child can't write pipe" ));
			chld_term(SIGQUIT);
		}
	}
}

/*
 * rbuf: read a buffer, selecting the appropriate output buffer if necessary
 */
static char *
rbuf(void)
{
	if (fflag) {
		setobuf();
		ibuf = obuf;
	}
	return (ibuf);
}

/*
 * wbuf: reader's write buf routine, it writes to the writer process,
 *	and tells him where & how much
 */
static
wbuf(void)
{
	shm_blk.cnt = obc;
	shm_blk.shm_ndx = shm_ndx;
	wpipe(wparent_fd, (char *) &shm_blk, sizeof(shm_blk));
}

/*
 * wcbuf: child's (writer's) write routine
 */
static
wcbuf(char *bp, int cnt)
{
	int c;
	char *cp;
	unsigned nunits;

	c = write(obf, bp, (unsigned)cnt);
	if (c != cnt) {
		if (c < 0)
			perror(MSGSTR(EWRITE,"dd: write error"));
		else {
			/* A 0-length block does not count as a write */
			if (c)
			    wnopr++;
			fprintf(stderr, MSGSTR( EWRFEW,
				"write of %d bytes, only wrote %d.\n"),cnt,c);
		}
		if(cflag&BLKO) {
			nunits = outunits(cnt);
			for(cp=bp, c=0; c<nunits; c++, cp+=oretrysz) {
				LLSEEK(obf, outpos+c*oretrysz, 0);
				if (write(obf,cp,(unsigned)oretrysz) != oretrysz)
					if ( oretrysz == UBSIZE )
						fprintf(stderr, MSGSTR( EBLKWRITE,
						"dd: block " FMT ": write error\n"),
						outunits(outpos)+c);
					else
						fprintf(stderr, MSGSTR( ESCTWRITE,
						"dd: sector " FMT ": write error\n"),
						outunits(outpos)+c);
			}
			/* In case last write failed */
			LLSEEK(obf, outpos+c*oretrysz, 0);
		} else {
			wpipe_err(wnofr, wnopr);
			chld_term(SIGINT);
		}
	}
	else wnofr++;
}

static
usage()
{
	fprintf(stderr, MSGSTR(USAGE1,
		"Usage: dd [if=InputFile] [of=OutputFile] [cbs=Number] [fskip=Number]\n"));
	fprintf(stderr, MSGSTR(USAGE2,
		"          [skip=Number] [seek=Number] [count=Number] [bs=Number]\n"));
	fprintf(stderr, MSGSTR(USAGE3,
		"          [ibs=Number] [obs=Number] [files=Number] [conv=Parameter[, ...]]\n"));
	exit(2);
}


static void wpipe_err(fr, pr)
int fr, pr;
{
	write(fildes[1], (char *) &fr, sizeof(fr));
	write(fildes[1], (char *) &pr, sizeof(pr));
}
