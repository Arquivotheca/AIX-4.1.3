/* @(#)14	1.10  src/bos/usr/include/IN/FSdefs.h, libIN, bos411, 9428A410j 6/16/90 00:17:03 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *  File System Utility Definitions
 *
 *      This file contains definitions and declarations used by various
 *      file system utilities.
 *
 *
 *  Definitions used for processing /etc/filesystems
 */

#ifndef _H_FSDEFS
#define _H_FSDEFS

#define FSYSname     "/etc/filesystems" /* name of filsys descr file    */
#define LFDIRnam     "lost+found"       /* name of lost & found dir.    */
#define MAXREC 512                      /* Maximum record size          */
#define MAXATR 32                       /* Max attributes per stanza    */

/*
 *  Miscellaneous definitions
 */

#define MAX_CYL     750         /* Maximum number of blocks per cyl.    */
#define MAX_INO     65000       /* Maximum number of I-nodes            */
#define DEF_CYL     1           /* Default blocks/cylinder value        */
#define DEF_SKP     1           /* Default skip factor for interleave   */
#define DEF_FSID    "NONAME"    /* Default file system name             */
#define DEF_VOLID   "VOLXXX"    /* Default volume name                  */

/*
 *  Free list interleave structure:
 *      Data in the following structure (supplied by FSintlv()) can be
 *      used to optimize the organization of a file system free list.
 */

struct fl_intlv         /* Free list interleave info ...                */
{   short  fl_cyl;      /* ... Number of (512-byte) blocks/cyl.         */
    short  fl_skip;     /* ... Skip factor                              */
};

/*
 *  File system info structure:
 *      Data in the following structure is provided by FSinfo().   The
 *      "fi_attrp" field points to an attribute record (extracted from
 *      /etc/filesystems) that describes the file system in question.
 *      It should be cast to an "ATTR_t" before it is used.
 */

struct fs_info                  /* File system info structure ...       */
{   char   *fi_bname;           /* ... Ptr to block special file name   */
    char   *fi_cname;           /* ... Ptr to char special file name    */
    char   *fi_iname;           /* ... Ptr to image file                */
    char   *fi_attrp;           /* ... Ptr to attribute record          */
};

extern struct fl_intlv *FSintlv();
extern int   FSinfo(), FSicalc();
extern char *FSattr(), *FSdskname(), *FSlabel(), *FSpack();

#endif /* _H_FSDEFS */
