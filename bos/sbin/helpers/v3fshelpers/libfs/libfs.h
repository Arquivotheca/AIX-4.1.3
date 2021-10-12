/* @(#)78	1.9  src/bos/sbin/helpers/v3fshelpers/libfs/libfs.h, cmdfs, bos411, 9428A410j 4/25/94 14:57:44 */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS:
 *	prototypes: rwfrag, rwinode, get_superblk, put_superblk,
 *	      	    ltop, fsopen, fsclose, inrange, get_pow2mult,
 *		    validate_super, fsmax
 *
 *	macros for: bread, bwrite, get_inode, put_inode,
 *		    get_super, get_super2, put_super, put_super2
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_LIBFS
#define _H_LIBFS

#include <sys/types.h>
#include <sys/vmdisk.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <libfs/fsmacros.h>

#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef GET
#undef GET
#endif
#define GET	0

#ifdef PUT
#undef PUT
#endif
#define PUT	1

/*
 *  defines for compression algorithms
 */
#ifdef NO_COMPRESS
#undef NO_COMPRESS
#endif
#define NO_COMPRESS             0       /* no compression */

#ifdef LZ_COMPRESS
#undef LZ_COMPRESS
#endif
#define LZ_COMPRESS            1	/* specify LZ compression algorithm */


#define LIBFS_SUCCESS		 0
#define LIBFS_SEEKFAIL		-2	
#define LIBFS_READFAIL		-3
#define LIBFS_WRITEFAIL		-4	/* write failed			    */
#define LIBFS_BADMAGIC		-5	/* magic number not recognized	    */
#define LIBFS_BADVERSION	-6	/* magic num ok, incompatible vers  */
#define LIBFS_BADFRAG		-7	/* invalid frag_t		    */
#define LIBFS_INTERNAL		-8
#define LIBFS_NODEV		-9	/* device doesn't exist (fsopen)    */
#define LIBFS_CORRUPTSUPER	-10  	/* fragsize, agsize, or iagsize bad */

#define COMP_KMOD_PATH		"/sbin/comp.kext"
#define COMP_USERLEVEL_PATH	"/sbin/comp.uext"

union frag_daddr
{
	frag_t	f;	
	ulong	d;
};
typedef union frag_daddr fdaddr_t;	/* trendy underscore t name	*/


/*
 *  function prototypes
 */

/*
 *  get_pow2mult returns the first power-of-2 multiple of <mult> that is just
 *  greater than or equal to <num> 
 *	a power-of-2 multiple of i is:  i * [1, 2, 4, 8, ...]
 */
int
get_pow2mult(int mult,
	     int num);

int
valid_algorithm(int algorithm);	/* NO_COMPRESS, LZ_COMPRESS */


int
decompress(int	algorithm,	/* NO_COMPRESS, LZ_COMPRESS	*/
           char *input,		/* input buffer			*/
           char *output,	/* output buffer		*/
           int	nbytes);	/* bytes in input buffer	*/

int
inrange(int num,	/* number to check	*/
	int low,	/* least possible value	*/
	int high);	/* max value		*/

int
validate_super(struct superblock *sb);  /* ptr to superblock 		*/
	       
int
get_superblk(int 		fd, 	/* filesys file descriptor	*/
	     struct superblock 	*sb,	/* filesys superblock		*/
	     int		bno);	/* block where we get superblk	*/

int
put_superblk(int 		fd,	/* filesys file descriptor	*/
	    struct superblock 	*sb,	/* filesys superblock		*/
	    int			bno);	/* block where we put superblk	*/

int
rwfrag(int	fd,		/* file descriptor for filesystem	*/
       char	*buf,		/* buffer to read or write		*/
       frag_t	frag,		/* disk address where we write buf	*/
       int	mode);		/* GET or PUT  (read or write)		*/

int
rwinode(int 		fd,	/* filesys file descriptor	*/
	struct dinode 	*di,	/* inode to read/write		*/
	ino_t 		inum,	/* number of inode to read/write*/
	int		mode);	/* GET or PUT  (read or write)	*/

int
rwdaddr(int 		fd,	/* file descriptor for filesystem	*/
        frag_t 		*frag,	/* disk address we want			*/
        struct dinode	*di,	/* ptr to disk inode			*/
        int 		lbno,	/* logical blk num we want from dp	*/
        int		mode);	/* GET/PUT frag from/in inode		*/

int
fsopen(char *fsname,		/* fs pathname (device or mountpt) */
       int  oflag);		/* mode for opening filsys	   */

int
fsclose(int fd);		/* filsys device file descriptor   */

int
fsmax(ino_t  	*imax,		/* first illegal inode (numinodes) */
      frag_t	*fmax);		/* first illegal frag  (numfrags)  */

/*
 *  macros for calling libfs functions
 */
#define bread(fd, buf, frag)		\
	rwfrag(fd, buf, frag, GET)
#define bwrite(fd, buf, frag)		\
	rwfrag(fd, buf, frag, PUT)

#define get_inode(fd, inode, inum)	\
	rwinode(fd, inode, inum, GET)	
#define put_inode(fd, inode, inum)	\
	rwinode(fd, inode, inum, PUT)     	

#define get_daddr(fd, frag, inode, lbno)	\
	rwdaddr(fd, frag, inode, lbno, GET)
#define put_daddr(fd, frag, inode, lbno)	\
	rwdaddr(fd, frag, inode, lbno, PUT)
#define ltop(fd, frag, inode, lbno)		\
	get_daddr(fd, frag, inode, lbno)

#define get_super(fd, sb)		\
	get_superblk(fd, sb, 1)
#define get_super2(fd, sb)		\
	get_superblk(fd, sb, 0)

#define put_super(fd, sb)		\
	put_superblk(fd, sb, 1)
#define put_super2(fd, sb)		\
	put_superblk(fd, sb, 0)
#endif /* _H_LIBFS */
