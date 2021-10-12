static char sccsid[] = "@(#)82	1.38.1.21  src/bos/sbin/helpers/v3fshelpers/op_check.c, cmdfs, bos41J, 9518A_all 4/28/95 10:17:43";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the filesystem
 *
 * FUNCTIONS: op_check, pr
 *
 * ORIGINS: 3, 26, 27
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
/*
 *	op_check
 *
 *	- Aix3 filesystem checker.
 */

static void read_super();
static void fbwrite();
static int fbread();

/*
 * turn on/off debug printfs
 */

/* #define DEBUG */
#ifdef DEBUG
#define	BDEBUG
#else
#undef	BDEBUG
#endif


/*
 * debugging
 */
#ifdef BDEBUG
# define	BPRINTF(args)	pr args
# define	BPASSERT(addr)	bpassert("addr count <= 0", addr, __LINE__)
#else /* BDEBUG */
# define	BPASSERT(addr)
# define	BPRINTF(args)
#endif /* BDEBUG */

#include <ctype.h>
#include <sys/types.h>
#include <locale.h>

#include <stdio.h>
#include <stdlib.h>
#include <fshelp.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <sys/vfs.h>
#include "fsop.h"
#include "fsck.h"
#include <pwd.h>
#include <varargs.h>
#include <jfs/filsys.h>
#include <sys/vmdisk.h>
#include <jfs/fsdefs.h>
#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include <jfs/ino.h>
#include <jfs/inode.h>

#include <time.h>
#include <string.h>

#include <errno.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/stat.h>
#include <IN/standard.h>
#include <libfs/libfs.h>
#include "bufpool.h"

#include <nl_types.h>
#include "op_check_msg.h"
#define MSGSTR(N,S)	catgets(catd,MS_OP_CHECK,N,S)
static nl_catd catd;

/*
 * somewhere there's a Debug macro that we don't need, that someone added
 * after our Debug global, so we get rid of it here
 */
#ifdef Debug
# undef Debug
#endif


/*
 *  prog to replay the version 3 log device
 */
#define	LOGREDO		"logredo"

/*
 * miscellaneous default limits and parameters
 */
#define LINSIZE 80      /* maximum length for user input line           */
#define DOTLEN LDIRSIZE(1)  /* length of dirent for .		        */
#define MAXDUP      10  /* limit on reported dup blks (per inode)       */
#define MAXINV      10  /* limit on reported inv blks (per inode)       */
#define MAXMIDFRAG  10  /* limit on reported frags in middle (per inode)*/
			/* (a frag in middle means that a file has a    */
			/* fragment somewhere other than the last disk  */
			/* address of a file)				*/
#define MAXCORRUPT 10	/* limit on reported corrupt frags (per inode)	*/
#define MAXBADALLOC 10	/* limit on reported map dword frags (per inode)*/
			/*   frags that span disk map dword boundries	*/
#define DUPTBLSIZE 800  /* maximum number of dup blks to remember       */
#define MAXLNCNT    50  /* maximum number of unref files to remember    */
#define MAXBUFS     25  /* maximum blocks in scratch file cache         */
#define MINBUFS      3  /* minimun possible number of blocks in cache   */
#define MAXIBUFS    10  /* maximum blocks in raw I-node buffer (pass 1) */
#define MAXPATHN   PATH_MAX  /* longest allowable pathname              */
#define	MAXSBLKS    10	/* maximum number of blocks to search for	*/
#define	MAXSINOS    20	/* maximum number of I-nodes to search for	*/

#define EMPTDIR	 DIRBLKSIZ	/* size of an otherwise empty directory	*/

#define MAXCYL  500     /* maximum blocks/cylinder (for interleave)     */
#define STEPSIZE  1     /* default interleave factor                    */

/*
 * YES and NO are the acceptable answers to y/n questions
 */
#define NO	0
#define YES	1
#define LF_FAIL 2	/* linkup() returns this when user requests  
			 * to put something in lost+found and we fail
			 */

/*
 * Almost all output to the console is performed via the routine "pr".
 * The first argument to "pr" is a flag word describing the message
 * to be printed and appropriate actions to take.  The following constants
 * are for use in that flag word.
 */
#define ERROR   0100    /* If answered no, set xcode to EXIT_FAIL       */
#define NOB     0200    /* NO Beginning of line leader before message   */
#define NOE     0400    /* NO End of line should be printed after msg   */
#define PINODE  01000   /* Print out info on the "current" I-node       */
#define QUIET   02000   /* no-op if in quiet (preen) mode               */
#define TRIVIA  04000   /* this message is normally suppressed, -v only */
#define DDEBUG  010000   /* this message is normally suppressed, -D only */
#define PINUM	020000	/* print out inum for "current" I-node		*/  
#define CONTIN  (ASKX|FATAL|YNX)/* ask YES/NO and die if no             */
#define FATAL   (ERROR|FATALX)  /* abort check if question answered no  */
#define GETLINE (ARGX|ASKX)     /* ask general question, return answer  */
#define ASK     (ARGX|ASKX|YNX) /* ask YES/NO question, return answer   */
#define PREEN   (ASK|PREENX)    /* confirm making an obvious fix        */
#define ASKN    (ASK|ERROR)     /* question is an important one         */
#define PREENN  (ASKN|PREENX)   /* confirm making an important fix      */

/* these bits are only used within the internals of pr                  */
#define ARGX    1       /* argument (question or buffer) provided       */
#define ASKX    2       /* Message asks a question of the operator      */
#define FATALX  4       /* If answered no, abort program immediately    */
#define PREENX  010     /* This question defaults to yes in PREEN mode  */
#define YNX     020     /* Question requires YES or NO answer           */
#define CANCEL_YESPREEN 040 	/* if yflag or preen mode on, return no */

/*
 * flags and parameters to the program
 */
static char    fflag;		/* fast (and gullible) check            */
static char    nflag;		/* assume a no response                 */
static char    tflag;		/* scratch file specified               */
static char    vflag;		/* verbose - print lots of information  */
static char    yflag;		/* assume a yes response                */
static char    cflag;		/* DEBUG force use of scratch file      */
static char    preen;		/* automatic, fix normal problems       */

/*
 * miscellaneous globals
 */
static jmp_buf errjmp;		/* longjump buffer for error resets     */
static int     groupnum;	/* number of the group we are checking  */
static int     xcode;		/* exit code to return from fsck        */
static uint    hint;		/* map page to start allocation		*/

static int     cylsize;		/* num blocks per cylinder              */
static int     stepsize;	/* num blocks for spacing purposes      */
static int     cachesize = MAXBUFS;		/* */

static int numsinos;		/* number of files to search for        */
static struct			/* array of files to search for         */
{       ino_t   inum;           /* I-number of file being sought        */
	fdaddr_t frag;          /* particular block of interest         */
} sinos[MAXSINOS];

static const char ADJUST_str[] = "ADJUST";
static const char ADJUSTED_str[] = "ADJUSTED";
static const char ALDIR_str[] = "allocdir: cannot get inode %ld";
static const char ALLOCATED_str[] = "        --> %s Map bit %d allocated in map";
static const char ALLOCINO_str[] = "allocino: cannot get inode %ld";
static const char BADBLK_str[] = "Block %ld: %s Inode=%u fragment=%x";
static const char BADENT_str[] = "Directory %s, '%s' entry %s";
static const char BADFMAP_str[] = "Bad %s Map";
static const char BADVERSION_str[] = "%%_OS_%% filesystem, Incompatible version number.";
static const char BLK1MAP_str[] = "Cannot get first block of %s Map";
static const char BLKCLB_str[] = "Block number should be on a cluster boundary";
static const char BLKCNTW_str[] = "Block count wrong, Inode=%u";
static const char BLKMISS_str[] = "%ld blocks missing";
static const char BLKNOT_str[] = "Block size not %d";
static const char BLOCK_str[] = "Block";
static const char CACHEP_str[] = "--- Cache Performance %ld/%ld with %d buffers";
static const char CANTEXP_str[] = "Cannot expand %s: filesystem full";
static const char CANTEXPDIR_str[] = "Cannot expand %s.";
static const char CANTGET_str[] = "Cannot get %s directory";
static const char CANTOPEN_str[] = "cannot open s.%s.";
static const char CANTRESUP_str[] = "Unable to read superblock";
static const char CHECKING_str[] = "\n** Checking %s ";
static const char CLEAR1_str[] = "CLEAR";
static const char CLEAR1ED_str[] = "CLEARED";
static const char CLNMNT_str[] = "** %s cleanly - Check suppressed";
static const char CNTMAPB_str[] = "Cannot map in block %ld";
static const char CONTINUE_str[] = "CONTINUE";
static const char CONTINUED_str[] = "CONTINUED";
static const char CORRUPTFRAG_str[] = "corrupt disk address";
static const char CORRUPTSUPER_str[] = "Corrupt superblock.";
static const char COUNTC_str[] = " count %d should be %d";
static const char CREATEM_str[] = "CREATE";
static const char CREATEMD_str[] = "CREATED";
static const char DIRCONN_str[] = "Directory Inode=%u connected, parent was Inode=%u";
static const char DIRCOR_str[] = "DIRECTORY CORRUPTED";
static const char DIRLENM_str[] = "DIRECTORY %s: LENGTH %d NOT MULTIPLE OF %d";
static const char DIRSHRT_str[] = "DIRECTORY TOO SHORT";
static const char DIRWORD_str[] = "directory";
static const char DUPBAD_str[] = "Duplicate or invalid";
static const char DUPINRI_str[] = "Duplicate or invalid data in root inode";
static const char DUPLICATE_str[] = "duplicate";
static const char DUPOVER_str[] = "Duplicate table overflow.";
static const char EMBBAD_str[] = "Illegal character (\\0%o) embedded in pathname '%s'";
static const char EMBNULL_str[] = "Embedded nulls in directory entry for '%s'";
static const char ENTERSC_str[] = "Enter name of scratch file (%ld blks): ";
static const char EXCESD_str[] = "Excessive duplicate blocks Inode=%u";
static const char EXCESSBADALLOC_str[] = "Excessive illegally allocated fragments (Inode=%u)";
static const char EXCESSB_str[] = "Excessive invalid blocks Inode=%u";
static const char EXCESSCORRUPT_str[] = "Excessive corrupt disk addresses (Inode=%u)";
static const char EXCESSMIDFRAG_str[] = "Excessive partial blocks found before the last block (Inode=%u)";
static const char EXPAND_str[] = "EXPAND";
static const char EXPANDED_str[] = "EXPANDED";
static const char FATIO_str[] = "Fatal I/O error";
static const char FILENOFRAG_str[] = "Fragment allocated to file larger than 32k (Inode=%u)";
static const char FILE_str[] = "file";
static const char FIX_str[] = "FIX";
static const char FIXED_str[] = "FIXED";
static const char FOUNDB_str[] = "Found reference to file containing block %lu, name=%s";
static const char FOUNDF_str[] = "Found reference to inode %u, name=%s";
static const char FRAGBADALLOC_str[] = "illegally allocated fragment";
static const char FSIZEER_str[] = "File size error Inode=%u";
static const char FSYSBAD_str[] = "Filesystem integrity is not guaranteed";
static const char FSYSSTAT1_str[] = "--- Cluster: %6d\tInodes:  %6u";
static const char FSYSSTAT_str[] = "--- Filesystem size: %6ld\tInode list size:%6ld";
static const char FTOOLARG_str[] = "Filesystem too large to be checked in memory";
static const char IMAPER_str[] = "Cannot get inode for %s Map file";
static const char INCOMPMAP_str[] = "Incompatible map version number";
static const char INODE_str[] = "Inode";
static const char INVLD_str[] = "invalid";
static const char LNCNT_str[] = "link count %s";
static const char LNKOVR_str[] = "Link count table overflow";
static const char LOGREDO_FAIL_str[] = "logredo failed, fsck continuing";
static const char MAPBADAGSIZE_str[] = "	 --> Map page %d, agsize = %d";
static const char MAPBADCLSIZE_str[] = "	 --> Map page %d, clsize = %d";
static const char MAPBADVERS_str[] = "	 --> Map page %d, version = %d";
static const char MAPINC_str[] = "%s Map inconsistent: size = %ld, mapsize = %ld.";
static const char MIDFRAG_str[] = "partial block found before the last block";
static const char MISMATTREE_str[] = "Tree size has changed - cannot continue.";
static const char MISS_str[] = "is missing.";
static const char MNTCTLE_str[] = "mntctl failed: %s";
static const char MNTD_str[] = "Mounted";
static const char MNTFSYS2_str[] = "Checking a mounted filesystem does not produce dependable results.";
static const char MNTFSYS_str[] = "MOUNTED FILE SYSTEM; ";
static const char MODIFIED_str[] = "***** Filesystem was modified *****";
static const char MODMOUNT_str[] = "*** Mounted filesystem has been modified - REBOOT (NO SYNC!) ***";
static const char MTIMEM_str[] = "mode=%o\nsize=%ld mtime=%s ";
static const char NAME_str[] = "name";
static const char NODATA_str[] = "(CONTAINS NO DATA)";
static const char NODIRECT_str[] = "Cannot expand %s: no direct blocks left";
static const char NOLEFT_str[] = "NO SPACE LEFT IN %s";
static const char NOLFD_str[] = "There is no lost+found directory.";
static const char NOMEME_str[] = "Insufficient memory";
static const char NOMKDIR_str[] = "CANNOT CREATE %s DIRECTORY";
static const char NOSPACE_str[] = "out of space";
static const char NOTAIX3_str[] = "\nNot a recognized filesystem type.";
static const char OWNER_str[] = " owner=";
static const char PHASE0_str[] = "** Phase 0 - Check Log";
static const char PHASE1B_str[] = "** Phase 1b - Rescan For More Duplicate Blocks";
static const char PHASE1_str[] = "** Phase 1 - Check Blocks and Sizes";
static const char PHASE2_str[] = "** Phase 2 - Check Pathnames";
static const char PHASE3_str[] = "** Phase 3 - Check Connectivity";
static const char PHASE4_str[] = "** Phase 4 - Check Reference Counts";
static const char PHASEXX_str[] = "** Phase %d - Check %s Map";
static const char PHASEX_str[] = "** Phase %db - Salvage %s Map";
static const char PHYBLK_str[] = "Cannot read physical block %ld of %s Map";
static const char P_ALLOCATED_str[] = "        --> %s Map bit %d allocated in pmap, not in wmap";
static const char RANGEE_str[] = "Inode number out of range";
static const char REALLOCATE_str[] = "REALLOCATE";
static const char REALLOCATED_str[] = "REALLOCATED";
static const char RECON_str[] = "RECONNECT";
static const char RECONED_str[] = "RECONNECTED";
static const char REMOVE_str[] = "REMOVE";
static const char REMOVED_str[] = "REMOVED";
static const char RINOTDIR_str[] = "Root inode not directory";
static const char RIUNALLOC_str[] = "Root inode unallocated.";
static const char RWERR1_str[] = "Cannot fbwrite: block %d";
static const char RWERR2_str[] = "Cannot seek: block %d";
static const char RWERR3_str[] = "Cannot read: block %d";
static const char RWERR4_str[] = "Cannot write: block %d";
static const char RWERR9_str[] = "unknown disk transfer error: block %d";
static const char SALV_str[] = "SALVAGE";
static const char SALVAGED_str[] = "SALVAGED";
static const char SBLKDIR_str[] = "Superblock is marked dirty";
static const char SCREQ_str[] = "Filesystem too large, scratch file required";
static const char SCRWRE_str[] = "Write error on scratch file";
static const char SIZECHK_str[] = "Size check: fsize %ld";
static const char SIZEWRONG_str[] = "Size wrong, Inode=%u";
static const char SNOTDIR_str[] = "%s IS NOT A DIRECTORY";
static const char SUMMARY_str[] = "%d files %ld blocks %ld free";
static const char TBLINCORE_str[] = "--- Tables allocated in memory: %ld bytes";
static const char TBLPAGE_str[] = "--- Tables paged out to file: %s, %d buffers in cache";
static const char TBLSIZ_str[] = "--- Table sizes: BM=%ld + ST=%ld + LC=%ld, TOTAL=%ld";
static const char TERMIN_str[] = " (TERMINATED)";
static const char TOOMANYI_str[] = "Too many -i specifications";
static const char TOOMNYD_str[] = "Too many -d specifications";
static const char UNALLOCATED_str[] = "        --> %s Map bit %d un-allocated in map";
static const char UNALLOC_str[] = "Unallocated";
static const char UNKFLAG_str[] = "s: unrecognized flag\n";
static const char UNKFTYPE_str[] = "Unknown file type";
static const char UNLABELFS_str[] = "unlabeled filesystem";
static const char UNMNTD_str[] = "Unmounted";
static const char UNREFS_str[] = "Unreferenced %s ";
static const char UNREF_str[] = "Unreferenced";
static const char USETFLAG_str[] = "Insufficient memory, -t flag required";
static const char WRONG1_str[] = "refers to the wrong inode.";
static const char WRSUP_str[] = "WRITING SUPPRESSED; ";
static const char ZEROLDIR_str[] = "ZERO LENGTH DIRECTORY";

/*
 * files to stick into an empty directory...
 */
struct empty {
	char		*name;
	ino_t		 inum;
	} initfiles[] = { { "." }, { ".." }, { NULL }, };

static int numsblks;		/* number of blocks to search for       */
static daddr_t sblks[MAXSBLKS];	/* list of blocks to search for         */

static char *devname;	/* name of the device we are currently checking */

extern  char   *strerror();  /* messages associated with errors      */

/* maptable. the kth bit of dmaptab[x] is a one if the byte x
 * contains a  sequence of k consecutive zeros, k = 1,2,..8.
 * leftmost bit of char is bit 1.
 */
static unsigned char dmaptab[256];
#define ONES ((unsigned) 0xffffffff)
#define UZBIT ((unsigned) 0x80000000)
#define	min(x,y)	((x) < (y) ? (x) : (y))
static int sbagsize;
static int sbiagsize;

/*
** arguments passed in through opflags (and other globals)
*/

static bool_t	Preen = False;
static bool_t	No = False;
static bool_t	Yes = False;
static bool_t	Fast = False;
static bool_t	Mounted = False;
static bool_t	Mountable = False;
static bool_t	Mytype = False;
static bool_t	Readonly = False;
static bool_t	Verbose = False;
static char	Device[PATH_MAX];
static int	Inum = -1;
static int	Block = -1;
static int	Cache = -1;
static int	Debug = 0;
static bool_t	Logquiet = False;
static bool_t	Nologredo = False;
static char	Logflags[BUFSIZ];
static char sfilename[ PATH_MAX ];  /* scratch file name in this process    */
static int	compress = 0;
static int	Sbcompress = 0;

struct superblock *Super;		/* pointer to superblock */

void	ckfini();
void	rwerr();

int checkargs();
void check(), new2old();

/*
 *	read_super
 *
 *	- read superblock and validate the filesystem magic number
 */
static void
read_super(int devfd,
	   struct stat *statarea,
	   bool_t quiet)
{
	int 			rc;
	fdaddr_t		frag;
	struct superblock 	sb;


	if(fstat(devfd, statarea) < 0 )
		pr(FATAL, MSGSTR(CANTOPEN, CANTOPEN_str), devname,
			strerror(errno));

	/* get_super validates stuff in the superblock.  If get_super()
	 * fails and we are working in preen mode then attempt to read
	 * secondary superblock and perform a copy to the first superblock.
	 */
	if (((rc = get_super(devfd, &sb)) == LIBFS_SUCCESS) ||
	    (preen && (get_super2(devfd, &sb) == LIBFS_SUCCESS) && 
	     (sb.s_fmod = FM_MDIRTY) && 
	     (put_super(devfd, &sb) == LIBFS_SUCCESS)))
	{
	    sbagsize = sb.s_agsize;
	    sbiagsize = sb.s_iagsize;
	    Sbcompress = sb.s_compress;	
	    frag.d = BLK2FRAG(SUPER_B);
	    if ((Super = (struct superblock *)bpread(frag.f)) == NULL)
		    pr(FATAL, MSGSTR(CANTRESUP, CANTRESUP_str));
	}
	else
	    if (rc != LIBFS_BADMAGIC && rc != LIBFS_BADVERSION &&
		rc != LIBFS_CORRUPTSUPER)
	    {
		    pr(FATAL, MSGSTR(CANTRESUP, CANTRESUP_str));
	    }
	    else
	    {
		    if (quiet == True)
			    xcode = EXIT_FAIL;
		    else
			    switch (rc)
			    {
			    case LIBFS_BADMAGIC:
				    pr(FATAL, MSGSTR(NOTAIX3, NOTAIX3_str));
				    break;
			    case LIBFS_BADVERSION:
				    pr(FATAL, MSGSTR(BADVERSION, BADVERSION_str));
				    break;
			    case LIBFS_CORRUPTSUPER:
				    pr(FATAL, MSGSTR(CORRUPTSUPER, CORRUPTSUPER_str));
				    break;
			    }
	    }
}


/*
** op_check
**
** checks a v3 filesystem
**
** returns FSHERR_GOOD.  the actual exit code is written out to PipeFd
*/
int
op_check (devfd, opflags)
int      devfd;		/* file descriptor of device to check	*/
char    *opflags;	/* flags ("flag1, flag2")		*/
{
	register int    rc;
	struct stat statbuf;
	char xcode_buf[5]; /* must be big enuf to hold all EXIT_ codes */

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_OP_CHECK, NL_CAT_LOCALE);

	if (debug (FSHBUG_SUBDATA))
		fprintf(stderr,	"v3fshelper: opflags=\"%s\"\n", opflags);

	/*
	 * process option flags
	 */
	if (checkargs(opflags) < 0)
		RETURN ("op_make/get_args", FSHERR_SYNTAX);

	/*
	 * convert to 'old style' flags
	 */
	new2old();

	/*
	 * init buffer pool
	 */
	bpinit(devfd, rwerr);

	/*
	 * give pr() a place to jump on FATAL errors...
	 */
	if (setjmp(errjmp) == 0) {

		/*
		 * read the superblock (if Mytype = True, then get_super
		 * will be quiet if this is not a v3 filesystem)
		 */
		read_super(devfd, &statbuf, Mytype);

		/*
		 * just check if this filesystem is a v3 filesystem?
		 */
		if (Mytype == False)
			/*
			 * only see if the filesystem is clean
			 */
			if (Mountable == True)
				xcode = (Super->s_fmod == FM_CLEAN) ?
						EXIT_OK : EXIT_FAIL;

			else
				check(devfd, &statbuf);
		}

	ckfini();

	sprintf(xcode_buf, "%d", xcode);
	write(PipeFd, xcode_buf, strlen(xcode_buf) + 1); /* include NULL ... */

	return (FSHERR_GOOD);
}

/*
 *	checkargs
 *
 *	checkargs processes the options sent from the front end
 */
static int
checkargs(opflags)
char *opflags;
{
	extern int getarg();
	extern char *argarg;
	register int ind, return_code, c;

	/*
	 * indices into names table.  must match names table below
	 */
#define	IND_NOLOGREDO	0
#define	IND_PREEN	1
#define	IND_NO		2
#define	IND_YES		3
#define	IND_FAST	4
#define	IND_MOUNTED	5
#define	IND_MOUNTABLE	6
#define	IND_SCRATCH	7
#define	IND_DEVICE	8
#define	IND_INUM	9
#define	IND_BLOCK	10
#define	IND_CACHE	11
#define	IND_GROUP	12
#define	IND_READONLY	13
#define	IND_MYTYPE	14
#define	IND_VERBOSE	15
#define	IND_LOGQUIET	16
#define	IND_LOGFLAGS	17
	static char *names[] = {
		"nologredo", "preen", "no", "yes", "fast", "mounted",
		"mountable", "scratch", "device", "inum", "block", "cache",
		"group", "readonly", "mytype", "verbose", "logquiet",
		"logflags", NILPTR (char),
		};

/*
 *	macros to process arguments
 *
 *	SET_BOOL		- process a boolean (true, false) argument
 *	SET_STRING		- process a string argument
 *	SET_INT			- int argument
 *
 */
#define	SET_BOOL(bool)	bool = argarg == NULL || *argarg == 't' ? TRUE : FALSE
#define	SET_STRING(string)	strcpy(string, argarg == NULL ? "" : argarg)
#define	SET_INT(int)		int = argarg == NULL ? 0 : atoi(argarg)

	return_code = 0;

	/*
	 * getarg reads thru 'opflags' and returns the index of
	 * each flag it recognizes.
	 */
	while ((ind = getarg(opflags, names)) >= -1)
		switch (ind) {
			case IND_PREEN:
				SET_BOOL(Preen);
				break;
			case IND_NO:
				SET_BOOL(No);
				break;
			case IND_YES:
				SET_BOOL(Yes);
				break;
			case IND_FAST:
				SET_BOOL(Fast);
				break;
			case IND_MOUNTED:
				SET_BOOL(Mounted);
				break;
			case IND_MOUNTABLE:
				SET_BOOL(Mountable);
				break;
			case IND_SCRATCH:
				SET_STRING(sfilename);
				break;
			case IND_DEVICE:
				SET_STRING(Device);
				break;
			case IND_INUM:
				if (numsinos >= MAXSINOS) {
					pr(0, MSGSTR(TOOMANYI, TOOMANYI_str));
					return_code--;
					}
				else {
					sinos[numsinos].inum = atoi( argarg );
					sinos[numsinos++].frag.d = 0;
					}
				break;
			case IND_BLOCK:
				if (numsblks >= MAXSBLKS)
				{
					pr(0, MSGSTR(TOOMNYD, TOOMNYD_str));
					return_code--;
				}
				else
				{
					sblks[numsblks++] = atol( argarg );
					for( c = 0; c < numsblks - 1; c++ )
					{
						if (sblks[c] ==
						    sblks[numsblks - 1])
						{
							numsblks--;
							break;
						}
					}
				}
				break;
			case IND_CACHE:
				SET_INT(Cache);
				break;
			case IND_GROUP:
				SET_INT(groupnum);
				break;
			case IND_READONLY:
				SET_BOOL(Readonly);
				break;
			case IND_MYTYPE:
				SET_BOOL(Mytype);
				break;
			case IND_VERBOSE:
				SET_BOOL(Verbose);
				break;
			case IND_LOGQUIET:
				SET_BOOL(Logquiet);
				break;
			case IND_LOGFLAGS:
				SET_STRING(Logflags);
				break;
			case IND_NOLOGREDO:
				SET_BOOL(Nologredo);
				break;
			default:
				fprintf(stderr, MSGSTR(UNKFLAG, UNKFLAG_str),
					argarg == NULL ? "(null)" : argarg);
				return_code--;
				break;
			}

	return (return_code);
}

#ifdef DEBUG
#define COLS 6

int
dumpbuf(int *ptr, int off, int words)
{
	int i, bit;

	words += off;
	bit = off * 32;
	while (off < words)
	{
		printf("[%7.7d]   ", bit);
		for (i = 0; i < COLS && i < words; i++, off++, bit+=32)
			printf("%8.8x ", ptr[off]);
		printf("\n");
	}
}
#endif

/*
 *	new2old
 *
 *	- convert new variables to old (old fsck) variables ...
 */

/*
 * CAN_MODIFY is the condition under which we can modify the fs...
 */
#define	CAN_MODIFY	(Mounted == False || Readonly == True)

static void
new2old()
{
	fflag = Fast == True;
	nflag = (No == True) || !CAN_MODIFY;
	tflag = sfilename[0] != NULL;
	vflag = Verbose == True;
	Debug = debug(FSHBUG_SUBDATA);
	yflag = (Yes == True) && CAN_MODIFY;
	if (Cache < 0)
		cflag = 0;
	else {
		cflag = 1;
		cachesize = Cache;
		}
	preen = Preen == True;
	devname = Device;
}

/*
 * FSCK
 *
 *      This is the module of fsck that actually performs the integrity
 *      checks and repairs on a filesystem.  This is the part of fsck
 *      that understands the format of a filesystem, the types of errors
 *      that are likely to occur and how to correct them.
 *
 *      This module is entered through "op_check", and has access to most
 *      of the program parameters through globals which are shared between
 *      the two modules.
 *
 * SPECIAL NOTE:
 *
 *      Whenever we talk block numbers to the user we talk 512 byte blocks,
 *      independently of the block size in use.  This is to keep the
 *      documentation and interface independent of a kernel implementation
 *      decision.  This means that whenever a user specifies a block we must
 *      divide it by the cluster size and whenever we print out a block number
 *      we must multiply it by the cluster size.
 */

/*
 * As you might guess, fsck makes use of many tables and buffers to
 *    assess the integrity of a filesystem.  This structure is further
 *    complicated by the fact that most of the tables can be put out on
 *    a scratch file (if insufficient memory is available).  It is important
 *    that the purposes of each table and pass be understood before you
 *    try to modify this code.
 *
 * FILE STATE MAP       two bits per I-node, for each I-node on filesystem
 *
 *      During the first pass, the state map is built up, describing
 *      the type of every file as being unallocated, a file or a directory.
 *      During subsequent passes (as the directory structure is traversed)
 *      most directories will have their state changed to FILE.  When
 *      damaged files are encountered, the state is set to CLEAR.
 */
static char   *statemap;	/* address (in memory) of state map     */
static daddr_t smapblk;		/* starting blk (on disk) of state map  */

#define LSTATE  2               /* number of bits required for a state  */
#define SMASK   03              /* mask for a two bit field             */
#define STATEPB (BITSPRBYTE/LSTATE) /* number of states per table byte  */


#define USTATE  0               /* STATE: this I-node is not allocated  */
#define FSTATE  01              /* STATE: this I-node is a normal file  */
#define DSTATE  02              /* STATE: this I-node is a directory    */
#define CLEAR   03              /* STATE: this I-node should be cleared */

#define setstate(x) dostate(x,0)/* MACRO: set state of I-node inum      */
#define getstate()  dostate(0,1)/* MACRO: get state of I-node inum      */

/*
 * LINK COUNT TABLE     short per I-node for every I-node on filesystem
 *
 *      During pass 1, the link count on each allocated file is noted and
 *      stored in the link count table.  In pass 2, this count is decremented
 *      for each directory entry that refers to the file.  After this, any
 *      non-zero entry in the table is indicative of an error.
 */
static short   *lncntp;		/* starting addr (in memory) of table   */
static daddr_t lncntblk;	/* starting blk (on disk) of table      */

#define setlncnt(x) dolncnt(x,0)/* MACRO: set linkcnt for I-node inum   */
#define getlncnt()  dolncnt(0,1)/* MACRO: get linkcnt for I-node inum   */
#define declncnt()  dolncnt(0,2)/* MACRO: decrement cnt for I-node inum */

/*
 * If, during the first pass, a file is found to have a link count of zero,
 *      this is recorded in the badlncnt table.  These files should all be
 *      either reconnected or destroyed - but they shouldn't be left dangling.
 */
static ino_t	badlncnt[MAXLNCNT];	/* table of inos with zero link cnts */
static ino_t	*badlnp;		/* next entry in table */


/*
 * ALLOCATED AND FREE BLOCK MAPS
 *
 * During the first pass, a bit map is built up of all allocated blocks.
 *      Later another bit map of all free blocks is built up.  By comparing
 *      these two lists it is possible to find duplicate or missing blocks.
 *      By taking the compliment of the allocated block map, it is possible
 *      to generate a good free list.
 */
static uint    totmpages;	/* size of blkmap in 4k pages		*/
static char   *blkmap;		/* pointer to allocated block bit map   */
static long    bmapsz;		/* number of bytes in allocated map     */

#define SET_BITS	0
#define GET_BITS	1
#define CLR_BITS	2
#define setbmap(frag)	domap(frag, SET_BITS)
#define getbmap(frag)	domap(frag, GET_BITS)
#define clrbmap(frag)	domap(frag, CLR_BITS)

/*
 *  more constants for playing with the internal frag bitmap
 */
#define WORDPERPAGE		(BLKSIZE / sizeof(int))	/* 1024		*/
#define L2WORDPERPAGE		10			/* log2(1024)	*/
#define WORDPERPAGEMASK		(WORDPERPAGE - 1)  	/* 1023		*/

#define BITSPERBYTE		BITSPRBYTE
#define L2BITSPERBYTE		3			/* log2(8) */
#define BITSPERBYTEMASK		(BITSPERBYTE - 1)

#define BITSPERPAGE		(DBWORD << L2WORDPERPAGE)
#define L2BITSPERPAGE		(L2DBWORD + L2WORDPERPAGE)
#define BITSPERPAGEMASK		(BITSPERPAGE - 1)

#define BITSPERWORD		DBWORD
#define L2BITSPERWORD		L2DBWORD
#define BITSPERWORDMASK		(DBWORD - 1)

#define BITSPERDWORD		(BITSPERWORD << 1)
#define L2BITSPERDWORD		(L2BITSPERWORD + 1)
#define BITSPERDWORDMASK	(BITSPERDWORD - 1)

#define WORDPERDWORD	2

/*
 * DUP TABLE
 *      If duplicate blocks are found in the first pass, their numbers are
 *      recorded in the dup table.  For each duplication, the block number
 *      will appear in the dup table.  If a block appears 5 times (4 dups)
 *      there will be 4 entries in the dup table.  The dup table is divided
 *      into two subtables by the muldup pointer:
 *          All first instances of block numbers will occur before muldup.
 *          All repeat instances of block numbers will occur after muldup.
 *
 *      For merely flagging duplicates, it would be sufficient to record only
 *      one copy of each block number - but in order to know if a block is
 *      free yet, it is necessary to know how many references there are to it.
 */
static frag_t duplist[DUPTBLSIZE]; /* table of block numbers of dup blocks */
static frag_t *enddup;		/* first free entry in the dup table    */
static frag_t *muldup;		/* first duplicated entry in dup table  */

/*
 * globals pertaining to filsys
 */
static int Clsize = BLKSIZE/512;/* the cluster size for this system	*/
static int Bsize  = BLKSIZE;	/* block size for this system		*/
static int Bshift = BLKSHIFT;	/* log2 of Bsize			*/

/*
 * Information about the filesystem being checked
 */
static int a_ok;	/* filesystem checks out perfectly             */
static int cache_bufs;	/* number of buffers allocated to cache         */
static long cache_tries;/* number of calls to getblk                    */
static long cache_hits;	/* number of cache hits in getblk               */
static char *fslabel;	/* label on the filesystem we are checking     */
static daddr_t fmin;	/* lowest numbered data block in filesystem    */
static daddr_t fmax;	/* total number of blocks in filesystem        */
static ino_t imax;	/* total number of I-nodes in filesystem       */
static ino_t lastino;	/* number of the highest allocated I-node in fs */
static ino_t n_files;	/* number of allocated files on filesystem     */
static int n_free;	/* number of free blocks on this filesystem    */
static int n_blks;	/* number of allocated blocks on filesystem    */

/*
 * Information about the particular file being checked
 */
static ino_t curdir;	/* I-node number of the dir we are searching    */
static ino_t pardir;	/* I-node number of parent of the dir we search */
static ino_t inum;	/* I-node number of the file being checked      */
static int invblk;	/* number of inv blocks seen (in this file)     */
static int midfrag;	/* number of middle frags seen (in this file)	*/
static int dupblk;	/* number of dup blocks seen (in this file)     */
static int corruptfrag; /* number of corrupt frags seen (in this file)	*/
static int mapdwordfrag;/* number of map dword frags seen (in this file)*/
static off_t filsize;	/* amount of file already processed (by ckinode)*/
static off_t holefrags;	/* number of frags in holes (per file)		*/
static off_t datablocks; /* count of data blocks we found in file	*/
static int   xptno;	/* inc'd in dblock, current xpt number		*/
static int   lastxpt;	/* last non zero disk address in file		*/
static int   fixsize;	/* 1 = fix size in sizechk(), 0 = size ok	*/


static ino_t new_inum;	/* inum for mkentry() to create			*/
static char *new_name;	/* filename for mkentry()	*/

static char dotok;          /* state of the dot entry for the current dir   */
static char dotdotok;       /* state of the dot-dot entry for current dir   */

/* Values for dotok and dotdotok                                        */
#define MISSING 0       /* no such entry found                          */
#define PROPER  1       /* found and pointing to right directory        */
#define WRONG   2       /* found but not point to right directory       */

/*
 * Information used for directory/connectivity checks and corrections
 */
static ino_t lfdir;	/* I-number of the lost and found directory     */
static char lfname[] =  "lost+found"; /* name of lost and found directory */

static char *pathname; /* running full path name of directory entry */
static int pathl = 0;
static int pathp;	/* index into the current end of path name      */
static int thisname;	/* index into start of current entry name      */

static char *srchname;	/* name being searched for by findino */
static ino_t orphan;	/* I-number of highest disconnected directory   */
static ino_t parentdir;	/* I-number of ex-parent of that directory      */


/*
 * The pass mechanism involves calling a ckinode on each I-node of the
 *     filesystem.  Ckinode will call another (pass specific) function
 *     on each block associated with that I-node.  What function it calls
 *     is determined by the global "pfunc".  An additional parameter to
 *     ckinode will have the value ADDR or DATA, which tells ckinode if
 *     the block address or the block contents will be used.  More on this
 *     in the comments to ckinode.
 *
 *     Pass functions return a word of flags, which are use to control the
 *     progress of the pass.
 */
static int (*pfunc)();	/* function to call to check each block of file */
#define ADDR    0       /* call pass function directly to check blocks  */
#define DATA    1       /* pass the block to dirscan for handling       */
#define ALTERD  010     /* the file has been modified                   */
#define KEEPON  04      /* continue with the pass                       */
#define SKIP    02      /* skip this file and go on to next one         */
#define STOP    01      /* fatal error, halt the pass                   */

#define ALLOC	(dp->di_nlink > 0)
/*
 * macros for file types (of disk-format I-nodes)
 */
#define DIR	((dp->di_mode & IFMT) == IFDIR)
#define REG	((dp->di_mode & IFMT) == IFREG)
#define BLK	((dp->di_mode & IFMT) == IFBLK)
#define CHR	((dp->di_mode & IFMT) == IFCHR)
#define FIFO    ((dp->di_mode & IFMT) == IFIFO)
#define LNK     ((dp->di_mode & IFMT) == IFLNK)
#define SOCK	((dp->di_mode & IFMT) == IFSOCK)
#ifdef SPECIAL
# undef	SPECIAL
#endif /* SPECIAL */
#define SPECIAL (BLK || CHR || SOCK)

/*
 * filecntl structures are used to keep track of the read and write
 *      file descriptors for the disk and the scratch file
 */
struct filecntl {
	int     rfdes;          /* read file descriptor for file        */
	int     wfdes;          /* write file descriptor for file       */
	int     mod;            /* has file ever been written to        */
};

static struct filecntl sfile;	/* file descriptors for scratch file    */

/*
 * In order to reduce the amount of disk I/O which must be performed,
 * fsck maintains a cache of disk blocks.  These blocks are only written
 * out to disk when a block is displaced from the cache or when an explicit
 * cache flush is performed.  The buffers in this cache are used to contain
 * all types of blocks (superblocks, I-node blocks, indirect blocks, free
 * blocks, directory blocks and also paged out tables.
 *
 * In the event that the machine's address space is too small to permit the
 * tables to all be maintained in core, these tables are paged out to a
 * scratch file.  In this case the cache becomes even more important, since
 * the locality of table references is generally very high.
 *
 * A few special purpose buffers (such as that used for superblocks) are
 * reserved and not part of the general cache.  All buffers, however are
 * in the same BUFAREA data structure - since the I/O and buffer manipulation
 * routines usually expect a BUFAREA as an argument.
 */
typedef struct dinode	DINODE;
typedef struct direct	DIRECT;

#ifdef SUCCESS
#undef SUCCESS
#endif
#define SUCCESS			0

#define GROW_INOFULL		-150
#define GROW_FSFULL		-151
#define GROW_FAIL		-152

typedef union
{
	char		b_buf[BLKSIZE];
	fdaddr_t	b_sin[DADDRPERBLK];
	struct idblock	b_din[INDIRPERBLK];
}blkbuf;


#define SPERB   (BLKSIZE/sizeof(short))   /* shorts storable in a block   */
static int	Sperb   = SPERB;

struct bufarea {
	struct bufarea  *b_next;                /* LRU chain link       */
	daddr_t b_bno;                          /* what block is this   */
	union {
		char    b_buf[BLKSIZE];		/* buffer of characters */
		short   b_lnks[SPERB];		/* block of link counts */
		char	b_dir[BLKSIZE];		/* block from directory */
	} b_un;
	char    b_dirty;                        /* write this block out */
};

typedef struct bufarea BUFAREA;

static BUFAREA *poolhead;	/* header for general buffer cache      */

/*
 * MACROs to initialize BUFAREAs and mark them as dirty
 */
#define initbarea(x)    ( (x)->b_dirty = 0, (x)->b_bno = (daddr_t)-1 )
#define dirty(x)	(x)->b_dirty = 1

static char Nulstr[] =  "";

/*
 * Other miscellaneous macros
 */
#undef howmany
#undef roundup

#define howmany(x,y)    (long) (((x)+((y)-1))/(y))
#define roundup(x,y)    (long) ((((x)+((y)-1))/(y))*(y))
#define outrange(frag)     ((frag).addr < fmin || 			\
			    (frag).addr  + FRAGSIN((frag)) - 1 >= fmax)
#define zapino(x)	memset((void *)(x), 0, (size_t)sizeof(DINODE)), setstate(CLEAR)

#define	DIRECT		direct_t

/*
 * declarations for system calls, utility routines and internal routines
 */
extern long	 ulimit();
extern off_t	 lseek();
extern time_t 	time(time_t *timer);

char	*getfslabel();
int	bmap();

static int mapsearch(uint *mp, uint fraglen, uint hint);
static int allocfrag(frag_t *frag, uint hint);
static void initdmap(int diskagsize, int inoagsize);
DINODE	*ginode();
BUFAREA	*getblk();
BUFAREA	*search();
int	dirscan();
int	ckinode();
int	iblock();
int	dblock();
int	findino();
int	mkentry();
int	pass1();
int	pass1b();
int	pass2();
int	pass4();
int	pass5();
int	direrr();
int	active_log();
void	flush();
void	descend();
void	adjust();
void	pinode();
void	blkerr();
void	needscratch();
void	sizechk();
void	catch(int);
int	map_chk();
bool_t	checklog();
struct vmdmap *readvmd();
int	inodefree(), blockfree();
ino_t	lost_found();
ino_t	allocdir();
int	makeentry();
void	initdirblk();
void	freeblk();
ino_t	allocino();
int	dirmangled();
int	dirbadname();
void	freedir();
void	freeino();
int	chgino();

#ifdef DEBUG
void	xdump();
#endif

/*
 * prototypes/defines for code that adds . and .. to directories
 */
#define NUMENTRIES	3
static int dir_overlap(struct direct *, struct direct *[], int);
static void rm_dirent(struct direct *[], int);
static void mk_dot(struct direct *[], ino_t);
static void mk_dotdot(struct direct *[], ino_t);
static void mk_dots(struct dinode *, char *, ino_t);
static void mk_dlist(char *, struct direct *[], int);
 
/*
 *  buf is a buffer that contains a directory block
 *  de[] is an array of ptrs to dirents in buf, and it is null terminated
 *  (eg. de[0] == first entry, de[1] == second entry, ...)
 *  num is the length of the de[] array.
 */
void
mk_dlist(
char *		buf,
struct direct *	de[],
int		num)
{
	int i, off;

	num--;
	for (off = i = 0; off < 512 && i < num; off += de[i]->d_reclen, i++)
		de[i] = (struct direct *)(buf + off);

	for (; i <= num; i++)
		de[i] = (struct direct *)NULL;
}

/*
 *  At the point where rm_dirent is called in fsck, the inode's link
 *  count has already been decremented, and we need to inc it since 
 *  it will no longer be in the directory tree.  Then in pass4, we can
 *  put it in lost+found.
 */
static void
rm_dirent(
struct direct *	de[],
int		i)
{
	ino_t	oino;

	if (de[i]->d_ino)
	{
		oino = inum;
		inum = de[i]->d_ino;
		setlncnt(getlncnt() + 1);
		inum = oino;
		de[i]->d_ino = 0;
	}
}

/*
 *  return codes
 *     -1:	entries currently overlap, but can be shrunk so they don't
 *	0:	entries don't overlap
 *	1:	entries overlap
 */
int
dir_overlap(
struct direct *	slot,
struct direct *	de[],
int		i)
{
	int soff, doff, minlen;
	
	if (de[i] == slot)
		return 1;
	
	soff = (uint)slot - (uint)de[0];
	doff = (uint)de[i] - (uint)de[0];
	minlen = LDIRSIZE(de[i]->d_namlen);

	if (doff < soff)
	{
		if (doff + de[i]->d_reclen > soff)
		{
			/*
			 * will shrinking help?
			 */
			if (doff + minlen > soff)
				return 1;
			return -1;
		}
	}
	else
		if (soff + DOTLEN > doff)
			return 1;
	return 0;
}

/*
 *  insert dirent in a dir and fixup the entry that comes before you (first)
 *  and after you (last)
 *
 *  slot :  ptr to place where new entry goes (ptr in dirbuf)
 *  de   :  array of ptrs to existing dirents (ptr in dirbuf)
 *  first:  index in de to first dirent that overlaps slot
 *  last :  index in de to last dirent that overlaps slot
 */
static void
fix_neighbors(
struct direct *	slot,	
struct direct *	de[],
int		first,
int		last)
{
	int soff, foff, loff, new_reclen;

	soff = (uint)slot - (uint)de[0];
	foff = (uint)de[first] - (uint)de[0];
	loff = (uint)de[last] - (uint)de[0];
	new_reclen = loff - soff + de[last]->d_reclen;

	/*
	 * see if we need to shrink or delete first entry
	 * (earlier in dir_overlap, we set d_ino to 0 if should be deleted)
	 */
	if (de[first]->d_ino)
		de[first]->d_reclen = soff - foff;
	else
		if (first > 0)
		{
			foff = (uint)de[first - 1] - (uint)de[0];
			de[first - 1]->d_reclen = soff - foff;
		}
		else
			if (first == 0 && soff > 0)
				de[first]->d_reclen = soff;
	slot->d_reclen = new_reclen;
}

/*
 *  make entry for '.' in directory
 *
 *  de  : array of ptrs to first few entries in dir
 *  ino : inode number for '.'
 *
 *  if entry for '..' overlays place for '.', then we need to move ..'s entry
 *  do this by removing .. and then calling mk_dotdot
 */
static void
mk_dot(
struct direct *	de[],
ino_t		ino)
{
	struct direct	*slot, dot;
	ino_t		ddino, oino;

	slot = de[0];
	dot.d_ino = ino;
	dot.d_namlen = 1;
	dot.d_reclen = DOTLEN;
	dot.d_name[0] = '.';
	dot.d_name[1] = dot.d_name[2] = dot.d_name[3] = 0;

	ddino = 0;
	if (!strcmp(de[0]->d_name, ".."))
		ddino = de[0]->d_ino;
	rm_dirent(de, 0);
	fix_neighbors(slot, de, 0, 0);
		
	/*
	 * dec link count on this dir (".")
	 */
	oino = inum;
	inum = ino;
	setlncnt(getlncnt() - 1);
	inum = oino;

	/*
	 * copy entry for "." to slot
	 */
	dot.d_reclen = slot->d_reclen;
	memcpy(slot, &dot, DOTLEN);	

	/*
	 * if we killed ".." in the process, regenerate de[] and 
	 *    call mk_dotdot with ddino
	 */
	if (ddino)
	{
		mk_dlist((char *)de[0], de, NUMENTRIES);
		mk_dotdot(de, ddino);
	}
}

/*
 *  make entry for '..' in directory
 *
 *  de  : array of ptrs to first few entries in dir
 *  ino : inode number for '..'
 *
 *  if entry for '.' overlays place for '..', then we need to move .'s entry.
 *  do this by removing '.' and then calling mk_dot.
 */
static void
mk_dotdot(
struct direct *	de[],
ino_t		ino)
{
	struct direct	*slot, ddot;
	ino_t		dino, oino;
	int		rc, i, first, last;

	slot = (struct direct *)((char *)(de[0]) + DOTLEN);
	ddot.d_ino = ino;
	ddot.d_namlen = 2;
	ddot.d_reclen = DOTLEN;
	ddot.d_name[0] = ddot.d_name[1] = '.';
	ddot.d_name[2] = ddot.d_name[3] = 0;

	first = last = -1;
	for (i = dino = 0; de[i]; i++)
		if (rc = dir_overlap(slot, de, i))
		{
			if (first < 0)
				first = i;
			last = i;
			if (rc > 0)
			{
				if (!strcmp(de[i]->d_name, "."))
					dino = de[i]->d_ino;
				rm_dirent(de, i);
			}
		}

	if (first > -1)
		fix_neighbors(slot, de, first, last);

	/*
	 * dec link count on this dir
	 */
	oino = inum;
	inum = ino;
	setlncnt(getlncnt() - 1);
	inum = oino;

	/*
	 * copy entry for ".." to slot
 	 */
	ddot.d_reclen = slot->d_reclen;
	memcpy(slot, &ddot, DOTLEN);

	/*
	 * if we killed "." in the process, regenerate de[] and 
	 *    call mk_dot with dino
	 */
	if (dino)
	{
		mk_dlist((char *)de[0], de, NUMENTRIES);
		mk_dot(de, dino);
	}
}

/*
 *  dp  : inode of dir missing . or ..
 *  name: name of missing entry
 *  ino : inode num for missing entry
 *
 * by the time this is called (from descend), we have traversed and
 * validated the entries in dp.  we know that the only problem with
 * this directory is that it is missing . or ..
 *
 * read in first dirblk, initialize list of dirents, and call mk_dot or
 * mk_dotdot.
 */
static void
mk_dots(
struct dinode *	dp,	/* directory inode		*/
char *		name,	/* name of direc		*/
ino_t		ino)	/* inode number to put in dir	*/
{
	frag_t		blk;
	struct direct	*de[NUMENTRIES];
	int		off, i;

	bmap(dp, 0, &blk);
	de[0] = (struct direct *)bpread(blk);

	mk_dlist((char *)de[0], de, NUMENTRIES);

	if (strcmp(name, ".."))
		mk_dot(de, ino);
	else
		mk_dotdot(de, ino);
	bptouch(de[0]);
}



/*
 *      this is the routine that really does the work
 */
static void
check(devfd, statarea)
int devfd;
struct stat *statarea;
{
	int   mntd = (Mounted == True);  /* whether it is currently mounted */
	register DINODE *dp;
	register n;
	register ino_t *blp;
	register ino_t savino;
	register daddr_t blk;
	register BUFAREA *bp1, *bp2;
	int first, result;
	frag_t frag;

	/*
	 * open the device, allocate memory and initialize tables and vars
	 */
	if (!setup(devfd, mntd, statarea))
		return;
	initdmap(sbagsize, sbiagsize);

	/*
	 * PASS I
	 *      note wierd I-nodes, link counts and allocated blocks
	 */
	pr(QUIET, MSGSTR(PHASE1, PHASE1_str));
	pfunc = pass1;

	for(inum = 0; inum <= imax; inum++)
	{
		if (inum == INODES_I)
			continue;
		if((dp = ginode()) != NULL) {
			/*
			 * if the inode isn't allocated, release and ignore it
			 */
			if (!ALLOC) {
				bprelease((void *)dp);	/* free inode */
				continue;
				}
			lastino = inum;
			/*
			 * check the file type
			 */
			if(inum > 0 && ftypeok(dp) == NO) {
				if( pr(PINODE|ASK|FATAL, MSGSTR(CLEAR1, CLEAR1_str),
				   MSGSTR(UNKFTYPE, UNKFTYPE_str)) ) {
					zapino(dp);
					bptouch((void *)dp);
					}
				}
			else {
				n_files++;
				/*
				 * track link count
				 */
				if(setlncnt(dp->di_nlink) <= 0) {
					if(badlnp < &badlncnt[MAXLNCNT])
						*badlnp++ = inum;
					else {  /*
						 * This will prevent us from
						 * fixing all of the detached
						 * files in this filesystem.
						 */
						a_ok = NO;
						pr(CONTIN, MSGSTR(LNKOVR, LNKOVR_str));
						}
					}
				/*
				 * and file state
				 */
				setstate(DIR ? DSTATE : FSTATE);
				invblk = midfrag = dupblk = corruptfrag =
					mapdwordfrag = 0;
				/*
				 * and check disk addrs
				 */
				ckinode(dp, ADDR);
				if((n = getstate()) == DSTATE || n == FSTATE)
					sizechk(dp);
				}
			bprelease((void *)dp);	/* free inode */
			}
	}

	/*
	 * If we found any duplicated blocks, we reported which files
	 *    the second and subsequent references.  Go back and re-scan
	 *    to find and report the first instances of files which
	 *    share those blocks.
	 */
	if(enddup != &duplist[0]) {
		pr(QUIET,MSGSTR(PHASE1B, PHASE1B_str));
		pfunc = pass1b;
		for(inum = 0; inum <= lastino; inum++)
		{
			if (inum == INODES_I)
				continue;
			if(getstate() != USTATE && (dp = ginode()) != NULL) {
				if(ckinode(dp, ADDR) & STOP) {
					bprelease((void *)dp);	/* free inode */
					break;
					}
				bprelease((void *)dp);	/* free inode */
				}
		}
	}

	/*
	 * PASS II
	 *      search all directories to find all references to files
	 */

	pr(QUIET, MSGSTR(PHASE2, PHASE2_str));

	inum = curdir = pardir = ROOTDIR_I;
	if (!pathl)
		pathname = (char *) malloc((size_t)(pathl = MAXPATHN));
	thisname = pathp = 0;	/* beginning of pathname[] */
	pfunc = pass2;

	switch(getstate()) {
		case USTATE:    /* there is no root I-node      */
			pr(ASK|FATAL, MSGSTR(FIX, FIX_str), MSGSTR(RIUNALLOC, RIUNALLOC_str));
			setstate(FSTATE);
		case FSTATE:    /* root I-node is no longer a directory */
			pr(ASK|FATAL, MSGSTR(FIX, FIX_str), MSGSTR(RINOTDIR, RINOTDIR_str));
			/*
			 * make it a directory
			 */
			if( !(dp = ginode()) )
				pr(FATAL, Nulstr);
			dp->di_mode &= ~IFMT;
			dp->di_mode |= (IFDIR | IFJOURNAL);
			bptouch((void *)dp);	/* dirty inode */
			bprelease((void *)dp);	/* free inode */
			setstate(DSTATE);

		case DSTATE:    /* normal, healthy root I-node  */
			descend();
			break;

		case CLEAR:     /* root I-node is damaged       */
			pr(CONTIN, MSGSTR(DUPINRI, DUPINRI_str));
			a_ok = NO;
			setstate(DSTATE);
			descend();
	}

	/*
	 * PASS III
	 *      Traverse the filesystem again, looking for directories which
	 *      we could not find when we enumerated the directory structure
	 *      from the root (in pass II).  If possible, these should be
	 *      reconnected into the root hierarchy.
	 *
	 * NOTE: as a side effect of pass II, all directories which could be
	 *       reached had their state changed to FSTATE.  Thus any file
	 *       which is still in DSTATE is an inaccessible directory.
	 */

	pr(QUIET, MSGSTR(PHASE3, PHASE3_str));

	for(inum = ROOTDIR_I; inum <= lastino; inum++)
	{
		if (inum == INODES_I)
			continue;

		if(getstate() == DSTATE) {
			/*
			 * This is a disconnected directory.  Search back
			 *      along the .. chain until we find the directory
			 *      at the top of the unreachable sub-tree.
			 */
			pfunc = findino;
			srchname = "..";
			savino = inum;
			do {
				orphan = inum;
				if((dp = ginode()) == NULL)
					break;
				parentdir = 0;
				ckinode(dp, DATA);
				bprelease((void *)dp);	/* free inode */
				if((inum = parentdir) == 0)
					break;
			} while(getstate() == DSTATE);
			inum = orphan;

			/*
			 * inum refers to top level unreachable directory.
			 *      If we reconnect this sub-tree into lost+found
			 *      it will become reachable and so we should
			 *      rerun pass II on it, since its isolation
			 *      caused it to be missed by pass II.
			 *
			 * if we wanted to put inum in lost+found but
			 * 	failed (linkup returned LF_FAIL), then
			 *	let this dir remain unconnected.  Set the
			 *	state (as if this dir isn't orphaned) and 
			 *	don't look at any of its children.
			 */
			if((result = linkup()) == YES)
			{
				thisname = pathp = 0; /* begin of pathname[]*/
				pathname[pathp++] = '?';
				pfunc = pass2;
				curdir = inum;
				pardir = lfdir;
				descend();
			}
			else
				if (result == LF_FAIL)
					setstate(FSTATE);
			inum = savino;
		}

	}
	/*
	 * PASS IV
	 *
	 *      make a final pass over all of the files, looking for
	 *      unreferenced files and link/reference mismatches.  Adjust
	 *      the link counts to agree with the reference counts, give
	 *      the operator a chance to re-attach unreferenced files and
	 *      then clear files which remain unreferenced.
	 */

	pr(QUIET, MSGSTR(PHASE4, PHASE4_str));

	pfunc = pass4;
	for(inum = ROOTDIR_I; inum <= lastino; inum++) {
		/*
		 * do NOT check reference counts on inodes <= LAST_RSVD_I.
		 * these are reserved and one cannot count on them having
		 * proper reference counts.  of course, always check
		 * ROOTDIR_I though!
		 */
		if (inum != ROOTDIR_I && inum <= LAST_RSVD_I)
			continue;

		switch(getstate()) {
			case FSTATE:
				if(n = getlncnt())
					adjust((short)n);
				else {
					for(blp = badlncnt;blp < badlnp; blp++)
						if(*blp == inum) {
							clri(MSGSTR(UNREF,
							     UNREF_str), PINODE);
							break;
						}
				}
				break;
			case DSTATE:
				clri(MSGSTR(UNREF, UNREF_str), PINODE);
				break;
			case CLEAR:
				clri(MSGSTR(DUPBAD, DUPBAD_str), PINODE);
		}
	}

	/*
	 * PASS V
	 *
	 *	- we make this pass 5, and leave the block map as the last pass
	 *	  since (at least for future purposes) this pass could
	 *	  theoretically change the block allocation for this filesystem
	 *	  (who knows, someday we may allocate blocks in this pass?)
	 *
	 */

	/*
	 * act like we saw all of the inodes until LAST_RSVD_I since we
	 * expect the inode map to have them all allocated...
	 */
	for (inum = 0; inum <= LAST_RSVD_I; inum++)
		setstate(FSTATE);

	/*
	 * note: someday we should split map_chk into an inode_map_chk
	 * and a block_map_chk.  even though the code is exactly the
	 * same (except for the parameters), calling a function a zillion
	 * times may be very expensive.  esp when the function is really
	 * just calling a macro...
	 */
	(void) map_chk(5, MSGSTR(INODE, INODE_str), INOMAP_I);

	n_free = map_chk(6, MSGSTR(BLOCK, BLOCK_str), DISKMAP_I);

	/*
	 *    check to see if we are missing anything or if the
	 *    superblock free count is wrong.
	 */
	if((n_blks+n_free) != (fmax-fmin))
		pr(0, MSGSTR(BLKMISS, BLKMISS_str),
			       (long) FRAG2DEVBLK(fmax-fmin-n_blks-n_free));

       /*
	* If everything is now OK and the superblock fmod flag doesn't
	*    agree, change that flag to reflect its clean bill of health.
	*
	* NOTE:
	*       if filesystem is unmounted, s_fmod should be zero
	*       if filesystem is mounted, s_fmod should be one
	*       if s_fmod is not one or zero, it is definitely wrong
	*/
	if (a_ok && Super->s_fmod && (Super->s_fmod != mntd)) {
		if (pr( PREEN, MSGSTR(FIX, FIX_str), MSGSTR(SBLKDIR, SBLKDIR_str))) {
			Super->s_fmod = 0;
			bptouch((void *)Super);
		}
	} else if (!a_ok)
		pr( 0, MSGSTR(FSYSBAD, FSYSBAD_str));


	/*
	 * finish off with a basic report on the state of the filesystem
	 */
	pr(0, MSGSTR(SUMMARY, SUMMARY_str), n_files,
		 (long) FRAG2DEVBLK(n_blks), (long) FRAG2DEVBLK(n_free));

       /*
	* If we have modified a mounted filesystem, a reboot will be needed.
	* We should also update the last mounted date on the filesystem
	* to reflect the changes we just made.
	*/
	if(Bp_touched) {
#ifndef STANDALONE
		time(&Super->s_time);
		bptouch((void *)Super);
		if (mntd) {
			pr(0, MSGSTR(MODMOUNT, MODMOUNT_str));
			xcode |= EXIT_BOOT;
		} else
#endif
			pr(QUIET, MSGSTR(MODIFIED, MODIFIED_str));
	}

	if (cache_bufs)
		pr(TRIVIA, MSGSTR(CACHEP, CACHEP_str),
		     cache_hits, cache_tries, cache_bufs );

	devname = NULL;
}

/*
 * CKINODE
 *      This routine is called in order to have a pass function performed
 *      on every block of a file.  The flag is a (crude) indication of
 *      whether the purpose of the pass is to enumerate all of the blocks
 *      or to scan the data in every block (i.e. to search a directory).
 *      In the former case, CKINODE invokes the pass function on each block.
 *      In the latter case, CKINODE invokes DIRSCAN on each block and
 *      leaves further action up to DIRSCAN.
 */
static int
ckinode(dp, flg)
register DINODE *dp;
register int flg;
{
	int ret;
	daddr_t blocks;		/* number of blocks in file		*/

	datablocks = holefrags = fixsize = 0;
	xptno = lastxpt = -1;

	/* special files contain no pages - so we don't check them      */
	if (SPECIAL)
		return(KEEPON);

	/*
	 * check for bizarre case of size indicating indirection yet
	 * the indirect block is 0 (this can cause an assert in the
	 * kernel so we at least have to prevent the assert!)
	 */
	if (!NOINDIRECT(NUMDADDRS(*dp)) && dp->di_rindirect == 0)
    	{
		pr(0, MSGSTR(SIZEWRONG, SIZEWRONG_str), inum);
		setstate(CLEAR);
		return KEEPON;
    	}

	/*
	 * symbolic links smaller than sizeof(dp->di_symlink) don't have
	 * data addresses.  instead the link name is in the inode itself.
	 */
	if (LNK && dp->di_size <= sizeof(dp->di_symlink))
		return (KEEPON);

	filsize = dp->di_size;
	blocks = NUMDADDRS(*dp);

	/* calculate global compress variable for this inode.
	 */
	if (Sbcompress)
	{
		compress = (!(dp->di_mode & IFJOURNAL))  && (inum > SPECIAL_I);
	}

	if (NOINDIRECT(blocks))
		ret = dblock(flg, dp->di_rdaddr, NDADDR);
	else
		ret = iblock(dp->di_rindirect, ISDOUBLEIND(blocks) ? 2 : 1, flg);

	return (ret);
}

/*
 *	dblock
 *
 *	- invoke pass function on each disk block in a block of disk addresses
 *
 */

static int
dblock(flg, daddrs, ct)
int	 flg;			/* type of pass function	*/
daddr_t	*daddrs;		/* block of disk addresses	*/
int	 ct;			/* number of disk addresses	*/
{
	fdaddr_t	*frag;
	register int (*func)(), ret;

	func = (flg == ADDR) ? pfunc : dirscan;

	/* invoke the pass function on each direct block */
	for (frag = (fdaddr_t *)daddrs;
	     frag < (fdaddr_t *)daddrs + ct && filsize > 0;
	     frag++)
	{
		/*
		 * note that inum 0 has block 0 in it.  and we want
		 * to run the normal checks on block 0...  but, for
		 * the rest of the inodes, block 0 is a hole in the
		 * file...
		 * note also that this loop only occurs once for
		 * inum 0 since filsize controls it...
 		 * for compression, we increment holefrags as if 
		 * the file was an integral number of BLKSIZE units.
		 * on loop exit, holefrags is adjusted.
		 */
		xptno++;
		if (frag->f.addr || inum == 0)
		{
			lastxpt = xptno;
			datablocks += FRAGSIN(frag->f);
			if ((ret = (*func)(frag->f)) & STOP)
				return (ret);
			if (compress)
				holefrags += FragPerBlk - FRAGSIN(frag->f);
		}
		else
		{
			/*
			 *  current disk address is 0 and inode number isn't 0.
			 *  If remaining filsize >= BLKSIZE then the current
			 *  disk address represents a "hole", and we increment
			 *  holefrags accordingly.
			 *  However, if filesize < BLKSIZE then:
			 *     1) this is NOT a hole
			 *     2) di_size is wrong (bigger than the disk
			 *	  allocation for the current inode)
			 *  In this case, we don't want to increment holefrags
			 *  because our check in sizechk() will fail to catch
			 *  the problem.
			 *  
			 */
			if (filsize >= BLKSIZE)
			{
				holefrags += FragPerBlk;
				if (filsize == BLKSIZE && FragSize < BLKSIZE)
					fixsize = 1;
			}

		}
		filsize -= compress ? BLKSIZE : FRAGLEN(frag->f);
	}

	/* adjust holefrags 
	 */
	if (filsize < 0 && compress)
	{
		holefrags -= (-filsize)/FragSize;
	}

	return KEEPON;
}

/*
 * IBLOCK is really just an extension to CKINODE.  It handles the
 * recursion of indirect blocks and invokes a specified pass function
 * (or DIRSCAN) on each of the blocks pointed to by the indirect blocks.
 */
static int
iblock(frag_t frag,
       int ilevel,
       int flg)
{
	char *indblk;		/* indirect block */
	register int (*func)(), ret;
	register struct idblock *iddp, *eddp;
	fdaddr_t ifrag;

	BPRINTF((DDEBUG, "BLOCK %ld is a %s direct block", frag.addr,
		ilevel > 1 ? "double" : "single"));

	/*
	 * If the indirect block is null, figure out how large a hole
	 *    in the file it accounts for.
	 */
	if( frag.addr == 0 ) {
		/*
		 * first, figure out how many bytes a single indirect block
		 * accounts for...
		 */
		register off_t nbytes = Bsize * DADDRPERBLK;

		/*
		 * if this is the double indirect block, account for all of
		 * the single indirect blocks it points to...
		 */
		if (ilevel > 1)
			nbytes *= INDIRPERBLK;

		filsize -= nbytes;
		holefrags += BYTE2FRAG(nbytes);
		xptno += DADDRPERBLK;
		return KEEPON;
	}

	/*
	 * If we are enumerating all of the blocks in the file, call
	 *    the pass function on the indirect block before calling it
	 *    on the blocks it points to.  Otherwise only call DIRSCAN
	 *    on the actual data blocks.
	 */
	if(flg == ADDR) {
		func = pfunc;
		if(((ret = (*func)(frag)) & KEEPON) == 0)
			return(ret);
	}
	else
		func = dirscan;

	if(outrange(frag))		/* protect thyself */
		return(SKIP);

	/*
	 * read in the indirect block
	 *      if it contains pointers to other indirect blocks
	 *         call iblock on each one of them
	 *      else
	 *         call the pass function on each data block
	 */
	if ((indblk = (char *)bpread(frag)) == NULL)
		return(SKIP);

	/*
	 * do direct blocks...
	 */
	if (--ilevel == 0)
		ret = dblock(flg, indblk, DADDRPERBLK);

	/*
	 * else do indirect blocks...
	 */
	else {
		for (iddp = (struct idblock *)indblk,
		     eddp = iddp + INDIRPERBLK; iddp < eddp && filsize > 0;
		     iddp++)
		{
			ifrag.d = iddp->id_raddr;
			if ((ret=iblock(ifrag.f, ilevel, flg)) & STOP) {
				bprelease((void *)indblk); /* free up indirect block */
				return (ret);
				}
		}

		ret = KEEPON;
		}

	bprelease((void *)indblk);	/* free up indirect block */

	return(ret);
}

/*
 *  return 1 if frag1 and frag2 overlap.
 *  make sure frag1 starts before frag2, then see if frag2 begins before
 *  frag1 ends
 */
static int
overlap(frag_t frag1, frag_t frag2)
{
	frag_t temp;

	if (frag1.addr > frag2.addr)
	{
		temp = frag1;
		frag1 = frag2;
		frag2 = temp;
	}
	return (frag2.addr <= frag1.addr + FRAGSIN(frag1) - 1) ? 1 : 0;
}



/*
 * This is the pass function for pass I.  It is called on every block
 *      which is allocated to any file on the filesystem.  It validates
 *      that the block number is valid, and that it is not a block which
 *      is already known to be allocated to another file.  It also reports
 *      on which files contain specified blocks (specified with the -d flag).
 *
 * NOTE:
 *      Each file in which a duplicate block appears will be reported to
 *      the operator.  The first pass will not catch the first instance
 *      of the duplicated block however - since it was not a duplicate the
 *      first time it was seen.  For reasons to be described later, we want
 *      to keep track of HOW MANY times each block is referenced.  To
 *      facilitate this, the dup list is divided into two parts.  The first
 *      portion contains distinct block numbers.  The second portion contains
 *      subsequent instances of block numbers which have already appeared in
 *      the first portion of the list.
 *
 *      If we ever lose track of a duplicated block reference (because of
 *      table space limitations) we will have to rebuild the free-list from
 *      scratch to be sure that we didn't unknowingly free duplicate blocks.
 *      (see pass 4).
 */
static int
pass1(frag_t frag)
{       register int i;
	register frag_t *dlp;
	daddr_t	first_devblk, last_devblk;
	/*
	 *  check for frags that don't exist
	 */
	if(outrange(frag)) {
		blkerr(MSGSTR(INVLD, INVLD_str), frag);
		if(++invblk >= MAXINV) {
			pr(CONTIN, MSGSTR(EXCESSB, EXCESSB_str), inum);
			return(STOP);
		}
		return(SKIP);
	}
	/*
	 *  check for corrupt frag_t (invalid nfrags)
	 */
	if (frag.nfrags >= FragPerBlk) {
		blkerr(MSGSTR(CORRUPTFRAG, CORRUPTFRAG_str), frag);
		if (++corruptfrag >= MAXCORRUPT) {
			pr(CONTIN, MSGSTR(EXCESSCORRUPT, EXCESSCORRUPT_str), inum);
			return STOP;
		}
		return SKIP;
	}

	/*
	 *  check for partial blocks before last block
	 *  if filesize > BLKSIZE, then this isn't last block... and
	 *  	a nonzero nfrags value is illegal
	 */
	if (frag.nfrags && filsize > BLKSIZE && !compress) {
		blkerr(MSGSTR(MIDFRAG, MIDFRAG_str), frag);
		if(++midfrag >= MAXMIDFRAG) {
			pr(CONTIN, MSGSTR(EXCESSMIDFRAG, EXCESSMIDFRAG_str), inum);
			return(STOP);
		}
		return(SKIP);
	}
	/*
	 *  check for frag's allocation state spanning a double
	 *  word in the diskmap
	 */
	if ((frag.addr & BITSPERDWORDMASK) + FRAGSIN(frag) - 1 >=
	    BITSPERDWORD)
	{
		blkerr(MSGSTR(FRAGBADALLOC, FRAGBADALLOC_str), frag);
		if (++mapdwordfrag >= MAXBADALLOC)
		{
			pr(CONTIN, MSGSTR(EXCESSBADALLOC, EXCESSBADALLOC_str), inum);
			return STOP;
		}
		return SKIP;
	}

	/*
	 * see if this block is one we're searching for.  if so, stick this
	 * inode number on the ino search list
	 */
	for( i = 0; i < numsblks; i++ )
	{
		first_devblk = FRAG2DEVBLK(frag.addr);
		last_devblk =  first_devblk + FRAG2DEVBLK( FRAGSIN(frag) ) -1;
		if (sblks[i] >= first_devblk && sblks[i] <= last_devblk &&
		    numsinos < MAXSINOS)
		{
			sinos[numsinos].inum = inum;
			sinos[numsinos++].frag.f = frag;
		}
	}

	/*
	 * check for duplicate blocks
	 */
	if(getbmap(frag)) {
		blkerr(MSGSTR(DUPLICATE, DUPLICATE_str), frag);
		if(++dupblk >= MAXDUP) {
			pr(CONTIN, MSGSTR(EXCESD, EXCESD_str),
				inum);
			return(STOP);
		}
		if(enddup >= &duplist[DUPTBLSIZE]) {
			pr(CONTIN, MSGSTR(DUPOVER, DUPOVER_str));
			return(STOP);
		}
		for(dlp = duplist; dlp < muldup; dlp++) {
			if(overlap(*dlp, frag)) {
				*enddup++ = frag;
				break;
			}
		}
		if(dlp >= muldup) {
			*enddup++ = *muldup;
			*muldup++ = frag;
		}
	}
	else {
		n_blks += FRAGSIN(frag);
		setbmap(frag);
	}

	return(KEEPON);
}


/*
 * This is the pass routine for pass IB.  Its purpose is to find the
 *      first reference to each block which is referenced by multiple
 *      files.  Once it has found this reference, it moves it off of the
 *      first part of the dup list so it will not be found again.
 */
static int
pass1b(frag_t frag)
{
	register frag_t *dlp;

	/*
	 *  protect ourselves from illegal frag_t's
	 */
	if (outrange(frag) || frag.nfrags >= FragPerBlk)
		return SKIP;

	for(dlp = duplist; dlp < muldup; dlp++) {
		if(overlap(*dlp, frag)) {
			blkerr(MSGSTR(DUPLICATE, DUPLICATE_str), frag);
			*dlp = *--muldup;
			*muldup = frag;
			return muldup == duplist ? STOP : KEEPON;
		}
	}

	return(KEEPON);
}

/*
 * This is the pass function for pass II.  It is called on every entry of
 *      every directory in the filesystem.  Its purpose is to check the
 *      entry for gross validity, keep track of the references and recursively
 *      invoke descend on each directory it encounters (it was descend which
 *      invoked us through ckinode).
 *
 *      During this pass, the character array "pathname" is being extended
 *      and contracted to always reflect the fully qualified name of the
 *      entry we are currently checking.
 *
 * NOTE:To prevent the recursion from looping on loops in the directory
 *      hierarchy, each directory encountered is marked (in the state table)
 *      as being an ordinary file.  This prevents us from trying to descend
 *      into it a second time.  It also allows us to easily recognize
 *      directories which we couldn't reach by enumerating the directory
 *      hierarchy - they will still be marked as directories.
 *
 * NOTE:At this point, we will ask the user to allow us to remove links to
 *      damaged files.  If we can successfully remove these links, we can
 *      later clear those files with no harm done.  If the user refuses to
 *      allow this cleanup, we will cancel the death-mark that was placed
 *      on the file.
 */
static int
pass2(dirp)
register DIRECT *dirp;
{
	register char *p;
	register DINODE *dp;
	register int i;
	int trash = NO;
	int altered = 0;
	ino_t savepar;

	/* ignore unused entry */
	if ((inum = dirp->d_ino) == 0)
		return(KEEPON);

	/* if we overflow the pathname buffer */
	/* then expand it		      */
	if (pathp + dirp->d_namlen + 1 >= pathl)
		if(!(pathname = (char *) realloc((void *)pathname,
						 (size_t)(pathl += 2048))))
			pr(FATAL, MSGSTR(NOMEME, NOMEME_str));

	/* copy file name into path name, checking for weird characters */
	thisname = pathp;
	for(p = dirp->d_name;
		p < &dirp->d_name[dirp->d_namlen];
			pathname[pathp++] = *p++) {
		if (*p == 0)
			break;

		if (*p == '/') {
			pathname[pathp] = 0;
			if (pr( ASK, MSGSTR(FIX, FIX_str),
				MSGSTR(EMBBAD, EMBBAD_str), *p, pathname )) {
				*p = '#';
				altered = ALTERD;
			}
		}
	}
	pathname[pathp] = 0;

	/* check for embedded nulls in directory entry */
	while(p < &dirp->d_name[dirp->d_namlen])
	{
		if (*p)
		{
			if (pr(PREEN, MSGSTR(FIX, FIX_str),
			       MSGSTR(EMBNULL, EMBNULL_str), pathname ))
			{
				while( p < &dirp->d_name[dirp->d_namlen] )
					*p++ = 0;
				altered = ALTERD;
			}
			break;
		}
		p++;
	}

	/* see if we are looking for this file                  */
	for( i = 0; i < numsinos; i++ )
		if (inum == sinos[i].inum)
			if (sinos[i].frag.d)
				pr(0, MSGSTR(FOUNDB, FOUNDB_str),
				   (long)FRAG2DEVBLK(sinos[i].frag.f.addr),
				   pathname);
			else
				pr(0, MSGSTR(FOUNDF, FOUNDF_str), inum,
				   pathname );

	/* validate and fix . and .. entries                    */
	if (dirp->d_name[0] == '.')
		if (dirp->d_name[1] == 0)
		{
			if (inum != curdir &&
			    pr(PREENN, MSGSTR(FIX, FIX_str),
			       MSGSTR(BADENT, BADENT_str), pathname, ".",
			       MSGSTR(WRONG1, WRONG1_str)))
			{
				dirp->d_ino = inum = curdir;
				bptouch(dirp);
				dotok = PROPER;
			}
			else
			{
				dotok = WRONG;
				inum = curdir;
			}
		}
		else
			if (dirp->d_name[1] == '.' && dirp->d_name[2] == 0)
				if (inum != pardir &&
				    pr(PREENN, MSGSTR(FIX, FIX_str),
				       MSGSTR(BADENT, BADENT_str), pathname,
				       "..", MSGSTR(WRONG1, WRONG1_str)))
				{
					dirp->d_ino = inum = pardir;
					bptouch(dirp);
					dotdotok = PROPER;
				}
				else
				{
					dotdotok = WRONG;
					inum = pardir;
				}

	if(inum > imax || inum < 0)
		trash = direrr(PREEN, MSGSTR(RANGEE, RANGEE_str),
			       MSGSTR(REMOVE, REMOVE_str));
	else {
	again:
		switch(getstate()) {
			case USTATE:    /* ref to unallocated file      */
				trash = direrr(PREEN,
					       MSGSTR(UNALLOC, UNALLOC_str),
					       MSGSTR(REMOVE, REMOVE_str));
				break;
			case CLEAR:     /* file is marked for death     */
				if((trash =
				    direrr(0,MSGSTR(DUPBAD, DUPBAD_str),
					   MSGSTR(REMOVE, REMOVE_str))) == YES)
					break;
				/*
				 * Operator, in his infinite wisdom has
				 * decided not to let us remove this directory
				 * reference - so we must cancel the death
				 * mark we placed on the file in the last pass
				 */
				if((dp = ginode()) == NULL)
					break;
				setstate(DIR ? DSTATE : FSTATE);
				bprelease((void *)dp);	/* free inode */
				goto again;
			case FSTATE:    /* ordinary file                */
				declncnt();
				BPRINTF((DDEBUG, "   file %s ...", pathname));
				break;
			case DSTATE:    /* traverse another directory   */
				BPRINTF((DDEBUG, "   dir %s ...", pathname));
				declncnt();

				savepar = pardir;
				pardir = curdir;
				curdir = inum;

				descend();

				curdir = pardir;
				pardir = savepar;
		}
	}

	pathp = thisname;

	if (trash != NO) {  /* see if we should trash directory entry */
		dirp->d_ino = 0;
		altered = ALTERD;
	}

	if (altered == ALTERD)
		bptouch((void *)dirp);

	return(KEEPON|altered);
}

/*
 * In pass 4, we delete all of the flawed files (to which all references
 * were deleted during pass 3).  This pass function is used to mark all
 * of the blocks in a deleted file free.  If the block is on the duplicate
 * list, we don't mark it free - but just remove one instance of the block
 * from the duplicate list.  In this way, we can be sure that a block is
 * not marked free until all references are removed.
 */

static int
pass4(frag_t frag)
{
	register frag_t *dlp;

	if(outrange(frag))
		return(SKIP);

	if(getbmap(frag)) {
		for(dlp = duplist; dlp < enddup; dlp++)
			if(!memcmp(dlp, &frag, sizeof(frag))) {
				*dlp = *--enddup;
				return(KEEPON);
			}
		freeblk(frag);
	}

	return(KEEPON);
}

/*
 * This routine is used to log the fact that a bad or duplicate block has
 * been found in a particular file.  It reports the occurance and puts a
 * death-mark on the file.
 */
static void
blkerr(char *s, frag_t frag)
{
	pr(0, MSGSTR(BADBLK, BADBLK_str),
			(long)FRAG2DEVBLK(frag.addr), s, inum, frag);
	setstate(CLEAR);	/* mark for possible clearing */
}

/*
 * This is part of the recursive main loop of pass II (enumeration of the
 * directory structure).  Descend is called on a directory I-node,
 * saves the current directory/pathname context and then invokes ckinode
 * on the directory.  Ckinode will invoke dirscan, which will invoke pass2
 * on each directory entry, which will recursively invoke descend on entries
 * that refer to sub-directories.
 */
static void
descend()
{
	register DINODE *dp;
	register int savname;
	register off_t savsize;
	char 	saved, savedd;
	ino_t	curino, parino;

	/* call this directory a file to avoid recursion loops  */
	setstate(FSTATE);

	if((dp = ginode()) == NULL)
		return;

	/* save the previous file name and size         */
	savname = thisname;
	pathname[pathp++] = '/';
	savsize = filsize;
	saved = dotok;
	savedd = dotdotok;
	curino = curdir;
	parino = pardir;

	/* enumerate this directory sub-tree            */
	dotok = MISSING;
	dotdotok = MISSING;
	filsize = dp->di_size;

	ckinode(dp, DATA);

	/* restore the previous file name and size      */
	thisname = savname;
	pathname[--pathp] = 0;
	filsize = savsize;

	/* report on the state of the . and .. entries  */


	if (dotok == MISSING &&
	    pr(PREENN, MSGSTR(FIX, FIX_str), MSGSTR(BADENT, BADENT_str),
	       pathname, ".", MSGSTR(MISS, MISS_str)))
		mk_dots(dp, ".", curino);
	if (dotdotok == MISSING &&
	    pr(PREENN, MSGSTR(FIX, FIX_str), MSGSTR(BADENT, BADENT_str),
	       pathname, "..", MSGSTR(MISS, MISS_str)))
		mk_dots(dp, "..", parino);

	bprelease((void *)dp);	/* free inode */
	dotok = saved;
	dotdotok = savedd;
}


/*
 * This routine is called by CKINODE to enumerate all of the directory
 * entries in a block from some file.  It invokes the specified pass
 * function on each directory entry.
 *
 * NOTE:because the directory block is read into a single global buffer,
 *      and because this is a recursive operation, the contents of that
 *      global buffer may have changed when we return from the pass function.
 */
static int
dirscan(frag_t frag)
{
	register char *db, *edb, *ecdb, *dirblk;
	register DIRECT *dirp;
	register int n, offset;

	if ( outrange(frag) )		/* don't play with bad blocks */
		return(SKIP);

	/*
	 * read in the directory block
	 */
	if ((dirblk = (char *) bpread(frag)) == NULL)
		return (SKIP);

	/*
	 * point to the end of the directory block
	 */
	edb = &dirblk[filsize >= BLKSIZE ? BLKSIZE : filsize];

	/*
	 * these loops get sticky.   basically the outside loop walks thru
	 * the entire filesystem block, while...
	 */
	for (db = dirblk, ecdb = db + DIRBLKSIZ; db < edb;
	     db = ecdb, ecdb += DIRBLKSIZ)
		/*
		 * ... this inside loop walks thru each directory block
		 * (since a directory entry won't span directory blocks)
		 */
		for (dirp = (DIRECT *) db;
		     dirp < (DIRECT *) ecdb;
		     dirp = (DIRECT *) ((char *) dirp + dirp->d_reclen) ) {
			offset = (char *) dirp - db;
			/*
			 * check entry integrity...
			 */
			if (dirmangled(dirp, offset)) {
				dirp->d_reclen = DIRBLKSIZ - offset;
				dirp->d_ino = 0;
				if (pr(PREENN, MSGSTR(FIX, FIX_str), MSGSTR(DIRCOR, DIRCOR_str))) {
					dirp->d_namlen = 0;
					dirp->d_name[0] = '\0';
					bptouch((void *)dirblk);
					}
				}
			n = (*pfunc)(dirp);
			if (n & ALTERD)
				bptouch((void *)dirblk);	  /* dirty directory block */
			if(n & STOP) {
				bprelease((void *)dirblk); /* free up directory block */
				return(n);
				}
		}

	bprelease((void *)dirblk);		/* free up directory block */

	return(filsize > Bsize? KEEPON : STOP);
}

/*
 * This routine is called to report a bad directory entry (one which points
 *      to an unallocated file or a file with a death-mark).  It reports
 *      the error and asks the operator for permission to destroy the
 *      directory entry.  If the operator declines to let us zap the
 *      directory reference, all bets are off on this filesystem
 */
static int
direrr(flg, s, remove)
int	 flg;
char	*s;
char	*remove;
{
	int rc;
	register DINODE *dp = NULL;

	pr(PINODE, "%s", s);

	rc = pr(flg|ASKN, remove, "%s=%s",
		((dp = ginode()) && ftypeok(dp)) ?
		(DIR ? MSGSTR(DIRWORD, DIRWORD_str) : MSGSTR(FILE, FILE_str)):MSGSTR(NAME, NAME_str), pathname);

	if (dp != NULL)
		bprelease((void *)dp);	/* free inode */

	return (rc);
}

/*
 * This routine is called to take appropriate action on files with reference/
 * link count anomalies.  If the file has no references to it and the operator
 * doesn't want it saved, we destroy the file.  If the link count merely
 * disagrees with the reference count, we try to correct it to agree.
 */
static void
adjust(lcnt)
register short lcnt;
{
	register DINODE *dp;
	int		result;

	if((dp = ginode()) == NULL)
		return;
	
	if(dp->di_nlink == lcnt)       /* no references found  */
	{
		/* The only way to get here with a directory is if
		 * the user said to put it in lost+found, and for
  		 * some reason we couldn't.  If the user said
  		 * to delete this orphaned directory, its state would
  		 * be DSTATE, and we would have cleared it in phase 4.
  		 * Since we couldn't put the dir in lost+found, leave
		 * it disconnected and returnd.
  		 */
		if (DIR)
		{
			clri(MSGSTR(UNREF, "Unref"), CANCEL_YESPREEN | PINUM);
			bprelease((void *)dp);
			return;
		}
		if ((result = linkup()) == YES)
			lcnt = getlncnt();
		else
		{
			clri(MSGSTR(UNREF, "Unref"),
			    PINUM | (result == LF_FAIL ? CANCEL_YESPREEN : 0));
			bprelease((void *)dp);	/* free inode */
			return;
		}
	}

	/* a linkup may or may not fix the link count discrepancy */
	if (lcnt) {
		pr(PINODE, MSGSTR(LNCNT, LNCNT_str), (lfdir == inum) ? lfname :	
		(DIR ? MSGSTR(DIRWORD, DIRWORD_str) : MSGSTR(FILE, FILE_str)));

		if(pr(PREENN, MSGSTR(ADJUST, ADJUST_str), 	
		      MSGSTR(COUNTC, COUNTC_str), dp->di_nlink,
		      dp->di_nlink-lcnt))
		{
			dp->di_nlink -= lcnt;
			bptouch((void *)dp);	/* dirty inode */
		}
	}
	bprelease((void *)dp);	/* free inode */
}

/*
 * This function destroys an unreferenced file.  If we are allowed to do so,
 * we will use ckinode with pass4 to make sure that all of the blocks in this
 * file get marked free.
 *
 * Note: preen permits the automatic destruction of invalid files and of
 *       directory references to unallocated files.  Other sorts of file
 *       anomalies (bad blocks, dup blocks) can only be destroyed with
 *       operator permission.  This code would make it appear that preen
 *       can destroy any file, but in pass 2 we canceled the death mark
 *       on files that preen couldn't touch.
 */
static int
clri(s,flg)
char *s;
{
	register DINODE *dp;

	if((dp = ginode()) == NULL)
		return( NO );

	if (flg & CANCEL_YESPREEN)
		flg |= PREEN;
	else
		flg |= PREENN;

	if( pr(flg, MSGSTR(CLEAR1, CLEAR1_str), "%s %s", s,
	       DIR ? MSGSTR(DIRWORD, DIRWORD_str) : MSGSTR(FILE, FILE_str)))
	{
		n_files--;
		pfunc = pass4;
		ckinode(dp, ADDR);
		zapino(dp);
		bptouch((void *)dp);		/* dirty inode */
		bprelease((void *)dp);		/* free inode */
		return( YES );
	}

	bprelease((void *)dp);

	return( NO );
}


/*
 *  globals referenced:
 *	imax	last legal inode number
 *	fmax	first illegal frag.addr (also number of frags in fs)
 *	blkmap	char ptr to internal fragment allocation bitmap
 *
 *  initdmap
 *	mark all inode frags allocated
 *	mark "slop" at end of device as allocated
 *	mark 4k alignment padding bits as allocted
 *		(i want the bitmap to occupy some multiple of 4k byte)
 */
void
initdmap(int diskagsize, int inoagsize)
{
	frag_t frag;
	int	i;
	int	inoblks;	/* number of 4k inode blocks per ag */
	int	nag;		/* number of ags in filesystem	    */
	int	ag;		/* current ag number (loop var)     */
	int	mpage;		/* page offset to fmax in diskmap   */
	int	byteoff;	/* byte offset to fmax in last mpage*/
	int	bitoff;		/* bit offset to fmax in last mbyte */
	int	rem;		/* number of frags in slop	    */
	char	*mp;		/* ptr to fmax's map page	    */
	unsigned char mask;	/* mask for padding fmax's byte w/1 */
	BUFAREA	*bp;		/* stupid scratch file... ARGH!	    */

	/*
	 *  mark all inode blocks allocated
	 */
	inoblks = INO2BLK(inoagsize);
	nag = (imax + 1) / inoagsize;
	frag.new = frag.nfrags = 0;
	for (ag = 0; ag < nag; ag++)
	{
		frag.addr = ag ? ag * diskagsize : BLK2FRAG(INODES_B);
		for (i = 0; i < inoblks; i++, frag.addr += FragPerBlk)
		{
			if (getbmap(frag) == 0)
				n_blks += FragPerBlk;
			setbmap(frag);
		}
	}
	/*
	 *  also mark the "slop" at the end of lv that was not sufficient to
	 *  start another ag.  (not enough frags for ag's inodes)
	 *
	 *  fmax is the number of fragments in the filesystem,
	 *  and for aixv4 and v4 fs's, it is always a multiple of
	 *  FragPerBlk.  all possible aixv3 and v4 agsizes are also
	 *  multiples of all possible FragPerBlk values.  even if you
	 *  specify mkfs -s <size>, where <size> is a number of 512-byte
	 *  blocks that isn't a multiple of 8 (4k/512), sup->s_fsize is
	 *  rounded down to a mult of 8.
	 *
	 *  this means that the number of extra frags left over is always
	 *  evenly divisible by FragPerBlk, and I don't have to worry about
	 *  the "slop" at the end not being a multiple FragPerBlock
	 */
	nag = fmax / diskagsize;
	rem = fmax - nag * diskagsize;
	if (rem < INO2FRAG(inoagsize))
	{
		frag.new = frag.nfrags = 0;
		for (frag.addr = fmax - rem; frag.addr < fmax;
		     frag.addr += FragPerBlk)
		{
			setbmap(frag);
			n_blks += FragPerBlk;
		}
	}
	/*
	 *  now, set the block map's "4k padding bits" to 1.
	 *  i am rounding internal frag allocation bitmap up to a
	 *  multiple of 4k.  setbmap() would barf because the padding
	 *  bits are out of range, so i have to twiddle bits right here.
	 *  basically, the plan is to generate a byte mask (for the byte
	 *  where fmax lives), "or" it into the map, and set the remaining
	 *  bytes to 1 (allocated).
	 *
	 *  if the byte offset is 0, then we don't have any slop in our
	 *  4k map page, so return
	 */
	mpage = BYTE2BLK(fmax >> L2BITSPERBYTE);
	if ((byteoff = (fmax >> L2BITSPERBYTE) & BLKMASK) == 0)
		return;
	bitoff = fmax & BITSPERBYTEMASK;

	if (blkmap)
		mp = blkmap + BLK2BYTE(mpage);
	else
	{
		if ((bp = getblk(mpage)) == NULL)
			pr(FATAL, MSGSTR(FATIO, FATIO_str));
		else
			mp = (char *)&bp->b_un.b_buf;
		dirty(bp);
	}

	for (mask = 1 << (BITSPERBYTE - bitoff - 1); !(mask & 1);
	     mask |= mask >> 1)
		;
	mp[byteoff++] |= (char)(mask & 0xFF);
	memset((void *)(mp + byteoff), (char)0xFF,
	       (size_t)(BLKSIZE - byteoff));
}

/*
 * This routine initializes data structures, allocates memory and opens
 *      the device in preparation of checking a filesystem.
 */
static int
setup(devfd, mntd, statarea)
int devfd;
int   mntd;             /* whether or not it is mounted         */
struct stat *statarea;
{
	int nag, rem;
	register long  smapsz, lncntsz, totsz;
	register int   i;               /* generic integer              */
	register long  l;               /* generic long                 */
	register int   c;               /* generic integer              */
	char   *membase;		/* base of allocated memory     */
	frag_t frag;

	sfile.wfdes = sfile.rfdes = -1;
	n_files = 0;
	n_blks = n_free = 0;
 	muldup = enddup = &duplist[0];
	badlnp = &badlncnt[0];
	lfdir = 0;
	a_ok = YES;
	fslabel = NULL;
	poolhead = NULL;
	cache_tries = cache_hits = cache_bufs = 0;


#ifndef STANDALONE
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, catch);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, catch);
	signal(SIGTERM, catch);
#endif

	/*
	 * The printing here is just a tad tricky.  If we are preening,
	 *     all messages will automatically be preceeded with the name
	 *     of the device we are checking (devname).  If we aren't
	 *     preening we should print out the name of the filesystem
	 *     being checked at the start of each check so that the user
	 *     will know which filesystem errors pertain to.  By printing
	 *     things QUIETly, they will only come out if we are not preening.
	 */
	pr(QUIET+NOB+NOE, MSGSTR(CHECKING, CHECKING_str), devname );

	/*
	 * attempt to validate that this device contains a reasonable
	 * filesystem before we try to check it.  Verify that the
	 * block cluster size is compatible with our system and that
	 * the filesystem sizes are not absurd.
	 */
	fslabel = getfslabel(Super);
	if (fslabel)
		pr(QUIET+NOB+NOE, "(%s) ", fslabel );

	if (mntd)
		pr(QUIET+NOB+NOE, MSGSTR(MNTFSYS, MNTFSYS_str));

	/* if necessary, open the filesystem for write access too      */
	if( nflag )
		pr(QUIET+NOB+NOE, MSGSTR(WRSUP, WRSUP_str));
	pr(QUIET+NOB, "");

	if( Super->s_bsize != Bsize )
		pr(FATAL, MSGSTR(BLKNOT, BLKNOT_str), Bsize);

	/*
	 * See whether or not we can skip checking his filesystem
	 * (first, check before we check the log.  if we fail here,
	 * run the log.  then, check again...)
	 */
	if (fflag && ((Super->s_fmod == FM_CLEAN && !mntd) ||
		      (Super->s_fmod == FM_MOUNT && mntd)))
	{       pr(0, MSGSTR(CLNMNT, CLNMNT_str),
		       mntd ? MSGSTR(MNTD, MNTD_str) : MSGSTR(UNMNTD, UNMNTD_str));
		return( NO );
	}

	/*
	 * Try to get through to the user that errors reported against
	 * a mounted filesystem should be viewed as suspect.
	 */
	if (mntd)
		pr(QUIET+NOB, MSGSTR(MNTFSYS2, MNTFSYS2_str));

	/*
	 * run Phase 0 (Check Log) now before we get the inodes file...
	 */
	if (checklog(devfd, statarea) == True)
	{
		/* If logredo failed continue with consistency checking */
		if (xcode != EXIT_OK)
			pr(0, MSGSTR(LOGREDO_FAIL, LOGREDO_FAIL_str));
	}
			

	/* calculate number of blocks.
	 */
	fsmax(&imax, &frag);
	fmax = frag.addr;

	fmin = 0;	/* first data blk num */
	/* calculate imax = inum of last valid inode */
	imax--;

	if((imax == 0) || (fmin >= fmax))
		pr(FATAL, MSGSTR(SIZECHK, SIZECHK_str), (long) Super->s_fsize);

	/*
	 * If the operator wants noise, print out the filesystem statistics
	 */
	pr(TRIVIA, MSGSTR(FSYSSTAT, FSYSSTAT_str),
	   (long)FRAG2BLK(fmax), (long)INO2BLK(imax + 1));
	pr( TRIVIA, MSGSTR(FSYSSTAT1, FSYSSTAT1_str),
	   Super->s_bsize, imax + 1);

	/*
	 * See whether or not we can skip checking his filesystem
	 */
	if (fflag && ((Super->s_fmod == FM_CLEAN && !mntd) ||
		      (Super->s_fmod == FM_MOUNT && mntd)))
	{       pr( 0, MSGSTR(CLNMNT, CLNMNT_str),
		       mntd ? MSGSTR(MNTD, MNTD_str) : MSGSTR(UNMNTD, UNMNTD_str));
		return( NO );
	}

	/*
	 * Figure out how much memory it will take to check this file
	 * system.  We need to allocate the allocated block map, a file
	 * state map and a link count table.  We will also need a free
	 * block map, but we can overlay that on the state map and link
	 * count tables which we will have finished with by then.
	 *
	 * Optimal memory allocation for fsck is quite tricky.  If we can
	 * get all of our tables in memory that is very important.  If we
	 * can't get them all in memory, we need to use a scratch file and
	 * will need at least MINBUF cache buffers.  Once we have that,
	 * our next priority is a raw I-node buffer which will dramatically
	 * speed up the first pass.  Then, if we have any space left over,
	 * we will try to fill out our cache a bit more.  If we can't, its
	 * no big deal, because the raw I-node buffer will be returned to
	 * the cache at the end of the first pass.
	 */
	/*
	 * make bmapsz be multiple of 4096, so that I am guaranteed that
	 * that I have no partial diskmap pages.
	 */
	bmapsz = roundup(howmany(fmax,BITSPRBYTE),BLKSIZE);
	totmpages = bmapsz >> BLKSHIFT;
	smapsz = roundup(howmany((long)(imax+1),STATEPB),sizeof(*lncntp));
	lncntsz = (long)(imax+1) * sizeof(*lncntp);

	if(bmapsz > smapsz+lncntsz)
		smapsz = bmapsz-lncntsz;
	totsz = bmapsz+smapsz+lncntsz;
	pr( TRIVIA, MSGSTR(TBLSIZ, TBLSIZ_str),
		bmapsz, smapsz, lncntsz, totsz );

	/*
	 * Try to get enough memory for all of the tables.
	 *     Note that the debugging -c flag
	 *     will prevent us from even trying to put our tables in
	 *     memory.
	 */
	if (cflag == NO && (membase = (char *) malloc((size_t)totsz)) == NULL)
		totsz = 0;

	if (cflag == NO  &&  totsz > 0)
	{       memset((void *)membase, 0, (size_t)totsz );
		blkmap = membase;
		statemap = membase + bmapsz;
		lncntp = (short *)(statemap + smapsz);

		pr( TRIVIA, MSGSTR(TBLINCORE, TBLINCORE_str), totsz );
	} else
#ifdef STANDALONE
		pr( FATAL, MSGSTR(FTOOLARG, FTOOLARG_str));
#else
	{       /*
		 * Insufficient memory to keep the tables in memory, so
		 * instead we should keep our tables in a scratch file
		 * and use memory to cache the scratch file.  Scratch file
		 * contains:
		 *      block allocation map
		 *      file state table | later overlayed by
		 *      link count table | free block map
		 */
		bmapsz = roundup(bmapsz,Bsize);
		smapsz = roundup(smapsz,Bsize);
		lncntsz = roundup(lncntsz,Bsize);
		l = (bmapsz+smapsz+lncntsz) >> Bshift;
		needscratch( l );
		blkmap = statemap = (char *) NULL;
		lncntp = (short *) NULL;
		smapblk = bmapsz / Bsize;
		lncntblk = smapblk + (smapsz/Bsize);

		i = MINBUFS * sizeof (BUFAREA);
		if (i < 0  ||  (membase = (char *) malloc((size_t)i)) == NULL)
			pr( FATAL, MSGSTR(NOSPACE, NOSPACE_str) );

		carveup( membase, i );
	}

	/*
	 * Now that we have all of the basics, we should see if we can
	 * make our cache any bigger.
	 */
	if (poolhead)
	{       for( c = (cachesize - MINBUFS); c > 0; c-- )
		{       l = c * sizeof (BUFAREA);
			i = l;
			if (i != l  ||  i < sizeof (BUFAREA))
				continue;
			if ((membase = (char *) malloc((size_t)i)) != NULL) {
				carveup( membase, i );
				break;
				}
		}

		pr( TRIVIA, MSGSTR(TBLPAGE, TBLPAGE_str),
			sfilename, cache_bufs );
	}
#endif
	return( YES );
}

/*
 * 	checklog
 *
 *	- check the log
 */

static bool_t
checklog(int 		devfd,
	 struct stat *	stat)
{
	static int ran_log = 0;

	/*
	 * PASS 0
	 *      check log...
	 *
	 *	if ...
	 *
	 *	(a)	haven't replayed it yet, and
	 *	(b)	f flag isn't on, or if the fs is dirty, and
	 *	(c)	-n isn't on, and
	 *	(d)	we can modify the fs, and
	 *	(f)	it's log isn't already active
	 *	(g)	Nologredo hasn't been set
	 */
	if (!ran_log && (!fflag || Super->s_fmod != FM_CLEAN) &&
	    No == False && CAN_MODIFY &&
	    !active_log(devfd, stat) && Nologredo == False) {
		int rc;
		char command[BUFSIZ];

		pr(QUIET, MSGSTR(PHASE0, PHASE0_str));

		sprintf(command, "PATH=/usr/sbin:/etc:$PATH ; %s %s %s",
			LOGREDO, Logflags, devname);

		if (Logquiet == True)
			strcat(command, " > /dev/null");

		bpflush((void *)Super);
		if ((rc = system(command)) != 0)
			xcode = rc;
		read_super(devfd, stat, False);
		
		ran_log = 1;
		return (True);
		}

	return (False);
}

/*
 *
 *	active_log - return 1 if this filesystem's log is currently active,
 *		     else 0
 *
 */
static int
active_log(
int		devfd,
struct stat *	devstat)
{
	int count, i, ct, logmajor, logminor;
	struct vmount *vmt;
	struct stat fsstat;
	static int mntsiz = 0;
	static char *mnttab = NULL;
	dev_t mountdev;

	logmajor = major(Super->s_logdev);
	logminor = minor(Super->s_logdev);
	
	/*
	 * first time in, point to count
	 */
	if (mnttab == NULL)
	{
		mnttab = (char *) &count;
		mntsiz = sizeof(count);
	}

	/*
	 * loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0)
	{
		/*
		 * error?
		 */
		if (ct < 0)
		{
			pr(FATAL, MSGSTR(MNTCTLE, MNTCTLE_str),
			   strerror(errno));
			/* pr should exit, so we should not reach here... */
			return (NO);
		}

		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ? (char *) malloc((size_t)mntsiz):
			(char *) realloc((void *)mnttab, (size_t)mntsiz);
		if (mnttab == NULL)
			pr(FATAL, MSGSTR(NOSPACE, NOSPACE_str));
	}
	/*
	 *  walk thru the mount table, statting as we go.
	 *  (skip ro mounts since they don't have logs.)
	 *  if we find a logdev match, then someone else is mounted
	 *  using our log, which means our log is active...
	 */
	for (vmt = (struct vmount *) mnttab; ct > 0;
	     ct--, vmt = (struct vmount *)((char *)vmt + vmt->vmt_length))
	{
		memcpy(&mountdev, vmt2dataptr(vmt, VMT_INFO),
		       sizeof(mountdev));
		if (!(vmt->vmt_flags & MNT_READONLY)	&&
		    major(mountdev) == logmajor 	&&
		    minor(mountdev) == logminor		&&
		    stat(vmt2dataptr(vmt, VMT_OBJECT), &fsstat) == 0)
			return 1;
	}
	return 0;
}


/*
 * turn a region of memory into cache buffers
 */
static int
carveup( adr, cnt )
  register char *adr;
  register int cnt;
{       register BUFAREA *bp1, *bp2;
	register int nbufs = 0;

	if (cnt < sizeof (BUFAREA))
		return( 0 );

	memset((void *)adr, 0, (size_t)cnt );

	if (poolhead)
		for( bp1 = poolhead; bp1->b_next; bp1 = bp1->b_next );
	else
		bp1 = 0;

	bp2 = &((BUFAREA *)adr)[cnt/sizeof(BUFAREA)];
	while(--bp2 >= (BUFAREA *)adr)
	{       initbarea(bp2);
		bp2->b_next = NULL;
		cache_bufs++;
		nbufs++;
		if (bp1)
			bp1->b_next = bp2;
		else
			poolhead = bp2;
		bp1 = bp2;
	}
	return( nbufs );
}


#ifndef STANDALONE
/*
 * create a scratch file for the search tables
 */
static void
needscratch( blks )
    long blks;
{       char zeroes[ BLKSIZE ];
	char grpstr[2];
	struct stat statarea;
	register int i;

	/* we need to know the name of a scratch file           */
	if (!tflag)
	    if (preen)
		    pr( FATAL, MSGSTR(USETFLAG, USETFLAG_str) );
	    else
	    {       pr( GETLINE, sfilename, MSGSTR(ENTERSC, ENTERSC_str),
			(long) BLK2DEVBLK(blks));
		    tflag = YES;
	    }

	if (sfilename[0] == 0)
		pr( FATAL, MSGSTR(SCREQ, SCREQ_str));

	/* if we are a parallel pass, use a variant on sfilename name     */
	if (groupnum >= 0)
	{       grpstr[0] = 'A'+groupnum;
		grpstr[1] = 0;
		strcat( sfilename, grpstr );
	}

	/* try to create the scratchfile                */
	if((sfile.wfdes = creat(sfilename,0666)) < 0 ||
	    (sfile.rfdes = open(sfilename,0)) < 0)
		pr(FATAL, "%s: %s", sfilename, strerror(errno));
#ifndef DEBUG
	if( fstat(sfile.rfdes, &statarea) == 0 &&
	    (statarea.st_mode & S_IFMT) == S_IFREG )
		unlink(sfilename);
#endif

	BPRINTF((DDEBUG, "just created %s: %d blocks", sfilename, blks));

	/* clear the scratch file                       */
	memset( (void *)zeroes, 0, (size_t)Bsize );
	for( i = 0; i < blks; i++ )
		if (write( sfile.wfdes, zeroes, Bsize ) != Bsize)
			pr( FATAL, MSGSTR(SCRWRE, SCRWRE_str));
}
#endif

/*
 * This routine returns a pointer to a desired I-node (number in inum).
 *
 * note that this uses the global 'inum'...
 */
static DINODE *
ginode()
{
	register DINODE *dp;		/* inode pointer to return	*/
	int ag;
	int iblk;
	frag_t ifrag = {0,0,0};

	/* the inodes in the first allocation group start at
	 * block INODES_B. all others at the first block of
	 * their allocation group.
	 */
	ag = inum / sbiagsize;
	iblk = (ag ? FRAG2BLK(ag * sbagsize) : INODES_B) +
		INO2BLK(inum % sbiagsize);
	ifrag.addr = BLK2FRAG(iblk);

	/* get iblk into buffer pool
	 */
	if ((dp = (DINODE *) bpread(ifrag)) == NULL)
		return (NULL);
	dp += INOINDEX(inum);
	dp->di_nlink &= 0x7FFF;

	return(dp);
}


/*
 * This routine validates that a file is of a known type
 */
static int
ftypeok(dp)
DINODE *dp;
{
	switch(dp->di_mode & IFMT) {
		case IFDIR:
		case IFREG:
		case IFBLK:
		case IFCHR:
		case IFIFO:
		case IFLNK:
		case IFSOCK:
			return(YES);
		}

	return(NO);
}

/*
 * This routine implements the operations on the file state table
 *
 *      flag    0       set state
 *              1       get state
 */
static int
dostate(s,flg)
int	s;
int	flg;
{
	register char *p;
	register unsigned byte, shift;
	register BUFAREA *bp;

	BPRINTF((DDEBUG, "dostate(inum = %d, s = %d, flg = %d)\n", inum, s, flg));

	byte = (inum)/STATEPB;
	shift = LSTATE * ((inum)%STATEPB);
	if(statemap != NULL) {
		bp = NULL;
		p = &statemap[byte];
	}
	else if((bp = getblk((daddr_t)(smapblk+(byte/Bsize)))) == NULL)
		pr(FATAL, MSGSTR(FATIO, FATIO_str));
	else
		p = &bp->b_un.b_buf[byte%Bsize];
	switch(flg) {
		case 0:
			*p &= ~(SMASK<<(shift));
			*p |= s<<(shift);
			if(bp != NULL)
				dirty(bp);
			return(s);
		case 1:
			return((*p>>(shift)) & SMASK);
	}
	return(USTATE);
}


/*
 *  domap sets/clears/gets bits in op_check's personal frag allocation bitmap.
 *  (it doesn't operate on .diskmap).  it also doesn't have to worry about
 *  out of range frags because we have already tossed out of range frag's
 *  by now.
 *
 *  This bitmap isn't based on struct vmdmap's, it is just a bunch
 *  of 4k pages, and each page is completely used for the bitmap
 *  (no control info like vmdmaps).  This also has to deal with the
 *  silly fsck scratch file in case we don't have enough memory to keep
 *  everything in memory...
 *
 *      flag:   SET_BITS	mark block allocated
 *           	GET_BITS	get allocation status
 *              CLR_BITS	clear allocation status
 */
int
domap(frag_t	frag,		/* frag to set/get/clear	*/
      uint	flg)		/* set/get/clear		*/
{
	uint	i;
	uint	page;		/* page where frag starts in map      	  */
	uint	word;		/* word where frag starts in map page 	  */
	uint	bit;		/* bit where frag starts in map word 	  */
	uint	rem;		/* bit where frag starts in map page  	  */
	uint	*mp;		/* ptr to 4k map page that describes frag */
	uint	mask;		/* mask used to get at bits in a mapword  */
	BUFAREA *bp = NULL;	/* for scratch file			  */
	uint	nbits;		/* num bits used to describe frag in map  */
				/*    (number of FragSize frags in frag)  */
	uint	wordbits;	/* num frag bits in 1 word of map dword	  */
				/*    (in case frag's allocation state    */
				/*    spans a map word)			  */
	uint	*p;
	int	n;

	nbits = FRAGSIN(frag);

	BPRINTF((DDEBUG, "domap(blk = %x, flg = %d)\n", frag, flg));
	page = frag.addr >> L2BITSPERPAGE;
	if (blkmap)
		mp = (uint *)(blkmap + BLK2BYTE(page));
	else
		if((bp = getblk(page)) == NULL)
			pr(FATAL, MSGSTR(FATIO, FATIO_str));
		else
			mp = (uint *)&bp->b_un.b_buf;
	rem  = frag.addr & BITSPERPAGEMASK;
	word =  rem >> L2BITSPERWORD;
	bit = rem & BITSPERWORDMASK;

        /*
	 *  now we process the first word.
         */
	wordbits = min(BITSPERWORD - bit, nbits);
	p = mp + word;
        for (n = 0; n < wordbits; n++, bit++)
		if (flg == GET_BITS)
		{
			if (*p & (UZBIT >> bit))
				return 1;
		}
		else
		{
			if (bp)
				dirty(bp);
			if (flg == SET_BITS)
				*p |= (UZBIT >> bit);
			else
				*p &= ~(UZBIT >> bit);
		}
        /*
	 *  process the second word.
         */
	wordbits = nbits - wordbits;
	p++;
        for (n = bit = 0; n < wordbits; n++, bit++)
		if (flg == GET_BITS)
		{
			if (*p & (UZBIT >> bit))
				return 1;
		}
		else
		{
			if (bp)
				dirty(bp);
			if (flg == SET_BITS)
				*p |= (UZBIT >> bit);
			else
				*p &= ~(UZBIT >> bit);
		}

	return 0;
}



/*
 * This routine implements the operations on the linkcount table (in core
 *      or on the scratch file).
 *
 *      flag    0       set link count
 *              1       get link count
 *              2       decrement link count
 */
static int
dolncnt(val,flg)
short val;
{
	register short *sp;
	register BUFAREA *bp;

	BPRINTF((DDEBUG, "dolncnt(inum = %d, val = %d, flg = %d)\n", inum, (int) val, flg));

	if(lncntp != NULL) {
		bp = NULL;
		sp = &lncntp[(unsigned)inum];
	}
	else if((bp = getblk((daddr_t)(lncntblk+(inum/Sperb)))) == NULL)
		pr(FATAL, MSGSTR(FATIO, FATIO_str));
	else
		sp = &bp->b_un.b_lnks[(unsigned)inum%Sperb];
	switch(flg) {
		case 0:
			*sp = val;
			break;
		case 1:
			bp = NULL;
			break;
		case 2:
			(*sp)--;
	}
	if(bp != NULL)
		dirty(bp);
	return(*sp);
}

/*
 * This routine is used to read a bloc into a buffer.  If the previous
 * contents of that buffer are dirty, it is flushed first.  If the file to
 * be read is the scratch file, the cache is used to minimize I/O.  No
 * caching is performed for disk I/O - since the pass structure of the
 * program makes caching ineffective anyway.
 *
 * note:  this was changed to handle the scratch file stuff only.  all
 *	  other i/o goes thru bpread() and the rest of the buffer pool
 *	  family.
 */
static BUFAREA *
getblk(blk)
daddr_t blk;
{
	BUFAREA *bp;
	register struct filecntl *fcp;

	BPRINTF((DDEBUG, "getblk(%ld)\n", blk));

	bp = search(blk);
	fcp = &sfile;

	cache_tries++;
	if (bp->b_bno == blk) {
		cache_hits++;
		return(bp);
		}

	flush(fcp, bp);

	if(fbread(fcp, bp->b_un.b_buf, blk) != NO) {
		bp->b_bno = blk;
		return(bp);
	}

	bp->b_bno = (daddr_t)-1;

	return(NULL);
}


/*
 * 	flush
 *
 *	- flush a BUFAREA file (if dirty)
 */
static void
flush(fcp, bp)
struct filecntl *fcp;
register BUFAREA *bp;
{
	if(bp->b_dirty) {
		fbwrite(fcp,bp->b_un.b_buf,bp->b_bno,Bsize);
		bp->b_dirty = 0;
		}
}


/*
 * This routine reports a disk transfer error
 */
static void
rwerr(errtype,blk)
int errtype;
frag_t blk;
{
	/*
	 * This routine supports the following error types:
	 * 1: fbwrite; 2: seek; 3: read; 4: write 5: read (clear in preen mode)
	 * others may be added, but you must also add the corresponding
	 * message to the message catalog and the definitions
	 * please be careful not to construct messages!
	 */
	switch (errtype) {
		case 1: pr(CONTIN,MSGSTR(RWERR1, RWERR1_str),
			(long) BLK2DEVBLK(blk.addr));
			break;
		case 2: pr(CONTIN,MSGSTR(RWERR2, RWERR2_str),
			(long) BLK2DEVBLK(blk.addr));
			break;
		case 3: pr(CONTIN,MSGSTR(RWERR3, RWERR3_str),
			(long) BLK2DEVBLK(blk.addr));
			break;
		case 4: pr(CONTIN,MSGSTR(RWERR4, RWERR4_str),
			(long) BLK2DEVBLK(blk.addr));
			break;
		case 5: pr(PREEN|FATAL, MSGSTR(CLEAR1, CLEAR1_str),
			MSGSTR(RWERR3, RWERR3_str), blk.addr);
			break;
		default: pr(CONTIN,MSGSTR(RWERR9, RWERR9_str),
			(long) BLK2DEVBLK(blk.addr));
			break;
	}
	a_ok = NO;
}

static int
hasdots(char *dirbuf)
{
	struct direct *dirp;

	dirp = (struct direct *)dirbuf;
	if (strcmp(dirp->d_name, "."))
		return 0;
	dirp = (struct direct *)((char *)dirp + dirp->d_reclen);
	if (strcmp(dirp->d_name, ".."))
		return 0;
	return 1;
}

/*
 * This routine checks to see if a file's size is in gross dis-accord
 * with the number of blocks allocated to it.
 */
static void
sizechk(dp)
register DINODE *dp;
{
	register int trouble = 0;
	register long size = dp->di_size;
	fdaddr_t frag;
	int	 lbno;
	int	 rc, numfrags, minfrags;
	char 	 *dirbuf;

	/*
	 *  scheme for checking di_size...
	 *
	 *  numfrags: all frags allocated to file (includes "holes" also)
	 *	      determines the maximum acceptable value for di_size
	 *  minfrags: determines minimum acceptable value for di_size.
	 *
	 *  di_size error conditions
	 *  	1) di_size too big if both:
	 *		a) di_size > bytes in numfrags
	 *		b) ino not a symlink stored completely in inode
	 *	2) di_size too small if di_size < num bytes in minfrags
	 */

	numfrags = holefrags + datablocks;
	minfrags = numfrags - (NOINDIRECT(NUMDADDRS(*dp)) ? 1 : FragPerBlk);

	if (dp->di_size > FRAG2BYTE(numfrags) && !(datablocks == 0 && LNK) ||
	    dp->di_size <= FRAG2BYTE(minfrags))
	{
		pr(0, MSGSTR(FSIZEER, FSIZEER_str), inum);
		setstate(CLEAR);
		trouble++;
		return;
	}

	/* for v4 fs's with fragsize < 4k, make sure di_size
	 * is backed.  fixsize is set in dblock().
	 */
	if (fixsize)
		if (pr(PREEN, MSGSTR(ADJUST, ADJUST_str),
		       MSGSTR(FSIZEER, FSIZEER_str), inum))
		{
			dp->di_size = BLK2BYTE(lastxpt + 1);
			bptouch((void *)dp);
		}
		else
		{
			setstate(CLEAR);
			trouble++;
			return;
		}

	    
	/*
	 *  file is single or double indirect, and numfrags not
	 *  a multiple of frags per block.  (sgl and double indirect
	 *  files only allocate BLKSIZE disk addresss... no frags)
	 */
	if (numfrags > NDADDR*FragPerBlk && numfrags % FragPerBlk && !compress)
	{
		pr(0, MSGSTR(FILENOFRAG, FILENOFRAG_str), inum);
		setstate(CLEAR);
		trouble++;
		return;
	}

	if (dp->di_nblocks != datablocks &&
	    pr(PREEN, MSGSTR(ADJUST, ADJUST_str), MSGSTR(BLKCNTW, BLKCNTW_str),
	       inum))
	{
		BPRINTF((DDEBUG,
			 "datablks=%ld, nblks=%ld, %ld bytes (%ld frags)",
			 datablocks, dp->di_nblocks, size,
			 BYTE2FRAG(size + FragSize - 1)));
		dp->di_nblocks = datablocks;
		bptouch((void *)dp);
	}

	/*
	 * check for empty dirs, dirs that are too small,
	 * and weird size dirs
	 */
	if (DIR)
	{
		if (dp->di_size == 0)
		{
			if (direrr(PREEN,MSGSTR(ZEROLDIR, ZEROLDIR_str),
					MSGSTR(REMOVE, REMOVE_str))==YES)
				zapino(dp);
			a_ok = NO;
			trouble++;
		}
		else if (dp->di_size < EMPTDIR)
		{
			if (direrr(PREEN, MSGSTR(DIRSHRT, DIRSHRT_str),
					MSGSTR(FIX, FIX_str)) == YES)
			{
				dp->di_size = EMPTDIR;
				if (((frag.d = dp->di_rdaddr[0]) == 0) ||
				 ((dirbuf = (char *)bpread(frag.f)) == NULL) ||
				    ! hasdots(dirbuf))
					zapino(dp);
				bptouch((void *)dp);
			}
			trouble++;
		}
		else if (dp->di_size % DIRBLKSIZ != 0)
		{
			pr(0, MSGSTR(DIRLENM, DIRLENM_str),
			   pathname, dp->di_size, DIRBLKSIZ);
			if( pr(PREENN, MSGSTR(ADJUST, ADJUST_str), "") == YES)
			{
				dp->di_size = roundup(dp->di_size, DIRBLKSIZ);
				bptouch((void *)dp);
			}
			trouble++;
		}
	}

	/*
	 * if trouble reported, tell us which path it is later...
	 */
	if (trouble && numsinos < MAXSINOS)
	{
		sinos[numsinos].inum = inum;
		sinos[numsinos++].frag.d = 0;
	}
}

/*
 * This routine is used to flush out buffers and close files at the
 * end of a check.
 */
static void
ckfini()
{
	numsblks = numsinos = 0;

	/*
	 * close down buffer pool
	 */
	bpclose();

#ifndef STANDALONE
	close(sfile.rfdes);
	close(sfile.wfdes);
	sync();         /* to flush out any delayed writes      */
#endif
}

/*
 * This routine is called by pr to display information about a particular
 * I-node.  It lists the I-number, owner, mode, size and time of last
 * modification.
 */
static void
pinode()
{

	register char *p;
	register DINODE *dp;
	register struct passwd *pw;
	struct tm *timestr;
	char tbuf [NLTBMAX];

	printf(" I=%u ", inum);

	if((dp = ginode()) != NULL) {
		printf(MSGSTR(OWNER, OWNER_str));
		if( pw = getpwuid((uid_t)dp->di_uid) )
			printf("%s ", pw->pw_name);
		else
			printf("%d ", dp->di_uid);

		timestr = localtime((time_t *)&dp->di_mtime);
		strftime (tbuf, NLTBMAX, "%sD %sT %Y", timestr);
		printf (MSGSTR(MTIMEM, MTIMEM_str),
			dp->di_mode, dp->di_size, tbuf, timestr);

		bprelease((void *)dp);	/* free inode */
		}
}


typedef enum { NOT_ASKED, DO_SALVAGE, DONT_SALVAGE } salvage_t;

#define	CHECKSALVAGE()							\
	if (salvage == NOT_ASKED) {					\
		if( pr(PREENN, MSGSTR(SALV, SALV_str), MSGSTR(BADFMAP, BADFMAP_str), mapname) ) {	\
			pr(QUIET, MSGSTR(PHASEX, PHASEX_str), phase, mapname); \
			salvage = DO_SALVAGE;				\
			}						\
		else							\
			salvage = DONT_SALVAGE;				\
		}

#define	SALVAGE()	(salvage == DO_SALVAGE)

/*
 *
 *	map_chk
 *
 *	- compare an allocation map against fsck's internal table
 *	- fix it
 *	- (note: indentation is 4 spaces for this function)
 *
 *	- main idea here is to make one pass thru the the allocation map.
 *	  if we find something broke, ask to fix it.  if we're given the
 *	  ok, fix the rest of the map as we find broken pieces...
 *
 */
static int
map_chk(phase, mapname, mapinode)
int	 phase;			/* which fsck phase this is ...		*/
char	*mapname;		/* name of map for descriptive purposes	*/
ino_t	 mapinode;		/* inode of map file			*/
{
    DINODE *dp;
    salvage_t salvage;
    struct vmdmap *vm, *vm1, *vm0;
    uint tmptree[sizeof(vm->tree)/sizeof(vm->tree[0])];	/* temporary vm tree */
    uint totbits, pages, page, bitno;
    unsigned int pword, wword, bit, lbit;
    int w, fw, freed, ag, agfree, wperag, totfree, mapfree, totags, rem;
    int clustersz, mapvers, agrsize, nblocks, nag, nb, fixup = 0;
    uint *wmap, *pmap, totpages, np, dbperpage;

    vm = vm1 = vm0 = NULL;
    totfree = 0;	/* how many free items we've found	*/

    pr(QUIET, MSGSTR(PHASEXX, PHASEXX_str), phase, mapname);

#ifdef DEBUG
/*	    dumpbuf(blkmap, 0, bmapsz / 4);*/
#endif

    /* salvage is used in order to not repeatedly ask the user whether to fix
     * the map.  NOT_ASKED = "not asked yet", DONTSALVAGE = "asked and user
     * said don't salvage", SALVAGE = "asked and user said salvage"
     */
    salvage = NOT_ASKED;

    /* read the map inode ...
     */
    inum = mapinode;
    if((dp = ginode()) == NULL) 
    {
        pr(FATAL, MSGSTR(IMAPER, IMAPER_str), mapname);
        goto out;
    }

    /* read in the first page of the map ...
     */
    if ((vm = readvmd(dp, 0, mapname)) == NULL)
	goto out;

    /* Use the sizes calculated from the superblock since it has a backup.
     * If we attempted to use the summary info in the first page of map, 
     * and that page was unreadable then we'd never have the possibility 
     * of fixing the map. 
     */
    if (mapinode == DISKMAP_I)
    {
	totbits = fmax;
	totags = totbits / sbagsize;
	rem = totbits % sbagsize;
	if (rem >= (sbiagsize * sizeof(struct dinode) / FragSize))
		totags++;
    }
    else /* Inode map */
    {
	totbits = (imax + 1); 
	totags = totbits / sbiagsize;
    }

    /* Fix total allocation group count.  Only recorded in first page.
     */
    if (vm->totalags != totags)
    {
	CHECKSALVAGE();
	if (SALVAGE())
	{
	   vm->totalags = totags;
	   fixup++;
	}
     }

     /* Fix map size.  Only recorded in first page.
      */
     if (vm->mapsize != totbits)
     {
	CHECKSALVAGE();
	if (SALVAGE())
	{
	   vm->mapsize = totbits;
	   fixup++;
	}
    }

    nblocks = totbits;
    mapfree = vm->freecnt;
    mapvers = sbiagsize == sbagsize && FragSize == PAGESIZE ?
	    ALLOCMAPV3 : ALLOCMAPV4;
    
    dbperpage = WBITSPERPAGE(mapvers);
    totpages = pages = (totbits + dbperpage - 1) / dbperpage;
    if (mapvers == ALLOCMAPV4)
	    totpages += (pages + 7)/8;

    /* Consistency check map pages against size in directory inode.  Map
     * inode errors are fatal.
     */
    if (BYTE2BLK(dp->di_size) < totpages)
    {
	    pr(FATAL, MSGSTR(MAPINC, MAPINC_str), mapname, 
		BYTE2BLK(dp->di_size), pages);
	    goto out;
    }

    BPRINTF((DDEBUG, "  --> %s Map: %ld bits, %ld pages,  %ld free",
        	mapname, totbits, pages,  mapfree));
    /*
     * loop thru each page...
     */
    agrsize = (mapinode == INOMAP_I) ? sbiagsize : sbagsize;
    clustersz = mapvers ? CLDEFAULTV4 : CLDEFAULT;
    bprelease(vm);
    vm = NULL;
    bitno = 0;		/* which bit (item) we're looking at	*/
    for (page = 0; page < pages; page++)
    {
	/*
         * read the next page...  vm1 will point to control info,
	 * vm will point to the block containing bit-maps, and 
	 * wmap and pmap to the bit-maps.
         */
	if (mapvers == ALLOCMAPV3) 
	{
		if (vm != NULL) bprelease(vm);
		if ((vm = readvmd(dp, page, mapname)) == NULL) 
		{
			totfree = 0;
			goto out;
		}
		wmap = (uint *)(vm) + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
		vm0 = vm1 = vm;
	}
	else 
	{
		/* read control page ? */
		if ((page & 0x7) == 0) 
		{
			if (vm0 != NULL) bprelease(vm0);
			np = 9*(page >> 3);
			if ((vm0 = readvmd(dp, np, mapname)) == NULL) 
			{
				totfree = 0;
				goto out;
		
			}
		}
		vm1 = (struct vmdmap *) ((char *)vm0 + ((page & 0x7) << 9));
		/* read in bit-map page */
		if (vm) bprelease(vm);
		np = 9*(page >> 3) + 1 + (page & 0x7);
		if ((vm = readvmd(dp, np, mapname)) == NULL) 
		{
			totfree = 0;
			goto out;
		}
		wmap = (uint *)vm;
		pmap = wmap + WPERPAGEV4;
	}


	/* check out map's version number
	 */
	 if (vm1->version != mapvers)
	 {
		pr(DDEBUG, MSGSTR(MAPBADVERS, MAPBADVERS_str), page, 
			vm1->version);
		CHECKSALVAGE();
		if (SALVAGE())
		{
		   vm1->version = mapvers;
		   fixup++;
		}
	 }

	 /*  check out map's agsize
	  */
	 if (vm1->agsize != agrsize)
	 {
		printf("map agsize bad, agrsize = %d\n", agrsize);
		pr(DDEBUG, MSGSTR(MAPBADAGSIZE, MAPBADAGSIZE_str), page, 
			vm1->agsize);
		CHECKSALVAGE();
		if (SALVAGE())
		{
		   vm1->agsize = agrsize;
		   fixup++;
	        }
	}

	/* check out map's clsize and clmask
	 */
	if (vm1->clsize != clustersz)
	{
		pr(DDEBUG, MSGSTR(MAPBADCLSIZE, MAPBADCLSIZE_str), page, 
		  	vm1->clsize);
	        CHECKSALVAGE();
	        if (SALVAGE())
	        {
		   if (mapvers == ALLOCMAPV3)
			vm1->clmask = (ONES << (8 - clustersz)) & 0xFF;
		   vm1->clsize = clustersz;
		   fixup++;
	        }
	}

	/* Calculate number of allocation groups for this page, then check
	 * the agcnt in the summary information.
	 */
	nb = MIN(nblocks, dbperpage);
	nblocks -= nb;
	nag = nb / agrsize;
	if (mapinode == DISKMAP_I)
	{
		rem = nb % agrsize;
		if (rem >= (sbiagsize * sizeof(struct dinode) / FragSize))
			nag++;
	}

	if (vm1->agcnt != nag)
	{
		pr(DDEBUG, "    --> page %ld has %d allocgroups ", page, 
			vm1->agcnt);
	        CHECKSALVAGE();
	        if (SALVAGE())
	        {
		   vm1->agcnt = nag;
		   fixup++;
	        }
	}
	
        /*
         * check each allocation group on this page.
         */
        for (ag = 0; ag < vm1->agcnt; ag++) {

            agfree = 0;
	    wperag = agrsize >> L2DBWORD;
	    fw = ag*wperag;

            /* check each word in this alloc group
             */
            for (w = fw; w < wperag + fw; w++) 
  	    {
                pword = pmap[w];	/* permanent word */
                wword = wmap[w];	/* working word	*/
                /*
                 * check each bit in the permanent word ...
                 */
                for (bit = UZBIT; bit && bitno < totbits; bit >>= 1, bitno++) 
		{
                    /*
                     * see if our fsck thinks this item is free ...
                     */
		    if (mapinode == INOMAP_I)
			    freed = inodefree(bitno);
		    else
			    freed = blockfree(bitno);
		    
                    if (pword & bit) {    /* pmap says allocated */
                        if (freed) {
			    pr(DDEBUG, MSGSTR(ALLOCATED, ALLOCATED_str), 
				mapname, bitno);
                            /*
			     * if the pmap says allocated, but we didn't
			     * see it allocated, fix both the working and
			     * permanent maps to reflect that the item is free
			     */
                            CHECKSALVAGE();
                            if (SALVAGE()) {
                                pmap[w] &= ~bit;
                                wmap[w] &= ~bit;
				agfree++;
				fixup++;
                                }
                            }
                        else if (!(wword & bit)) {
			    /*
			     * if the permanent map indicates allocated,
			     * but the working map indicates free, fix
			     * the working map...
			     */
			    pr(DDEBUG, MSGSTR(P_ALLOCATED, P_ALLOCATED_str), 
				mapname, bitno);

                            CHECKSALVAGE();
                            if (SALVAGE()) {
                                wmap[w] |= bit;
				fixup++;
                                }
                            }
                        }

                    else {            /* pmap says the item is free    */
                        if (!freed) {
			    /*
			     * if the pmap says the item is free, but we
			     * think it's allocated, fix both maps...
			     */
		   	    pr(DDEBUG, MSGSTR(UNALLOCATED, UNALLOCATED_str), 
				mapname, bitno);

                            CHECKSALVAGE();
                            if (SALVAGE()) {
                                pmap[w] |= bit;
                                wmap[w] |= bit;
				fixup++;
                                }
                            }
                        else
			    /*
			     * track number of free items in this ag    
			     */
                            agfree++;
                        } 
                 } 
            } 

            pr(DDEBUG, "      --> %s Map/pg %ld/ag %d: %d free; %d counted",
		mapname, page, ag, vm1->agfree[ag], agfree);

            totfree += agfree;        /* update total free count */

            /*
             * check agfree count against map
             */
            if (vm1->agfree[ag] != agfree) {
                CHECKSALVAGE();
                if (SALVAGE()) {
                    vm1->agfree[ag] = agfree;
		    fixup++;
                    }
                }
            } 

	/* rebuild a tmp tree.
	 */
	fixuptree(wmap, tmptree, mapvers);

	/* if map page ok, compare tmp tree against map tree.
	 */
	if (!fixup &&
	    memcmp((void *) vm1->tree, (void *) tmptree, sizeof(tmptree))) {
		if (sizeof(tmptree) != sizeof(vm1->tree))
			pr(FATAL, MSGSTR(MISMATTREE, MISMATTREE_str));

		CHECKSALVAGE();
		if (SALVAGE())
			fixup++;
		}

	/* rebuild map tree using tmp tree we built above
	 */
	if (fixup) {
		memcpy((void *) vm1->tree, (void *) tmptree, sizeof(tmptree));
		bptouch((void *)vm);	/* make sure this page gets updated */
		bptouch((void *)vm0);	/* make sure this page gets updated */
		}
    } 

    pr(DDEBUG, "  --> %s Map: %d free; %d counted", mapname, mapfree, totfree);

    /* check total free count against map free count ...
     */
    if (totfree != mapfree) 
    {
        CHECKSALVAGE();
        if (SALVAGE()) 
	{
    	    if (vm) 
		bprelease (vm);

            if ((vm = readvmd(dp, 0, mapname)) == NULL)
		totfree = 0;
	    else 
	    {
                vm->freecnt = totfree;
	        bptouch((void *)vm);
            }
	}
     }

out:			/* cleanup and return */
    if (vm)
	bprelease((void *)vm);
    if (vm0 != NULL && vm0 != vm)
	bprelease((void *)vm0);
    if (dp)
	bprelease((void *)dp);

    return (totfree);
}

/*
 * fixuptree(vm)
 * recomputes  the tree for a page of a vmdmap
 */
int
fixuptree(wmap, tree, version)
int *wmap;
uint tree[];
int version;
{
	uint k, wperpage;

	/* initialize dmap table if necessary
	 */
	if (dmaptab[0] == 0)
		idmaptab();

	/* init tree to zero.
	 */
	for (k = 0; k < TREESIZE; k++)
		tree[k] = 0;

	/* recompute tree. loop increment is 2 because
	 * each leaf-word covers 2 words for each of its bytes.
	 */
	wperpage = (version == ALLOCMAPV3) ? WPERPAGE : WPERPAGEV4;
	for (k = 0; k < wperpage; k += 2)
		updtree(wmap ,k, tree, version);
		
	return 0;
}


/*
 *  return length of longest 0-bit string in 64bits (2 map words)
 */
static int
freelen(unsigned int *mapword)
{
	unsigned int cmax, i, max, mask;

	for (i = max = cmax = 0; i < 2; i++, mapword++)
		for (mask = 1 << 31; mask; mask >>= 1)
			if (*mapword & mask)
			{
				if (cmax > max)
					max = cmax;
				cmax = 0;
			}
			else
				cmax++;
	return max > cmax ? max : cmax;
}
/*
 * update vmdmap tree.
 *
 * input parameters:
 *		wmap - pointer to working-bit map of a page.
 *		ind - index of the word which changed.
 */
static int
updtree(int 		*wmap,
	int 		ind,
	uint		tree[],
	int		version)
{
	uint n, lp, k,index, maxc, max; 
	uchar *cp0, *cp1;
	struct vmdmap * p0;

	/* calculate max for the two words of the section
	 */
	ind = ind & (ONES << 1);
	cp0 = (uchar *)(wmap + ind);

	switch(version)
	{
	case ALLOCMAPV3:
		max = 0;
		p0 = (struct vmdmap *) (wmap - LMAPCTL/4);
		maxc = p0->clmask;
		for(n = 0; n < 8; n++, cp0++)
		{
			max = MAX(max, dmaptab[*cp0]);
			if (max >= maxc)
			{
				max = maxc;
				break;
			}
		}
		break;
	case ALLOCMAPV4:
		max = freelen((uint *)cp0);
		break;
	default:
		pr(FATAL, MSGSTR(INCOMPMAP, INCOMPMAP_str));
		break;
	}

	/* calculate pointers to leaf word and to the character
	 * in it that corresponds to ind; leaf words cover 8
	 * sections (32 bytes of map) so the low order 3-bits 
	 * of ind are shifted right by one.
	 */
	lp = LEAFIND + (ind >> 3);
	cp0 = (uchar *) &tree[lp];
	cp1 = cp0 + ((ind & 0x7) >> 1);

	/* there are at most four levels of the tree to process.
	 */
	for (k = 0; k < 4; k++)
	{
		/* if old max is same as max, nothing to do.
	 	 */
		if (*cp1 == max)
			return 0;

	 	/* set the value of the new maximum.
		 * get the maximum in the word after change
	 	 */
		*cp1 = max;
		max = treemax(cp0);

		/* get parent of lp.
		 * parent covers four words so division is by 4.
		 * calculate pointers to word and character.
		 */
		index = (lp - 1) & 0x3;
		lp = (lp - 1) >> 2;
		cp0 = (uchar *) &tree[lp];
		cp1 = cp0 + index;
	}
	return 0;
}

/*
 * treemax(cp0)
 *
 * returns the maximum sequence in a tree word.
 * cp0 points to first character of the word.
 */
static int
treemax(cp0)
uchar * cp0;
{
	uint max1, max2;
	max1 = MAX(*cp0, *(cp0+1));
	max2 = MAX(*(cp0+2), *(cp0+3));
	return (MAX(max1,max2));
}

/*
 * initializes dmaptab.
 */
static int
idmaptab()
{
	unsigned int mask, tmask;
	int k,n,j,shift;

	dmaptab[255] = 0;
	dmaptab[0] = ONES;

	/* other than  0 and 255, all have sequences of zeros
	 * between 1 and 7.
	 */
	for (k = 1; k < 255; k++)
	{
		for (n = 7; n > 1; n--)
		{
			shift = 8 - n;
			mask = (ONES << shift) & 0xff;
			tmask = mask;
			for (j = 0; j <= shift; j++)
			{
				if ((tmask & k) == 0)
					goto next;
				tmask = tmask >> 1;
			}
		}
		next : dmaptab[k] = (j <= shift) ? mask : 0x80;
	}

}

/*
 *
 *	inodefree
 *
 *	- return 1 if inode 'inode' is free, else 0
 */
static int
inodefree(inode)
daddr_t inode;
{
	int state;

	inum = inode;

	return ((state = getstate()) == CLEAR || state == USTATE);
}

/*
 *
 *	blockfree
 *
 *	- return 1 if block 'block' is free, else 0
 *
 */
static int
blockfree(daddr_t blk)
{
        frag_t frag;

        frag.new = 0;
        frag.nfrags = FragPerBlk - 1;
        frag.addr = blk;
        return !getbmap(frag);
}

/*
 *	readvmd
 *
 *	- read in a map page # 'logical_block' off of inode 'dp'
 *
 */
static struct vmdmap *
readvmd(dp, logical_block, mapname)
DINODE	*dp;
daddr_t	 logical_block;
char	*mapname;
{
	struct vmdmap *vm;
	frag_t phys_block;

	vm = NULL;	/* return NULL if anything fails	*/

	if (bmap(dp, logical_block, &phys_block) < 0)
		pr(FATAL, MSGSTR(BLK1MAP, BLK1MAP_str), mapname);

	else if ((vm = (struct vmdmap *) bpread(phys_block)) == NULL)
		pr(FATAL, MSGSTR(PHYBLK, PHYBLK_str), phys_block, mapname);

	return (vm);
}

/*
 * This routine is part of the cache management code.  It searches
 * the buffer pool for a buffer containing the desired block.  If it
 * can't find it, it stops at the last block.  The block is removed
 * from the list, reinserted at the front, and returned to the caller.
 */
static BUFAREA *
search(uint blk)
{
	register BUFAREA *pbp, *bp;

	for(bp = (BUFAREA *) &poolhead; bp->b_next; ) {
		pbp = bp;
		bp = pbp->b_next;
		if(bp->b_bno == blk)
			break;
	}
	pbp->b_next = bp->b_next;
	bp->b_next = poolhead;
	poolhead = bp;
	return(bp);
}

/*
 * This pass function checks a specified directory entry to see if it
 * has a specified name.  If it does, it stops the pass and sets parentdir
 * to the associate I-node number.
 */
static int
findino(dirp)
register DIRECT *dirp;
{
	if (dirp->d_ino != 0 && strcmp(dirp->d_name, srchname) == 0) {
		if(dirp->d_ino >= ROOTDIR_I && dirp->d_ino <= imax)
			parentdir = dirp->d_ino;

		return(STOP);
		}

	return(KEEPON);
}


/*
 *	allocfrag
 *		allocate frag of len FragPerBlk - frag.nfrags
 *	return:
 *		0   :	frag allocated
 *		-1  :	error: invalid nfrags, or failed getting map page
 *		1   :	frag can't be allocated... no space
 */
int
allocfrag(frag_t	*frag,	/* frag.nfrags tells us how much	*/
	  uint		hint)	/* map bit where we start the search	*/
{
	uint	i;		/* map page counter			*/
	uint	*mp;		/* ptr to map page			*/
	int	pgbit;		/* bit in map page where frag starts	*/
	uint 	fraglen;	/* number of frags in frag		*/
	uint	page;		/* page offset in fragmap		*/
	BUFAREA *bp;		/* silly scratch file...  ARGH!		*/

	if (frag->nfrags >= FragPerBlk)
		return -1;
	fraglen = FRAGSIN(*frag);
	for (i = 0, page = (hint >> L2BITSPERPAGE);
	     i <= totmpages;
	     i++, page = (page + 1) % totmpages)
	{
		/*
		 *  get map page for mapsearch()
		 */
		if (blkmap == NULL)
		{
			if((bp = getblk(page)) == NULL)
				pr(FATAL, MSGSTR(FATIO, FATIO_str));
			mp = (uint *)(bp->b_un.b_buf);
		}
		else
			mp = (uint *)(blkmap + BLK2BYTE(page));
		/*
		 *  hint is a the map bit where we start our search.
		 *  on the first time we call mapsearch, pass hint
		 *  as a bit offset relative to the beginning of this
		 *  map page.  if this map page can't satisfy the allocation
		 *  request, call mapsearch with hint = 0.
		 */
		if ((pgbit = mapsearch(mp, fraglen, hint & BITSPERPAGEMASK))
		    >= 0)
		{
			frag->new = 0;
			frag->addr = ((page << L2WORDPERPAGE) << L2DBWORD) +
				pgbit;
			setbmap(*frag);
			n_blks += fraglen;
			if (blkmap == NULL)
				dirty(bp);
			return 0;
		}
		else
			if (hint)
				hint = 0;
	}
	return 1;
}


/*
 *  "grow" directory by 1 frag, if it wants to grow from direct to single
 *	indir return error, just like original code... for now
 */
static int
growdir(struct dinode *dp)
{
	uint		numdaddrs;
	int		rc;
	fdaddr_t	oldfrag, newfrag;
	blkbuf		*oldblk = NULL;

	oldfrag.d = 0;
	numdaddrs = NUMDADDRS(*dp);
	if (numdaddrs && bmap(dp, numdaddrs - 1, &oldfrag.f))
		return GROW_FAIL;

	/*
	 *  if nfrags of lastblk not 0, then the block can be expanded
	 *  without increasing the number of disk addresses in inode.
	 */
	newfrag.f.new = 0;
	if (oldfrag.f.nfrags)
	{
		if ((oldblk = (blkbuf *)bpread(oldfrag.f)) == NULL)
			return GROW_FAIL;
		newfrag.f.nfrags = oldfrag.f.nfrags - 1;
	}
	else
	{
		newfrag.f.nfrags = FragPerBlk - 1;
		numdaddrs++;
	}

	if (!NOINDIRECT(numdaddrs))
	{
		if (oldblk)
			bprelease(oldblk);
		return GROW_INOFULL;
	}

	/*
	 *  clear old frag in our map and use oldfrag.f.addr
	 *  as the hint (cheap in-place expansion)
	 */
	if (oldblk)
		clrbmap(oldfrag.f);

	if (rc = allocfrag(&newfrag.f, oldfrag.f.addr))
	{
		if (oldblk)
		{
			setbmap(oldfrag.f);
			bprelease(oldblk);
		}
		return rc == 1 ? GROW_FSFULL : GROW_FAIL;
	}
	BPRINTF((DDEBUG, "growdir: of= %8.8x nf=%8.8x \n", oldfrag.f,
		 newfrag.f));
	if (oldblk)
	{
		if (bpexpand(oldblk, newfrag.f) == NULL)
		{
			clrbmap(newfrag.f);
			setbmap(oldfrag.f);
			bprelease(oldblk);
			return GROW_FAIL;
		}
		bprelease(oldblk);
	}
	bptouch((void *)dp);
	dp->di_rdaddr[numdaddrs - 1] = newfrag.d;
	dp->di_nblocks++;
	return SUCCESS;
}



static int
expanddir(dp, ino, name, parentino)
struct dinode	*dp;		/* inode of directory to expand		*/
ino_t		 ino;		/* inode number of directory		*/
char		*name;		/* name of directory 			*/
ino_t		 parentino;	/* inode number of parent directory	*/
{
	int new = 0;		/* new filesystem block?	*/
	int spot, rc;
	char *dirblk;
	daddr_t lastbn;
	fdaddr_t frag, newblk;

	lastbn = NUMDADDRS(*dp);
	/*
	 * directories can't have holes so, if FRAG2BYTE(di_nblocks) ==
	 * di_size, then the dir is full, and we need to allocate another
	 * frag.
	 */
	if (FRAG2BYTE(dp->di_nblocks) == dp->di_size)
	{
		/*
		 * but at least for now, we'll only allocate a direct
		 * block.  no indirect blocks yet.
		 */
		if (rc = growdir(dp))
		{
			switch(rc)
			{
			case GROW_INOFULL:
				pr(0, MSGSTR(NODIRECT, NODIRECT_str), name);
				break;
			case GROW_FSFULL:
				pr(0, MSGSTR(CANTEXP, CANTEXP_str), name);
				break;
			default: 
				pr(0, MSGSTR(CANTEXPDIR, CANTEXPDIR_str),
				   name);
			}
			return 0;
		}
	}
	/*
	 * update size
	 */
	dp->di_size += DIRBLKSIZ;
	lastbn = NUMDADDRS(*dp) - 1;

	/*
	 * read the block in...
	 */
	if (bmap(dp, lastbn, &newblk.f) < 0)
	{
		pr(0, MSGSTR(CNTMAPB, CNTMAPB_str), lastbn);
		return (0);
	}
	if ((dirblk = (char *)bpread(newblk.f)) == NULL)
		return (0);

	/*
	 * empty (i.e. new) directory.  create entries for "." and ".." ...
	 */
	if (dp->di_size == DIRBLKSIZ)
	{
		initfiles[0].inum = ino;
		initfiles[1].inum = parentino;
		initdirblk(dirblk, initfiles);
		spot = DIRBLKSIZ;
	}
	else
		spot = dp->di_size & BLKMASK;

	/*
	 * initialize new blocks...
	 */
	for (; spot < FRAGLEN(newblk.f); spot += DIRBLKSIZ)
		initdirblk(&dirblk[spot], (struct empty *) NULL);

	bptouch((void *)dirblk);	/* dirty directory block	*/
	bprelease((void *)dirblk);	/* free up directory block	*/

	bptouch((void *)dp);		/* dirty the inode		*/
	return (1);
}



/*
 *  returns bit in map page where frag of length <fraglen> starts
 *  returns -1 if frag can't be allocated from this page
 */
static int
mapsearch(uint *mp,	/* our internal diskmap page (not vmdmap) 	*/
	  uint fraglen,	/* number of contiguous frags to allocate	*/
	  uint hint)	/* map bit where we start looking		*/
{
	uint w;		/* word offset in map page			*/
	uint mw;	/* map word that <w> references			*/
	uint bit;	/* current bit offset in map double word	*/
	uint mask;	/* bit mask to look at <bit>			*/
	uint nbits;	/* cur length of 0-bit string in double word	*/

	/*
	 *  all frags must be fully allocated with in a map double word
	 */
	for (w = hint >> L2BITSPERWORD, mw = *(mp + w); w < WORDPERPAGE;)
	{
		/*
		 *  if w is odd, then we are starting our search in the
		 *  2nd word of the double word, so set bit to 32.
		 *  (this can only happen on the first time through)
		 */
		bit = (w & 1) ? BITSPERWORD : 0;
		nbits = 0;

		/*
		 *  loop over bits in the double word.  if we find a
		 *  0-bit str of length <fraglen>, return its bit offset
		 */
		do
		{
			for (mask = UZBIT; mask; mask >>= 1, bit++)
				if (mw & mask)
					nbits = 0;
				else
					if (++nbits == fraglen)
						return bit + 1 - fraglen +
							((w & ~1) << L2DBWORD);
			mw = *(mp + ++w);
		}
		while (w & 1);
	}
	return -1;
}



/*
 *	freeblk
 *
 *	- mark a block as unallocated
 */
static void
freeblk(frag_t frag)
{
	clrbmap(frag);	/* mark as free		*/
}

/*
 *	allocino
 *
 *	- allocate and initialize a generic inode
 *
 */
static ino_t
allocino(mode)
mode_t mode;
{
	register ino_t ino;
	ino_t save_inum = inum;
	register DINODE	*dp;

	/*
	 * find a free inode
	 */
	for (ino = LAST_RSVD_I + 1; ino <= imax && !inodefree(ino); ino++)
		;

	/*
	 * none free?
	 */
	if (ino > imax)
		ino = 0;

	else {
		/*
		 * found one.  mark it as allocated.
		 */
		inum = ino;
		setstate(FSTATE);

		/*
		 * read and init the inode itself
		 */
		if ((dp = ginode()) == NULL) {
			pr(0, MSGSTR(ALLOCINO, ALLOCINO_str), inum);
			ino = 0;
			}
		else {
			memset((void *) dp, 0, (size_t)sizeof(*dp));

			/*
			 * initialize 'generic' parts (right now allocino
			 * is just called from allocdir, but Berkeley was
			 * calling it from somewhere else also...  hence
			 * the generic inode allocation)
			 */
			time((time_t *)&dp->di_ctime);
			dp->di_nlink = 1;
			dp->di_uid = geteuid();
			dp->di_gid = getegid();
			dp->di_mtime = dp->di_ctime;
			dp->di_atime = dp->di_ctime;
			dp->di_gen = dp->di_ctime;
			dp->di_mode = mode;

			bptouch((void *)dp);
			bprelease((void *)dp);
			}
		}

	inum = save_inum;

	return (ino);
}

/*
 *	freeino
 *
 *	- free an previously allocated inode
 */
static void
freeino(ino)
ino_t	ino;
{
	ino_t save_inum = inum;
	register DINODE	*dp;

	inum = ino;
	if ((dp = ginode()) != NULL) {
		n_files--;
		pfunc = pass4;
		ckinode(dp, ADDR);
		zapino(dp);
		bptouch((void *)dp);
		bprelease((void *)dp);
		}

	inum = save_inum;
}

/*
 *	initdirblk
 *
 *	- initialize a directory block
 *
 */

static void
initdirblk(blk, files)
char	 	 	*blk;		/* block to init		*/
register struct empty	*files;		/* files (if any) to add	*/
{
	register int reclen;
	register DIRECT *dirp;

	/*
	 * reclen = dirp->d_reclen of current (& last) entry.
	 * set to 0 in case 'files' is NULL and loop doesn't execute
	 */
	reclen = 0;

	dirp = (DIRECT *) blk;
	dirp->d_ino = 0;

	/*
	 * walk thru, filling in directory entries
	 */
	for ( ; files != NULL && files->name != NULL; files++) {
		dirp = (DIRECT *) ((char *) dirp + reclen);
		dirp->d_ino = files->inum;
		strcpy(dirp->d_name, files->name);
		dirp->d_namlen = strlen(dirp->d_name);
		reclen = dirp->d_reclen = LDIRSIZE(dirp->d_namlen);
		}

	/*
	 * fill in last entry with remaining bytes left in block
	 */
	dirp->d_reclen = DIRBLKSIZ - reclen;
}

/*
 * This pass function tries to find an empty slot in a directory,
 * and when it does it adds creates a new link to the orphaned file.
 */
static int
mkentry(dirp)
register DIRECT *dirp;
{
	DIRECT newent;
	int newlen, oldlen;
	DIRECT *save = dirp;

	/*
	 * compute record length of new entry
	 */
	newent.d_namlen = strlen(new_name);
	newlen = LDIRSIZE(newent.d_namlen);

	if (dirp->d_ino != 0)
		oldlen = LDIRSIZE(strlen(dirp->d_name));
	else
		oldlen = 0;

	/*
	 * see if it will fit...
	 */
	if (dirp->d_reclen - oldlen < newlen)
		return (KEEPON);		/* no, won't fit */

	newent.d_reclen = dirp->d_reclen - oldlen;	/* new record length */
	dirp->d_reclen = oldlen;	/* reset old record length	*/

	/*
	 * setup new entry
	 */
	dirp = (struct direct *)(((char *)dirp) + oldlen);
	dirp->d_ino = new_inum;
	dirp->d_reclen = newent.d_reclen;
	dirp->d_namlen = newent.d_namlen;
	strcpy(dirp->d_name, new_name);

	return (ALTERD|STOP);
}

/*
 * This routine attempts to reconnect an unreferenced file into the lost
 * and found directory.  If we have not yet located the lost+found directory,
 * we search for it.  In the event that the unreferenced file was a directory,
 * we must also change its ".." pointer to point to lost+found and adjust the
 * reference count on lost+found accordingly.
 */
static int
linkup()
{
	int state;
	register DINODE *dp;
	register lostdir;
	char name[MAXNAMLEN+1];
	register ino_t pdir, save_orphan;

	if((dp = ginode()) == NULL)
		return(NO);

	lostdir = DIR;
	pdir = parentdir;

	/* see if it is worth saving */
	pr(PINODE|NOE, MSGSTR(UNREFS, UNREFS_str),
		lostdir ? MSGSTR(DIRWORD, DIRWORD_str) : MSGSTR(FILE, FILE_str));
	if (preen && (FIFO || SPECIAL || (dp->di_size == 0))) {
		pr( NOB, MSGSTR(NODATA, NODATA_str) );	
		bprelease((void *)dp);
		return(NO);
	}

	bprelease((void *)dp);

	if ( !pr(PREEN|NOB, MSGSTR(RECON, RECON_str), Nulstr) )
		return(NO);

 	/*
	 * after this point, the user has told us to save the unreferenced
	 * file or dir by putting it in lost+found.  If for some reason
	 * we can't put it there, return LF_FAIL not NO.  This will cause
	 * us not to zap the file.
	 */
	save_orphan = orphan = inum;

	/* find the lost+found directory, if we haven't already */
	if((lfdir = lost_found()) == 0)
		return (LF_FAIL);

	orphan = inum = save_orphan;

	/* find a free slot in lost+found and enter the orphan  */
	inum = lfdir;
	if((dp = ginode()) == NULL || !DIR || getstate() != FSTATE) {
		inum = orphan;
		pr(ERROR, MSGSTR(NOLFD, NOLFD_str));
		if (dp != NULL)
			bprelease((void *)dp);
		return(LF_FAIL);
	}

	bprelease((void *)dp);

	sprintf(name, "%u", orphan);

	if (makeentry(lfdir, lfname, orphan, name) == 0)
		return(LF_FAIL);

	inum = orphan;

	declncnt();

	/* change .. in an orphan directory to point from old
	 * directory to lost+found      */
	if(lostdir) {
		ino_t save_parentdir = parentdir;

		/*
		 * find out who .. was
		 */
		pfunc = findino;
		srchname = "..";
		parentdir = 0;
		dp = ginode();

		ckinode(dp, DATA);
		bprelease((void *) dp);

		if (parentdir) {
			/*
			 * decrement their link count
			 */
			inum = parentdir;
			if ((dp = ginode()) != NULL) {
				if (dp->di_nlink > 0) {
					dp->di_nlink--;
					setlncnt(getlncnt()-1);
					}
				bptouch((void *)dp);
				bprelease((void *)dp);
				}
			}

		/*
		 * change .. to be lost+found...
		 */
		pfunc = chgino;
		parentdir = lfdir;
		srchname = "..";
		inum = orphan;
		dp = ginode();

		ckinode(dp, DATA);
		bprelease((void *) dp);

		/*
		 * increment lost+found's link count
		 */
		inum = lfdir;
		if((dp = ginode()) != NULL) {
			dp->di_nlink++;
			bptouch((void *)dp);
			bprelease((void *)dp);
			setlncnt(getlncnt()+1);
		}
		inum = orphan;
		pr(0, MSGSTR(DIRCONN, DIRCONN_str), orphan, pdir);
		parentdir = save_parentdir;
	}

	return(YES);
}

/*
 *	lost_found
 *
 *	- return the inode for the lost+found directory ...
 *
 */
static ino_t
lost_found()
{
	ino_t oldlfdir;
	register DINODE *dp;
	ino_t save_parentdir = parentdir;

	/*
	 * already have it?
	 */
	if (lfdir != 0)
		return (lfdir);

	/*
	 * get the root dir inode for ckinode ...
	 */
	inum = ROOTDIR_I;
	if((dp = ginode()) == NULL) {
		inum = orphan;
		return(0);
		}

	/*
	 * set up for the search
	 */
	pfunc = findino;
	srchname = lfname;
	parentdir = 0;

	ckinode(dp, DATA);

	bprelease((void *)dp);

	inum = orphan;

	/*
	 * if we didn't find it, try to create it...
	 */
	if((lfdir = parentdir) == 0) {
		if (!pr(PREEN, MSGSTR(CREATEM, CREATEM_str), MSGSTR(NOLFD, NOLFD_str))){
			parentdir = save_parentdir;
			return (0);
			}

		/*
		 * get a directory inode
		 */
		if ((lfdir = allocdir(lfname)) != 0)
			/*
			 * add it to root...
			 */
			if (makeentry(ROOTDIR_I, "/", lfdir, lfname)) {
				}
			else {
				freedir(lfdir, ROOTDIR_I);
				lfdir = 0;
				if (preen)
					pr(0, "");
				}
		if (lfdir == 0) {
			pr(0, MSGSTR(NOMKDIR, NOMKDIR_str), lfname);
			parentdir = save_parentdir;
			return (0);
			}
		}

	inum = lfdir;
	if ((dp = ginode()) == NULL) {
		pr(0, MSGSTR(CANTGET, CANTGET_str), lfname);
		parentdir = save_parentdir;
		return (0);
		}

	if (!DIR) {
		bprelease((void *)dp);

		/*
		 * lost+found exists, but it's not a directory!
		 */
		if (!preen &&
		    !pr(ASK, MSGSTR(REALLOCATE, REALLOCATE_str), MSGSTR(SNOTDIR, SNOTDIR_str), lfname)) {
			parentdir = save_parentdir;
			return (0);
			}

		oldlfdir = lfdir;	/* save old lost+found ino	*/

		/*
		 * allocate a new directory...
		 */
		if ((lfdir = allocdir(lfname)) == 0) {
			pr(0, MSGSTR(NOMKDIR, NOMKDIR_str), lfname);
			parentdir = save_parentdir;
			return (0);
			}

		pfunc = chgino;
		parentdir = lfdir;	/* new inumber for lost+found */
		srchname = lfname;

		/*
		 * change the directory entry to point to our new directory
		 */
		inum = ROOTDIR_I;
		dp = ginode();
		if ((ckinode(dp, DATA) & ALTERD) == 0) {
			pr(0, MSGSTR(NOMKDIR, NOMKDIR_str), lfname);
			bprelease((void *)dp);
			parentdir = save_parentdir;
			return (0);
			}
		bptouch((void *)dp);
		bprelease((void *)dp);

		inum = oldlfdir;
		if ((dp = ginode()) != NULL) {
			setlncnt(getlncnt()+1);
			adjust(dp->di_nlink);
			bptouch((void *)dp);
			bprelease((void *)dp);
			}
	}
	else
		bprelease((void *)dp);

	parentdir = save_parentdir;

	return (lfdir);
}

/*
 * allocate a new directory
 */

/*
 * inode mode for a directory:  directory, setgid, rwx for everyone!
 */
#define	DIR_MODES	 (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO)

static ino_t
allocdir(dirname)
char	*dirname;
{
	char *cp;
	DINODE *dp;
	int save_inum = inum;
	ino_t ino, parent = ROOTDIR_I;

	/*
	 * allocate an inode...
	 */
	if ((inum = allocino(DIR_MODES)) == 0) {
		inum = save_inum;
		return (0);
		}

	/*
	 * get it
	 */
	if ((dp = ginode()) == NULL) {
		pr(0, MSGSTR(ALDIR, ALDIR_str), inum);
		inum = save_inum;
		return (0);
		}

	/*
	 * init rest of directory inode
	 */
	dp->di_nlink = 2;
	dp->di_mode |= IFJOURNAL;	/* N E C E S S A R Y ! ! ! */

	bptouch((void *)dp);

	if (expanddir(dp, inum, dirname, parent) == 0) {
		freeino(inum);
		inum = save_inum;
		bprelease((void *)dp);
		return (0);
		}

	bprelease((void *)dp);

	ino = inum;

	/*
	 * update root link count (assume here we're sticking this directory
	 * immediately under root)
	 */
	inum = ROOTDIR_I;
	if ((dp = ginode()) != NULL) {
		dp->di_nlink++;
		bptouch((void *)dp);
		bprelease((void *)dp);
		}

	inum = save_inum;	/* reset inum in case it was needed	*/

	return (ino);
}

/*
 * make an entry in a directory
 */
static int
makeentry(parent, parent_name, ino, name)
ino_t	 parent;			/* parent directory inode */
char	*parent_name;			/* parent directory name  */
ino_t	 ino;				/* new entry's inode	  */
char	*name;				/* new entry's name	  */
{
	int rc;
	DINODE *dp;

	/*
	 * sanity checks
	 */
	if (parent < ROOTDIR_I || parent >= imax || ino < ROOTDIR_I ||
	    ino >= imax)
		return (0);

	/*
	 * read parent's inode
	 */
	inum = parent;
	if ((dp = ginode()) == NULL)
		return (0);

	/*
	 * check parent's size
	 */
	if (dp->di_size % DIRBLKSIZ) {
		dp->di_size = roundup(dp->di_size, DIRBLKSIZ);
		bptouch((void *)dp);
		}

	inum = ino;

	pfunc = mkentry;
	new_name = name;
	new_inum = ino;

	/*
	 * attempt to make the entry...
	 */
	if (ckinode(dp, DATA) & ALTERD) {
		bprelease((void *)dp);		/* release directory inode */
		return (1);
		}

	/*
	 * try to expand the parent, and...
	 */
	if (!pr(PREEN, MSGSTR(EXPAND, EXPAND_str), MSGSTR(NOLEFT,
			     NOLEFT_str), parent_name) ||
	    expanddir(dp, parent, parent_name, ROOTDIR_I) == 0)
		return (0);

	/*
	 * try again to make the entry.
	 */
	rc = (ckinode(dp, DATA) & ALTERD);

	bprelease((void *)dp);		/* release directory inode */

	return (rc);
}

/*
 *	
 *	chgino
 *
 *	- if we find 'srchanme', change the inode number to 'parentdir'
 *
 */
static int
chgino(dirp)
register DIRECT *dirp;
{
	if (strcmp(dirp->d_name, srchname) != 0)
		return (KEEPON);
	
	dirp->d_ino = parentdir;
	bptouch((void *)dirp);
	return (ALTERD|STOP);
}

/*
 * free a directory inode
 */
static void
freedir(ino, parent)
ino_t	ino;
ino_t	parent;
{
	DINODE *dp;
	ino_t save_inum = inum;

	if (ino != parent) {
		inum = parent;
		if ((dp = ginode()) != NULL) {
			if (dp->di_nlink > 0)
				dp->di_nlink--;
			bptouch((void *)dp);
			bprelease((void *)dp);
			}
		}

	freeino(ino);

	inum = save_inum;
}


/*
 *	fbread:  read from scratch file
 *
 *
 *	- do a read on a struct filecntl structure...
 */
static int
fbread(fcp, buf, blk)
register struct filecntl *fcp;
char *buf;
daddr_t blk;
{
	int rc;

        if (lseek(fcp->wfdes, (long) (blk << Bshift),  0) < 0)
                rwerr(2, blk);

        else if (read(fcp->wfdes, buf, (unsigned) BLKSIZE) == BLKSIZE)
		return (YES);

	return(NO);
}

/*
 *	fbwrite: write to scratch file
 *
 *	- write a block out
 */

static void
fbwrite(fcp, buf, blk, size)
daddr_t				 blk;
register struct filecntl	*fcp;
register int			 size;
char				*buf;
{
	int rc;

	if (fcp->wfdes < 0)
		return;

	BPRINTF((DDEBUG, "BWRITE block %ld", blk));

        if (lseek(fcp->wfdes, (long) (blk << Bshift),  0) < 0)
                rwerr(2, blk);

        else if (write(fcp->wfdes, buf, (unsigned) size) == size)
                fcp->mod = 1;

        else
                rwerr(1, blk);
		fcp->mod = 1;
}

#ifndef STANDALONE
/* ARGSUSED */
static void
catch(int sig)
{
	/*
	 * just set xcode and jump up to the setjmp in op_check()
	 */
	xcode = EXIT_INTR;
	longjmp(errjmp, 1);
}
#endif

/*
 * pr   print out a diagnostic message
 *
 *      This routine is used throughout fsck to handle the output of
 *      diagnostic messages and interaction with the operator.  It
 *      prints out messages (like printf), asks questions, handles
 *      defaults, acknowleges answers and knows how to handle errors of
 *      various severities.  In previous versions of fsck, all of this
 *      functionality was spread around dozens of places - and as a result
 *      operator message handling was non-uniform and hard to work on.
 *
 *      The first argument to pr is a word of flags which describe
 *      the significance of the message and how it should be handled.
 *
 *      The second argument may be (if specified in the flags) a buffer
 *      into which an answer is to be returned.
 *
 *      The subsequent arguments are a format string and data items which
 *      are bound for printf.
 */
/* VARARGS1 */
int
pr(va_alist)
va_dcl
{
	va_list ap;
	char line[LINSIZE];
	register int flags;
	register char *p;
	register int retyes = NO, defyes;
	register char *fmt;

	va_start(ap);
	flags = va_arg(ap, int);

	/*
	 * Running automatically normally suppresses the printing of
	 * simple progress messages.  Trivia messages are always
	 * suppressed unless we were asked for them.
	 */
	if (!Debug && (flags&DDEBUG))
		return retyes;
	if (!vflag && (flags&TRIVIA))
		return retyes;
	if( preen && !vflag && (flags&QUIET) )
		return retyes;

	/*
	 * The default question is whether or not to continue, but the
	 * caller may have specified something else.
	 */
	p = MSGSTR(CONTINUE, "CONTINUE");
	if( flags&ARGX )
		p = va_arg(ap, char *);

	/*
	 * each new line of output should be prefixed with the name of
	 * the filesystem with which the error is associated (unless
	 * there is no possible doubt about which filesystem was involved).
	 */
	if (!(flags&NOB)  &&  devname  &&  preen) {
		if( devname )
			printf("%s", devname);
		if( fslabel )
			printf(" (%s)", fslabel);
		printf(": ");
	}

	/* call printf to print the actual message      */
	fmt = va_arg(ap, char *);
	vprintf(fmt, ap);
	va_end(ap);

	/* if an I-node dump was requested, provide it  */
	if( flags&PINODE )
		pinode();
	else
		if (flags & PINUM)
			printf("  I=%u ", inum);

	/* if we are supposed to ask a question ...     */
	if( flags&ASKX ) {
		/* should we normally answer yes to the question ??? */
		defyes = yflag || (preen && (flags & PREENX));

		/* if we're authorized to do this, and we are not
	         * cancelling effect of yflag or preen mode default yes 
		 */
		if(defyes && !(flags & CANCEL_YESPREEN) && CAN_MODIFY)
			retyes = YES;

		/* if we have an answer, print out our default  */
		if((retyes || nflag || (defyes && flags & CANCEL_YESPREEN)) &&
		    flags & YNX)
		{
			if (!strcmp(p, MSGSTR(ADJUST, ADJUST_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(ADJUSTED, ADJUSTED_str));
			else if (!strcmp(p, MSGSTR(CLEAR1, CLEAR1_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(CLEAR1ED, CLEAR1ED_str));
			else if (!strcmp(p, MSGSTR(CREATEM, CREATEM_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(CREATEMD, CREATEMD_str));
			else if (!strcmp(p, MSGSTR(CONTINUE, CONTINUE_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(CONTINUED, CONTINUED_str));
			else if (!strcmp(p, MSGSTR(EXPAND, EXPAND_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(EXPANDED, EXPANDED_str));
			else if (!strcmp(p, MSGSTR(FIX, FIX_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(FIXED, FIXED_str));
			else if (!strcmp(p, MSGSTR(REALLOCATE, REALLOCATE_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(REALLOCATED, REALLOCATED_str));
			else if (!strcmp(p, MSGSTR(RECON, RECON_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(RECONED, RECONED_str));
			else if (!strcmp(p, MSGSTR(REMOVE, REMOVE_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(REMOVED, REMOVED_str));
			else if (!strcmp(p, MSGSTR(SALV, SALV_str)))
				printf(" (%s%s)", retyes ? Nulstr : "NOT ",
					MSGSTR(SALVAGED, SALVAGED_str));
		}
		else if (!preen)
		{       /* ask the question     */
			if( flags&YNX )
				printf("; %s? ", p);
			fflush(stdout);

			/* get the answer       */
			if( getline(flags&YNX? line : p) == EOF )
				flags |= FATAL;
			else if ( flags&YNX )
				if( *line == 'q' || *line == 'Q' )
					flags |= FATAL;
				else {
					/* check CAN_MODIFY just in case */
					if(CAN_MODIFY &&
					  (rpmatch(line) == 1 ))
						retyes = YES;
				}

			/* user has supplied the NL at end of line      */
			flags |= NOE;
		}
	}

	/* see if a serious error condition has come about              */
	if( flags&ERROR && !retyes ) {
		xcode |= EXIT_FAIL;
		a_ok = NO;
		if( flags&FATALX ) {
			pr(flags&NOE? 0 : NOB, MSGSTR(TERMIN, TERMIN_str));
			longjmp(errjmp, 1);
		}
	}

	/* print a newline at the end of line (unless it's suppressed)  */
	if( !(flags&NOE) ) {
		putchar('\n');
		fflush(stdout);
	}

	return retyes;
}

/*
 * getline - get a line of input from the user into a provided buffer,
 *      eating all blanks and assuming buffer to be LINSIZE bytes long.
 *
 *   returns
 *      number of bytes read
 *      EOF
 */
static int
getline(loc)
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[LINSIZE-1];
	while((n = getchar()) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return(p - loc);
}

/*
 * get filesystem label from superblock
 *
 * returns "unlabeled fs" if superblock doesn't have a label
 */
static char *
getfslabel(super)
struct superblock *super;
{
	static char fsname[sizeof(super->s_fname) + 1];

	if (super->s_fname[0] == NULL)
		return(MSGSTR(UNLABELFS, UNLABELFS_str));

	return (strncpy(fsname, super->s_fname,(size_t)sizeof(super->s_fname)));
}

/*
 * Block map.  Given page offset ie block # bn, map that to a real
 * block # in the disk inode given in dp and return actual block number
 */
static int
bmap(dp, bn, return_blk)
DINODE		*dp;		/* Disk inode to read		*/
daddr_t		 bn;		/* Page offset in file		*/
frag_t		*return_blk;
{
	daddr_t nb;			/* number of blocks 		*/
	daddr_t *indblk;		/* indirect block		*/
	fdaddr_t frag;

	BPRINTF((DDEBUG | NOE, "bmap(%ld): ", bn));
	/*
	 *  get number of blocks in file & check against requested block number
	 */
	if (((nb = NUMDADDRS(*dp)) <= bn) || bn < 0)
		return -1;

	/*
	 *  No indirect blocks
	 */
	if (NOINDIRECT(nb))
	{
		frag.d = dp->di_rdaddr[bn];
		*return_blk = frag.f;
		BPRINTF((DDEBUG, "direct: returning (%d,%d)", frag.f.nfrags,
			frag.f.addr));
		return (0);
	}

	/*
	 *  Get the single or double indirect block
	 */
	frag.d = dp->di_rindirect;
	if ((indblk = (daddr_t *) bpread(frag.f)) == NULL)
		return (-1);

	BPRINTF((DDEBUG | NOE, "indirect: reading %ld", dp->di_rindirect));


	/*
	 * double indirect?
	 */
	if (ISDOUBLEIND(nb))
	{
		/*
		 * yep.  need to read the single indirect
		 */
		struct idblock *iddp;

		iddp = (struct idblock *) indblk + (DIDNDX(bn));

		/*
		 * release double block
		 */
		bprelease((void *)indblk);

		/*
		 * get single indirect block
		 */
		frag.d = iddp->id_raddr;
		if ((indblk = (daddr_t *) bpread(frag.f)) == NULL)
			return (-1);

		BPRINTF((DDEBUG | NOE, "double: reading %ld", iddp->id_raddr));

		/*
		 * Fall through to common code.
		 */
	}


	frag.d = indblk[SIDNDX(bn)];
	*return_blk = frag.f;

	BPRINTF((DDEBUG, ": returning 0x%8.8x", frag.d));

	/*
	 * release indirect block
	 */
	bprelease((void *)indblk);

	return (0);
}

/*
 *	dirmangled
 *
 *	- return 1 if directory entry in question is mangled
 *
 */
static int
dirmangled(dp, offsetinblock)
register DIRECT *dp;
int offsetinblock;
{
	register char *cp;
	register int size = LDIRSIZE(strlen(dp->d_name));
	int spaceleft = DIRBLKSIZ - (offsetinblock % DIRBLKSIZ);
	int rc;

	/*
	 * validity checks...
	 */
	if (dp->d_ino <= imax &&
	    dp->d_reclen != 0 &&
	    dp->d_reclen <= spaceleft &&
	    (dp->d_reclen & (DIROUND - 1)) == 0 &&
	    dp->d_reclen >= size &&
	    dp->d_namlen <= MAXNAMLEN)
	{
		if (dp->d_ino == 0)
			return (0);	/* valid on null entry */
		/*
		 * check name
		 */
		rc = dirbadname(dp->d_name, (int) dp->d_namlen);
		return rc;
	}
	/*
	 * invalid
	 */
	return (1);
}

/*
 *	dirbadname
 *
 *	- return 1 if dirname in question has nulls
 *
 */
static int
dirbadname(sp, l)
register char *sp;
register int l;
{
	register char c;

	while (l--) 		/* check for nulls */
	{	c = *sp++;
		if (c == 0)
			return 1;
	}

	return *sp;		/* check for terminating null */
}
