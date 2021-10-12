static char sccsid[] = "@(#)02  1.27.1.20  src/bos/sbin/helpers/v3fshelpers/op_make.c, cmdfs, bos411, 9432A411a 8/8/94 14:45:22";
/*
 *   COMPONENT_NAME: CMDFS
 *
 *   FUNCTIONS: Debug
 *		ERR_MSG
 *		PROTO_MSG
 *		STAT_MSG
 *		WARN_MSG
 *		add_name
 *		alloc_map
 *		calc_spec_sizes
 *		copy_boot
 *		create_log
 *		dbg_printf
 *		dbget
 *		debug
 *		dir_create
 *		dir_fill
 *		dispatch
 *		dump_daddrs
 *		dump_directory
 *		dump_inos
 *		dump_sindir
 *		fill_super
 *		format_log
 *		maxstring
 *		get_logname
 *		get_protoheader
 *		get_v3agsize
 *		idmaptab
 *		iget
 *		iinit
 *		imappage
 *		init_specs
 *		initmap
 *		inittree
 *		iprealloc_dblk
 *		lookup
 *		markit
 *		newblk
 *		op_make
 *		open_proto
 *		operr_to_helperr
 *		perrmsg
 *		perrmsg_proto
 *		prealloc_dblk
 *		proto_alloc
 *		proto_count
 *		proto_count_blocks
 *		proto_getl
 *		proto_getmode
 *		proto_getperm
 *		proto_gets
 *		roundup
 *		run_proto
 *		sltop
 *		treemax
 *		updtree
 *		write_specs
 *		
 *
 *   ORIGINS: 3,26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <lvm.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <jfs/inode.h>
#include <sys/stat.h>

#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL

#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/flock.h>
#include <sys/vfs.h>
#include <sys/vmdisk.h>
#include <jfs/log.h>
#include <sys/wait.h>
#include <fshelp.h>
#include <string.h>
#include <varargs.h>    
#include <stdio.h>
#include <memory.h>
#include <libfs/libfs.h>
#include <libfs/fslimits.h>
#include "fsop.h"
#include <locale.h>
#include <nl_types.h>
#include "op_make_msg.h"
    
static nl_catd	catd;

#define	UZBIT	((unsigned) 0x80000000)

#define ERR_MSG(num,str)   catgets (catd, MS_OPMAKE_ERR, num, str)
#define WARN_MSG(num,str)  catgets (catd, MS_OPMAKE_WARN, num, str)
#define STAT_MSG(num,str)  catgets (catd, MS_OPMAKE_STAT, num, str)
#define PROTO_MSG(num,str) catgets (catd, MS_OPMAKE_PROTO, num, str)
    
#define FSHERR_LASTIMP			FSHERR_COMPNOTLOADED
#define operr_to_helperr(operr)		((operr) + (FSHERR_1STIMP))

static char *errmsgs[] =
{
    NILSTR,				/* no error */ 
    "Filesystem size is too small",
    "Requested filesystem size is bigger than the logical volume size",
    "Boot program is not accessible",
    "Boot program is too big",
    "No filesystem or volume label was specified",
    "No boot program was specified",
    "Could not access prototype file",
    "Read of proto file failed",
    "Could not open prototype file",
    "Could not stat prototype file",
    "Prototype file is empty",
    "Error in prototype file on line %d",
    "Log create failed",
    "Log format failed",
    "File is too large for this filesystem",
    "Internal error, strcpy failed",
    "Internal error, fstat failed",
    "Internal error, alloc_map failed",
    "Internal error, irdwr called before Inodes allocated",
    "Internal error, irdwr memcpy failed",
    "Internal error, proto_count called with unknown mode",
    "Internal error, proto_alloc called with  unknown mode",
    "Internal error, lookup's args are bad or Rootdir is unallocated",
    "Internal error, ltop called with inconsistent size",
    "Internal error, bwrite called with inconsistent size",
    "Internal error, dispatch(): dup2 failed",
    "Internal error, dispatch(): exec failed",
    "%s: A file system with nbpi = %d cannot exceed %d 512 blocks\n",
    "Invalid option specified",
    "Invalid option syntax",
    "nbpi value must be 512, 1024, 2048, 4096, 8192, or 16384",
    "%s: The fragment size must be 512, 1024, 2048, or 4096",
    "The filesystem ran out of inodes.",
    "The filesystem ran out of blocks.",
    "Internal error, ioctl of device failed.",
    "Internal error, get/put inode failed.",
    "Internal error, get/put super failed.",
    "Internal error, add_name failed.",
    "Not a JFS filesystem.",
    "Invalid superblock version number.",
    "Corrupt superblock.",
    "Internal error, bread/bwrite failed.",
    "Internal error, ltop failed.",
    "Internal error, newblk failed.",	    
    "Unrecognized compression algorithm.",
    "Fragment size must be 2048 or less.",
    "Compression not supported for proto option.",
    "%s: A file system with frag = %d cannot exceed %d 512-byte blocks\n",
    "%s: A file system with size of %d 512-byte blocks cannot have\n\t    nbpi = %d.  Either increase file system size or decrease nbpi.\n",
    "The compression algorithm is either not present or not loaded\n\t    into the kernel.\n",
    NILSTR
};

/*
** if no log is specified and or the log device doesn't yet look
** like a log it will be formatted; so we need the path to logform
*/
#define LOGFORM_PATH	"/usr/sbin/logform"

/*
** all of the following are used to parse the proto file
*/
    
#define Proto_DOLLAR		"$"
#define Proto_SLASHS		"/"
#define Proto_OCTAL		8
#define Proto_NOBOOT		"<noboot>"
#define Proto_WHITESPACE	" 	\n"

#define Proto_SLASH		'/'
#define Proto_NEWLINE		'\n'
#define	Proto_COMMENT		'#'
#define Proto_NULLC		'\0'

/*
** for making directories
*/
#define	DOT	"."
#define DOTDOT	".."

/*
** since there'll never be a UNIX-domain socket use this mode
** to define a hard link name
*/
#define S_IFHARD	S_IFSOCK


#define IPL_I		NON_B
/*
 * the maximum number of reserved blocks for a given special "file"
 */
#define MAX_RSVD_SPEC_B	2


/*
** handy
*/

typedef	struct { char b[BLKSIZE]; } fsblk_t;		/* filesystem block */
typedef	struct { char b[DIRBLKSIZ]; }	dirblk_t;	/* directory block  */

/*
** rounds x up to the nearest multiple of y
*/
#ifdef roundup
#undef roundup
#endif
#define roundup(x,y)	(((x) + ((y)-1)) & ~((y)-1))


/*
 * allocation group size is expressed in number of frags.
 * for v3fs:
 * AGDEFAULT = 2048 is the default size of allocation group. 
 * if file system size (Nblocks) is less than AGDEFAULT,
 * the allocation unit size is set to Nblocks rounded
 * up to a multiple of MINAGSIZE = 256 which also is a divisor
 * on DBPERPAGE = 7*2048. 
 * for v4fs:
 * same as for V3fs except that MINAGSIZEV4 is 512 and DBPERPAGEV4
 * is 8*2048.
 */

static int DiskAgsize;
static int InodeAgsize;
static int clsize;
static int n_agroups;
static unsigned char dmaptab[256];
#define	ONES ((unsigned) 0xffffffff)

/*
** from main (fshelper)
*/

extern int   DebugLevel;	/* as in fsop.h                         */
extern int   Mode;		/* helper mode - see fsop.h             */
extern char *ProgName;  	/* for debugging messages               */ 
static int   DevFd;		/* file descriptor of filesystem device */

#define debug(level)	(DebugLevel >= abs(level))
#define Debug(level)    (debug(level) && interactive())

#define dbg_printf(level,prspec) if (Debug (level)) (void) printf prspec;


/*
** arguments passed in through opflags (and other globals)
*/
long		       Nfrags;		  /* number of frags in fs*/	
long                   Nblocks;		  /* size of filesystem   */
long		       Inodes;		  /* no. initial inodes   */
char                   Name[PATH_MAX];	  /* 'mount point' name   */
char                   Label[PATH_MAX];   /* filesystem label     */
char                   Vol[32];		  /* pack name            */
char                   Dev[PATH_MAX];     /* device name          */
bool_t                 New;		  /* not defined yet      */
char                   Log[PATH_MAX];	  /* log device's name    */
char                   Proto[PATH_MAX];	  /* proto device name    */
char                   Boot[PATH_MAX];	  /* boot program name    */
bool_t                 LabelIt;		  /* only change label    */
bool_t                 CopyBoot;	  /* only change boot     */
int		       Nbpi=0;		  /* bytes per inode	  */
int		       FragmentSize=0;	  /* fragsize		  */
int		       Dbg;		  /* debug level          */
char		       Compress[32];	  /* name of compression alg */
int		       compress;	  /* integer representation */
int			 MakeVer3 = 0;
static struct arg Args[] =
{
  "size",        (caddr_t)  &Nblocks,  INT_T,
  "inodes",	 (caddr_t)  &Inodes,   INT_T,
  "name",        (caddr_t)   Name,     STR_T,
  "label",       (caddr_t)   Label,    STR_T,
  "vol",         (caddr_t)   Vol,      STR_T,
  "dev",         (caddr_t)   Dev,      STR_T,
  "new",         (caddr_t)  &New,      BOOL_T,
  "log",         (caddr_t)   Log,      STR_T,
  "proto",       (caddr_t)   Proto,    STR_T,
  "debug",	 (caddr_t)  &Dbg,      INT_T,
  "boot",	 (caddr_t)   Boot,     STR_T,
  "labelit",	 (caddr_t)  &LabelIt,  BOOL_T,
  "copyboot",    (caddr_t)  &CopyBoot, BOOL_T,
  "nbpi",	 (caddr_t)  &Nbpi,     INT_T,
  "frag",	 (caddr_t)  &FragmentSize, INT_T,
  "compress",	 (caddr_t)  Compress, STR_T,
  NILPTR (char), NIL (caddr_t),        NIL_T
}; 
/*
** Prototype file's syntax:
** <name> <type> <setuid> <setgid> <sticky> <uid> <gid> <mode>
**							[<mode-specific>]
** Rules:
**  Comments start with the number sign (#); the number sign must be the
**    first character in the token.
**  Comments extend until the end of a line.
**  The root directory name must not be present.
**  No token is optional.
**  Tokens are white-space separated.
**  The <boot program name> is either a pathname or the string "<noboot>".
**  The <size> is a numerical value.  Zero (0) indicates the default value.
**    The default size is the device size.
**  The <inodes> is a numerical value.  Zero (0) indicates the default value.
**    The default number of inodes is the total needed to handle the files
**    defined in the proto file rounded up to the nearest multiple of DBWORD.
**  The <type> is one of the following:
**	"-"	regular
**	"b"	block
**	"d"	directory
**	"c"	character
**	"L"	link to previously <named> file, relative to this fs root
**	"p"	named fifo
**	"l"	symbolic (soft) link
** The <setuid> is one of the following:
**	"-"	not set-user mode
**	"u"	set-user mode
** The <setgid> is one of the following:
**	"-"	not set-group mode
**	"g"	set-group mode
** The <sticky> is one of the following:
**	"-"	not sticky
**	"t"	sticky mode
** The <mode> is a 3-digit octal value.
**
** The <mode specific data> is as below:
**	File type	<mode specific value>
**	---------	--------------------
**	regular		<name>
**			a pathname to a file from which to copy data
**
**	directory	more <name>s until the token "$"
**
**	block		<major> <minor>	
**			two decimal numbers
**
**	character	<major>	<minor>
**			two decimal numbers
**
**	link		<name>
**			a pathname in the new filesystem which has
**			been defined previously to this entry
**
**	named fifo	no mode specific value is appropriate
**
**	symbolic link	<name>
**			a pathname
**
*/

/*
** Internal Proto
**  fsck will add a lost+found, iff necessary, so we don't need to do it
*/

char *FakeProto    = "<noboot> 0 0 d-g- 755 3 3 $";
char *ProtoBuf     = NILPTR (char);	/* buffer holding proto file */
char *ProtoPos     = NILPTR (char);	/* position in proto file    */
char *ProtoStartPos= NILPTR (char);     /* start of real proto info  */
char *ProtoErr     = NILPTR (char);	/* ptr to erroneous token    */
char  ProtoMsg[32];			/* hint as to the error      */
int   ProtoCurLine = 0;                 /* line no.                  */
int   ProtoStartLine=0;			/* lineno where real proto   */
					/*     info starts	     */

  
struct modes
{
  char   *keys;
  mode_t  mod[7];		/* 7 = max recognized */
}
Modes [] =
{ { "-cbpldL",
    { S_IFREG, S_IFCHR, S_IFBLK, S_IFIFO, S_IFLNK, S_IFDIR, S_IFHARD }},
  { "-u",	{ 0, S_ISUID }},
  { "-g",	{ 0, S_ISGID }},
  { "-t",	{ 0, S_ISVTX }}
};

/*
** to keep statistcs on size of filesystem described by proto file
*/
#define DIR	0
#define SGL	1
#define DBL	2
struct proto_stats
{
  long	nb[3];	/* blocks needed */
  long  ni;	/* inodes needed */
} Pstat;

/*
** used to construct internal file name space
** to create hard links
*/

struct pname
{
  char		 *name;  	/* file/dirname	*/  	
  ino_t          inum;
  bool_t	 is_dir;	/* to prevent links to directories    	*/
				/* and from trying to descend through 	*/
  				/* non-directories                    	*/
  int		dirsize;	/* if this struct describes a directory */
  				/*    (ie. is_dir is true), dirsize  	*/
  				/*    is set to the dir's size in bytes */
  				/*    if not a dir, set to 0		*/
  struct pname	*parent;
  struct pname	*first_born;	/* 1st child	*/
  struct pname	*siblings;	/* middles  	*/
  struct pname  *runt;		/* last born	*/
};

struct pname *RootDir = NILPTR (struct pname);

/*
** blkbufs are handy for pre-allocating (doubly) indirect blocks
** with a little work they could be turned into a buffer cache
** XXX use fsdb's caching routines here and in op_check
*/

struct blkbuf
{
  ulong			bb_bno;			/* physical block no. */
  union
  {
    fsblk_t		_bb_buf;
    daddr_t		_bb_ind[BLKSIZE];	/* to reference as ind blks */
    struct idblock	_bb_dbl[BLKSIZE];	/* to reference as dbl blks */
  } _bb_un;
};
#define bb_buf  _bb_un._bb_buf
#define bb_ind	_bb_un._bb_ind
#define	bb_dbl	_bb_un._bb_dbl

/*
** since the traditional superblock functions are implemented
** in a number of different "files", these are pre-allocated
** in special inodes, these will not be present in the filesystem
** name space but will exist in the inode "file", inode map "file", etc.
*/

#define REG_PERM	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	REG_MODE	(S_IFREG | REG_PERM)

struct init_ino
{
  char           *name;		/* only used for debugging messages        */
  ino_t           inum;		/* inumber                                 */
  ushort          nlink;	/* link count                              */
  mode_t          mode;		/* extended mode - perm etc                */
  bool_t          set_ugid;	/* bother to set uid and gid?              */
  long            siz;		/* bytes                                   */
  long            nblocks;	/* actual no. of blocks allocated          */
  bool_t          set_time;	/* bother to set time fields?              */
  ulong           rsvdblk[MAX_RSVD_SPEC_B];	/* reserved blocks         */
  ulong          *moreblks;	/* added blocks, before regular allocation */
  ulong           no_ind;	/* no. of indirect blocks                  */
  ulong           rindirect;    /* real indirect block in dinode           */
  struct blkbuf  *indblks;	/* moreblks, for indirect blocks           */
  struct blkbuf  *dblblks;	/* ditto, for doubly indirect blocks       */
} init_inos[] = 
{
  { ".ipl", IPL_I,
     1, NIL (mode_t), False, sizeof (fsblk_t), 1, False, { IPL_B }
  },
  { ".superblock", SUPER_I,
     1, REG_MODE, True, 2*sizeof (fsblk_t), 2, True, { SUPER_B, SUPER_B1 }
  },
  { ".inodes", INODES_I,
     1, IFJOURNAL | REG_MODE, True, NIL(off_t), 0, True, {NON_B}
    /* size depends on file system size. all of the disk blocks
     * in each allocation unit are allocated at fixed locations.
     * INODES_B is the first in the first allocation unit.
     */
  },
  { ".indir", INDIR_I,
     1, IFJOURNAL | REG_MODE, True, 2*sizeof (fsblk_t), 0, True, {NON_B}
     /* size is 2 pages. */
  },
  { ".inomap", INOMAP_I,
     1, IFJOURNAL | REG_MODE, True, NIL (off_t), 0, True, { NON_B }
    /* inomap size is calculated from size of filesystem */
  },
  { ".diskmap", DISKMAP_I,
     1, IFJOURNAL | REG_MODE, True, NIL (off_t), 0, True, { NON_B}
     /* diskmap size is calculated from size of filesystem */
  },
  { ".inodex", INODEX_I,
     1, IFJOURNAL | REG_MODE, True, NIL (off_t), 0, True, { NON_B }
     /* allocated but empty file */
  }
};

/*
 * the inodes initialized inode struct (set in calc_spec_sizes & used in
 * irdwr and iflush)
 * 
 */
static struct init_ino  *Inode_ino;

/*
** maps resemble v.m. maps (coincidentally)
*/

struct vmdmap  *DiskMap  = NILPTR (struct vmdmap);
struct vmdmap  *InodeMap = NILPTR (struct vmdmap);

fsblk_t       *Indirect = NILPTR (fsblk_t);	  /* working copy .indirect */

/*
** misc. globals
*/
struct superblock Sb;				/* super block */
fsblk_t           Cb;				/* clean block */
ino_t		  NextInum = LAST_RSVD_I+1;	/* next available inum */
daddr_t           NextDfrag;			/* next available disk block */
daddr_t		  LastDfrag;			/* last frag in current ag   */
ulong             MaxBno = 0;			/* max block no. written out */

/*
** forward refs
*/  

static int      fill_super();
static int      format_log();
static bool_t   get_logname();
static bool_t   create_log();
static bool_t   dispatch();
static int      init_specs();
static int      write_specs(); 
static int      open_proto();
static void     markit();
static void     initmap();
void     initsect();
static int      alloc_map();
static int      iinit();
static int     iget();
static int     dbget();


static char    *proto_gets();
static long     proto_getl();
static int      proto_count();
static void     proto_count_blocks();
static void     perrmsg_proto();
static void     perrmsg();
static int      sltop();
static long     proto_alloc(), newblk(), lookup();
static int copy_boot(), /*irdwr(),*/ iflush(), run_proto(), calc_spec_sizes(), prealloc_dblk();
static int clear_inodes(),  add_name(), proto_getmode(), proto_getperm();
static int inittree(), idmaptab(), updtree(), treemax();
#ifdef DEBUG
void     dump_inos();
void     dump_directory();
#endif


	
/*
 *  command line options have already been parsed.
 *  the command line options override anything in the proto file header.
 *  assume version 3 when:
 *	a) the three version 4 options have not been specified
 *	b) the version 4 options are set to version 3 values
 *	   (nbpi == fragsize == blocksize == 4096)
 *
 *  version 3 header:
 *	<boot file name> <512block size> <number inodes>
 *  version 4 header:
 *	<boot file name> <512block size> <number inodes> <fragsize> <nbpi>
 *
 *  after the proto file header, the first token is a mode string (eg. "d---").
 *  so... read the first 3 header tokens and save position in proto file.
 *  next, try to read a long.  If this fails, then this must be a version
 *  3 proto file, and we restore the proto file position, and assume v3
 *  defaults.  (Of course if it succeeds, then read in the rest of the
 *  v4 header)
 *
 *  also, validate nbpi, blocksize, and fragsize. check that Compress
 *  hasn't been specified.
 */
int
get_protoheader()
{
	int curline, rc;
	char *pos, *cp;
	mode_t mode;
	
	if (strcmp (Boot, NILSTR) == 0)
	{
		if ((cp = proto_gets()) != NILPTR (char))
			if (strncpy (Boot, cp, (unsigned) sizeof Boot) != Boot)
			{
				perrmsg(FSHERR_STRCPYFAIL);
				return -FSHERR_STRCPYFAIL;
			}
	}
	else	
		(void) proto_gets();

	if (Nblocks == 0)
	{
		if ((rc = proto_getl (&Nblocks)) < 0)
		{
			perrmsg_proto (rc);
			return rc;
		}
	}
	else
		(void) proto_getl (NILPTR (long));
	
	if (Inodes == 0)
	{
		if ((rc = proto_getl (&Inodes)) < 0)
		{
			perrmsg_proto (rc);
			return rc;
		}
	}
	else
		(void) proto_getl (NILPTR (long));
	
	/*
	 * save proto file position
	 */
	curline = ProtoCurLine;
	pos = ProtoPos;
	
	/*
	 * if a mode string is next, then this is a v3 proto file
	 */
	if ((rc = proto_getmode(&mode)) < 0)
	{
		/*
		 *  since reading the mode just failed, this is either
		 *  a v4 proto file, or an invalid v3 proto file
		 */
		ProtoCurLine = curline;
		ProtoPos     = pos;

		if (FragmentSize != 0)
			(void) proto_getl(NILPTR (long));
		else			
			if ((rc = proto_getl(&FragmentSize)) < 0)
			{
				perrmsg_proto(rc);
				return rc;
			}
		if (Nbpi != 0)
			(void) proto_getl (NILPTR (long));
		else			
			if ((rc = proto_getl (&Nbpi)) < 0)
			{
				perrmsg_proto (rc);
				return rc;
			}
	}
	else
	{
		ProtoCurLine = curline;
		ProtoPos     = pos;
	}
	/*
	 *  if the command line was quiet, use v3 defaults
	 */
	if (FragmentSize == 0)
		FragmentSize = FRAGDEFAULT;
	if (Nbpi == 0)
		Nbpi = NBPIDEFAULT;

	ProtoErr = 0;
	memset((void *)ProtoMsg, 0, (size_t)sizeof(ProtoMsg));
	ProtoStartLine = ProtoCurLine;
	ProtoStartPos = ProtoPos;

	return FSHERR_GOOD;
}

int
superr(int rc, int def)
{
	switch(rc)
	{
	case LIBFS_BADMAGIC:
		return FSHERR_NOTJFS;
	case LIBFS_BADVERSION:
		return FSHERR_INVALVERS;
	case LIBFS_CORRUPTSUPER:
		return FSHERR_CORRUPTSUPER;
	default:
		return def;
	}
}

/*
** op_make
**
** creates a new filesystem
**
** returns FSHERR_...
**
*/
int
op_make (devfd, opflags)
	int      devfd;
	char    *opflags;
{
	struct devinfo  dinfo;
	int             rc;
	int             devsize;
	char           *cp = NILPTR (char);
	struct stat     stbf;
	int		OutIsFile;
	fdaddr_t	frag;

	memset((void *)&Cb, 0, (size_t)sizeof(fsblk_t));
	errno = 0;	/* paranoia */
	
	(void) setlocale (LC_ALL, NILSTR);
	
	catd = catopen (MF_OP_MAKE, NL_CAT_LOCALE);
	
	memset ((void *)Name,  0, (size_t)sizeof(Name));
	memset ((void *)Vol,   0, (size_t)sizeof(Vol));
	memset ((void *)Dev,   0, (size_t)sizeof(Dev));
	memset ((void *)Log,   0, (size_t)sizeof(Log));
	memset ((void *)Proto, 0, (size_t)sizeof(Proto));
	memset ((void *)Label, 0, (size_t)sizeof(Label));
	
	compress = Nblocks  = Nbpi = FragmentSize = 0;
	Inodes   = 0;
	New      = False;
	Dbg      = 0;
	LabelIt  = False;
	CopyBoot = False;
	DevFd    = devfd;
	
	if ((rc = get_args (Args, opflags)) != FSHERR_GOOD)
	{
		if (rc == FSHERR_INVALARG)
			perrmsg(FSHERR_BADOPT);
		else
			if (rc == FSHERR_BADSYNTAX)
				perrmsg(FSHERR_BADSYNTAX);
		return operr_to_helperr (rc);
	}
	
	/*
	 ** explicitly requesting a debuglevel overrides passed-in value
	 */
	if (Dbg != 0)
		DebugLevel = Dbg;
	
	/*
	 ** if just changing label, be sure one was specified
	 ** may change pack label and/or filesystem label
	 */
	if (LabelIt)
	{
		if (strcmp (Label, NILSTR) == 0 && strcmp (Vol, NILSTR) == 0)
		{
			perrmsg (FSHERR_NOLABEL);
			return operr_to_helperr (FSHERR_NOLABEL);
		}
	}
	/*
	 ** if just putting new boot program on filesystem, be
	 ** sure one was specified
	 */
	else if (CopyBoot)
	{
		if (strcmp (Boot, NILSTR) == 0 ||
		    strcmp (Boot, Proto_NOBOOT) == 0)
		{
			perrmsg (FSHERR_NOBOOT);
			return operr_to_helperr (FSHERR_NOBOOT);
		}
	}
	/*
	 *  prepare to actually make a filesystem
	 */
	else	
	{
		/*
		 *  if proto file was specified and we don't already have
		 *  value for Boot, Nblocks or Inodes use values in proto
		 *  (if proto values are null, use device size for Nblocks
		 *  and as many as necessary (rounded up by DBWORD) for
		 *  Inodes;  there is no default boot program.)
		 */
		if ((rc = open_proto()) < 0)
		{
			perrmsg(-rc);
			return operr_to_helperr (-rc);
		}

		/*
		 *  read header from proto file
		 *  check validity of FragmentSize and Nbpi
		 */
		if ((rc = get_protoheader()) < 0)
		{
			if (rc != -FSHERR_PROTOERR)
				perrmsg(-rc);
			return operr_to_helperr(-rc);
		}
		if (!inrange(FragmentSize, MINFRAGSIZE, MAXFRAGSIZE))
		{
			perrmsg (FSHERR_BADFRAGSIZE);
			return operr_to_helperr(FSHERR_BADFRAGSIZE);
		}
		if (!inrange(Nbpi, MINNBPI, MAXNBPI))
		{
			perrmsg (FSHERR_BADNBPI);
			return operr_to_helperr(FSHERR_BADNBPI);
		}

		if (strcmp(Compress, NILSTR))
		{
			if ((rc = valid_compression(Compress)) != -1)
				compress = rc;
			else if (strcmp(Compress, "no") != 0)
			{
				perrmsg (FSHERR_COMPNOTLOADED);
				return operr_to_helperr(FSHERR_COMPNOTLOADED);
			}
			if (compress && FragmentSize > 2048)
			{
				perrmsg (FSHERR_COMFRAGSIZE);
				return operr_to_helperr(FSHERR_COMFRAGSIZE);
			}

			if (compress && ProtoBuf != FakeProto)
			{
				perrmsg (FSHERR_COMPROTO);
				return operr_to_helperr(FSHERR_COMPROTO);
			}
		}

		/*
		 *  should we make a version 3 filesystem?
		 */
		if (Nbpi == NBPIDEFAULT && FragmentSize == FRAGDEFAULT)
			MakeVer3 = TRUE;
		else
			MakeVer3 = FALSE;

		/*
		 *  see if DevFd is device or file (ioctl fails on a file)
		 */
		if (fstat(DevFd, &stbf) < 0)
		{
			perrmsg (FSHERR_FSTATFAIL);
			return operr_to_helperr (FSHERR_FSTATFAIL);
		}
		if ((stbf.st_mode & IFMT) == IFCHR ||
		    (stbf.st_mode & IFMT) == IFBLK)
		{
			OutIsFile = FALSE;
			memset((void *)&dinfo, 0, (size_t)sizeof(dinfo));
			if (ioctl (DevFd, IOCINFO, &dinfo) < 0)
			{
				perrmsg (FSHERR_DEVIOCTLFAIL);
				return operr_to_helperr (FSHERR_DEVIOCTLFAIL);
			}
		}
		else
			OutIsFile = TRUE;
		
		if (OutIsFile == FALSE)
		{
			devsize = DEVBLK2BLK(dinfo.un.dk.numblks);
			Nblocks = DEVBLK2BLK(Nblocks);
			/*
			 *  sensible size?
			 */
			if (Nblocks > devsize)
			{
				perrmsg (FSHERR_FS2BIG);
				return operr_to_helperr (FSHERR_FS2BIG);
			}
			
			/*
			 *  default filesystem size is the full device  
			 */
			if (Nblocks == 0)
				Nblocks = devsize;
			
			if (interactive() && fshelperror() &&
			    BLK2DEVBLK(Nblocks) < dinfo.un.dk.numblks)
				fprintf (stderr, WARN_MSG (WARN_WASTE,
			             "Warning: %d (%d-byte) blocks wasted.\n"),
				     dinfo.un.dk.numblks - BLK2DEVBLK(Nblocks),
				     DEVBLKSIZE);
		}
		else
		{
			if (Nblocks == 0)
			{
				perrmsg (FSHERR_FS2SMALL);
				return operr_to_helperr (FSHERR_FS2SMALL);
			}
			Nblocks = DEVBLK2BLK(Nblocks);
		}

		/*
		 * Check that the specified size is valid
		 * for the given geometry.
		 */
		if (rc = validate_size (BLK2DEVBLK(Nblocks), 
					Nbpi, FragmentSize))
		{
			return (rc);
		}
	}
	/*
	 *  by now, Nblocks has been converted from devblks to full blocks (4k)
	 *  on error the lower level routines return -FSHERR_...
	 */
	Nfrags = Nblocks * (BLKSIZE / FragmentSize);
	
	/*
	 ** just change filesystem and/or volume label
	 */
	if (LabelIt)
	{
		/*
		 * since LabelIt set, fill_super will call
		 * get_super, and that will init libfs globals
		 */
		if (rc = fill_super(LabelIt))
		{
			if (rc < 0)
			{
				rc *= -1;				
				perrmsg(rc);
			}
			return operr_to_helperr (rc);
		}
		if ((rc = put_super(DevFd, &Sb)) < 0)
		{
			perrmsg(FSHERR_GETPUTSUPFAIL);
			return operr_to_helperr(FSHERR_GETPUTSUPFAIL);
		}
		if ((rc = put_super2(DevFd, &Sb)) < 0)
		{
			perrmsg(FSHERR_GETPUTSUPFAIL);
			return operr_to_helperr(FSHERR_GETPUTSUPFAIL);
		}
		
		if (interactive ())
		{
			if (Dev && strcmp (Dev, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_DEV,
						   "Device %s:\n"), Dev);
			if (Name && strcmp (Name, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_SHORTFORMAT,
							   "  %-13s %s\n"),
					 STAT_MSG (STAT_MOUNT, "Mount point:"),
					 Name);
			if (Label && strcmp (Label, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_SHORTFORMAT,
							   "  %-13s %s\n"),
					 STAT_MSG (STAT_LAB, "Label:"), Label);
			if (Vol && strcmp (Vol, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_SHORTFORMAT,
							   "  %-13s %s\n"),
					 STAT_MSG (STAT_VOLAB, "Volume Label:")
					 , Vol);
		}
	}
	else if (CopyBoot)
	{
		/*
		 *  just change the boot program, don't have to call
		 *  fill_super, because doesn't use any of libfs
		 */

		if ((rc = copy_boot()) < 0)
		{
			perrmsg (-rc);
			return operr_to_helperr (-rc);
		}
		else if (interactive())
		{
			if (Dev && strcmp (Dev, NILSTR) != 0)
				fprintf (stderr, STAT_MSG(STAT_DEV,
						  "Device %s:\n"), Dev);
			if (Name && strcmp (Name, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_SHORTFORMAT,
							   "  %-13s %s\n"),
					 STAT_MSG (STAT_MOUNT, "Mount point:"),
					 Name);
			fprintf (stderr, STAT_MSG (STAT_SHORTFORMAT,
						   "  %-13s %s\n"),
				 STAT_MSG (STAT_BOOT, "Boot program:"), Boot);
		}
	}
	else
	{
		/*
		 *  make a new filesystem, call fill_super first to get
		 *     	libfs globals initialized
		 */
		if (rc = fill_super(LabelIt))
		{
			if (rc < 0)
			{
				rc *= -1;
				perrmsg(rc);
			}
			return operr_to_helperr(rc);
		}
		
		/*
		 *  if DevFd is a file, write out the last block
		 *  so that the file will appear to be the specified
		 *  size.
		 */
		if (OutIsFile == TRUE)
		{
			frag.d = BLK2FRAG(Nblocks - 1);
			if (bwrite(DevFd, &Cb, frag.f) != FRAGLEN(frag.f))
			{
				perrmsg (FSHERR_BRDWRFAIL);
				return operr_to_helperr (FSHERR_BRDWRFAIL);
			}
		}
		
		if (OutIsFile == FALSE)
		{
			if ((rc = format_log()) < 0)
			{
				perrmsg (-rc);
				return operr_to_helperr (-rc);
			}
		}
		if ((rc = copy_boot()) < 0)
		{
			perrmsg (-rc);
			return operr_to_helperr (-rc);
		}
		if ((rc = run_proto()) < 0)
		{
			if(rc == -FSHERR_PROTOERR)
				perrmsg_proto (-rc);
			else
				perrmsg(-rc);
			return operr_to_helperr(-rc);
		}
		/*
		 *      write out superblks and deal with errors
		 */
		if ((rc = put_super(DevFd, &Sb)) < 0)
		{
			rc = superr(rc, FSHERR_GETPUTSUPFAIL);
			perrmsg(rc);
			return operr_to_helperr(rc);
		}
		if ((rc = put_super2(DevFd, &Sb)) < 0)
		{
			rc = superr(rc, FSHERR_GETPUTSUPFAIL);
			perrmsg(rc);
			return operr_to_helperr(rc);
		}
		    
		if (interactive())
		{
			if (Dev && strcmp (Dev, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_DEV,
						   "Device %s:\n"), Dev);
			if (!Proto || strcmp (Proto, NILSTR) == 0)
				fprintf (stderr,
					 STAT_MSG (STAT_INDENTFMT, "  %s\n"),
					 STAT_MSG (STAT_STDMT,
					   "Standard empty file system"));
			if (Name && strcmp (Name, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_LONGFMT,
							   "  %-15s %s\n"),
					 STAT_MSG (STAT_MOUNT, "Mount point:"),
					 Name);
			fprintf (stderr,
				 STAT_MSG (STAT_SIZE1,
				   "  %-15s %d %d-byte (UBSIZE) blocks\n"),
				 STAT_MSG (STAT_SIZE2, "Size:"),
				 BLK2DEVBLK (Nblocks), UBSIZE);
			
			if (Label && strcmp (Label, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_LONGFMT,
							   "  %-15s %s\n"),
					 STAT_MSG (STAT_LAB, "Label:"), Label);
			
			if (Vol && strcmp (Vol, NILSTR) != 0)
				fprintf (stderr, STAT_MSG (STAT_LONGFMT,
							   "  %-15s %s\n"),
					 STAT_MSG (STAT_VOLAB,
						   "Volume Label:"), Vol);
			
			if (Inodes)
				fprintf (stderr, STAT_MSG (STAT_LONGFMT1,
							   "  %-15s %d\n"),
					 STAT_MSG (STAT_INODES,
						   "Initial Inodes:"), Inodes);
			
			if (Boot && strcmp (Boot, NILSTR) != 0 &&
			    strcmp (Boot, Proto_NOBOOT) != 0)
				fprintf (stderr, STAT_MSG (STAT_LONGFMT,
							   "  %-15s %s\n"),
					 STAT_MSG (STAT_BOOT, "Boot program:"),
					 Boot);
		}
	}
	return FSHERR_GOOD;
}

/*
** format_log
**
** format log partition, iff needed
*/
#define BAD_FD (-1)

static int    
format_log ()
{
  int                rc;
  struct devinfo     dinfo;              /* to get size of log device */
  struct logsuper    logsb;
  int                logfd;        
  bool_t             logsb_ok = False;
  char              *av[3];

  /*
  ** create new log (from Log)         -or-
  ** create new log (from FSfile/ODM)
  */

  if (strcmp (Log, NILSTR) == 0 && get_logname (Name, Log) == False)
      return FSHERR_GOOD;
  
  errno = 0;
  if ((access (Log, F_OK) < 0))
    if (create_log (Log) == False)
      return -FSHERR_LOGCREATE;

  logfd = BAD_FD;
  if ((logfd = open (Log, O_RDWR)) <= 0 || errno)
  {
    dbg_printf (FSHBUG_SUBDATA,
		("format_log/open (%s, O_RDWR)=%d failed\n", Log, logfd));
  }
  else if ((rc = lseek(logfd, (off_t)BLK2BYTE(LOGSUPER_B), SEEK_SET)) < 0 || errno)
  {
    dbg_printf (FSHBUG_SUBDATA,
		("format_log/lseek (logfd=%d, LOGSUPER_B) = %d\n",
				logfd, rc));
  }
  else if ((rc = read (logfd, (char *)&logsb, (unsigned)sizeof(logsb))) != sizeof logsb || errno)
  {
    dbg_printf (FSHBUG_SUBDATA,
		("format_log/read (logfd, logsb, sizeof logsb=%d) = %d\n",
		 sizeof logsb, rc));
    close (logfd);
    logfd = BAD_FD;
  }
  else if ((rc = ioctl (logfd, IOCINFO, &dinfo)) != 0 || errno)
  {
    dbg_printf (FSHBUG_SUBDATA,
		("format_log/ioctl (logfd, IOCINFO, dinfo) = %d\n", rc));
  }
  /*
   ** up to here failures are severe, after this, if this is a
   ** log that was specified explicitly but isn't yet in the proper
   ** format, then format it later
   */
  else
  {
    if (logsb.magic == LOGMAGIC || logsb.magic == LOGMAGICV4)
	logsb_ok = True;
  }
	     
  if (logfd != BAD_FD)
  {
    (void) close (logfd);
    logfd = BAD_FD;
  }
  /*
  ** if there was possible valid log, call logform
  ** otherwise error (a filesystem must have a log available)
  */
  rc = FSHERR_GOOD;
  if (!logsb_ok)
  {
    av[0] = (char *)basename (LOGFORM_PATH); av[1] = Log; av[2] = NILPTR (char);
    rc = dispatch (LOGFORM_PATH, av) != True? -FSHERR_LOGFORM: FSHERR_GOOD;
  }
  return rc;
}

/*
** get_logname
**  found       stores into Log, returns True
**  not found:                   returns False
** stores logname in *log
*/

static bool_t
get_logname (fsys, log)
     char *fsys;
     char *log;
{
  struct old_fs         *ofsp;

  if ((ofsp = get_oldfs (fsys)) != NILPTR (struct old_fs))
  {
      if (ofsp->log || strncmp (ofsp->log, NILSTR, (int) strlen (ofsp->log)))
      {
        if (strncpy (log, ofsp->log, (int) strlen (ofsp->log)) == log)
           return True;
      }
  }
  return False;
}

/*
** create_log
**
** call out to system management commands to do the actual work
*/    

static bool_t
create_log (log)
     char         *log;
{
  char            *av[5];

  av[0] = "mklv";  av[1] = "-t";  av[2] = LOGTYPE_AIX3;  av[3] = log;
  av[4] = NILPTR (char);
  
  if (dispatch (av[0], av) != True)
      return -FSHERR_LOGCREATE;

  return FSHERR_GOOD;
}

/*
** dispatch
**
** like system() but grabs output if necessary
*/    

/*
** used by pipe from cmd back up to this level
*/
#define STDOUT_FD 1
#define STDERR_FD 2  
#define READ      0
#define WRITE     1

static bool_t
dispatch (cmd, av)
     char   *cmd;
     char   **av;
{
  int           pid;
  int           pipefds[2];
  char          pipbuf[BUFSIZ];
  int           rdcnt;
  int           child;

  errno = 0;
  if (pipe (pipefds) < 0 || errno)
  {
    return False;
  }
  
  if (!(pid = fork ()))
  {                                       /* child */
    (void) close (pipefds[READ]);
    (void) close (0);                    /* shut stdin */
    if (dup2 (pipefds[WRITE], STDOUT_FD) < 0 || errno)
    {
      exit (FSHERR_DUP2FAIL);          /* parent grabs this down below */
    }
    (void) execvp (cmd, av);
    exit (FSHERR_EXECFAIL);              /* ditto */
  }
  else if (pid)                          /* parent */
  {
    (void) close (pipefds[WRITE]);
    /*
    ** spray debugging output from below
    */
    if (Debug (FSHBUG_SUBDATA))
    {
      while ((rdcnt = read (pipefds[READ], pipbuf, sizeof pipbuf)) > 0)
	  (void) write (STDERR_FD, pipbuf, (unsigned)rdcnt);
    }
    (void) wait (&child);
    if (WEXITSTATUS (child))
    {
	return False;
    }
  }
  else
      return False;
  return True;
}

/*
** copy boot program, iff needed
*/
static int
copy_boot ()
{
  struct stat  stbf;
  int          bootfd;
  long         rdcnt;
  fsblk_t      rdbuf;
  int          rc, nbytes, i;
  fdaddr_t     frag;

  if (strcmp (Boot, NILSTR) == 0 || strcmp (Boot, Proto_NOBOOT) == 0)
      return FSHERR_GOOD;
  
  if (access (Boot, R_OK | F_OK) < 0)
      return -FSHERR_BOOTBAD;
  
  if (stat (Boot, &stbf) < 0)
      return -FSHERR_BOOTBAD;
  
  if (stbf.st_size > sizeof (fsblk_t))
    return -FSHERR_BOOT2BIG;
  
  if ((bootfd = open (Boot, O_RDONLY, 0)) < 0)
    return -FSHERR_BOOTBAD;

  memset ((void *) &rdbuf, 0, (size_t)sizeof(fsblk_t));
  
  frag.d = BLK2FRAG(IPL_B);  
  if ((rc = read(bootfd, &rdbuf, (unsigned)stbf.st_size)) < 0)
	  return -FSHERR_BOOTPROGRW;
  if ((rc = bwrite(DevFd, &rdbuf, frag.f)) != FRAGLEN(frag.f))
	  return -FSHERR_BOOTPROGRW;
		  
  close (bootfd);
  return FSHERR_GOOD;
}
/*
 *  returns version 3 disk allocation group size
 *  if there aren't enough blocks for the default AGSIZE,
 *	<agsize> becomes <numblk> rounded up to a multiple of MINAGSIZE
 *      that is also a factor of DBPERPAGE (because an ag's allocation
 *	state must be wholly contained in 1 map page)
 */
int		
get_v3agsize(int numblks)	/* number of 4k blocks in fs */
{
	int agsize;

	if ((agsize = MIN(AGDEFAULT, numblks)) != AGDEFAULT)
	{
		agsize = roundup(numblks , MINAGSIZE);
		while((DBPERPAGE / agsize) * agsize != DBPERPAGE)
			agsize += MINAGSIZE;
	}
	return agsize;
}

static int
fill_super (bool_t  labelonly)
{
	char	*magic;
	int	rc, i, j;
	int	def_agsize, numfrags, numags;

	memset((void *)&Sb, 0, (size_t)sizeof(Sb));

	if (labelonly && (rc = get_super(DevFd, &Sb)) < 0)
		return -superr(rc, FSHERR_GETPUTSUPFAIL);

	if (strcmp(Label, NILSTR) != 0)
	{
		if (strncpy(Sb.s_fname, Label,
			    (int)sizeof Sb.s_fname) != Sb.s_fname)
		{
			return -FSHERR_STRCPYFAIL;
		}
	}
	
	if (strncpy(Sb.s_fpack, Vol, (int)sizeof Sb.s_fpack) != Sb.s_fpack)
		return -FSHERR_STRCPYFAIL;
	
	if (!labelonly)
	{
		/*
		 * Initialize the superblock statistics
		 */
		Sb.s_bsize = BLKSIZE;
		Sb.s_fragsize = FragmentSize;
		Sb.s_fsize = BLK2DEVBLK(Nblocks);
		Sb.s_fmod  = FM_CLEAN;
		/*
		 * invalid logdev for logredo
		 */
		Sb.s_logdev = (unsigned) ~0;

		/*
		 * Pick a magic number and version number:
		 * The only way to get v3 magic and v3 version is
		 * if size <= 2G and nbpi=4096 and fragsize=4096
		 */
		Sb.s_version = fsv3pvers;		
		magic	= fsv3pmagic;

		if (MakeVer3 && Sb.s_fsize <= FS_V3MAXSIZE)
		{
			Sb.s_version = fsv3vers;
			magic	= fsv3magic;
		}
		
		if (strncpy(Sb.s_magic, magic,
			    (int)sizeof(Sb.s_magic)) != Sb.s_magic)
		{
			return -FSHERR_STRCPYFAIL;
		}
		
		/*
		 * allocation unit size is AGDEFAULT if Nblocks is at least
		 * as big as it. otherwise it is the next multiple of MINAGSIZE
		 * which is a divisor of DBPERPAGE.
		 */
		if (MakeVer3) 
		{
			clsize = CLDEFAULT;	    	    
			Sb.s_agsize = Sb.s_iagsize = InodeAgsize = DiskAgsize =
				get_v3agsize(Nblocks);
			Sb.s_compress = 0;
		}
		else
		{
			clsize = CLDEFAULTV4;
			numfrags = Nblocks * (BLKSIZE / FragmentSize);
			def_agsize = (int)BLK2BYTE(AGDEFAULT) / FragmentSize;  
			Sb.s_agsize = DiskAgsize = (numfrags < def_agsize)  ?
				get_pow2mult(MINAGSIZEV4, numfrags) :
					def_agsize;
			Sb.s_iagsize = InodeAgsize =
				DiskAgsize * FragmentSize / Nbpi;
			if (InodeAgsize < MINAGSIZEV4)
			{
				rc = FSHERR_CANTMAKENBPI;
				fprintf(stderr, ERR_MSG(rc, errmsgs[rc]),
					ProgName, BLK2DEVBLK(Nblocks), Nbpi);
				return rc;
			}
			Sb.s_compress = compress;
		}
		if (rc = validate_super(&Sb))
			return -superr(rc, FSHERR_GETPUTSUPFAIL);
		
		/*
		 * Now that we have the libfs globals initialized,
		 * convert the 4k  "special" block numbers in the
		 * init_inos structure into frag addresses.  Use
		 * init_inos[i].nblocks for number of disk addresses,
		 * not number of frags.
		 */
		for (i = 0;
		     i < sizeof(init_inos) / sizeof(struct init_ino); i++)
		{
			if (init_inos[i].nblocks)
			{
				for (j = 0; j < init_inos[i].nblocks; j++)
				{
					init_inos[i].rsvdblk[j] =
					BLK2FRAG(init_inos[i].rsvdblk[j]);
				}
			}
		}
	}
	time (&Sb.s_time);
	return FSHERR_GOOD;
}


/*
** initialize working copies of special "files"
*/
static int
init_specs()
{
  int              i, j, k, rc;
  struct dinode    di;
  long		   size, nb;
  fdaddr_t	   frag;
  fdaddr_t	   ifrag;
  fdaddr_t	   difrag;

  difrag.d = ifrag.d = frag.d = 0;
  /* indirect - needs to be zeroed and to have disk blocks.
  */
  size = 0;
  for (i = 0; i < sizeof init_inos / sizeof (struct init_ino); i++)
      if (init_inos[i].inum == INDIR_I)
      {
	size = init_inos[i].siz;
	break;
      }
  if ((Indirect = (fsblk_t *) malloc (size)) == NILPTR (fsblk_t))
    RETURN ("init_specs/indirect", -FSHERR_NOMEM);
  memset ((void *)Indirect, 0, (size_t)size);

  for (i = 0; i < sizeof init_inos / sizeof (struct init_ino); i++)
  {
    /*
    ** mark the Inode and Disk maps with the special system "files"
    ** and their associated blocks
    **
    ** IPL_I inode is special
    */ 
    if (init_inos[i].inum != IPL_I)
    {
      markit(InodeMap, (int)init_inos[i].inum, True);
      nb = init_inos[i].nblocks;

      for (j = 0; j < nb; j++)
      {
	      frag.d = sltop(&init_inos[i], j);
	      markit(DiskMap, frag.f, True);
      }
      
      /* INODES_I does not have indirect blocks and its inode
       * is zero size and has zero blocks.
       */
      if (init_inos[i].inum == INODES_I)
      {
	 nb = 0;
	 init_inos[i].nblocks = 0;
	 init_inos[i].siz = 0;
      }
      
      /*
       ** construct the indirect and doubly indirect blocks
       ** hang them off of the init_inos struct
       */
      if (!NOINDIRECT (nb))
      {
	if (ISDOUBLEIND (nb))
	{
	   /*
	    * only ever need to get one blkbuf, since
	    * this fs implementation only uses one doubly
	    * indirect block
	    */
	    init_inos[i].dblblks =
		    (struct blkbuf *)malloc(sizeof(struct blkbuf));
	    
	    if (init_inos[i].dblblks == NILPTR (struct blkbuf))
		    RETURN ("init_specs/malloc dblblks failed", -FSHERR_NOMEM);
	    memset((void *)init_inos[i].dblblks, 0,
		   (size_t)sizeof(struct blkbuf));
	    
	    if ((rc = dbget(&difrag.f, BLKSIZE)) < 0)
		    return rc;
	    
	    init_inos[i].dblblks->bb_bno = difrag.d;
	    init_inos[i].rindirect = init_inos[i].dblblks->bb_bno;
	    markit(DiskMap, difrag.f, True);
	    
	    init_inos[i].no_ind = roundup(nb, DADDRPERBLK) /
		    DADDRPERBLK;

	    init_inos[i].indblks = (struct blkbuf *)
		    malloc(init_inos[i].no_ind * sizeof (struct blkbuf));
		   
	    if (init_inos[i].indblks == NILPTR (struct blkbuf))
		    RETURN ("init_specs/indblks malloc failed", -FSHERR_NOMEM);
		   
	    memset((void *)init_inos[i].indblks, 0,
		   (size_t)(init_inos[i].no_ind * sizeof (struct blkbuf)));
		   
	    /*
	     ** loop through each indirect block, storing it's
	     ** block no. in the doubly indirect block
	     ** and the data block no.'s in it
	     */
	    for (j = 0; j < init_inos[i].no_ind;
		 j++, nb -= MIN (DADDRPERBLK, nb))
	    {
	       if ((rc = dbget(&ifrag.f, BLKSIZE)) < 0)
		       return rc;
	       init_inos[i].indblks[j].bb_bno           = ifrag.d;
	       init_inos[i].dblblks->bb_dbl[j].id_raddr = ifrag.d;
	       markit(DiskMap, ifrag.f, True);

	       for (k = 0; k < DADDRPERBLK && k < nb; k++)
		      init_inos[i].indblks[j].bb_ind[k] = sltop(&init_inos[i],
					    (ulong) (j * DADDRPERBLK) + k);
	    
	       if ((rc = bwrite(DevFd,
				(char *)&(init_inos[i].indblks[j].bb_buf),
				ifrag.f)) < 0)
		       return -FSHERR_BRDWRFAIL;
       	    }	
	    if ((rc = bwrite(DevFd, (char *)&(init_inos[i].dblblks->bb_buf),
			     difrag.f)) < 0)
		    return -FSHERR_BRDWRFAIL;
    	}
	else
	{
	  /*
	  ** single indirect
	  */  
	  init_inos[i].no_ind  = 1;
	  init_inos[i].indblks =
		  (struct blkbuf *) malloc(sizeof(struct blkbuf));
	  if (init_inos[i].indblks == NILPTR (struct blkbuf))
		  RETURN ("init_specs/indblks malloc failed", -FSHERR_NOMEM);
          memset((void *)init_inos[i].indblks, 0, (size_t)BLKSIZE);
	  
	  if ((rc = dbget(&ifrag.f, BLKSIZE)) < 0)
		  return rc;
	  
	  init_inos[i].indblks->bb_bno = ifrag.d;
	  init_inos[i].rindirect       = init_inos[i].indblks->bb_bno;

	  markit(DiskMap, ifrag.f, True);

	  for (j = 0; j < nb; j++)
		  init_inos[i].indblks->bb_ind[j] = sltop(&init_inos[i], j);

	  if ((rc = bwrite(DevFd, (char *)&(init_inos[i].indblks->bb_buf),
			   ifrag.f)) != FRAGLEN(ifrag.f))
		  return -FSHERR_BRDWRFAIL;
	}
      }
    }

    /* initialize the inode
    */    
    if ((rc = iinit (&di, &init_inos[i])) < 0)
	return rc;
    
    if ((rc = put_inode(DevFd, &di, (ino_t)init_inos[i].inum)) < 0)
	return -FSHERR_GETPUTINOFAIL;
  }

  /* mark the rest of the reserved inodes
  */
  for (i=init_inos[(sizeof init_inos / sizeof (struct init_ino) ) - 1].inum+1;
	i <= LAST_RSVD_I;
	i++)
  {
    markit(InodeMap, i, True);
  }
  return FSHERR_GOOD;
}
/*
** alloc map
**
*/
static int
alloc_map (ppmap, inum)
     struct vmdmap **ppmap;	/* ptr to ptr to map */
     ino_t inum;
{
  int      i;
  long     size;
   
  for (i = 0; i < sizeof init_inos / sizeof (struct init_ino); i++)
      if (init_inos[i].inum == inum)
	  break;

  if (i >= sizeof init_inos / sizeof (struct init_ino))
    return -FSHERR_ALLOCMAPFAIL;

  size = init_inos[i].siz;
  if (size <= 0)
      return -FSHERR_ALLOCMAPFAIL;

  if ((*ppmap = (struct vmdmap *) malloc (size)) == NILPTR (struct vmdmap))
      return -FSHERR_NOMEM;
  
  memset ((void *)*ppmap, 0, (size_t)size);
  return FSHERR_GOOD;
}

/*
** initi
*/
static int
iinit (dip, pin)
	struct dinode  *dip;
	struct init_ino *pin;
{
	int     i;
     
	memset((void *)dip, 0, (size_t)sizeof(struct dinode));

	dip->di_nlink   = pin->nlink;
	dip->di_mode    = pin->mode;
	dip->di_size    = pin->siz;
	/*
	 *  pin->nblocks for init_inos[] is number of disk addresses
	 *	not number of fragments.
	 */
	dip->di_nblocks = BYTE2FRAG(pin->siz + FragSize - 1);  
	
	for (i = 0; i < MIN(NDADDR, pin->nblocks); i++)
		dip->di_rdaddr[i] = sltop (pin, (ulong) i);
	
	if (! NOINDIRECT(pin->nblocks))
		dip->di_rindirect = pin->rindirect;
	
	if (pin->set_ugid)
	{
		dip->di_uid = geteuid();
		dip->di_gid = getegid();
	}
	dip->di_gen = time(NILPTR (long));
	if (pin->set_time)
		dip->di_ctime = dip->di_atime = dip->di_mtime = dip->di_gen;
	
	return FSHERR_GOOD;
}  


/*
** open_proto
**  set up pointer into proto buffer
*/
int
open_proto ()
{
	struct stat     stbf;
	int             pfd;
	int		i, nbytes, rc;
	char		*p;
	
	memset((void *)ProtoMsg, 0, (size_t)sizeof(ProtoMsg));
	if (strcmp(Proto, NILSTR) == 0)
	{
		ProtoBuf  = FakeProto;
	}
	else
	{
		if (access(Proto, F_OK | R_OK) < 0)
			return -FSHERR_PROTOAXS;
		
		if ((pfd = open(Proto, O_RDONLY, 0)) < 0)
			return -FSHERR_PROTOPN;
		
		if (stat(Proto, &stbf) < 0)
			return -FSHERR_PROTOSTAT;
		
		if (stbf.st_size == 0)
			return -FSHERR_PROTOEMPTY;
		
		/*
		 *  trust malloc more than shmat; besides proto files
		 *  aren't very big
		 */
		if ((ProtoBuf = (char *)malloc(stbf.st_size + 1)) ==
		    NILPTR (char))
			return -FSHERR_NOMEM;

		for (i = stbf.st_size, nbytes = MIN(i, BLKSIZE), p = ProtoBuf;
		     i > 0;
		     i -= nbytes, nbytes = MIN(i, BLKSIZE), p += rc)
		{
			rc = read(pfd, p, (unsigned)nbytes);
			if (rc < 0 || (rc == 0 && i > 0))
				return -FSHERR_PROTOREAD;
		}
		close (pfd);
	}
	/*
	 *  ProtoStartPos and ProtoStartLine let us skip past the
	 *  proto file head when scanning a second time.
	 */
	ProtoPos     = ProtoStartPos  = ProtoBuf;
	ProtoCurLine = ProtoStartLine = 0;
	return FSHERR_GOOD;
}

/*
** run proto
**
** make 2 passes through the proto file
**  1. to calculate size of .inodes, .inodemap, .diskmap 
**  2. to actually allocate the files
**
*/
    
static int
run_proto()
{
  int		pass;
  int           rc;
  
  /*
  ** open proto file if not yet already
  */
  if (ProtoBuf == NILPTR (char))
    if ((rc = open_proto()) < 0)
	return rc;

  for (pass = 0; pass < 2; pass++)
  {
    /*
    **  rewind proto file to ProtoStartPos to start reading proto file.
    **  At this point we have already used the proto header, and we
    **	don't need to scan it again.
    */
    ProtoPos     = ProtoStartPos;
    ProtoCurLine = ProtoStartLine;
	    
    if (pass == 0)
    {
      /*
       *  calling proto_count() just for kicks... the info it collects
       *  isn't used anywhere
       */
      if ((rc = proto_count()) < 0)
	  return rc;
      if ((rc = calc_spec_sizes ()) < 0)
	  return rc;
      if ((rc = init_specs()) < 0)
	  return rc;
    }
    else if (pass == 1)
    {
      rc = proto_alloc (NILPTR (struct pname), NILPTR (struct dinode),
			NILSTR);
      if (rc < 0)
	  return rc;
      if ((rc = write_specs()) < 0)
	  return rc;
    }
  }
  return FSHERR_GOOD;
}

/*
** proto_count
**
** walk proto file, determining size of each file
** and counting the number of inodes
**
** tabulates number of blocks and inodes needed for a protofile.
**    this info isn't referenced anywhere
*/
static int    
proto_count()
{
  char        *tokp;
  mode_t       mode;
  struct stat  stbf;
  int          len;
  int          ndirblks;
  int          rem;
  int          last_size;
  int          rc;

  if ((rc = proto_getmode (&mode)) < 0)
      return rc;
  
  if ((rc = proto_getperm (&mode)) < 0)
      return rc;
  
  if ((rc = proto_getl (NILPTR (long))) < 0)	/* skip uid */
      return rc;
  if ((rc = proto_getl (NILPTR (long))) < 0)	/* skip gid */
      return rc;
  
  switch (mode & S_IFMT)
  {
  case S_IFREG:
    Pstat.ni++;
    if ((tokp = proto_gets()) == NILPTR (char))
      return  -FSHERR_PROTOERR;

    if (access (tokp, F_OK | R_OK) < 0)
    {
      strncpy (ProtoMsg,
	       PROTO_MSG (PROTO_AXS, "access failed"), sizeof ProtoMsg);
      return -FSHERR_PROTOERR;
    }

    if (stat (tokp, &stbf) < 0)
    {
      strncpy (ProtoMsg, PROTO_MSG (PROTO_STAT, "stat failed"),
	       sizeof ProtoMsg);
      return -FSHERR_PROTOERR;
    }

    proto_count_blocks (stbf.st_size);
    break;

  case S_IFLNK:
    Pstat.ni++;
    /*
    ** only use a disk block if the name won't fit in the inode itself
    */
    if ((tokp = proto_gets()) == NILPTR (char))
      return -FSHERR_PROTOERR;

    if ((len = strlen (tokp)) >= D_PRIVATE)
	proto_count_blocks ((long) len);
    break;

  case S_IFDIR:
    Pstat.ni++;

    ndirblks  = 1;
    rem       = sizeof (dirblk_t) - LDIRSIZE (strlen (DOT));
    last_size = LDIRSIZE (strlen (DOTDOT));

    /*
    ** loop through entries in this directory,
    ** adding more disk blocks to directory, if needed
    */
    if ((tokp = proto_gets()) == NILPTR (char))
      return -FSHERR_PROTOERR;

    while (strcmp (tokp, Proto_DOLLAR) != 0)
    {
      if (rem < LDIRSIZE (strlen (tokp)))
      {
	ndirblks++;
	rem = sizeof (dirblk_t)  - last_size;
	last_size = LDIRSIZE (strlen (tokp));
      }

      if ((rc = proto_count ()) < 0)
	  return rc;

      if ((tokp = proto_gets()) == NILPTR (char))
	  return -FSHERR_PROTOERR;
    }
    proto_count_blocks ((long) ndirblks * sizeof (dirblk_t));
    break;

  case S_IFBLK:
  case S_IFCHR:
    Pstat.ni++;
    if ((rc = proto_getl (NILPTR (long))) < 0)	/* skip major */
       return rc;
    if ((rc = proto_getl (NILPTR (long))) < 0)	/* skip minor */
       return rc;
    break;

  case S_IFIFO:
    Pstat.ni++;
    break;

  case S_IFHARD:
    if ((tokp = proto_gets()) == NILPTR (char))
        return -FSHERR_PROTOERR;
    break;

  default:
    return -FSHERR_PRCOUNTMODE;
    /*NOTREACHED*/
    break;
  }
  return FSHERR_GOOD;
}



/*
** calc spec sizes
**
**  calculate the sizes of the special files, based on the
**  data in the proto file
** also set sizes in init_ino struct and hook data blocks onto them
*/
static int
calc_spec_sizes ()
{
  int	i;
  int	rc;
  long	nb_inodes;  
  long	nb_inomap;
  long	nb_diskmap;
  long	old_nb;
  long  dbperpage;

  /* 
   * the disk blocks for all inodes are allocated in fixed
   * locations. for the first allocation group they begin at INODES_B.
   * for all others they begin at the first block of the allocation
   * group; the last allocation group must have room for all of the
   * inodes; otherwise it is ommitted. 
  */
  n_agroups = Nfrags / DiskAgsize;
  if (Nfrags % DiskAgsize >= BYTE2FRAG(InodeAgsize * sizeof(struct dinode)))
	  n_agroups++;
  
  Inodes = n_agroups * InodeAgsize;
  nb_inodes = INO2BLK(Inodes);
  Pstat.nb[DIR] += BLK2FRAG(nb_inodes);
  
  NextDfrag = BLK2FRAG(SUPER_B + 1); /* first free block for dbget() */
  LastDfrag = BLK2FRAG(SUPER_B1) - 1;

  /* minimum reasonable space 
   */
   if (Nblocks < SUPER_B1 + nb_inodes || n_agroups == 0)
      RETURN ("op_make/MIN_FSIZE", -FSHERR_FS2SMALL);

  /* V3 Maps can support DBPERPAGE bits in one disk block
   * For V4, there are DBPERPAGEV4 bits per bit-map block, and we
   * also must add one control block per 8 bit-map blocks.
   *
   *  add 1 to nb_inomap and nb_diskmap to overallocate maps
   *  so they have growing room when we do chfs in a full fs
   */

  if (MakeVer3)
  {
     nb_inomap = (Inodes + DBPERPAGE - 1) / DBPERPAGE + 1;
     nb_diskmap = (Nfrags + DBPERPAGE - 1) / DBPERPAGE + 1;
  }
  else
  {
     nb_inomap = (Inodes + DBPERPAGEV4 - 1) / DBPERPAGEV4 + 1;
     nb_inomap += ((nb_inomap + 7)/8);
     nb_diskmap = (Nfrags + DBPERPAGEV4 - 1) / DBPERPAGEV4 + 1;
     nb_diskmap += ((nb_diskmap + 7)/8);
  }
  proto_count_blocks ((long)BLK2BYTE(nb_inomap));
  proto_count_blocks ((long)BLK2BYTE(nb_diskmap));
  
  /*
   *  reserve the blocks for these "files": .diskmap, .inodemap, 
   *  .inodes and .indirect. for .inodes also clear them on disk.
   */
  for (i=init_inos[(sizeof init_inos/sizeof (struct init_ino))-1].inum;
	i > 0;
	--i)
  {
    old_nb = init_inos[i].nblocks;
    
    if (init_inos[i].inum == DISKMAP_I)
    {
      init_inos[i].nblocks = MAX (old_nb, nb_diskmap);
      init_inos[i].siz     = (long)BLK2BYTE(init_inos[i].nblocks);

      if ((rc = prealloc_dblk (&init_inos[i], old_nb)) < 0)
	  return rc;
    }
    else if (init_inos[i].inum == INOMAP_I)
    {
      init_inos[i].nblocks = MAX (old_nb, nb_inomap);
      init_inos[i].siz     = (long)BLK2BYTE(init_inos[i].nblocks);

      if ((rc = prealloc_dblk (&init_inos[i], old_nb)) < 0)
	  return rc;
    }
    else if (init_inos[i].inum == INODES_I)
    {
      Inode_ino = &init_inos[i];
      init_inos[i].nblocks = MAX (old_nb, nb_inodes);
      /*  
       * Currently can only have 8 inode segments. (2**24 inodes)
       * 2**24 x 2**7 == 2**31  (max num bytes for inodes, fits in 32 bits)
       * One day, when a JFS can have more than 2**24 inodes, just
       * zero out this .siz because it is only referenced in init_specs.
       * (where it is set to 0)...  might as well let it be for now.
       */
      init_inos[i].siz = (long)BLK2BYTE(init_inos[i].nblocks);
      if ((rc = iprealloc_dblk(&init_inos[i])) < 0)
	  return rc;
    }
    else if (init_inos[i].inum == INDIR_I)
    {
      init_inos[i].nblocks = MAX (old_nb, 2);
      init_inos[i].siz     = (long)BLK2BYTE(init_inos[i].nblocks);
      if ((rc = prealloc_dblk (&init_inos[i], old_nb)) < 0)
	  return rc;
    }
  }
  /*
   *  allocate diskmap, inodemap, and clear inodes on disk
   */
  if ((rc = alloc_map (&DiskMap, DISKMAP_I)) < 0)
	  return rc;
  initmap (DiskMap, Nfrags, FSDISKMAP);
  if ((rc = alloc_map (&InodeMap, INOMAP_I)) < 0)
	  return rc;
  initmap (InodeMap, Inodes, INODEMAP);
  return FSHERR_GOOD;
}
/*
** prealloc_dblk  
**
** reserve a disk block for a special system file other than .inodes
**
*/
static int
prealloc_dblk (pin, old_nb)
     struct init_ino  *pin;
     long              old_nb;	/* already allocated blocks */
{
  ulong    *mbp;	/* ptr to current position in moreblk list */
  int       nb_needed;	/* moreblk list length to be malloc'ed */
  int       i, j;
  fdaddr_t	frag;
  int		rc;

  if (pin->nblocks > (sizeof (pin->rsvdblk) / sizeof (ulong)))
  {
    nb_needed = pin->nblocks - (sizeof (pin->rsvdblk) / sizeof (ulong));
    pin->moreblks = (ulong *) malloc (nb_needed * sizeof (long));
    if (pin->moreblks == NILPTR (ulong))
      RETURN ("prealloc_dblk", -FSHERR_NOMEM);
  }
    
  /*
  ** Put as many as possible into dirblk, then into moreblks
  */
  mbp = pin->moreblks;
  for (i = old_nb, j = old_nb; i < pin->nblocks; i++)
  {
	  if ((rc = dbget(&frag.f, BLKSIZE)) < 0)
		  return rc;
	  if (j < (sizeof (pin->rsvdblk) / sizeof (ulong)))
		  pin->rsvdblk[j++] = frag.d;
	  else
		  *mbp++ = frag.d;
  }
  return FSHERR_GOOD;
}

/*
 * iprealloc_dblk  
 *
 * reserves disk blocks for .inodes and clears the disk blocks
 * chains the blocks to the init_ino structure.
 *
 */
static int
iprealloc_dblk (pin)
     struct init_ino  *pin;
{
  ulong    *mbp;	/* ptr to current position in moreblk list */
  int       nb_needed;	/* moreblk list length to be malloc'ed */
  int	    nag, ninodes;
  fdaddr_t  diskblk;	
  int       j;

  if (pin->nblocks > (sizeof (pin->rsvdblk) / sizeof (ulong)))
  {
    nb_needed = pin->nblocks - (sizeof (pin->rsvdblk) / sizeof (ulong));
    pin->moreblks = (ulong *) malloc (nb_needed * sizeof (long));
    if (pin->moreblks == NILPTR (ulong))
      RETURN ("iprealloc_dblk", -FSHERR_NOMEM);
  }
    
  /* Put as many as possible into rsvblk, then into moreblks
   * the first disk block in the first allocation unit is INODES_B.
   * for all others it is the first block of the allocation unit.
   */
  mbp = pin->moreblks;
  diskblk.d = BLK2FRAG(INODES_B);
  nag = ninodes = 0;
  for (j = 0;  j < pin->nblocks; j++)
  {
      /* put diskblock on lists in init_ino structure 
       */
      if (j < (sizeof (pin->rsvdblk) / sizeof (ulong)))
	  pin->rsvdblk[j] = diskblk.d;
      else
	  *mbp++ = diskblk.d;
	
      /* clear the disk block */

      
      if (bwrite(DevFd, (char *)&Cb, diskblk.f) != FRAGLEN(diskblk.f))
	      RETURN ("bwrite/write clean", -FSHERR_BRDWRFAIL);

      
      ninodes += INOPERBLK;
      if (ninodes < InodeAgsize)
		diskblk.d += FragPerBlk;
      else
      {
	  nag += 1;
	  diskblk.d = nag * DiskAgsize;
	  ninodes = 0;
      }
  }
  return FSHERR_GOOD;
}


		
/*
 *  proto_alloc
 *
 *  walk through proto file, allocating blocks and inodes
 *    returns < 0 on error
 *            = 0 ok for most files (except for hard links)
 *            > 0 ok for a link, return value = inumber
 *
 *  Since ino_t can be up to an unsigned long value
 *  this code assumes that no proto file will ever
 *  use so many inodes that the sign bit gets set.
 *  If not, rip it out and disable hard links.
 *  Anyway, No one else has them in their proto files.
 *    
 */
static long
proto_alloc(struct pname  *parent,	/* ptr to pname describing parent */
	    struct dinode *parino,	/* ptr to dinode of parent        */
	    char          *new_name)	/* name of new object             */
{
	fsblk_t		db;
	struct dinode	ino;
	long		hardlink;
	ino_t		inum = 0;
	int		rc;
	mode_t		mode;
	long		uid;
	long		gid;
	char 		*tokp;
	struct stat	stbf;
	int		fd;
	int		b;
	long		maj;
	long		min;
	frag_t		frag;
	int		len;
	int		i, nbytes;
	struct pname	*kid;
	
	if ((rc = proto_getmode(&mode)) < 0)
		return (long)rc;
	
	if ((rc = proto_getperm(&mode)) < 0)
		return (long)rc;
	
	if ((rc = proto_getl(&uid)) < 0)
		return (long)rc;
	
	if ((rc = proto_getl(&gid)) < 0)
		return (long)rc;
	
	memset((void *)&db, 0, (size_t)sizeof(db));
	memset((void *)&ino, 0, (size_t)sizeof(ino));
	
	ino.di_nlink = 1;
	ino.di_mode  = mode;
	ino.di_uid   = (uid_t) uid;
	ino.di_gid   = (uid_t) gid;
	ino.di_ctime = time(NILPTR(long));
	ino.di_atime = ino.di_ctime;
	ino.di_mtime = ino.di_ctime;
	ino.di_gen   = (long)ino.di_ctime;
	
	if (parent == NILPTR(struct pname))
	{
		inum = ROOTDIR_I;
		markit(InodeMap, (int)inum, True);
	}
	
	switch (mode & S_IFMT)
	{
	case S_IFREG:
		if ((tokp = proto_gets()) == NILPTR (char))
			return -FSHERR_PROTOERR;
		
		if (access (tokp, F_OK | R_OK) < 0)
		{
			strncpy (ProtoMsg, PROTO_MSG (PROTO_AXS,
						      "access failed"),
				 sizeof ProtoMsg);
			return -FSHERR_PROTOERR;
		}
		if (stat (tokp, &stbf) < 0)
		{
			strncpy (ProtoMsg, PROTO_MSG (PROTO_STAT,
						      "stat failed"),
				 sizeof ProtoMsg);
			return -FSHERR_PROTOERR;
		}
		if ((fd = open (tokp, O_RDONLY)) < 0)
		{
			strncpy (ProtoMsg, PROTO_MSG (PROTO_OPEN,
						      "open failed"),
				 sizeof ProtoMsg);
			return -FSHERR_PROTOERR;
		}
		if (rc = iget (&inum, True))
		{
			close(fd);
			return rc;
		}
		ino.di_size = stbf.st_size;		
		for (i = stbf.st_size; i > 0; i -= BLKSIZE)
			if ((rc = newblk(&ino, MIN(i, BLKSIZE))) < 0)
			{
				close(fd);
				return rc;
			}
		for(i = stbf.st_size, b = 0, nbytes = MIN(i, BLKSIZE);
		    i > 0;
		    i -= nbytes, b++, nbytes = MIN(i, BLKSIZE))
		{
			if ((len = read(fd, (char *)&db, nbytes)) != nbytes)
			{
				close(fd);
				rc = -FSHERR_BRDWRFAIL;
			}
			if ((rc = ltop(DevFd, &frag, &ino, b)) < 0)
			{
				close(fd);
				return -FSHERR_LTOPFAIL;
			}
			if (len < BLKSIZE)
				memset((void *)((char *)&db + len),
				       0, (size_t)(BLKSIZE - len));
			if ((rc = bwrite(DevFd, (char *)&db, frag)) !=
			    FRAGLEN(frag))
			{
				close(fd);
				return -FSHERR_BRDWRFAIL;
			}
		}
		close(fd);
		if ((rc = put_inode(DevFd, &ino, inum)) < 0)
			return -FSHERR_GETPUTINOFAIL;
		if (rc = add_name(&parent, new_name, inum, False))
			return rc;
		break;
		
	case S_IFDIR:
		if (inum == 0)
		{
			if (rc = iget(&inum, True))
				return rc;
			parino->di_nlink++;
		}
		ino.di_mode |= IFJOURNAL;
		ino.di_nlink++;
		if (rc = add_name(&parent, new_name, inum, True))
			return rc;
		/*
		 *  If we are root dir, root dir just got created in
		 *  add_name, and we want to populate parent.
		 *
		 *  If not root dir, then we want to use the
		 *  following while loop to populate parent->runt.
		 *  (parent->runt was created in add_name)
		 */
		if (inum != ROOTDIR_I)
			parent = parent->runt;
		if ((tokp = proto_gets()) == NILPTR (char))
			return -FSHERR_PROTOERR;
		
		while (strcmp (tokp, Proto_DOLLAR) != 0)
		{
			/*
			 *  all files but hard links will be allocated down
			 *  below hard links must be done at this level
			 *  (no new inode)
			 */
			hardlink = proto_alloc(parent, &ino, tokp);
			if (hardlink < 0)
				return hardlink;
			
			if (hardlink)
			{
				struct dinode lino;	/* inode to link to */
				
				if (rc = add_name(parent, tokp, inum, False))
					return rc;
				if ((rc = get_inode(DevFd, &lino,
						    (ino_t)hardlink)) < 0)
					return -FSHERR_GETPUTINOFAIL;
				lino.di_nlink++;
				if ((rc = put_inode(DevFd, &lino,
						    (ino_t)hardlink)) < 0)
					return -FSHERR_GETPUTINOFAIL;
			}
			if ((tokp = proto_gets()) == NILPTR(char))
				return -FSHERR_PROTOERR;
		}
		/*
		 *  create directory on disk, and then free it from
		 *  our internal tree. (we never need it again)
		 */
		if ((rc = dir_create(&ino, parent)) != 0)
			return rc;
		if ((rc = put_inode(DevFd, &ino, inum)) < 0)
			return -FSHERR_GETPUTINOFAIL;
#ifdef DEBUG
		if (Debug (FSHBUG_SUBDATA))
			dump_directory (&ino, inum);
#endif 
		break;
		
	case S_IFLNK:
		if (rc = iget (&inum, True))
			return rc;
		
		if ((tokp = proto_gets()) == NILPTR (char))
			return -FSHERR_PROTOERR;
		
		ino.di_size = strlen (tokp);
		if (ino.di_size <= sizeof ino.di_symlink)
		{
			if (strncpy (ino.di_symlink, tokp,
				     sizeof ino.di_symlink) != ino.di_symlink)
				return -FSHERR_STRCPYFAIL;
		}
		else
		{
			if ((rc = newblk(&ino, ino.di_size)) < 0)
				return rc;
			if (strncpy((char *) &db, tokp, BLKSIZE) !=
			    (char *) &db)
				return -FSHERR_STRCPYFAIL;
			if ((rc = ltop(DevFd, &frag, &ino, 0)) < 0)
				return -FSHERR_LTOPFAIL;
			if ((rc = bwrite(DevFd, (char *)&db, frag)) < 0)
				return -FSHERR_BRDWRFAIL;
		}
		if ((rc = put_inode(DevFd, &ino, inum)) < 0)
			return -FSHERR_GETPUTINOFAIL;
		if (rc = add_name(&parent, new_name, inum, False))
			return rc;
		break;
		
	case S_IFBLK:
	case S_IFCHR:
		if (rc = iget (&inum, True))
			return rc;
		
		if ((rc = proto_getl (&maj)) < 0)
			return rc;
		if ((rc = proto_getl (&min)) < 0)
			return rc;
		ino.di_rdev = makedev (major (makedev ((dev_t) maj, 0)),
				       minor (makedev (0, (dev_t) min)));
		if ((rc = put_inode(DevFd, &ino, inum)) < 0)
			return -FSHERR_GETPUTINOFAIL;
		if (rc = add_name(&parent, new_name, inum, False))
			return rc;
		break;
		
	case S_IFIFO:
		if (rc = iget (&inum, True))
			return rc;
		
		if ((rc = put_inode(DevFd, &ino, inum)) < 0)
			return -FSHERR_GETPUTINOFAIL;
		if (rc = add_name(&parent, new_name, inum, False))
			return rc;
		break;
		
	case S_IFHARD:
		if ((tokp = proto_gets()) == NILPTR (char))
			return -FSHERR_PROTOERR;
		
		if ((hardlink = lookup (tokp, parent)) <= 0)
			return hardlink;
		/*
		 ** allow future links to this entry
		 */
		if ((rc = add_name(&parent, new_name, (ino_t) hardlink, False))
		    < 0)
			return rc;
		return hardlink;
		/*NOTREACHED*/
		break;
		
	default:
		return -FSHERR_PRALLOCMODE;
		/*NOTREACHED*/
		break;
	}
	return FSHERR_GOOD;
}

/*
 *  build direct_t struct from name and inum.  since name has already
 *  been stat'd by proto_alloc, don't bother to make sure
 *  strlen(name) < MAXNAMLEN
 */
void
dir_fill(direct_t *	dirp,
	 char *		name,
	 ino_t		inum)
{
	char *p;	
	int  numnulls;
	
	dirp->d_ino = inum;
	strcpy(dirp->d_name, name);
	dirp->d_namlen = strlen(name);	
	dirp->d_reclen = LDIRSIZE(dirp->d_namlen);

	/*
	 *  direct_t's must be word aligned, so just to be tidy...
	 *  pad d_name with appropriate number of nulls
	 */
	numnulls = DIROUND - (dirp->d_namlen & (DIROUND - 1));
	p = dirp->d_name + dirp->d_namlen;
	while (numnulls--)
		*p++ = '\0';
}
	
	
/*
 *  dir_create
 *
 *  	dinop		directory's disk inode
 *	dlist		list of directory entries
 *
 *  allocate disk blocks for a directory, and fill the blocks
 *  with the directory entries in dlist
 *
 *  returns 0 or negative number.
 */
static int
dir_create(struct dinode *	dinop,
	   struct pname *	dlist)
{
	int 		rc, i;
	int		dbno = 0;	/* logical dir block (frag) number */
	int		slop;		/* free space in dir chunk 	   */
	int		dsize;		/* size of dir in bytes		   */
	fdaddr_t	dfrag;		/* disk addr of directory block	   */
	fsblk_t 	dirblk;		/* buffer for directory block	   */
	direct_t *	dirp = (direct_t *)&dirblk;
	struct pname *	pnp;

	/*
	 *  allocate frags for directory
	 */
	for (dsize = dlist->dirsize; dsize > 0; dsize -= BLKSIZE)
		if ((rc = newblk(dinop, MIN(dsize, BLKSIZE))) < 0)
			return rc;
	dinop->di_size = slop = DIRBLKSIZ;

	for (i = 0; i < BLKSIZE; i += DIRBLKSIZ)
	{
		((direct_t *)((char *)&dirblk + i))->d_ino = 0;
		((direct_t *)((char *)&dirblk + i))->d_reclen = DIRBLKSIZ;
	}

	/*
	 *  get first dir frag number
	 */
	if ((rc = ltop(DevFd, &dfrag.f, dinop, dbno++)) < 0)
		return -FSHERR_LTOPFAIL;
	
	dir_fill(dirp, DOT, dlist->inum);
	slop -= dirp->d_reclen;
	dirp = (direct_t *)((char *)dirp + dirp->d_reclen);
	
	dir_fill(dirp, DOTDOT, dlist->parent->inum);
	slop -= dirp->d_reclen;
	for (pnp = dlist->first_born; pnp; pnp = pnp->siblings)
	{		
		/*
		 *  is there enough room in current dirchunk for dir entry?
		 */
		if (slop < LDIRSIZE(strlen(pnp->name)))
		{
			dirp->d_reclen += slop;
			dirp = (direct_t *)((char *)dirp + dirp->d_reclen);
			slop = DIRBLKSIZ;
			dinop->di_size += DIRBLKSIZ;
			/*
			 *  do we need next frag of dirchunks ?
			 *  if so, write out current dirchunk frag and get
			 *  next one.
			 */
			if ((char *)dirp - (char *)&dirblk  >= BLKSIZE)
			{
				if ((rc = bwrite(DevFd, &dirblk, dfrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
				if ((rc = ltop(DevFd, &dfrag.f, dinop,
						dbno++)) < 0)
					return -FSHERR_LTOPFAIL;
				dirp = (direct_t *)&dirblk;
				for (i = 0; i < BLKSIZE; i += DIRBLKSIZ)
				{
				((direct_t *)((char *)&dirblk + i))->d_ino = 0;
				((direct_t *)((char *)&dirblk + i))->d_reclen =
					DIRBLKSIZ;
				}
			}
		}
		else
			dirp = (direct_t *)((char *)dirp + dirp->d_reclen);
		/*
		 * add entry to dir
		 */
		dir_fill(dirp, pnp->name, pnp->inum);
		slop -= dirp->d_reclen;
	}
	/*
	 *  fix up d_reclen in last directory entry and write last dirchunk
	 *  frag to disk
	 */
	dirp->d_reclen += slop;
	if ((rc = bwrite(DevFd, &dirblk, dfrag.f)) < 0)
		return -FSHERR_BRDWRFAIL;
	
	return FSHERR_GOOD;
}
/*
 *  FUNCTION
 *  	initialize RootDir if inum == ROOTDIR_I 
 *	add <name> and <inum> to internal file tree
 *	also maintains dirsize (number of bytes needed for a directory)
 */
static int
add_name(struct pname **par,
	 char          *name,
	 ino_t          inum,
	 bool_t         is_dir)
{
	struct pname *	pnp;
	struct pname *	brat;
	struct pname *	parent;
	int		len;
	int		slop;
	int		whocares;

	if ((brat = (struct pname *)malloc(sizeof(*brat))) == NULL)
		return -FSHERR_NOMEM;
	memset((void *)brat, 0, (size_t)sizeof(struct pname));
	
	/*
	 *  if parent null, then this is the root dir...
	 *  	so, set parent and RootDir to brat
	 */
	if (*par == NILPTR(struct pname))
		*par = RootDir = brat;
	parent = *par;
	brat->name = name;
	brat->inum   = inum;
	brat->parent = parent;
	if ((brat->is_dir = is_dir) == True)
		brat->dirsize = LDIRSIZE(strlen(DOT)) +
				LDIRSIZE(strlen(DOTDOT));
	
	if (inum == ROOTDIR_I)
		return FSHERR_GOOD;
	/*
	 *  sanity checks,
	 *  	make sure parent is a directory
	 *  	make sure <name> isn't already in parent dir
	 */
	if (parent->is_dir == False)
	{
		free(brat);
		return -FSHERR_ADDNAMEFAIL;
	}
	for (pnp = parent->first_born; pnp; pnp = pnp->siblings)
		if (! strcmp(name, pnp->name))
		{
			free(brat);
			return -FSHERR_PROTOERR;
		}
	/*
	 *  insert brat into parent's directory.
	 *  fix parent's first_born, sibling, and runt ptrs
	 */
	if (parent->first_born == NILPTR(struct pname))
		parent->first_born = brat;
	else
		parent->runt->siblings = brat;

	parent->runt = brat;
	slop = DIRBLKSIZ - parent->dirsize % DIRBLKSIZ;
	len = LDIRSIZE(strlen(name));
	if (slop < len)
		parent->dirsize = roundup(parent->dirsize, DIRBLKSIZ);
	parent->dirsize += len;
	return FSHERR_GOOD;
}


static int
dump_sindir(daddr_t *blk, int howmany)
{
	int i;
	
	if (howmany > DADDRPERBLK)
		howmany = DADDRPERBLK;

	for (i = 0; howmany; howmany--, blk++, i++)
	{
		if (i && (!(i & 7)))
			printf("\n");
		printf("%8.8x ", *blk);
	}
	printf("\n");
}
	
static int
dump_daddrs(struct dinode di)
{
	int 		i, numaddrs, rc;
	fdaddr_t 	frag;
	struct idblock	dblbuf[INDIRPERBLK];
	struct idblock	*dbl = dblbuf;
	daddr_t		sglbuf[DADDRPERBLK];

	numaddrs = NUMDADDRS(di);
	printf("\n\nInode has %d disk addresses.\n", numaddrs);
	if (NOINDIRECT(numaddrs))
		dump_sindir(&(di.di_rdaddr), numaddrs);
	else
	{
		if (ISDOUBLEIND(numaddrs))
		{
			printf("Double Indirect Block: %8.8x\n",
			       di.di_rindirect);
			frag.d = di.di_rindirect;			
			if ((rc = bread(DevFd, dblbuf, frag.f)) != BLKSIZE)
				return -FSHERR_BRDWRFAIL;
		}
		else
			dblbuf[0].id_raddr = di.di_rindirect;

		for (i = numaddrs; i > 0; i -= DADDRPERBLK, dbl++)
		{
			printf("Single Indirect Block: %8.8x\n",
			       dbl->id_raddr);
			frag.d = dbl->id_raddr;
			if ((rc = bread(DevFd, sglbuf, frag.f)) != BLKSIZE)
				return -FSHERR_BRDWRFAIL;
			dump_sindir(sglbuf, MIN(i, DADDRPERBLK));
		}
	}
}
			
		
		
	
/*
** Assigns as next block the one passed in as bno
** returns actual next block (may vary if on a boundary)
**
** ltop() requires that the single and doubly indirect
** blocks be out on disk, previously
**
**  Modifies dip->di_nblocks, dip->di_rdaddr, dip->di_rindirect
**           and appropriate indirect and doubly indirect blocks
**
**  assumption:
**	  upon entry dip's disk addresses are all full blocks
**	  (ie. newblk() does not expand partial allocations)
*/
static int 
newblk(struct dinode  *dip,	/* ptr to growing dinode */
       ulong           size)	/* size of block wanted */
{
	fdaddr_t	frag;	
	fdaddr_t	ifrag;		/* new ind block disk address	*/
	fdaddr_t	difrag;		/* new doubly ind block disk address */
	fsblk_t		fsblk;	        /* block buffer   		*/
	daddr_t		*daddrp;	/* refer to fsblk as singly ind */
	struct idblock	*idblkp;	/* refer to fsblk as doubly ind */
	int		rc;
	int		oldnaddr;

	
	frag.d = ifrag.d = difrag.d = 0;
	daddrp = (daddr_t *) &fsblk;
	idblkp = (struct idblock *) &fsblk;
	/*
	 *  don't deal with files that already have partial allocations
	 *  should never happen...	 
	 */
	if (dip->di_nblocks & FragPerBlkMask)
		return -FSHERR_NEWBLKFAIL;
	/*
	 *  don't want newblk() mess with di_size, so don't use
	 *  NUMDADDRS() macro, and get number of disk addresses
	 *  based on di_nblocks.  (It is impossible for dip to have a
	 *  hole).
	 */
	oldnaddr = dip->di_nblocks + FragPerBlk - 1 >> FragPerBlkShift;
	if ((rc = dbget(&frag.f, NOINDIRECT(oldnaddr + 1) ?
					size : BLKSIZE)) < 0)
		return rc;
	
	if (NOINDIRECT(oldnaddr + 1))
	{
		dip->di_rdaddr[oldnaddr] = frag.d;
		markit(DiskMap, frag.f, True);
	}
	else if (!ISDOUBLEIND (oldnaddr + 1))
	{
		/*
		 ** just went indirect? attach first indirect block
		 ** copy direct block addresses into first NDADDR slots
		 ** and new one after that
		 */
		if (NOINDIRECT (oldnaddr))
		{
			memset((void *)daddrp, 0, (size_t)BLKSIZE);
			memcpy((void *)daddrp, (void *)dip->di_rdaddr,
				(size_t)sizeof(dip->di_rdaddr));
			
			if ((rc = dbget(&ifrag, BLKSIZE)) < 0)
				return rc;
			dip->di_rindirect = ifrag.d;
			*(daddrp + NDADDR) = frag.d;
			markit(DiskMap, ifrag.f, True);
			markit(DiskMap, frag.f, True);
		}
		else
		{
			/*
			 *  we were already indirect, copy address into next
			 *  available slot and write the block back out
			 */
			ifrag.d = dip->di_rindirect;
			if ((rc = bread(DevFd, (char *)daddrp, ifrag.f)) < 0)
				return -FSHERR_BRDWRFAIL;
			*(daddrp + SIDNDX(oldnaddr)) = frag.d;
			markit(DiskMap, frag.f, True);
		}
		if ((rc = bwrite(DevFd, (char *)daddrp, ifrag.f)) < 0)
			return -FSHERR_BRDWRFAIL;
	}
	else if (ISDOUBLEIND (oldnaddr + 1))
	{
		/*
		 *  just went double indirect?  will need a second single
		 *  indirect block
		 */
		if (!ISDOUBLEIND (oldnaddr))
		{
			/*
			 *  put original sgl indir blk in slot 0 dbl indir blk
			 *  get disk block for new dbl indir blk & put in inode
			 *  get disk block for new sgl indir blk & put in
			 *  	slot 1 of dbl indir blk
			 *  mark sgl and dbl indir blks as allocated
			 *  write out dbl indir blk
			 */
			memset((void *)idblkp, 0, (size_t)BLKSIZE);
			idblkp->id_raddr = dip->di_rindirect;
			
			if ((rc = dbget(&difrag.f, BLKSIZE)) < 0)
				return rc;
			dip->di_rindirect      = difrag.d;
			
			if ((rc = dbget(&ifrag.f, BLKSIZE)) < 0)
				return rc;
			(idblkp + 1)->id_raddr = ifrag.d;
			
			markit(DiskMap, difrag.f, True);
			markit(DiskMap, ifrag.f, True);
			
			if ((rc = bwrite(DevFd, (char *)idblkp, difrag.f)) < 0)
				return -FSHERR_BRDWRFAIL;
			
			memset((void *)daddrp, 0, (size_t)sizeof(fsblk_t));
			*daddrp = frag.d;
			markit(DiskMap, frag.f, True);
			
			if ((rc = bwrite(DevFd, (char *)daddrp, ifrag.f)) < 0)
				return -FSHERR_BRDWRFAIL;
		}
		else
		{
			/*
			 ** gargantuan?
			 */
			if (oldnaddr + 1 > AIX_FILE_MAXBLK)
				return -FSHERR_FILE2BIG;
			/*
			 ** more room in current indirect block?
			 */
			if (SIDNDX(oldnaddr))
			{
				/*
				 ** copy address into next available slot
				 ** and write the block back out
				 */
				difrag.d = dip->di_rindirect;	      
				if ((rc = bread(DevFd, (char *)idblkp,
						difrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
				
				ifrag.d = (idblkp+DIDNDX(oldnaddr))->id_raddr;
				if ((rc = bread (DevFd, (char *)daddrp,
						 ifrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
				
				*(daddrp + SIDNDX(oldnaddr)) = frag.d;
				markit(DiskMap, frag.f, True);
				if ((rc = bwrite(DevFd, (char *)daddrp,
						 ifrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
			}
			else
			{
				/*
				 * no remaining space in singly indirect
				 *  block, so get another
				 */
				
				difrag.d = dip->di_rindirect;      
				if ((rc = bread(DevFd, (char *)idblkp,
						 difrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
				
				if ((rc = dbget(&ifrag.f, BLKSIZE)) < 0)
					return rc;
				
				(idblkp+DIDNDX(oldnaddr))->id_raddr = ifrag.d;
				markit(DiskMap, ifrag.f, True);
				if ((rc = bwrite(DevFd, (char *)idblkp,
						  difrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
				
				memset((void *)daddrp, 0, (size_t)BLKSIZE);
				*daddrp = frag.d;
				markit(DiskMap, frag.f, True);
				
				if ((rc = bwrite(DevFd, (char *)daddrp,
						 ifrag.f)) < 0)
					return -FSHERR_BRDWRFAIL;
			}
		}
	}
	dip->di_nblocks += FragPerBlk - frag.f.nfrags;
	return LIBFS_SUCCESS;
}
    
/*
** lookup
**
** matches name with entry in file tree, enforces pathnames to only
** go through directories and prevents links to directories
**  <= 0 error
**  >  0 match, value is inumber of matched entry
*/
static long
lookup (char          *pathname,	/* pathame     */
	struct pname  *cur_dir)		/* current dir */
{
	struct pname  *pnp; 
	char          *slashp;
	int            left_len;
	char          *left_most;
	int            up; 
	
	if (pathname == NILPTR (char) || RootDir == NILPTR (struct pname)
	    || cur_dir == NILPTR (struct pname))
		return -FSHERR_LKUPARGSBAD;
	
	/*
	 ** 3 cases:
	 **   1. pathname is absolute
	 **   2. pathname is a branch in current directory
	 **   3. pathname is a leaf in current directory
	 */
	
	if (*pathname == Proto_SLASH)
	{
		return lookup (++pathname, RootDir);
	}
	else if ((slashp = strchr (pathname, Proto_SLASH)) != NILPTR (char))
	{
		left_len = slashp - pathname;
		if ((left_most = (char *) malloc(left_len + 1)) ==
		    NILPTR (char))
			return -FSHERR_NOMEM;
		
		memset((void *)left_most, 0, (size_t)(left_len + 1));
		if (strncpy (left_most, pathname, (int) left_len) != left_most)
			return -FSHERR_STRCPYFAIL;
		
		if (strcmp (left_most, DOT) == 0 ||
		    (up = strcmp (left_most, DOTDOT)) == 0)
		{
			return lookup (slashp + 1, up == 0 ? cur_dir->parent :
				       cur_dir);
		}
		
		for (pnp = cur_dir->first_born; pnp; pnp = pnp->siblings)
		{
			if (strcmp (pnp->name, left_most) == 0)
				break;
		}
		
		if (pnp == NILPTR (struct pname))
		{
			strncpy (ProtoMsg, PROTO_MSG (PROTO_NOBRANCH,
						      "branch doesn't exist"),
				 sizeof ProtoMsg);
			return -FSHERR_PROTOERR;
		}
		
		if (pnp->is_dir == True)
		{
			return lookup (slashp+1, pnp);
		}
		
		strncpy (ProtoMsg,
			 PROTO_MSG (PROTO_BRANCHNOTDIR,
				    "branch is not a directory"),
			 sizeof ProtoMsg);
		return -FSHERR_PROTOERR;
	}
	else	/* must be a leaf in cur_dir */
	{
		for (pnp = cur_dir->first_born; pnp; pnp = pnp->siblings)
		{
			if (strcmp (pnp->name, pathname) == 0)
			{
				if (pnp->is_dir == True)
				{
					strncpy (ProtoMsg,
						 PROTO_MSG(PROTO_MATCHDIR,
						        "matched a directory"),
						 sizeof ProtoMsg);
					return -FSHERR_PROTOERR;
				}
				return pnp->inum;
			}
		}
	}
	strncpy (ProtoMsg, PROTO_MSG (PROTO_NOTARGET, "target not found"),
		 sizeof ProtoMsg);
	return -FSHERR_PROTOERR;
}

/*
** returns next available inode number
** if the address given is non-zero
**   store this inode number there mark the inode map
*/  

static long
iget (ino_t *inum,
      bool_t mark_map)
{
	if (NextInum >= Inodes)
		return -FSHERR_NOINODES;
	if (mark_map)
		markit(InodeMap, (int) NextInum, mark_map);
	*inum = NextInum++;
	return 0;
}

/*
 * returns next available disk block number
 *
 */  

#define BITSPERDWORD 64

/*
 * returns next available disk block number
 * if we eventually return a block number out of range, we
 *     complain somewhere else when the bwrite is done
 */  
static long
dbget (frag_t *frag,	/* disk address of frag we're getting	*/
       int    size) 	/* num bytes frag has to hold		*/
{
	int i, curag, nfrags, rem;
	
	if (size <= 0 || size > BLKSIZE)
		return -FSHERR_DBGETSIZE;
	
	if (NextDfrag >= BLK2FRAG(Nblocks) - 1)
		return -FSHERR_NOBLOCKS;

	nfrags = BYTE2FRAG(size + FragmentSize - 1);
	/*
	 * make sure we don't let a frag's allocation span a double
	 * word in the map
	 */
	rem = NextDfrag & (BITSPERDWORD - 1);
	if (rem + nfrags - 1 >= BITSPERDWORD)
		NextDfrag += BITSPERDWORD - rem;
	if (NextDfrag + nfrags - 1 > LastDfrag)
	{
		curag = (NextDfrag + nfrags - 1) / DiskAgsize;
		LastDfrag = (curag + 1) * DiskAgsize - 1;
		NextDfrag = (curag ? (curag * DiskAgsize) :
			             BLK2FRAG(INODES_B)) +
			    INO2FRAG(InodeAgsize);
	}
	frag->new = 0;	
	frag->nfrags = FragPerBlk - nfrags;
	frag->addr = NextDfrag;
	NextDfrag += nfrags;
	return 0;
}

/*
 *  sltop
 *  special logical block to physical block
 *  note:  no special files are fragged... 
 */
static int
sltop (pin, lbno)
     struct init_ino  *pin;
     ulong             lbno;
{
  ulong   bno;
  
  bno = lbno < (sizeof (pin->rsvdblk) / sizeof (ulong)) ? pin->rsvdblk[lbno]:
           *(pin->moreblks + lbno - (sizeof (pin->rsvdblk) / sizeof (ulong)));
  return bno;
}


int
proto_getmode (modep)
	mode_t *modep;
{
	char      *tokp;
	char      *cp;
	mode_t     mode; 
	int        i;
	
	if ((tokp = proto_gets()) == NILPTR (char))
		return -FSHERR_PROTOERR;
	
	if (strlen (tokp) != sizeof Modes / sizeof (struct modes))
	{
		strncpy (ProtoMsg, PROTO_MSG (PROTO_BADMODELEN,
					      "bad mode length"),
			 sizeof ProtoMsg);
		return -FSHERR_PROTOERR;
	}
	
	mode = 0;
	for (i = 0; i < sizeof Modes / sizeof (struct modes); i++)
	{
		if ((cp = strchr (Modes[i].keys, (int)*(tokp+i))) ==
		    NILPTR (char))
		{
			strncpy (ProtoMsg, PROTO_MSG (PROTO_BADMODECHAR,
						      "bad mode character"),
				 sizeof ProtoMsg);
			return -FSHERR_PROTOERR;
		}
		mode |= Modes[i].mod[(int) (cp - Modes[i].keys)];
	}
	memcpy((void *)modep, (void *)&mode, (size_t)sizeof(mode_t));
	return FSHERR_GOOD;
}



/*
** proto get permissions
*/
static int
proto_getperm (modep)
     mode_t  *modep;
{
	mode_t  perm;
	char    *cp;
	char    *tokp;
	
	if ((tokp = proto_gets()) == NILPTR (char))
		return -FSHERR_PROTOERR;
	
	cp = NILPTR (char);
	perm = strtol (tokp, &cp, Proto_OCTAL);
	if ((perm == 0 && cp == tokp) ||
	    (cp != NILPTR (char) && *cp != Proto_NULLC))
	{
		strncpy (ProtoMsg, PROTO_MSG (PROTO_BADCONV, "bad conversion"),
			 sizeof ProtoMsg);
		return -FSHERR_PROTOERR;
	}
	
	*modep |= perm;
	return FSHERR_GOOD;
}



/*
** proto_gets
**
**
**   copies the next token from the proto file to a private string
**   and returns a pointer to it
**
**   NILPTR (char) = eof or error, context is maintained by the caller 
**
*/
static char *
proto_gets()
{
	char	     *tok = NILPTR (char);
	int	      toksiz;
	static int  incomment = 0;
	
	if (*ProtoPos == Proto_NULLC)
	{
		strncpy (ProtoMsg, PROTO_MSG (PROTO_PREMATUREOF,
					      "premature EOF"),
			 sizeof ProtoMsg);
		return NILPTR (char);
	}
	/*
	 ** calculate size of buffer needed for this token
	 ** skip over leading whitespace, but first check for
	 ** need to increment line counter
	 */
	if (ProtoPos == ProtoBuf || *(ProtoPos-1) == Proto_NEWLINE)
	{
		incomment = 0;
		ProtoCurLine++;
	}    
	toksiz = strcspn (ProtoPos, Proto_WHITESPACE);
	if (toksiz == 0)
	{
		ProtoPos += strspn (ProtoPos, Proto_WHITESPACE);
		return proto_gets();
	}
	if ((tok = (char *) malloc (toksiz+1)) == NILPTR (char))
	{
		strncpy (ProtoMsg,
			 PROTO_MSG (PROTO_INTERNALMEM,
				    "internal error: out of memory"),
			 sizeof ProtoMsg);
		return NILPTR (char);
	}
	
	memset((void *)tok, 0, (size_t)(toksiz+1));
	if ((int)strncpy (tok, ProtoPos, (int) toksiz) <= 0)
	{
		strncpy (ProtoMsg,
			 PROTO_MSG (PROTO_INTERNALSCPY,
				    "internal error: strncpy failed"),
			 sizeof ProtoMsg);
		return NILPTR (char);
	}
	ProtoPos += toksiz + 1;
	if (incomment || *tok == Proto_COMMENT)
	{
		incomment++;
		free ((void *)tok);	/* no one will ever need this */
		return proto_gets();
	}
	ProtoErr = tok;
	return tok;
}   

/*
** proto_getl
**
**  returns the value of the next token from the proto file interpreted
**  as a long
**
*/
static long
proto_getl(lp)
	long  *lp;
{
	char  *tokp;
	char  *cp;
	long   val;
	
	if ((tokp = proto_gets()) == NILPTR (char))
		return -FSHERR_PROTOERR;
	
	cp = NILPTR (char);
	val = strtol (tokp, &cp, 0);	/* 0 = any base */
	if ((val == 0 && cp == tokp) ||
	    (cp != NILPTR (char) && *cp != Proto_NULLC))
	{
		strncpy (ProtoMsg, PROTO_MSG (PROTO_BADCONV, "bad conversion"),
			 sizeof ProtoMsg);
		return -FSHERR_PROTOERR;
	}
	
	if (lp)
		*lp = val;
	return FSHERR_GOOD;
}


/*
** proto_count_blocks
**
**  counts the number of direct, singly indirect and doubly indirect needed
**
*/
static void
proto_count_blocks(long bytes)
{
	int ndaddrs;		/* number of disk addresses needed */
	int nfrags;
  
  	ndaddrs = BYTE2BLK(bytes + BLKSIZE - 1);
	nfrags = BYTE2FRAG(bytes + FragSize - 1);
	/*
	 *  increment number of actual disk addr's (direct disk addrs)
	 */
	Pstat.nb[DIR] += nfrags;
  
	/*
	 *  only need to go singly indirect -
	 *  increment by no. of blocks plus an indirect block
	 */
	if (!NOINDIRECT (ndaddrs))
	{
		if (ISDOUBLEIND(ndaddrs))
			Pstat.nb[DBL]++;
		Pstat.nb[SGL] += (ndaddrs + DADDRPERBLK - 1) / DADDRPERBLK;
	}		  
}

/*
** perrmsg_proto
*/
static void
perrmsg_proto (err)
   int err;
{
   if (abs(err) == FSHERR_PROTOERR && interactive())
   {
        fprintf (stderr, "%s: ", ProgName);
	fprintf (stderr,
		 ERR_MSG (FSHERR_PROTOERR, errmsgs[abs(err)]), ProtoCurLine);
	if (ProtoErr != NILPTR (char)) 
	    fprintf (stderr, " at \"%s\"", ProtoErr);
	if (strncmp (ProtoMsg, NILSTR, sizeof ProtoMsg) != 0)
	    fprintf (stderr, " (%s)", ProtoMsg);
	fprintf (stderr, "\n");
   }
   return;
}

/*
** write specs
*/
static int
write_specs()
{
  fsblk_t  *fp;
  int       i;
  int       b;
  long      no_blks;
  daddr_t   bno;
  fdaddr_t	frag;
  int       rc;
  
  for (i = 0; i < sizeof init_inos / sizeof (struct init_ino); i++)
  {
    no_blks  = init_inos[i].nblocks;
    if (init_inos[i].inum == INDIR_I)
	fp = Indirect;
    else if (init_inos[i].inum == INOMAP_I)
    {
	inittree(InodeMap);
	fp = (fsblk_t *) InodeMap;
    }
    else if (init_inos[i].inum == DISKMAP_I)
    {
	inittree(DiskMap);
	fp = (fsblk_t *) DiskMap;
    }
    else
	fp = NILPTR (fsblk_t);

#ifdef DEBUG
    if (Debug (FSHBUG_SUBDATA) && init_inos[i].inum == INODES_I)
	dump_inos();
#endif 
    if (fp)
    {
      /*
      ** write out data blocks
      ** indirect blocks were written out in init_specs
      */
      for (b = 0; b < no_blks; b++, fp++)
      {
	frag.d = sltop(&init_inos[i], (ulong)b);
	if ((rc = bwrite(DevFd, (char *)fp, frag.f)) < 0)
		return -FSHERR_BRDWRFAIL;
      }
    }
  }
  return FSHERR_GOOD;
}

#ifdef DEBUG
/*
** print out all inodes in internal copy of .inodes
*/

static void
dump_inos()
{
  struct dinode    dipbuf;
  struct dinode   *dip;
  int              i;
  int              j;
  int              rc;
  long             no_inos;
  daddr_t         *daddrp = NILPTR (daddr_t);
  struct idblock  *idblkp = NILPTR (struct idblock);
  
  no_inos = Inodes;
  for (i = 0, dip = &dipbuf; i < no_inos; i++)
  {
    if (get_inode(DevFd, dip, (ino_t)i) >= 0 && dip->di_nlink != 0)
    {
      dbg_printf (FSHBUG_SUBDATA, ("Inode: %d (%s%s%s)\n", i,
				   S_ISFIFO (dip->di_mode)? "fifo":
				   S_ISDIR (dip->di_mode)? "directory":
				   S_ISCHR (dip->di_mode)? "char. spec.":
				   S_ISBLK (dip->di_mode)? "block spec.":
				   S_ISREG (dip->di_mode)? "regular":
				   ((dip->di_mode & (S_IFMT)) == (S_IFLNK))?
				   "soft link": "unknown type",
				   dip->di_nblocks == 0? ",empty" :
				   NOINDIRECT(dip->di_nblocks)? NILSTR:
				   !ISDOUBLEIND (dip->di_nblocks)?
				   ",single": ",double",
				   (dip->di_mode&IFJOURNAL) == IFJOURNAL?
				   ",journalled": ",not journalled"));
      dbg_printf (FSHBUG_SUBDATA, ("di_nlink=%d\tdi_gen=%d\n",
				   dip->di_nlink, dip->di_gen));
      dbg_printf (FSHBUG_SUBDATA, ("di_mode=0%o\tdi_acct=%d\n",
				   dip->di_mode, dip->di_acct));
      dbg_printf (FSHBUG_SUBDATA, ("di_uid=%d\tdi_gid=%d\n",
				   dip->di_uid, dip->di_gid));
      dbg_printf (FSHBUG_SUBDATA, ("di_size=%d\tdi_nblocks=%d\n",
				   dip->di_size, dip->di_nblocks));
      dbg_printf (FSHBUG_SUBDATA, ("atime=%d\tctime=%d\tmtime=%d\n\n",
				   dip->di_atime, dip->di_ctime,
				   dip->di_mtime));
      if ((dip->di_mode & (S_IFMT)) == (S_IFLNK) && dip->di_nblocks == 0)
      {
	dbg_printf (FSHBUG_SUBDATA, (" -> %s\n", dip->di_symlink));
      }
      else if (dip->di_nblocks != 0)
      {
	if (NOINDIRECT (dip->di_nblocks))
	{
	  dbg_printf (FSHBUG_SUBDATA, ("Direct blocks: "));
	  for (j = 0; j < NDADDR; j++)
	      if (dip->di_rdaddr[j] != 0)
		  dbg_printf (FSHBUG_SUBDATA, ("%d ", dip->di_rdaddr[j]));
	  dbg_printf (FSHBUG_SUBDATA, ("\n"));
	}
	else
        {
	  dbg_printf (FSHBUG_SUBDATA, ("%sndirect block: %d ",
				       ISDOUBLEIND (dip->di_nblocks)?
				       "Doubly i": "I", dip->di_rindirect));
	  if (!ISDOUBLEIND (dip->di_nblocks))
	  {
	    dbg_printf (FSHBUG_SUBDATA, ("Data blocks: ", dip->di_rindirect));
	    if ((daddrp = (daddr_t *) malloc (sizeof (fsblk_t))) == NILPTR (daddr_t))
	    {
		dbg_printf (FSHBUG_SUBDATA, ("daddrp malloc failed\n"));
		return;
	    }
	    rc = bread ((char *) daddrp, (daddr_t) dip->di_rindirect, sizeof (fsblk_t));
	    if (rc < 0)
	    {
	      dbg_printf (FSHBUG_SUBDATA, ("dump_inos/bread failed\n"));
	      return;
	    }
	    
	    for (j = 0; j < DADDRPERBLK && *(daddrp+j); j++)
	    {
	      dbg_printf (FSHBUG_SUBDATA, ("%d ", *(daddrp+j)));
	    }
	    dbg_printf (FSHBUG_SUBDATA, ("\n"));
	 }
	 else
	 {
	    if ((idblkp = (struct idblock *) malloc (sizeof (fsblk_t))) == NILPTR (struct idblock))
	    {
	      dbg_printf (FSHBUG_SUBDATA,
				("dump_inos/idblock malloc failed\n"));
	      return;
	    }
	    rc = bread ((char *) idblkp, (daddr_t) dip->di_rindirect,
			sizeof (fsblk_t));
	    if (rc < 0)
	    {
	      dbg_printf (FSHBUG_SUBDATA, ("dump_inos/bread failed\n"));
	      return;
	    }
	    for (i = 0; i <= DIDNDX (dip->di_nblocks); i++)
	    {
	      dbg_printf (FSHBUG_SUBDATA,
			  ("\nIndirect block: %d Data blocks: ",
			   (idblkp+i)->id_raddr));
			   
	      if ((daddrp = (daddr_t *) malloc (sizeof (fsblk_t))) == NILPTR (daddr_t))
	      {
		dbg_printf (FSHBUG_SUBDATA,
				("dump_inos/daddrp malloc failed\n"));
		return;
	      }
	      rc = bread ((char *) daddrp,  (daddr_t) (idblkp+i)->id_raddr,
			  sizeof (fsblk_t));
	      if (rc < 0)
	      {
		dbg_printf (FSHBUG_SUBDATA, ("dump_inos/bread failed\n"));
		return;
	      }
	      for (j = 0; j < DADDRPERBLK && *(daddrp+j); j++)
	      {
		dbg_printf (FSHBUG_SUBDATA, ("%d ", *(daddrp+j)));
	      }
	      dbg_printf (FSHBUG_SUBDATA, ("\n"));
	    }
	    dbg_printf (FSHBUG_SUBDATA, ("\n"));
	 }
	  if (idblkp)
	  {
	     free ((void *)idblkp);
	     idblkp = NILPTR (struct idblock);
	  }
	  if (daddrp)
	  {
	      free ((void *)daddrp);
	      daddrp = NILPTR (daddr_t);
	  }
	}
      }
      dbg_printf (FSHBUG_SUBDATA, ("\n"));
    }
  }
  return;
}

/*
** dump directory
**
*/
static void
dump_directory (ino, inum)
     struct dinode  *ino;
     ino_t           inum;
{
  daddr_t    l;		/* logical block no.     */
  fdaddr_t    p;		/* physical block no.    */
  direct_t  *dp;	/* directory block ptr   */
  fsblk_t     fsblk;	/* fs block buffer       */
  fsblk_t    *fsbp;	/* fs block ptr          */
  int         doff;	/* offset into dir block */
  int         n;	/* no. of entries        */
  int         dirb;	/* dirblk within fsblk   */
  
  fsbp = &fsblk;
  n    = 0;
  
  if (ino->di_nblocks == 0)
    dbg_printf (FSHBUG_SUBDATA, ("dump_directory/dir is empty\n"));
 
  for (l = 0; l < ino->di_nblocks; l++)
  {
    if (ltop(DevFd, &p.f, ino, l) < 0)
    {
      dbg_printf (FSHBUG_SUBDATA, ("dump_directory/ltop failed\n"));
      return;
    }
  
    if ((rc = bread(DevFd, (char *)fsbp, p.f)) < 0)
    {
      dbg_printf (FSHBUG_SUBDATA, ("dump_directory/bread failed\n"));
      return;
    }

    for (dirb = 0; dirb < sizeof (fsblk_t) / sizeof (dirblk_t); dirb++)
    {
	dp = (direct_t *) ((char *) fsbp + dirb * sizeof (dirblk_t));
	if (dp->d_reclen == 0)
		continue;

	dbg_printf (FSHBUG_SUBDATA,
		("\ninum=%d\tdirblk %d of logical blk %d\n", inum, dirb, l));

	for (doff = 0; doff + dp->d_reclen < sizeof (dirblk_t);
		 dp = (direct_t *) ((char *) dp + dp->d_reclen))
    	{
	      dbg_printf (FSHBUG_SUBDATA,
		("%d\toffset %d\tino %d\treclen %d\tnamlen %d\t'%s'\n",
			   ++n, doff, dp->d_ino,
			   dp->d_reclen, dp->d_namlen, dp->d_name));
	      doff += dp->d_reclen; 
	}
	dbg_printf (FSHBUG_SUBDATA,
		("%d\toffset %d\tino %d\treclen %d\tnamlen %d\t'%s'\n",
		++n, doff, dp->d_ino, dp->d_reclen,
	     	dp->d_namlen, dp->d_name));
     }
  }
  dbg_printf (FSHBUG_SUBDATA, ("\n"));
  return;
}
#endif DEBUG


static void
markit(struct vmdmap *	p0,		/* pointer to page zero of map	*/
       frag_t		frag,		/* map object to set or clear	*/
       bool_t		setflag)	/* set to 1 or clear to 0	*/
{
	uint  nag, p, rem, w, bit, nbits, wordbits, n;
	struct vmdmap *p1;
	uint *wmap, *pmap, dbperpage;

	/*
	 *  calculate page number in map, alloc group number
	 *  in page , and  word and bit number in word.
	 */
	dbperpage = (MakeVer3) ? DBPERPAGE : DBPERPAGEV4;
	p = frag.addr / dbperpage;
	rem = frag.addr - p * dbperpage;
	nag = rem / p0->agsize;
	w = rem >> L2DBWORD;
	bit = rem - (w << L2DBWORD);

	if (MakeVer3 == TRUE)
	{
		p1 = p0 + p;
		wmap = (uint *)((uint *)p1 + LMAPCTL/4);
		pmap = wmap + WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = (struct vmdmap *)((char *) p1 + ((p & 0x7) << 9));
		pmap = wmap + WPERPAGEV4;
	}

	if (p0->maptype == INODEMAP)
		nbits = 1;
	else
		nbits = FragPerBlk - frag.nfrags;
	
        /*
	 *  now we process the first word.
         */
	wordbits = MIN(32 - bit, nbits);
        for (n = 0; n < wordbits; n++, bit++)
		if (setflag == True)
		{
			pmap[w] |= (UZBIT >> bit);
			wmap[w] |= (UZBIT >> bit);
		}
		else
		{
			pmap[w] &= ~(UZBIT >> bit);
			wmap[w] &= ~(UZBIT >> bit);
		}
			
        /*
	 *  process the second word.
         */
	wordbits = nbits - wordbits;
        w++;
        bit = 0;
        for (n = 0; n < wordbits; n++, bit++)
		if (setflag == True)
		{
			pmap[w] |= (UZBIT >> bit);
			wmap[w] |= (UZBIT >> bit);
		}
		else
		{
			pmap[w] &= ~(UZBIT >> bit);
			wmap[w] &= ~(UZBIT >> bit);
		}

	/*
	 *  now update the stats
	 */
	if (setflag == True)
	{
		p0->freecnt -= nbits;
		p1->agfree[nag] -= nbits;
	}
	else
	{
		p0->freecnt += nbits;
		p1->agfree[nag] += nbits;
	}
}

/*
 * initmap()
 * initialize a vmdmap map
 */
static void
initmap(p0, nblocks,maptype)
     struct vmdmap *p0;
     long nblocks;     
     int  maptype;
{
        int k,nb,npages;
	fdaddr_t	frag;
        int dbperpage, np;

	/* set map size and type.
	 * initialize each page.
	 */
	dbperpage = (MakeVer3) ? DBPERPAGE : DBPERPAGEV4;
	npages = (nblocks + dbperpage - 1)/dbperpage;
	p0->mapsize = nblocks;
	p0->maptype = maptype;
	for(k = 0; k < npages; k++)
	{
		nb = MIN(nblocks, dbperpage);
		nblocks -= nb;
		imappage(p0,k, nb);
	}
	/* allocate block 0 or inode 0  of the map
	 */
	frag.d = 0;
	markit(p0, frag.f, True);
	return;
}

/* 
 * imappage(p0,p,nblocks)
 *
 * initialize one page of a map
 *
 * input parameters
 *	p0 - pointer to page of map
 *	p  - page number in map.	
 *	nblocks - number of blocks this page
 *	
 */
imappage(p0,p,nblocks)
struct vmdmap *p0;
uint p;
uint nblocks;
{

	struct vmdmap *p1;
	uint k,n,nag, rem, nwords, nwords1;
	int agrsize;
	uint *wmap, *pmap, wperpage;

	agrsize =  (p0->maptype == FSDISKMAP) ? DiskAgsize : InodeAgsize;

	if (MakeVer3 == TRUE)
	{
		p1 = p0 + p;
		wmap = (uint *)(p1) + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
		wperpage = WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *)(p1 + 1 + (p & 0x7));
		p1 = (struct vmdmap *)((char *) p1 + ((p & 0x7) << 9));
		pmap = wmap + WPERPAGEV4;
		wperpage = WPERPAGEV4;
	}
	p1->agsize = agrsize;
	p1->clsize = clsize;
	if (MakeVer3 == TRUE)
	{
		p1->clmask = (ONES << (8 - clsize)) & 0xff;
		p1->version = ALLOCMAPV3;
	}
	else
	{
		p1->clmask = 0;
		p1->version = ALLOCMAPV4;
	}
	if (p != 0)
		p1->lastalloc = p1->mapsize = p1->freecnt  =
		                p1->maptype = p1->totalags = 0;
	/* complete allocation groups 
	 */
	nag = p1->agcnt  = nblocks/agrsize;
	for(k = 0; k < nag; k++)
		p1->agfree[k] = agrsize;
	
	nwords = nag*agrsize/DBWORD;

	/* partial allocation group ?
	 * blocks can be used only if enough space for all inodes.
	 * if not used they will be marked as allocated.
	 */
	if (p0->maptype == FSDISKMAP)
	{
		rem = nblocks % agrsize;
		if (rem >= InodeAgsize * sizeof(struct dinode) / FragmentSize )
		{
			p1->agfree[nag] = rem;
			p1->agcnt += 1;
			
			nwords1 = rem/DBWORD;
			for (k = 0; k < nwords1; k++)
			{
				pmap[k+nwords] = 0;
				wmap[k+nwords] = 0;
			}
			nwords += nwords1;

			/* partial word ?
			 */
			rem = rem - nwords1*DBWORD;
			if (rem)
			{
				pmap[nwords] = ONES >> rem;
				wmap[nwords] = ONES >> rem;
				nwords += 1;
			}
		}
		/*
		 *  since leftovers not enough for inodes, subtract it
		 *      from number of blocks described in this mappage
		 */
		else
			nblocks -= rem;
	}
	/* set the rest of the words in the page to ONES.
	 */
	for (k = nwords; k < wperpage; k++)
	{
		pmap[k] = ONES;
		wmap[k] = ONES;
	}

	/* update stats in page 0
	 */
	p0->freecnt += nblocks;
	p0->totalags += p1->agcnt;
}


/*
 * inittree(p0)
 * initialize the tree on each page of a map.
 * this is called after all allocates are done.
 */
static int
inittree(p0)
struct vmdmap * p0;
{
	int k,p,np;
	int dbperpage, wperpage;

	/* initialize the dmap table if necessary (ver3 only)
	 */
	if (MakeVer3 && dmaptab[0] == 0)
		idmaptab();

	/* calculate number of pages 
	 */
	if (MakeVer3)
	{
		dbperpage = DBPERPAGE;
		wperpage = WPERPAGE;
	}
	else
	{
		dbperpage = DBPERPAGEV4;
		wperpage = WPERPAGEV4;
	}
		
	np = (p0->mapsize + dbperpage -1)/dbperpage;
	for (p = 0; p < np; p++)
	{
		/* initialize the tree for page.
		 * (tree is zeros at this point).
	 	 */
		for(k = 0; k < wperpage; k += 2)
		{
			updtree(p0,p,k);
		}
	}
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
 * return longest string of zeros in 64-bit double word
 * addressed by ptr. used in find the maximum number of
 * contiguous free bits within a double word of an allocation
 * map.
 */
int
maxstring(ptr)
unsigned int *ptr;
{
	unsigned int max, m, mask, word;

	/* calculate longest string in first word.
	 */
	m = max = 0;
	mask = UZBIT;
	word = *ptr;
	while(mask)
	{
		if (word & mask)
		{
			max = MAX(max, m);
			m = 0;
		}
		else m = m + 1;
		mask = mask >> 1;
	}

	/* m is number of trailing zeros of first word.
	 * calculate longest string in second word,
	 * including trailing zeros of first word.
	 */
	mask = UZBIT;
	word = *(ptr + 1);
	while(mask)
	{
		if (word & mask)
		{
			max = MAX(max, m);
			m = 0;
		}
		else m = m + 1;
		mask = mask >> 1;
	}

	return (MAX(max,m));
}

/*
 * update vmdmap tree.
 *
 * input parameters:
 *		p0 - pointer to page zero of map
 *		p  - page number in map exclusive of control pages.
 *		ind - index within page of the word in bit map which changed 
 */

static int
updtree(p0,p, ind)
struct vmdmap *p0;
int p;
int ind;
{
	uint n, lp, k,index, maxc, max; 
	uchar *cp0, *cp1;
	struct vmdmap * p1;

	/* calculate max for the two words of the section
	 */
	ind = ind & (ONES << 1);
	
	if (p0->version == ALLOCMAPV3)
	{
		p1 = p0 + p;
		cp0 = (uchar *)(p1) + LMAPCTL + 4*ind; 
		max = 0;
		maxc = p1->clmask;
		for(n = 0; n < 8; n++, cp0++)
		{
			max = MAX(max, dmaptab[*cp0]);
			if (max >= maxc)
			{
				max = maxc;
				break;
			}
		}
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		cp0 = (uchar *)(p1 + 1 + (p & 0x7)) + ind*4;
		p1 = (struct vmdmap *)((char *)p1 + ((p & 0x7) << 9));
		max = maxstring((int *)cp0);
	}

	/* calculate pointers to leaf word and to the character
	 * in it that corresponds to ind; leaf words cover 8
	 * sections (32 bytes of map) so the low order 3-bits 
	 * of ind are shifted right by one.
	 */
	lp = LEAFIND + (ind >> 3);
	cp0 = (uchar *) &p1->tree[lp];
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
		cp0 = (uchar *) &p1->tree[lp];
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

static void
perrmsg (rc)
{
	if (rc > 0 && rc <= FSHERR_LASTIMP)
		if (strcmp(errmsgs[rc], NILSTR))
			fprintf(stderr, "%s: %s\n", ProgName,
				 ERR_MSG(rc, errmsgs[rc]));
	fflush (stderr);
}

/*
 * NAME:	validate_size
 *
 * FUNCTION:	Verify that the give size is legal for specified
 *		fragment and nbpi values. Print an error message if
 *		size is invalid.
 *		
 * PARAMETERS:	size_t	sz	- requested size in 512 blocks
 *		int	nbpi	- nbpi ratio
 *		int	fs	- fragment size
 *
 * RETURN:	if the size is legal return 0
 *		else return non-zero
 */
static int
validate_size (size_t	sz,
	       int	nbpi,
	       int	fs)
{	
	size_t	maxsz;
	int	rc = 0;

	maxsz = FS_NBPI_LIM(nbpi);
	if (sz > maxsz)
	{
		rc = FSHERR_NBPI;
		fprintf(stderr, ERR_MSG(FSHERR_NBPI, errmsgs[rc]), ProgName,
			nbpi, maxsz);
	}

	maxsz = FS_ADDR_LIM(fs);
	if (sz > maxsz)
	{
		rc = FSHERR_FRAG;
		fprintf(stderr, ERR_MSG(FSHERR_FRAG, errmsgs[rc]), ProgName,
			fs, maxsz);
	}

	if (rc)
	{
	    	fprintf(stderr, WARN_MSG(WARN_MAXFS, 
                "\nJFS file systems have the following size limitations:\n"));

	    	fprintf(stderr, WARN_MSG(WARN_FSMAX_TABLE, 
                "\nNBPI\tFragment Size\t\tMaximum Size (512-byte blocks)\n"));

		print_jfs_limits(stderr);
		fprintf(stderr, "\n");
		fflush(stderr);		
	 }
	 return rc;
}
