/* @(#)16	1.8  src/bos/usr/bin/dosdir/pcdos.h, cmdpcdos, bos41J, 9508A 2/8/95 12:17:29 */
/*
 * COMPONENT_NAME: LIBDOS  routines to read dos floppies
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  header file for PC-DOS internals      AIWS VERSION !!!!!
 *
 */

#include "dos.h"
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#ifdef DEBUG
#define TRACE(x) if (dostrace) _trace x; else
#else
#define TRACE(x)
#endif

typedef unsigned char byte;

/*
 * File attributes (df_attr)
 */
#define FIL_RO  0x01        /* Read-only file */
#define FIL_HDF 0x02        /* Hidden file */
#define FIL_SYS 0x04        /* System file */
#define FIL_VOL 0x08        /* Name/ext contain volume label (root dir only) */
#define FIL_HDD 0x10        /* Hidden subdirectory */
#define FIL_AR  0x20        /* Archive bit (file has been backed up) */

/*
 * Indicators from df_use
 */
#define DIR_NIL 0x00        /* Entry never used (past high water mark) */
#define DIR_MT  0xE5        /* Entry erased (was used) */

/*
 * Media descriptor
 */
#define MD_2SIDES 0x01      /* 2-sided if set, else 1-sided */
#define MD_8SEC   0x02      /* 8 sectors if set, else 9 sectors */
#define MD_REM    0x04      /* Removable if set, else not */
#define MD_MBO    0xF8      /* Must be ones */

/*
 * Device descriptor
 */
#define FAT_FD0     0xFF    /* 2-sided, 8-sector floppy */
#define FAT_FD1     0xFE    /* 1-sided, 8-sector floppy */
#define FAT_FD2     0xFD    /* 2-sided, 9-sector floppy */
#define FAT_FD3     0xFC    /* 1-sided, 9-sector floppy */
#define FAT_FD4	    0xF9    /* 2-sided, 15 sector floppy */
#define FAT_HD      0xF8    /* Hard disk ("Fixed", in IBM terminology) */

/*
 * Patition table constants
 */
#define PT_ADDR     0x1BE       /* offset in sector to table */
#define PT_SIGa     0x55        /* magic number at end of table (lo byte) */
#define PT_SIGb     0xAA        /* magic number at end of table (hi byte) */

#define PC_EOF          0xfff0
#define PC_ROOTDIR      1

#define MAXFILES        20
#define MAXDCB          20
#define DEVLEN          20	/* max length of a device name */

#define FCBMAGIC        0xfcb           /* magic number for FCB */
#define DCBMAGIC        0x4a41          /* magic number for DCB */

/*
 * Breakdown of time field !!INFO ONLY !!!
 *	       short dt_sec:5,  ** 2-second of modification **
 *		     dt_min:6,  ** Minute of modification (0-59) **
 *		     dt_hour:5; ** Hour of modification (0-23) **
 *
 * * Breakdown of date field     !!! INFO ONLY !!!
 *	       short dd_day:5,  ** Day of modification (1-31) **
 *		     dd_month:4,** Month of modification (1-12) **
 *		     dd_year:7; ** Year of modification (0-119: 1980-2099) **
 *
 * * Description of directory entries
 */
typedef struct {
	byte    df_use;         /* [0]          first byte of file name */
	byte    df_name[7];     /* [1-7]        File name */
	byte    df_ext[3];      /* [8-10]       File extension */
	byte    df_attr;        /* [11]         File attributes byte */
	byte    df_fill[10];    /* [12-21]      Reserved */
	byte    df_time[4];     /* [22-25]      modification time & date */
	byte    df_lcl;         /* [26]         starting cluster lo byte */
	byte    df_hcl;         /* [27]         starting cluster hi byte */
	byte    df_siz0;        /* [28]         file size lo byte */
	byte    df_siz1;        /* [29]         file size continued  */
	byte    df_siz2;        /* [30]         file size continued */
	byte    df_siz3;        /* [31]         file size hi byte */
} pc_dirent;


/*
 * BIOS Parameter block - Fixed disk only
 *      beginning of boot block on each partition
 *   !!!! INFO only  -- read as single bytes !!!!!
 */
typedef struct {
	byte    pb_jmp[3];      /* Jump to beginning of boot code */
	byte    pb_vers[8];     /* OEM name/version */
	ushort   pb_secsiz;      /* Bytes per sector */
	byte    pb_csize;       /* Cluster size (sectors, power of 2) */
	ushort   pb_res;         /* Number of reserved sectors */
	byte    pb_fatcnt;      /* Number of FATs */
	ushort   pb_dirsiz;      /* Number of (root) directory entries */
	ulong    pb_ptnsiz;      /* Size of disk partition in sectors */
	byte    pb_descr;       /* Media descriptor */
	ushort   pb_fatsiz;      /* FAT size in sectors */
	ushort   pb_sectrk;      /* Sectors per track */
	ushort   pb_headcnt;     /* Number of heads */
	ulong    pb_hidsec;      /* Number of hidden sectors */
} pc_bpb;


/*
 * Partition Descriptor
 */
typedef struct {
	byte    pt_bootind;     /* Boot Indicator */
	byte    pt_bhead;       /* Beginning Head */
	byte    pt_bsec;        /* Beginning Sector */
	byte    pt_bcyl;        /* Beginning Cylinder */
	byte    pt_sysind;      /* System Indicator */
	byte    pt_shead;       /* Ending Head */
	byte    pt_ssec;        /* Ending Sector */
	byte    pt_scyl;        /* Ending Cylinder */
	int     pt_start;       /* Starting sector */
	int     pt_size;        /* Partition size */
} pc_ptd;

/*
 * Partition Table
 */
typedef struct {
	pc_ptd          pt[4];
	char            siga;          /* Signature (cookie) (0x55) */
	char            sigb;          /*   Other half  (0xAA)*/
} pc_ptt;


/*
 *      incore FAT description
 */
typedef struct {
	unsigned short  cluster;    /* cluster value */
	unsigned short  usecount;   /* # users currently accessing cluster */
} icfat;
/*
 *      device data (one entry in table)
 */
typedef struct {
	long          magic;            /* DCB identifier word */
	int           lock;             /* semaphore for device access */
	icfat         *fat_ptr;         /* pointer to internal FAT */
	int           fat_desc;         /* shared-memory FAT descriptor */
	int           users;            /* user count */
	int	      home;		/* index for "current directory" */
	int           changed;          /* modified flag */
	int           ccount;           /* cluster count */
	int	      clsize;		/* cluster size in bytes */
	int           fatentsiz;        /* FAT entry size */
	int           zero;             /* relative byte offset start */
	int           root;             /* relative byte offset for root dir*/
	int           data;             /* relative byte offset for 1st cluster*/
	int           fd;               /* handle known to UNIX */
	int	      protect;		/* non-zero if open O_RDWR failed */
	pc_bpb        bpb;              /* BIOS param block */
	pc_ptt        ptn;              /* partition table */
	byte          dev_name[DEVLEN+1]; /* ASCII device name */
	key_t         devkey;           /* unique key based on dev name */
} DCB;

/*
 *      device table
 */
typedef struct
{
	DCB  dev_id[MAXDCB];
	char validity[10];
} DCB_TBL;

/*
 *      open file data (one entry in table)
 */

typedef struct {
	long          magic;            /* FCB identifier word */
	long          d_seek;           /* disk absolute location of dir*/
	DCB           *disk;            /* pointer into device table */
	int           changed;          /* modified flag */
	long          size;             /* file size in bytes */
	long          seek;             /* disk locn for current cluster */
	long          offset;           /* "file pointer" into file */
	int           clseek;           /* offset into current cluster */
	int	      startcluster;	/* starting cluster of file */
	int           nowcluster;       /* current cluster of file */
	int           clustsize;        /* cluster byte count */
	long          timestamp;        /* Unix timestamp, if changed */
	byte          oflag;            /* open flags */
} FCB;

/*
 *      device table
 */
typedef struct
{
	FCB     fcb[MAXFILES];
} FILE_TBL;

typedef struct  {
	long            seek;           /* real disk addr for "data area" */
	int             count;          /* number of dir entries in cluster */
	DCB             *disk;          /* DCB pointer for disk */
	int             mode;           /*          */
	int             tnxtcl;         /* the next cluster number */
} SRCHBLK;



/*	current directory and disk storage  */

typedef struct {
	DCB		*disk;		/* current disk */
	long            pathname;       /* seek value for directory entry */
	int             start;          /* 1st cluster # of this directory */
	int             nxtcluster;     /* next cluster of dir */
} HOME_DIR;

typedef struct {
	HOME_DIR	dir[MAXDCB];	/* home directory per device*/
}  DIR_TBL;

/*	file locking structures */
#define DOS_NLOCKS 200

/* actions for dlock */
#define L_LOCK 0
#define L_UNLOCK 1
#define L_TEST 2
#define L_CLEANUP 3

/* actions for lfind */
#define ACT_MATCH 1
#define ACT_MEMBER 2

/* macros for distinct file handle */
#define FHANDLE1(a) a->disk->devkey
#define FHANDLE2(a) a->startcluster

/* defaults for dread and dwrite, should be dosioctls */
#define LOCK_RETRY_TIME 1
#define LOCK_RETRY_COUNT 3

struct lockent {
	int pid;
	int fh1;
	int fh2;
	int offset;
	int length;
};

/***************** PER-system table ***********************/
DCB_TBL dev_tbl;                                            /* device table */

/***************** PER-process structures *****************/
FILE_TBL files;                                                /* FCB table */
DIR_TBL current;                                   /* "current" directories */

extern int     errno;
extern byte devicename[];

pc_dirent *dir;			/* pointer to a directory */
static byte cluster[16384];	/* 512 * 15 sectors */
