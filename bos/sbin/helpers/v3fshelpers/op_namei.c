static char sccsid[] = "@(#)91	1.13.1.10  src/bos/sbin/helpers/v3fshelpers/op_namei.c, cmdfs, bos411, 9428A410j 4/27/94 07:57:35";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the filesystem
 *
 * FUNCTIONS: op_namei
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* conversion to v3 fs notes:
    o inode table size is stored in special inode #3.
    o changed NADDR to NDADDR.
    o changed references to di_addr to di_rdaddr
    o be careful with block sizes.  The data stored in the superblock is
      the sizeof the fs in 512 byte blocks.  when we talk about a block,
      though, we talk about a BLKSIZE (4096) byte block.
*/

/*

	ff - fast(er) find

	ff produces a list of filenames for a specified filesystem,
	allowing selection criteria.

	strategy:

	- process command line, saving any selection criteria.
	- read superblock of fs to obtain size, and allocate data
	  areas (see below).
	- read the ilist and save:
		- inumbers and inode information from selected files
		- block numbers of directory files.
	- read all directory blocks saving all directory entries
	- for all selected inumbers, generate their names.

	data areas:

	- Ireq is simply a list of saved inumbers who have been selected
	  for name generation.
	- Db holds directory block data. Specifically we need to know which
	  blocks in the filesystem belong to directories, and which dir is
	  their owner. This associates the dirnames which we read with their
	  parent directory (.) pointer.
	- Dd is an array, with inumber as an index, whose entries are the
	  leaf name of a file. The pinum member of Dd points to an entry's
	  parent directory name, whose pinum points ...  So we go, climbing
	  the directory structure for each name we generate.
	  NOTE: Dd, being an array, MUST have as many elements as there are
	  inumbers. Can't compromise on space for this guy.
	- Indb holds info on the rare case of indirect directories.
	- Link is a Dd array in which we save info on linked files. If a
	  given inode already has an entry in Dd we save it in Link.

	  writ by ez for the ill-fated Large UNIX Systems Development Group
*/

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <fcntl.h>
#include <jfs/filsys.h>
#include <sys/vmdisk.h>
#include <jfs/inode.h>
#include <jfs/ino.h>
#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include <sys/stat.h>
#include <sys/signal.h>
#include <pwd.h>
#include <fshelp.h>
#include "fshargs.h"
#include <sys/vfs.h>
#include <setjmp.h>

#include <libfs/libfs.h>
#include "fsop.h"

#include <ctype.h>
#include <sys/types.h>
#include <locale.h>

#include <nl_types.h>
#include "op_namei_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_OP_NAMEI,Num,Str)


#define	A_DAY	86400l	/* seconds in a 24 hour period */
#define	ALLOC	0
#define	FREE	1

/*
	Formatting options for printout
*/
#define	F_INODE		(1<<0)
#define	F_SIZE		(1<<1)
#define	F_UID		(1<<2)
#define	F_PREPATH	(1<<3)

/*
	Run options
*/
#define	F_REQUEST	(1<<0)
#define	F_LINKSTOO	(1<<1)
#define	F_DEBUG		(1<<2)
#define F_MALLOC	(1<<3)	/* NOT USED */
#define F_IGSPECIAL	(1<<4)

/*	Data Structures	*/
/* Dd and Link - holds data from a directory entry */
struct	Dd	{
	ino_t	inum;		/* inumber of file represented by entry */
	ino_t	pinum;		/* inumber of parent directory */
	char	*dname;		/* file name */
};
static struct	Dd	*Ddlist, *Linklist;
static long	nLink;
static long	maxL;

/* Ireq - holds inumbers and any inode data for selected files */
struct	Ireq	{
	ino_t	inum;
	off_t	size;
	ushort	uid;
};
static struct	Ireq	*pIreq, *Ireqlist;
static long	nIreq;
static long	maxIreq;

/* Db - holds data on the blocks of the filesystem which are directories */
struct	Db	{
	ino_t	inum;		/* inumber of this directory */
	off_t	size;		/* # of data bytes in this directory blk */
	frag_t	blkno;		/* block number */
};
static struct	Db	*pDb, *Dblist;
static long	nDb;
static long	maxDb;
static long	sbagsize;
static ino_t	imax;


/* Nlist holds specific inode numbers requested with the -i option */
static struct fsh_argvector	*Nlist = 0;

/* ul[] holds a list of usernames/uids for username cacheing to avoid numerous
 * calls to getpwent() and its ilk.
 */
#define	NSZ	8	/* max length of a login name */
#define USIZE1	50	/* size of username cache */

static int usize1;		/* number of entries taken in username cache */
static struct ulist {
	char	uname[NSZ];	/* username */
	ushort	uuid;		/* uid */
} ul[USIZE1];		/* declare username cache */


extern	int	errno;		/* system error number */
extern	int	sys_nerr;	/* # errs in system error list */
extern	char	*sys_errlist[];	/* array of system error strings */

static jmp_buf err_out; 	/* jump buf to get out in case of emergency */

static struct stat statb;	/* global buffer for stat(2) */

static int	g_bufn,		/* internal buffer allocation counter */
		g_ilb,		/* ilist size in blocks */
		g_inodes,	/* ilist size in inodes */
		g_inopb,	/* number of inodes in a block */
		g_bsize,	/* actual filesystem block size */
		g_devfd;	/* input filesystem file pointer */

static char	*g_cmd,		/* our name, as invoked by the user */
		*g_dev,		/* name of filesystem we are processing */
		*g_path,	/* optional prefix path for generated names */
		*g_ibuf,	/* buffer containing the inode-list inode */
		*g_hook,	/* hook--a question mark */
		*g_unknownerr,	/* string representing an unknown error */
		*g_unresolved;	/* message indicating unresolved string */

static char *g_buf = NULL;/* [BLKSIZE];	/* our general purpose buffer */

static int criteria;		/* count of user selection criteria */
static int badnames;		/* incremented for unresolved path names */

static long	prtopt,		/* flags - print options */
		runopt,		/* flags - run time options */
		as,		/* sign of number for -a option */
		ms,		/* "        "      "  -m option */
		cs;

static long	atime,		/* abs(number) of days from -a option */
		mtime,		/* "	     "           "  -m option */
		crtime,		/* "         "           "  -c option */
		ntime,		/* "         "           "  -n option */
		today;		/* today as returned by time() */
static char *astr, *mstr, *crstr, *nstr, *device;
static char abuf[255], cbuf[255], mbuf[255], nbuf[255], pbuf[255], dbuf[255];

static struct arg Args[] =
{
	"runopt=",	(caddr_t)  &runopt,	INT_T,
	"prtopt=",	(caddr_t)  &prtopt,	INT_T,
	"atime=",	(caddr_t)  abuf,	STR_T,
	"crtime=",	(caddr_t)  cbuf,	STR_T,
	"mtime=",	(caddr_t)  mbuf,	STR_T,
	"ntime=",	(caddr_t)  nbuf,	STR_T,
	"inodes=",	(caddr_t)  &Nlist,	INTV_T,
	"prefix=",	(caddr_t)  pbuf,	STR_T,
	"device=",	(caddr_t)  dbuf,	STR_T,
	NILPTR (char),	NIL (caddr_t),		NIL_T,
};

/*
 * funcion declarations
 */
static char *storestring();  /* store a string in dynamicly allocated space */
static process(), super(), ilist(), saveinod(), savedir(), adddirb(), selecti(), scomp();
static readdirs(), xdname(), monikers(), format(), name(), seer(), ncheck(), blkcomp();
static exceed(), error(), errorx();
static char *uidtonam(), *getbuf();

/*
 * Entry point function.  Parse args and dispatch to process() to handle
 * the requested filesystem.
 */

/*
** op_namei
**
** fast, by-inode find.  this function should be the only function not
** declared static.  all op_namei functions and globals should be declared
** static to avoid confusion with the other op_'s.
**
** returns FSHERR_...
**
**  derived from sts/dcm code; thanks.
*/
int
op_namei(devfd, opflags)	/* main */
int devfd;
char *opflags;
{
    register int    rc;
    int xcode;
    char xcode_buf[5];	/* must be big enuf to hold all of the EXIT_ codes */

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_OP_NAMEI, NL_CAT_LOCALE);

    astr = (char *) strcpy(abuf,""); mstr = (char *) strcpy(mbuf,"");
    crstr = (char *) strcpy(cbuf,""); g_path = (char *) strcpy(pbuf,"");
    device = (char *) strcpy(dbuf,""); nstr = (char *) strcpy(nbuf,"");
    if ((rc = get_args (Args, opflags)) != FSHERR_GOOD)
	RETURN ("op_namei/get_args", rc);


    if (runopt&F_DEBUG) {
	PR_INT("runopt",runopt);
	PR_INT("prtopt",prtopt); fprintf(stderr,"\n");
	PR_STR("atime",astr);
	PR_STR("ctime",crstr);
	PR_STR("mtime",mstr);
	PR_STR("ntime",nstr); fprintf(stderr,"\n");
	PR_INTV("inodes",Nlist); fprintf(stderr,"\n");
	PR_STR("prefix",g_path);
	PR_STR("device",device); fprintf(stderr,"\n");
	fflush(stderr);
    }

    g_hook = MSGSTR(HOOK,"?");
    g_unresolved = MSGSTR(UNRESOLVED,"??unresolved??");
    g_unknownerr = MSGSTR(UNKNOWNERR,"??errno");
    g_cmd = "ff";
    g_dev = "";
    badnames = 0;
    criteria = 0;
    time(&today);
    if (astr[0]) {
	as = *astr;
	if ((atime = atoi(astr)) == 0)
	    errorx(MSGSTR (BADVALUE,"bad value %s for option %c\n"),astr,'a');

	++criteria;
    }
    if (mstr[0]) {
	ms = *mstr;
	if ((mtime = atoi(mstr)) == 0)
	    errorx(MSGSTR (BADVALUE,"bad value %s for option %c\n"),mstr,'m');

	++criteria;
    }
    if (crstr[0]) {
	cs = *crstr;
	if ((crtime = atoi(crstr)) == 0)
	    errorx(MSGSTR (BADVALUE,"bad value %s for option %c\n"),crstr,'c');

	++criteria;
    }
    if (nstr[0]) {
	if (stat(nstr,&statb) < 0)
	    errorx(MSGSTR (STATFAIL,"n option, %s stat failed\n"),nstr);

	++criteria;
	ntime = statb.st_mtime;
    }
    if (runopt&F_REQUEST) ++criteria;
    if (runopt&F_DEBUG) fprintf(stderr,"---- criteria=%d\n",criteria);

    if ((xcode=setjmp(err_out))==0) {
	/* fprintf(stderr,"calling process (%s)\n",device); -DBG */
	process(device);
	if (badnames)
	    error(MSGSTR (PARTRESOLVE,"NOTE! %d pathnames only partially resolved\n"), badnames);

	if (runopt&F_DEBUG) {
	   error(MSGSTR(STATS,"nLink=%d nIreq=%d nDb=%d\n"),maxL,maxIreq,maxDb);

	   error(MSGSTR(STATS,"nLink=%d nIreq=%d nDb=%d\n"),nLink,nIreq,nDb);
	}
    }
    sprintf(xcode_buf, "%d", 0);
    write(PipeFd, xcode_buf, strlen(xcode_buf) + 1); /* include NULL ... */

    return (FSHERR_GOOD);
}

/*
 * process:  dispatch to ilist to read inode list.  slice, dice, julienne,
 *	   output data.
 */
static
process(stanza_name)
char *stanza_name;
{
	int     blkcomp();

	/* at this point we assume that stanza_name is a valid
	 * filesystem name
	 */
	g_dev = stanza_name;

	if (stat(g_dev,&statb) < 0)
		errorx(MSGSTR (NOINPUT,"Input %s is non-existent\n"),g_dev);


	if ((((statb.st_mode&IFMT)==IFBLK)|((statb.st_mode&IFMT)==IFCHR))==0)
		errorx(MSGSTR (NOTSPECIAL,"Input device %s is not a special file\n"), g_dev);


	if ((g_devfd=open(g_dev,O_RDONLY)) < 0)
		errorx(MSGSTR (OPENFAILED,"Open failed for input on %s\n"), g_dev);


	if (super())
		return(0);

	ilist();
	if (nIreq == 0) {
		error(MSGSTR (NONESELECTED,"No files were selected.\n"));

		return(0);
	}

	qsort((char *)Dblist, (int) nDb, sizeof(struct Db), blkcomp);
	readdirs();

	error(MSGSTR (NUMSELECTED,"%d files selected\n"), nIreq);


	if (runopt&F_LINKSTOO)
		error(MSGSTR (NUMLINKS,"%d link names detected\n"), nLink);


#ifdef TEST
	if (runopt&F_DEBUG) debug_printout();
#endif /* TEST */
	monikers();

	return(1);

}

#ifdef TEST
static
debug_printout()
{
    int i;
	/* print Ddlist,  Linklist,  Ireqlist, and Dblist */
    fprintf(stderr,"-----------------------------------------------------------------------------\n");
    fprintf(stderr,"Ddlist:  directory array (%d elements).\n",
	g_ilb * g_inopb+1);
    for (i = 0; i < g_ilb * g_inopb+1; i++) {
	if (Ddlist[i].inum != 0) {
	    fprintf(stderr,"el: %4d  inum = %4d  pinum = %4d  dname = %s  ",
		i, Ddlist[i].inum, Ddlist[i].pinum, Ddlist[i].dname);
	    if (Ddlist[i].inum != i)
		fprintf(stderr,"INUM MISMATCH!\n");
	    else
		fprintf(stderr,"\n");
	}
    }

    fprintf(stderr,"Linklist:  array of links (%d elements).\n", nLink);
    for (i = 0; i < nLink; i++) {
	fprintf(stderr,"el: %4d  inum = %4d  pinum = %4d  dname = %s  ",
	    i, Linklist[i].inum, Linklist[i].pinum, Linklist[i].dname);
    }

    fprintf(stderr,"Ireq: holds inumbers and data for files (%d elements).\n",
	nIreq);
    for (i = 0; i < nIreq; i++) {
	fprintf(stderr,"el: %4d  inum = %4d  size = %4d  uid = %4d \n",
		i, Ireqlist[i].inum, Ireqlist[i].size, Ireqlist[i].uid);
    }

     fprintf(stderr,"Db: data on blocks which are directories (%d elements).\n",
	nDb);
    for ( i = 0; i < nDb; i++) {
	fprintf(stderr,"el: %4d  inum: %4d  size: %4d  blkno: %4d\n",
		i, Dblist[i].inum, Dblist[i].size, Dblist[i].blkno);
    }
    fprintf(stderr,"-----------------------------------------------------------------------------\n");
}
#endif /* TEST */

static int super()
{
    char *getbuf();
    int numblks, retry;
    long have, want, ulimit();
    struct superblock fs;
    struct dinode di;
    frag_t pb;
    struct vmdmap *vmd;
    frag_t fmin, fmax;
    long nag, rem;
    int	rc;

    if (runopt&F_DEBUG)
	    error(MSGSTR (READSUPER,"reading superblock.\n"));

    if ((rc = get_super(g_devfd, &fs)) < 0)
    {
	    switch(rc)
	    {
	    case LIBFS_BADMAGIC:
		    errorx(MSGSTR(UNKNOWNSUPER, "Unknown filesystem type.\n"));
		    break;
	    case LIBFS_BADVERSION:
		    errorx(MSGSTR(BADVERS,
				  "JFS filesystem, invalid version number.\n"));
		    break;
	    case LIBFS_CORRUPTSUPER:
		    errorx(MSGSTR(CORRUPTSUPER, "Corrupt superblock.\n"));
		    break;
	    default:
		    errorx(MSGSTR(CANTREAD, "Cannot read superblock.\n"));
		    break;
	    }
	    /*
	     *  shouldn't ever get here, but old code had it...
	     *  so i'll have it too.
	     */
	    return 1;
    }

    /*
     * setup global buffer
     */
    g_bsize = BLKSIZE;
    g_buf = getbuf(ALLOC, (char *)NULL);


    /*
     * fsmax returns first illegal inode num and first illegal frag num
     * subtract 1 from inum to get the max legal inode number
     */
    fsmax(&imax, &fmax);
    numblks = FRAG2BLK(fmax.addr);
    imax--;

    /*
     * get the inode map and figure out how many inodes are
     * currently in use.
     */
    if (rc = get_inode(g_devfd, &di, INOMAP_I))
    {
	    fprintf(stderr, MSGSTR(CANTGI,
				   "Cannot get inode for inode map\n"));
	    return rc;
    }
    if (rc = ltop(g_devfd, &pb, &di, 0))
	    return rc;

    g_ibuf = getbuf(ALLOC, (char *)NULL);
    seer(pb, g_ibuf);
    vmd = (struct vmdmap *)g_ibuf;
    g_inodes = vmd->mapsize - vmd->freecnt + 10;   /* give it a little slack */
    g_ilb = INO2BLK(g_inodes + INOPERBLK - 1);
    g_inopb = INOPERBLK;

    if (runopt&F_DEBUG)
	error(MSGSTR(SUPERSTATS,"numblks:%d  bsize:%d inopb:%d\n"),
		numblks, g_bsize, g_inopb);

    /*
     * do some error checking
     */
    if (g_ilb <= 0 || g_ilb > numblks)
    {
	    errorx(MSGSTR (BADSUPER,"Invalid superblock\n"));
	    return 1;
    }

    maxIreq = g_inodes; 	/* % of all inodes which are used */
    /*
     *  I guess these are just heuristics.  maxDb probably too big...
     */
    maxL = 1+g_inodes/2;		/* % of all files which are links */
    maxDb = FRAG2BLK(fmax.addr) / 2;	/* num blks owned by dirs */

    if (runopt&F_DEBUG) {
	fprintf(stderr,"g_ilb=%d  g_inopb=%d  maxIreq=%d  maxL=%d  maxDb=%d\n",
		g_ilb, g_inopb, maxIreq, maxL, maxDb);
    }

    retry = 0;
again:	  /* GOTO here if not enough mem */
    have = ulimit(3,0) - (long)sbrk(0) - 16000;       /* get memory available */
    /* fprintf(stderr,"ulimit(3,0) = %d  sbrk(0) = %d  have = %d \n",
	ulimit(3,0), sbrk(0), have); */
    /* get an idea about how much memory we'll need */
    want = maxIreq*sizeof(struct Ireq) + maxL*sizeof(struct Dd) +
	   maxDb*sizeof(struct Db) +
	   (1+g_inodes)*sizeof(struct Dd);
    if (runopt&F_DEBUG) {
	error(MSGSTR (HAVEWANT,"memory: have: %u  want: %d\n"), have, want);

	error(MSGSTR (MEMSTAT1,
	    "     Dd:   %5d (%5d bytes)   Link: %5d (%5d bytes) \n"),
	    g_inodes+1,(g_inodes+1)*sizeof(struct Dd),
	    maxL, maxL*sizeof(struct Dd));

	error(MSGSTR (MEMSTAT2,
	    "     Ireq: %5d (%5d bytes)   Db:   %5d (%5d bytes) \n"),
	    maxIreq, maxIreq*sizeof(struct Ireq),
	    maxDb, maxDb*sizeof(struct Db));
    }
    if (have < want) {
	if (retry++ > 4)
	    errorx(MSGSTR (NOMEMGIVEUP,"Insufficient memory. Giving up\n"));


	/* reduce demands by 10% */
	maxL = (long)((float)maxL*0.90);
	maxDb = (long)((float)maxDb*0.90);
	maxIreq = (long)((float)maxIreq*0.90);
	/* re-calculate */
	goto again;
    }
    if (retry)
	error(MSGSTR (LESSMEMMAYFAIL,"Reducing memory demands. May fail.\n"));


    if ((Ddlist=(struct Dd *)calloc(imax+1,sizeof(struct Dd))) == 0)
	errorx(MSGSTR (NODDMEM,"insufficient memory for Dd area\n"));


    if ((Linklist=(struct Dd *)calloc(maxL,sizeof(struct Dd))) == 0)
	errorx(MSGSTR (NOLINKMEM,"insufficient memory for Link area\n"));


    if ((pIreq=Ireqlist=(struct Ireq *)malloc((unsigned)(maxIreq*sizeof(struct Ireq)))) == 0)
	errorx(MSGSTR (NOIREQMEM,"insufficient memory for Ireq area\n"));


    if ((pDb=Dblist=(struct Db *)malloc((unsigned)(maxDb*sizeof(struct Db)))) == 0)
	errorx(MSGSTR (NODBMEM,"insufficient memory for Db area\n"));


    nIreq = nDb = nLink = 0;

    return(0);
}


static int
ilist()
{
    char	*getbuf();
    ino_t	ino;
    struct      dinode  di;
    int		rc;

    for (ino=0; ino <= imax; ino++)
    {
	    if (rc = get_inode(g_devfd, &di, ino))
		    return rc;
	    if (ino == 0 || ino == INODES_I || di.di_nlink == 0)
		    continue;
	    if (runopt&F_DEBUG )
		    error(MSGSTR (READINFO,"i: %3d  sz: %5d  ln: %2d\n"),
			  ino, di.di_size, di.di_nlink);
	    if ((di.di_mode & IFMT ) == IFDIR)
		    savedir(ino, &di);
	    if (selecti(ino, &di))
		    saveinod(ino, &di);
    }
    return 0;
}

static saveinod(inum,dip)
int	inum;
struct	dinode	*dip;
{

	if (runopt&F_DEBUG) 
		error(MSGSTR (SAVEINOD,"saveinod: nireq = %d\n"), nIreq);

	if (nIreq >= maxIreq)
	    exceed(maxIreq,MSGSTR(SELECTEXCEED,"Ireq - selected file list"));
	nIreq++;

	(*pIreq).inum = inum;
	if (dip != 0) {
		(*pIreq).size = dip->di_size;
		(*pIreq).uid = dip->di_uid;
	}
	pIreq++;

	return;

}

static savedir(inum,dip)
ino_t   inum;
struct  dinode  *dip;
{
    int i, size;
    int tsize, fragsize;
    frag_t bnum;

#define min(a,b)	(a < b ? a : b)
    /* check to make sure that inode is a directory */
    if (dip->di_mode == 0 || !((dip->di_mode&IFMT )==IFDIR ))
	return;

    size = NUMDADDRS(*dip);	/* num disk addresses in inode */
    tsize = dip->di_size;       /* size of directory */
    for (i=0; i < size; i++) {  /* for each block in file... */
	ltop(g_devfd, &bnum, dip, i);	      /* map to phys block # */
	fragsize = FRAGLEN(bnum);
	adddirb(inum, bnum, min(fragsize, tsize));     /* add to directory block list */
	tsize -= fragsize;
    }
    return;
#undef min
}

static adddirb(ino,blk,sz)
ino_t	ino;
frag_t	blk;
off_t	sz;

{
	if (runopt&F_DEBUG)
	    error(MSGSTR (ADDDIRB,"adddirb: nDb = %d\n"), nDb);

	if (nDb >= maxDb)
	    exceed(maxDb,MSGSTR(DIRBLKEXCEED,"Db - directory data block list"));
	nDb++;
	pDb->inum = ino;
	pDb->blkno = blk;
	pDb->size = sz;
	pDb++;
	return;

}

static addlink(i, p, d)
register ino_t i,p;
register char *d;
{

	if (nLink >= maxL)
	    exceed(maxL, MSGSTR(LINKEXCEED,"Link - linked file list"));
	Linklist[nLink].inum = i;
	Linklist[nLink].pinum = p;
	Linklist[nLink].dname = storestring(d,strlen(d));
	nLink++;

}

static selecti(curi,dip)
ino_t	curi;
struct dinode *dip;
{

    register int ok = 0;

    if (runopt&F_IGSPECIAL) {	/* if the user wants to ignore */
	switch(curi) {		/* the special . files in the */
	    case SUPER_I:		/* root of a filesystem. */
/*
 * ROOTDIR_I is commented out so that ff prints an entry for .
 * (this became uncommented in the initial frags drop, recomment it
 *  to preserve v3 behavior)
 *	    case ROOTDIR_I:		/*these files may not have directory */
	    case INODES_I:		/* entries so sometimes we want to */
	    case INDIR_I:		/* ignore them. */
	    case INOMAP_I:
	    case DISKMAP_I:
	    case INODEX_I:
	    case INODEXMAP_I:
		/* The following are pre-reserved inodes */
	    case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		return(0);
	    default:
		break;
	}
    }
    if (criteria == 0)
	return(1);

    if (atime)
	ok+=(scomp((long)((today-dip->di_atime)/A_DAY),atime,as))?1:0;
    if (mtime)
	ok+=(scomp((long)((today-dip->di_mtime)/A_DAY),mtime,ms))?1:0;
    if (crtime)
	ok+=(scomp((long)((today-dip->di_ctime)/A_DAY),crtime,cs))?1:0;

    if (ntime)
	ok += (dip->di_mtime > ntime) ? 1 : 0;

    if (runopt&F_REQUEST)
	ok += (ncheck(curi) ? 1 : 0);

    return(ok == criteria);
}

/*
 * Signed COMPare: for comparing relative dates.
 */
static scomp(a,b,s)
register long a, b;
register char s;
{
    if (s == '+')
	return(a > b);
    if (s == '-')
	return(a < (b * -1));
    return(a == b);
}

static readdirs()
{
    int i, j;
    struct      direct  *dirp;
    int cnt, tot;


    pDb = Dblist;
    for (i=0; i<nDb; i++, pDb++) {      /* for each block in dir.  blk list */
	seer(pDb->blkno, g_buf);	/* read block from disk */
	dirp = (struct direct *)g_buf;
	tot = 0;			/* init counters */
	while (tot < FRAGLEN(pDb->blkno))
	{       /* while tot < disk block size */
	    cnt = 0;		    /* init counter */
	    while (cnt < DIRBLKSIZ ) {	/* while cnt < directory block size */
		/* if (runopt&F_DEBUG)				--- DEBUG
		    error(MSGSTR (XDNAME,"calling xdname: inum = %d,  name = `%s', len = %d\n"),dirp->d_ino,dirp->d_name,dirp->d_reclen); */

		j = dirp->d_reclen; /* len of dir entry */
		if (j == 0) break;
		xdname(dirp,pDb->inum); /* put dir entry in list */
		dirp = (struct direct *)((char *)dirp+j); /* next (+j bytes) */
		cnt += j; tot += j;     /* adjust totals */
		if (runopt&F_DEBUG )
		    error(MSGSTR (READDIRS,"readdirs(): blk = %d, j = %d, cnt = %d, tot = %d\n"),i,j,cnt,tot);

	    }
	    if (j == 0)
		break;
	}
    }
    return;
}

static xdname(dirp,dotinum)
register struct direct  *dirp;
register ino_t  dotinum;
{

    register ino_t  dino = dirp->d_ino;

    if (runopt&F_DEBUG) {
	if (ISDOTS(dirp->d_name)) error(MSGSTR (NAMEISDOTS,"xdname: `%s' is dots.\n"),dirp->d_name);

	if (dino==0) error(MSGSTR (INODEISZERO,"xdname: inode of zero.\n"));

    }
    if (dino==0 || ISDOTS(dirp->d_name)) /* ISDOTS() in fsdefs.h */
	return;
    if (dino > imax)
	errorx(MSGSTR (BADINONUM,"Improbable inumber %d. Check filesystem.\n"), dino);


    if (Ddlist[dino].inum == 0) {
	if (runopt&F_DEBUG)
	    error(MSGSTR (XDNAMEADDING,"xdname: adding: inode %d, name = %s, parent %d\n"), dino, dirp->d_name, dotinum);

	Ddlist[dino].inum = dino;
	Ddlist[dino].pinum = dotinum;
	Ddlist[dino].dname = storestring(dirp->d_name,(dirp->d_namlen));
    }
    else
	addlink(dino,dotinum,dirp->d_name);

    return;

}


static monikers()
{

    register int i;
    char Pathname[1024];

    pIreq = Ireqlist;

    for (i=0; i<nIreq; i++, pIreq++) {
	name(pIreq->inum, Pathname, 0);
	format(Pathname, pIreq->inum, pIreq);
    }

    if (runopt&F_LINKSTOO)
	for (i=0; i<nLink; i++) {
	    name(Linklist[i].pinum, Pathname, Linklist[i].dname);
	    format(Pathname,Linklist[i].inum,0);
	}

    return;

}


static format(Path,inum,pi)
register	char *Path;
register	ino_t	inum;
register	struct Ireq *pi;
{
    char	*uidtonam(), *nam;

    printf(Path);
    if (!prtopt)
	printf("\n");
    else {
	if (prtopt&F_INODE)
	    printf("\t%d",inum);
	if (prtopt&F_SIZE)
	    if ( pi ) printf("\t%d",pi->size); else printf("0");
	if (prtopt&F_UID) {
	    if (pi) {
		if (*(nam=uidtonam(pi->uid)) == '?')
		    printf("\t%d",pi->uid);
		else
		    printf("\t%8s",nam);
	    } else printf("0");
	}
	printf("\n");
    }
}

static name(curi,Path,link)
register ino_t  curi;
register char   *Path;
char    *link;
{
    struct P { char *p; } P[50];
    int n = 0;

    if (prtopt&F_PREPATH)
	strcpy(Path, g_path);
    else
	strcpy(Path, ".");

    if (link)
	P[n++].p = link;

    for (; curi != 2; curi=Ddlist[curi].pinum) {
	if (runopt&F_DEBUG)
	    error(MSGSTR(INODESTATS,"current inode = %d, name = `%s', parent = `%d'\n"),
		  curi,Ddlist[curi].dname,Ddlist[curi].pinum);

	if (n > 50)
	    exceed(50,MSGSTR(PATHEXCEED,"path components"));
	if (Ddlist[curi].inum == 0) {
	    P[n++].p = g_unresolved;	/* MSG */
	    badnames++;
	    break;
	}
	P[n++].p = Ddlist[curi].dname;
    }
    while(n > 0) {
	strcat(Path,"/");
	strcat(Path,P[--n].p);
/*	strncat(Path,P[--n].p,14 ); */
    }
    return;
}

/*
 * convert uid to login name; interface to getpwuid that keeps up to USIZE1
 * names to avoid unnecessary accesses to passwd file
 * returns ptr to NSZ-byte name (not necessarily null-terminated)
 * returns pointer to "?" if we cannot resolve uid
 * lifted gleefully from the Unix accounting package. "Software Tools" indeed.
 */

static char * uidtonam(uid)
uid_t	uid;
{
    register struct ulist *up;
    register struct passwd *pp;

    for (up = ul; up < &ul[usize1]; up++)
	if (uid == up->uuid)
	    return(up->uname);
    setpwent();
    if ((pp = getpwuid(uid)) == NULL)
	return(g_hook);
    else {
	if (usize1 < USIZE1) {
	    up->uuid = uid;
	    strncpy(up->uname, pp->pw_name, sizeof(up->uname));
	    usize1++;
	}
	return(pp->pw_name);
    }
}

static seer(frag_t frag,
	    char *buf)
{
    int rv;

    if ((rv = bread(g_devfd, buf, frag)) != FRAGLEN(frag))
    {
	    if (rv == LIBFS_SEEKFAIL)
		    errorx(MSGSTR(SEEKFAIL,
				  "Seek failed for block %x,err=%d\n"),
			   frag, errno);
	    else
		    errorx(MSGSTR(READFAIL,
				  "Read failed for block %d,err=%d\n"),
			   frag.addr, errno);
    }
}

static int ncheck(curi)
register ino_t curi;
{
    int i;

    for (i=0; i < Nlist->iv_stride ; i++ )
	if (((ino_t)Nlist->iv_int[i]) == curi)
	    return(1);
    return(0);
}


/* blkcomp(): compare blocks by thier block numbers. */
static int blkcomp(a,b)
struct	Db	*a, *b;
{
    return( (*a).blkno.addr - (*b).blkno.addr);
}

/*
 * exceed, error, errorx:  error reporting routines
 */
static exceed(max, s)
long	max;
char	*s;
{
    errorx(MSGSTR(LIMITEXCEEDED,"program limit of %d exceeded for %s\n"),max,s);
}

static error(s,  e,z,  g,o,l,d,m,a,n)
{
    fprintf(stderr,"%s: %s: ", g_cmd, g_dev);
    fprintf(stderr, s,  e,z,  g,o,l,d,m,a,n);
}

static errorx(s,  e,z,  g,o,l,d,m,a,n)
{
    error(s,  e,z,  g,o,l,d,m,a,n);
    longjmp(err_out,1);
}

/*
 * storestring(s,l): an attempt to reduce the number of calls to malloc(3)
 * by allocating a big buffer (BLKSIZE bytes) and using it up as needed
 *
 */
static char *storestring(s,l)
char *s;		/* string to store */
int l;		  /* length of string */
{
    static int siz;
    static char *buffer = 0 ;
    char *p;

    if (runopt&F_MALLOC) {
	if (s == 0 ) errorx(MSGSTR (NULLTOSTORSTR,"NULL pointer passed to storestring\n"));

	if (buffer == 0  || l+siz+1 > g_bsize) {
	    error(MSGSTR (ALLOCNEWBUF,"storestring: allocate new buffer.\n"));
	    /* allocate a new buffer */
	    if ((buffer = (char *)malloc(g_bsize)) == NULL)
		    errorx(MSGSTR (MALLOCFAILS,
				   "storestring: malloc %d bytes fails.\n"),
			   g_bsize);
	    siz = 0;
	}
	p = (char *)strcpy(buffer,s); buffer+=(l+1); siz += (l+1);
	return (p);
    } else {
	p = (char *)malloc((unsigned)l+1);
	if (p == (char *)NULL)
	    errorx(MSGSTR (MALLOCFAILS,"storestring: malloc %d bytes fails.\n"),
		l+1);

	return ((char *)strcpy(p,s));
    }
}

static char *
getbuf(type,buf)
int	type;
char 	*buf;
{
	char	*p;

	switch(type) {

	case ALLOC :
		if((p=(char *)malloc(g_bsize)) == 0)
			errorx(MSGSTR(NOMEMORY,"out of memory\n"));
		return(p);

	case FREE :
		free((void *)buf);
		return (char *)NULL;
	}
}
