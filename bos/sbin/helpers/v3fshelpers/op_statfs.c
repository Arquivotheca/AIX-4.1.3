static char sccsid[] = "@(#)12	1.15.2.4  src/bos/sbin/helpers/v3fshelpers/op_statfs.c, cmdfs, bos411, 9428A410j 12/13/93 08:13:33";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: op_statfs, read_diskmap, read_inodemap
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <varargs.h>    
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <jfs/filsys.h>
#include <sys/statfs.h>
#include <fshelp.h>
#include <jfs/ino.h>
#include <sys/vmdisk.h>
#include <sys/param.h>

#include <libfs/libfs.h>
#include "fsop.h"

#ifndef PATH_MAX
#define PATH_MAX	256
#endif

/* The following macro is to print out debugging information */
/* The information will be sent to the file specified in     */
/* using the -d option. */

#define OUTPUT(a,b)							\
			if (outputon)					\
			{						\
				sprintf (buf,a,b);			\
				write (fd,buf,(unsigned)strlen(buf));	\
			}

extern int DebugLevel;
extern int Mode;

static int fd;		/* Debugging information written to this file.  */
static int outputon=0; /* Debugging turned off.                        */
static char buf[500];	/* Debugging output buffer			*/

void dump_super(struct superblock sb);
	
/*
 * NAME: op_statfs
 *                                                                    
 * FUNCTION: 
 * 	fills in a statfs structure for an unmounted filesystem.
 *
 * RETURN VALUE: FSHERR
 *		
 */  

int
op_statfs(int 	devfd,
	  char 	*opflags)
{
	struct statfs     *statfsp;
	struct superblock  sb;
	int count,where;
	int total_allocated;
	int total_inodes;
	char *file="",*c;
	extern void        *calloc();
	int rc;
	int	sflag = 0;

	/*
	 *	If an opflag is past in, take it as a file to open and
	 *	turn debugging on.
	 */
	if (*opflags)
	{
		c = opflags;
		while (*c != '\0')
		{
			switch (*c++)
			{
			case '-':
				switch (*c++)
				{
				case 'd':
					file = ++c;
					c = strchr (c,' ');
					if (c != NULL)
						*c++ = '\0';
					fd = open (file,O_RDWR|O_CREAT);
					if (fd < 0)
						return FSHERR_INVAL;
					outputon++;
					OUTPUT ("dbg file desc is:%d \n",fd);
					break;
				case 's':
					sflag++;
					break;
				default:
					return FSHERR_INVAL;
				}
				
			default:
				;
			}
		}
	}
	
	if (!(statfsp = (struct statfs *) calloc (1, sizeof (struct statfs))))
		return FSHERR_NOMEM;
	
	/* 
	 ** read in superblock
	 */ 
	if (rc = get_super(devfd, &sb))
		switch (rc)
		{
		case LIBFS_SEEKFAIL:
			OUTPUT("lseek() to %s failed.\n", "superblock");
			return FSHERR_DEVFAIL;
		case LIBFS_READFAIL:
			OUTPUT("read() of %s failed.\n", "superblock");
			return FSHERR_DEVFAILRD;
		case LIBFS_BADMAGIC:
			OUTPUT ("Unknown filesystem type, s_magic = %x\n",
				(int)sb.s_magic);
			return FSHERR_CORRUPT;
		case LIBFS_BADVERSION:
			rc = memcmp(sb.s_magic, fsv3magic, sizeof(sb.s_magic));
		OUTPUT("AIX filesystem, incompatible version number.%c", '\n');
			OUTPUT("s_version = %d, ", sb.s_version);
			return FSHERR_CORRUPT;
		case LIBFS_CORRUPTSUPER:
			OUTPUT("s_magic and s_version valid, but %s\n",
			       "superblock corrupt");
			return FSHERR_CORRUPT;
		}
	
	statfsp->f_bsize = sb.s_bsize;
	statfsp->f_fsize = sb.s_fragsize;
	/*
	 * Read in the inode cooresponding to the inode map blocks.
	 *   I think failing to read inode map should barf too
	 */
	if (read_map(devfd, INOMAP_I, &statfsp->f_files,
		     &statfsp->f_ffree) < 0)
	{
		OUTPUT ("Inodemap read failed:%d \n",devfd);
		return FSHERR_DEVFAILRD;
	}

	OUTPUT ("data read: s_ninode: %x\n",statfsp->f_files);
	OUTPUT ("data read: s_tinode: %x\n",statfsp->f_ffree);
	
	if (read_map(devfd, DISKMAP_I, &statfsp->f_blocks,
		     &statfsp->f_bavail) < 0)
	{
		OUTPUT ("Diskmap read failed:%d \n",devfd);
		return FSHERR_DEVFAILRD;
	}
	/*
	 *  convert f_blocks and f_bavail from frag units to 4k units
	 *  f_bavail might not be a multiple of FragPerBlk... then we 
	 *  	are forced to lie about free space (conservatively)
	 */
	statfsp->f_blocks = FRAG2BLK(statfsp->f_blocks);
	statfsp->f_bavail = FRAG2BLK(statfsp->f_bavail);
	OUTPUT("total full blocks       : %d\n",statfsp->f_blocks);
	OUTPUT("total full blocks free  : %d\n",statfsp->f_bavail);
	OUTPUT("Used blocks: %x\n ",statfsp->f_blocks - statfsp->f_bavail);
	
	/*
	 * Let's overdebug.
	 */
	OUTPUT("Writing the file: %d\n",fd);
	OUTPUT("data read statfsp: s_fsize: %x\n", statfsp->f_blocks);
	OUTPUT("data read statfsp: s_free: %x\n", statfsp->f_bavail);
	if (outputon)
		dump_super(sb);

	/*
	 * write statfs struct to the pipe
	 */
	if (write(PipeFd, (char *)statfsp, (unsigned)sizeof(struct statfs)) !=
	    sizeof(struct statfs))
		return FSHERR_INTERNAL;
	
	OUTPUT ("Returning Success: %d\n",FSHERR_GOOD);
	return FSHERR_GOOD;
}

void
dump_super(struct superblock sb)
{
	OUTPUT ("magic[0] is %x\n", sb.s_magic[0]);
	OUTPUT ("magic[1] is %x\n", sb.s_magic[1]);
	OUTPUT ("magic[2] is %x\n", sb.s_magic[2]);
	OUTPUT ("magic[3] is %x\n", sb.s_magic[3]);
	OUTPUT ("flag[0] is %x\n", sb.s_flag[0]);
	OUTPUT ("flag[1] is %x\n", sb.s_flag[1]);
	OUTPUT ("flag[2] is %x\n", sb.s_flag[2]);
	OUTPUT ("flag[3] is %x\n", sb.s_flag[3]);
	OUTPUT ("int     s_agsize   : %d\n", sb.s_agsize);        
	OUTPUT ("int     s_logserial: 0x%x\n", sb.s_logserial);	
	OUTPUT ("daddr_t s_fsize    : %d\n", sb.s_fsize);        
	OUTPUT ("short   s_bsize    : %d\n", sb.s_bsize);	
	OUTPUT ("short   s_spare    : %d\n", sb.s_spare);	
	OUTPUT ("char    s_fname[6] : %s\n", sb.s_fname);     
	OUTPUT ("char    s_fpack[6] : %s\n", sb.s_fpack);     
	OUTPUT ("dev_t   s_logdev   : %x\n", sb.s_logdev);	
	OUTPUT ("char    s_fmod     : 0x%x\n", (int)sb.s_fmod);
	OUTPUT ("char    s_ronly    : 0x%x\n", (int)sb.s_ronly);     
	OUTPUT ("time_t  s_time     : %d\n",  sb.s_time);         
	OUTPUT ("int     s_version  : 0x%x\n", sb.s_logserial);
	OUTPUT ("int     s_fragsize : %d\n", sb.s_fragsize);
	OUTPUT ("int     s_iagsize  : %d\n", sb.s_iagsize);
}


static int
read_map(int 	devfd,
	 ino_t 	ino,
	 int	*total,
	 int	*freecnt)
{
	struct dinode di;
	struct vmdmap map;
	frag_t 	      bno;
	int	      rc;
	
	*total = *freecnt = 0;
	OUTPUT ("getting map for inode %d\n",(int)ino);
	if (get_inode (devfd, &di, ino) < 0)
	{
		OUTPUT ("Couldn't find map inode: %d\n",ino);
		return FSHERR_INTERNAL;
	}
	OUTPUT ("map blks: %d\n",BYTE2BLK(di.di_size));
	OUTPUT ("map size: %d\n",di.di_size);
	if (rc = ltop(devfd, &bno, &di, 0))
	{
		OUTPUT("ltop failed: rc = %d\n", rc);
		return FSHERR_DEVFAILRD;
	}
	if ((rc = bread(devfd, &map, bno)) < 0)
	{
		OUTPUT("bread failed: rc = %d\n", rc);
		return FSHERR_DEVFAILRD;
	}
	*total = map.mapsize;
	*freecnt = map.freecnt;
	return 0;
}
