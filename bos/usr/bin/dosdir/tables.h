/* @(#)21	1.3  src/bos/usr/bin/dosdir/tables.h, cmdpcdos, bos411, 9428A410j 6/16/90 02:00:01 */
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "pcdos.h"              /* dos filesystem structures          */

#define bool           char
#define TRUE           1
#define FALSE          0

#define NUM_OPENFILES  20       /* number of open files max            */
#define NUM_PHYSICAL   36       /* number of physical devices          */
#define NUM_LOGICAL    36       /* number of logical  devices          */
#define STRINGSPACE    1000     /* size of string space                */

#define LD             short    /* index into logical_tbl              */
#define PD             short    /* index into physical_tbl             */
#define STRING         short    /* index into stringspace              */
/* DOSFILE short                   index into filetable from dos.h     */
#define DOS_MAGIC   0x312e3030  /* header for environment file         */

#define ld(i)          (logical_tbl[i])
#define pd(i)          (physical_tbl[i])
#define file(i)        (file_tbl[i])
#define str(i)         (stringspace+i)

#define err_return(i)       { doserrno = i; return(-1); }

     /* data structure created by _analyze() */

typedef  enum { k_absolute,k_relative,k_device } pathkind;

typedef  struct {
     LD       ldev;                    /* logical device table index  */
     PD       pdev;                    /* physical device table index */
     pathkind pkind;                   /* kind of pathname specified  */
     char     *basename;               /* base filename in fpath      */
     char     device[10];              /* DOS device name             */
     char     fpath[256];              /* DOS pathname after device   */
     char     upath[256];              /* full UNIX pathname          */
} APATH;

     /* One entry for each logical drive in the system.                 */

struct logical_device {      /* created by dosinit                      */
     PD     pdevice;         /* pointer to phys dev descriptor or       */
     LD     assigned_ld;     /* if nonzero, represents assigned LD      */
     short  open_count;      /* files open on this device               */
     char   device_id[5];    /* e.g. A:, COM1:                          */
     char   current_dir[128];/* DOS current directory                   */
     int    cd_off_t;        /* seek offset to currdir name on device   */

     /* x is multiplexed if x.pdevice.ldevice != x                      */
     /* in which case x.is_available = false                            */
} ;


typedef enum {is_dos,is_unix,is_io,is_unknown}  dstate;

struct physical_device {     /* created by dosmount                    */

     STRING attached_file;   /* device, path, or pipename              */
     LD     ldevice;         /* currently available logical dev        */
     dstate contents;        /* device, diskfile type, or unknown      */

			     /* unix filesystems only:                  */
     STRING mount_dir;       /* UNIX directory on which device mounted  */

			     /* DOS filesystems only:                   */
     DCB    *dcb;            /* DOS mount device control block pointer  */
     bool   removable;       /* if so close device when no files open  */
} ;

typedef enum   { unix_fmt, dos_fmt, tba_fmt } fmt;

struct open_file {           /* created by dosopen (first 5 by dosinit) */
     enum   { is_ready, is_open, is_closed , is_eof} status;
     char   *pathname;
     long   oflag;
     long   lseek;
     LD     ldevice;
     fmt    format;
     int    handle;          /* for unix files only                     */
     FCB    *fcb;            /* for dos files only                      */
} ;

struct environment {
     long                             dos_magic;
     enum { unix_paths, dos_paths }   pathtype;
     fmt                              format;
     LD                               current_disk;
     LD                               next_logical;
     PD                               next_physical;
     STRING                           next_string;
} _e;

struct open_file        file_tbl    [NUM_OPENFILES];
struct physical_device  physical_tbl[NUM_PHYSICAL];
struct logical_device   logical_tbl [NUM_LOGICAL];
char                    stringspace [STRINGSPACE];

extern int errno;
dstate _file_state();
char _depath[32];
