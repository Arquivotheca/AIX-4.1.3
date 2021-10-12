/* @(#)03	1.14  src/bos/usr/include/fshelp.h, cmdfs, bos411, 9428A410j 6/16/90 00:10:16 */
#ifndef __H_FSHELP
#define __H_FSHELP
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

/*
** fshelp.h
*/




#define NIL(type)      ((type) 0)
#define NILPTR(type)   ((type *) 0)

/*
** helper return codes
*/

#define FSHRC_GOOD 1
#define FSHRC_BAD  0
#define FSHRC_UGLY (-1)

/*
** debug levels
**
** individual helpers should define additional
** levels relative to FSHBUG_BLOOP
*/

#define FSHBUG_NONE       0      /* No debugging */
#define FSHBUG_INFO       1      /* Informative msgs */
#define FSHBUG_ACTION     2      /* Program action */
#define FSHBUG_LIBCALL    3      /* libcalls & return codes */
#define FSHBUG_LIBDATA    4      /* lib routine data */
#define FSHBUG_OPCALL     5      /* op calls & return codes */ 
#define FSHBUG_OPDATA     6      /* op data values */
#define FSHBUG_BLOOP      7      /* below the op level */
    
/*
** helper mode flags (op-independent)
*/
#define FSHMOD_INTERACT      0x00000001
#define FSHMOD_INTERACT_FLAG		'i'    
#define FSHMOD_FORCE         0x00000002
#define FSHMOD_FORCE_FLAG		'f'
#define FSHMOD_NONBLOCK      0x00000004
#define FSHMOD_NONBLOCK_FLAG		'n'
#define FSHMOD_PERROR        0x00000008
#define FSHMOD_PERROR_FLAG		'p'    
#define FSHMOD_ERRDUMP       0x00000010
#define FSHMOD_ERRDUMP_FLAG		'e'
#define FSHMOD_STANDALONE    0x00000020
#define FSHMOD_STANDALONE_FLAG		's'
#define FSHMOD_IGNDEVTYPE    0x00000040
#define FSHMOD_IGNDEVTYPE_FLAG		'I'
#define FSHMOD_ALL           0x0000007f

/*
** order must match above defines
*/
#ifndef _I_AM_FSHELP    
extern char   *fshmods[];
#endif /* _I_AM_FSHELP */

/*
** helper ops
*/

#define FSHOP_NULL       0
#define FSHOP_CHECK      1
#define FSHOP_CHGSIZ     2
#define FSHOP_FINDATA    3
#define FSHOP_FREE       4
#define FSHOP_MAKE       5
#define FSHOP_REBUILD    6
#define FSHOP_STATFS     7
#define FSHOP_STAT       8
#define FSHOP_USAGE      9
#define FSHOP_NAMEI	10
#define FSHOP_DEBUG	11

#define LAST_FSHOP	 FSHOP_DEBUG
#define FSHOP_NUM_OPS    (LAST_FSHOP + 1)

/*
** helper exec syntax:
**   OpName OpKey FilsysFileDescriptor Modeflags DebugLevel OpFlags
*/

#define A_NAME   0
#define A_OP     1
#define A_FSFD   2
#define A_COMFD  3    
#define A_MODE   4
#define A_DEBG   5
#define A_FLGS   6 
#define N_ARGS   7

/*
** error codes
** the helper library routines reserve FSHERR_GOOD to FSHERR_LAST
** helper implementations please start with FSHERR_1STIMP
*/
#define FSHERR_GOOD        0
#define FSHERR_INVAL       1
#define FSHERR_INVALFS     2
#define FSHERR_INVALFILNO  3
#define FSHERR_INVALDBLK   4
#define FSHERR_INVALMODE   5
#define FSHERR_INVALARG    6
#define FSHERR_AXSHELPER   7
#define FSHERR_ARGMISSING  8
#define FSHERR_NOP         9 
#define FSHERR_SYNTAX      10
#define FSHERR_UNKVFS      11
#define FSHERR_UNKOP       12
#define FSHERR_VFSMANGLED  13
#define FSHERR_VFSAXS      14
#define FSHERR_NOTSUP      15
#define FSHERR_WOULDBLOCK  16
#define FSHERR_ACTIVE      17
#define FSHERR_CANTLOCK    18
#define FSHERR_DEVOPEN     19
#define FSHERR_DEVFAIL     20
#define FSHERR_DEVFAILRD   21
#define FSHERR_DEVFAILWR   22
#define FSHERR_CORRUPT     23
#define FSHERR_NOMEM       24
#define FSHERR_INTERNAL    25

/*
** special
*/
#define FSHERR_LAST        FSHERR_INTERNAL
#define FSHERR_IMPERR      (FSHERR_LAST+1)
#define FSHERR_UNKERR      (FSHERR_LAST+2)

#define FSHERR_1STIMP      100

/*
** library routines which set fshlpr_errno
** clear it upon entry
*/

#define set_fsherr(err)    fshlpr_errno = err
#define clear_fsherr()     set_fsherr (FSHERR_GOOD)
#define fsherr()          (fshlpr_errno)    

#ifndef _I_AM_FSHELP
extern int              fshlpr_errno;
#endif
/*
** handy vfs_ent function declarations
*/

extern struct vfs_ent  *getvfsbyflag ();
extern struct vfs_ent  *getvfsbytype ();
extern struct vfs_ent  *getvfsbyname ();
extern struct vfs_ent  *getvfsent ();
extern void             setvfsent ();
extern void             endvfsent ();
extern int              fshelp ();

#endif /* __H_FSHELP */
