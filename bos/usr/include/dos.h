/* @(#)16	1.9  src/bos/usr/include/dos.h, cmdpcdos, bos411, 9428A410j 6/16/90 00:09:32 */
/*
 * COMPONENT_NAME: CMDPCDOS  routines to read dos floppies
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* dos.h	5.1 - 86/12/09 - 06:04:09 */
#ifndef _H_DOS
#define _H_DOS
#include <sys/types.h>

/*  open flags used by dosopen, doscreate, dospwd, dosfirst   */
/*  the low order bits are standard unix protection bits      */

#define DO_RDONLY        0
#define DO_WRONLY        1
#define DO_RDWR          2
#define DO_APPEND        8        /* append (writes at the end of file) */
#define DO_CREAT   0x00100        /* open with create (uses 3rd open arg) */
#define DO_TRUNC   0x00200        /* open with truncation        */
#define DO_EXCL    0x00400        /* exclusive open              */

#define DO_SYSTEM  0x08000        /* make or show system files  (DOS only) */
#define DO_HIDDEN  0x40000        /* make or show hidden files   */
#define DO_ASCII   0x80000        /* ascii normalization on read */

/****************** MODE FLAG VALUES *********************************/
#define S_READONLY      1       /* always ignored by search         */
#define S_HIDDEN        2       /* include hidden files in search   */
#define S_SYSTEM        4       /* include system files in serch    */
#define S_VOLUME        8       /* always ignored by search         */
#define S_DIR        0x10       /* include directories in search    */
#define S_ARCHIVE    0x20       /* always ignored by search         */
#define S_REG        0x80       /* include "normal" files in search */
#define S_ALL (S_HIDDEN+S_SYSTEM+S_DIR+S_REG)     /* get everything */

	   /**** used by doscreate() only *******/

#define M_HIDDEN 0x02<<24        /* Hidden file */
#define M_SYSTEM 0x04<<24        /* System file */

typedef long  DOSMODE ;
typedef short DOSFILE ;

typedef struct  {                       /* COPIED FROM pcdos.h              */
	long            seek;           /* real disk addr for "data area"   */
	int             count;          /* number of dir entries in cluster */
	long            *disk;          /* DCB pointer for disk             */
	int             mode;           /*                                  */
	int             tnxtcl;         /* the next cluster number          */
} dossrch;

typedef struct {
	       long     mode;
	       char     path[128];
	       char     *base;
	       char     *extn;
	       char     is_dos;
	       DOSFILE  handle;
	       short    index;
	       dossrch  dos_srch;
} DOSFIND ;

typedef struct {
	       char     st_drive_id;      /* 'A' 'B' ...     */
	       char     st_filetype;      /* 'u' 'd' 't' 'o' */
	       long     st_dev;
	       long     st_ino;
	       long     st_mode;
	       long     st_rdev;
	       long     st_nlink;
	       uid_t     st_uid;
	       gid_t     st_gid;
	       long     st_size;
	       time_t   st_atime;
	       time_t   st_mtime;
	       time_t   st_ctime;
} DOSSTAT;

typedef struct {
	char  upath[128];   /* device or file specified in configuration */
	char  umount[128];  /* unix directory mounted on it, or "" */
	char  volume[32];   /* volume name */
	int   freespace;    /* number of free bytes */
	char  fstype;       /* unix filesystem='u' dos filesystem='d' */
} DOSUSTAT;

extern int doserrno;
extern int dostrace;

char *dospwd();

#endif /* _H_DOS */
