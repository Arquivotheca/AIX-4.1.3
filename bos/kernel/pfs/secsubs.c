static char sccsid[] = "@(#)85	1.15.1.9  src/bos/kernel/pfs/secsubs.c, syspfs, bos411, 9439C411e 9/30/94 12:59:59";
/*
 * COMPONENT_NAME:  (SYSPFS) Physical File System
 *
 * FUNCTIONS:  smap_alloc(), smap_free(), sec_delete(), 
 *	       sec_free(), acl_mapin(), acl_mapout()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"jfs/jfslock.h"
#include	"jfs/inode.h"
#include	"sys/errno.h"
#include	"sys/syspest.h"
#include	"sys/malloc.h"

#define	DPRINTF(args)

/*
 * .inodex layout to store security extension of inode:
 *
 *	+---------+ 			A
 *	| bit map | <- "anchor page"	|
 *	|   ...   |			|
 *	+---------+			|
 *	|         |		     "region"
 *	|  lines  |			|
 *	|   ...   |			|
 *	+---------+			V
 *
 *	+---------+
 *	| bit map |
 *	|   ...   |
 *	+---------+
 *	|         |
 *	|  lines  |
 *	|   ...   |
 *
 * .inodex is partitioned into regions of 1 MByte each.
 * each region consists of 
 *  an anchor page containg allocation bit map, and
 *  lines of 128 bytes each to store acl/pcl.
 */

/* line size: 128 bytes - unit of allocation */
#define	LINESIZE	(1 << 7)
/* region size: 1 Mbyte */
#define	REGIONSIZE	((1 << 8) * PAGESIZE)
/* number of lines per region */
#define	NLINES		(REGIONSIZE / LINESIZE)

/* assumes <m> is a power-of-two */
#define	round(n,m)	(((n) + ((m)-1)) & ~((m)-1))
/* bytes to lines */
#define	btol(n)		(((n) + (LINESIZE-1)) / LINESIZE)

/* region structure of .inodex
 */
struct	sregion
{
	union
	{
		char	s_pad[REGIONSIZE];
		struct
		{
			unsigned long	_s_map[NLINES/32];
		} s_anchor;
	} s_un;
#define	s_map		s_un.s_anchor._s_map
};

/* bit mask for consecutive n bits */
static	unsigned  long	bits[] =
{
	0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
	0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
	0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
	0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
	0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};


/*
 *	nfree()
 *
 * return bit position of <n> 0 bits in the bitmap word <w>
 * returns -1 if <n> 0 bits were not found
 */
static
nfree(w, n)
unsigned long	w;
{
	int	nzero;
	int	bitpos;

	/* quick check for full entry */
	if (w == -1)
		return(-1);
	/* look for <n> consecutive 0 bits */
	nzero = 0;
	for (bitpos = 0; bitpos < 32; bitpos++, w >>= 1)
	{
		if (w & 0x01)
		{
			nzero = 0;
			continue;
		}
		nzero++;
		if (nzero >= n)
		{
			bitpos -= (n - 1);
			return(bitpos);
		}
	}
	return(-1);
}


/*
 *	smap_alloc()
 *
 * allocate <n> bytes of space in ".inodex".
 *
 * it is the responsibility of the caller to have mapped
 * ".inodex" into memory at <saddr>.
 * <ptr> is the returned pointer; this is relative to <saddr>.
 *
 * returns ENOSPC on failure
 *
 * SERIALIZATION: .inodex inode is locked on entry/exit. 
 */

smap_alloc(ixip, saddr, n, ptr)
struct	inode	*ixip;		/* the inode for the ".inodex" segment */
struct	sregion	*saddr;		/* the (mapped-in) ".inodex" segment */
int	n;			/* number of bytes required */
char	**ptr;			/* return value pointer */
{
	struct sregion	*s;
	int	lines;
	unsigned long	*mp;
	int	i;
	char	*p;
	unsigned long	size;

	lines = btol(n);

	/* can't handle over 32 lines (a page) right now.
         * If this algorithm changes, change smap_alloc_check() too.
	 */
	if (lines > 32)
	{
		DPRINTF(("smap_alloc(): too big\n"));
		return(ENOSPC);
	}

	/* scan each region of .inodex.
	 * don't worry about space exhaustion for now (!)
	 */
	for (s = saddr; 1; s++)
	{
		/* scan each word of bit map (32 bits corresponding to 
		 * 32 lines of each page) for a page with enough free lines.  
		 * skip word 0 of bit map representing lines in anchor page 
		 * of region.
		 */
		mp = &(s->s_map[1]);
		for (i = 1; i < NLINES / 32; i++, mp++)
		{
			int	bitpos;

			bitpos = nfree(*mp, lines);
			if (bitpos < 0)
				continue;

			/* found a page with enough free lines.
			 */
			DPRINTF(("smap_alloc(): region %d map[%d] 0x%x->0x%x\n", 
				s - saddr, mp - s->s_map, *mp, 
				*mp | (bits[lines] << bitpos)));
			/* get transaction lock on bit map word */
			vm_gettlock(mp, sizeof(unsigned long));

			/* set bit map for allocation. */
			*mp |= (bits[lines] << bitpos);

			/* compute the start address of lines allocated.
			 */
			p = ((char *)s) + (((i*32)+bitpos) * LINESIZE);
			size = (ulong)(((char *)p) - ((char *)saddr));
			*ptr = (char *)size;

			/* assure the ".inodex" segment includes this page
			 */
			size = round(size, PAGESIZE);
			if (ixip->i_size < size)
			{
				DPRINTF(("smap_alloc(): extending \".inodex\"\n"));
				ixip->i_size = size;
				imark(ixip, ICHG);
			}

			imark(ixip, IFSYNC);

			return(0);
		}
	}

	/* return (ENOSPC); */
}


/*
 *	smap_free()
 *
 * free space in ".inodex", starting from offset off.
 *
 * it is the responsibility of the caller to have
 * mapped ".inodex" into memory at <saddr>
 * <ptr> is a pointer relative to <saddr>: this will always 
 * point to the start of a line; the first word of this line 
 * is the size of the space to be freed (length field of
 * structure stored at the line).
 *
 * SERIALIZATION: .inodex inode is locked on entry/exit. 
 */

smap_free(ixip, saddr, off)
struct inode	*ixip;		/* the ".inodex" inode */
struct sregion	*saddr;		/* mapped address */
int	off;			/* the offset of the area to be freed */
{
	char	*ptr;
	struct sregion	*s;
	int	line;
	int	len;	/* must get this from data in segment */
	unsigned long	b;

	DPRINTF(("smap_free(): starting\n"));
	ptr = off + ((char *)saddr);
	len = *(int *)ptr;	/* the first word of an ACL/PCL is length */

	if (off & (LINESIZE-1))
	{
		DPRINTF(("smap_free(): not at line boundary!\n"));
		return(EINVAL);
	}

	/* point to base of region in .inodex in which space resides */
	s = &(saddr[off / REGIONSIZE]);

	/* compute the start line number and bit mask to free */ 
	line = off % REGIONSIZE;
	line /= LINESIZE;
	b = bits[btol(len)] << (line % 32);

	DPRINTF(("smap_free(): region %d map %d 0x%x->0x%x\n", s-saddr, line / 32,
		s->s_map[line / 32], s->s_map[line / 32] & ~b));
	if ((s->s_map[line / 32] & b) != b)
		DPRINTF(("smap_free(): not allocated!\n"));

	/* get transaction lock on bit map word */
	vm_gettlock(&(s->s_map[line/32]), sizeof(unsigned long));

	/* mark the specified line bits of bit map of the region free */
	s->s_map[line / 32] &= ~b;

	imark(ixip, IFSYNC);
	DPRINTF(("smap_free(): ending\n"));

	return(0);
}


/*
 *	sec_delete()
 *
 * free ondisk security resources in ".inodex" assocaited 
 * with the specified inode;
 *
 * this is called when the inode is unlinked.
 * if another process has already accessed this inode
 * (ip->i_count > 1 - i don't think the inode need
 * necessarily be open), the security information is copied
 * into malloc()'d space.  this will be freed when the inode
 * is finally iput().
 *
 * RETURN:	locked .inodex node
 *
 * SERIALIZATION: called when the inode is unlinked (jfs_remove()
 *		and jfs_rename()) with inode locked on entry/exit.
 *
 * 		Users of this routine must have a transaction block upon 
 *		entry.  When transactions locks are needed, the transaction 
 *		block must be garnered before the .inodex lock.
 */

struct inode *
sec_delete(ip)
struct inode	*ip;
{
	struct	inode	*ixip;		/* the ".inodex" segment */
	struct	acl	*acl;		/* the acl in the ".inodex" segment */
	struct	acl	*newacl;	/* the malloc()'d acl */
	caddr_t	saddr;
	label_t	jb;	/* jump buf pointer */
	int	rc;

	/* do nothing if this wasn't the last link! */
	if (ip->i_nlink)
	{
		DPRINTF(("smap_delete(): not last link\n"));
		return(NULL);
	}

	/* do nothing if this is a symbolic link! */
	if (S_ISLNK(ip->i_mode))
	{
		DPRINTF(("smap_delete(): symbolic link\n"));
		return(NULL);
	}

	/* if there is an incore ACL, free it... */
	if (ip->i_acl & ACL_INCORE)
	{
		ip->i_acl &= ~ACL_INCORE;
		if (ip->i_acl)
		{
			free((void *)(ip->i_acl));
			ip->i_acl = 0;
		}
	}

	/* nothing to do if there is no extended ACL or PCL */
	if ((ip->i_acl == 0) &&
	    (((ip->i_privflags & PCL_EXTENDED) == 0)  ||
	     (ip->i_privoffset == 0)))
	{
		return(NULL);
	}

	/*
	 * free security extension of the inode in .inodex
	 */

	/* get addressibility to ".inodex" */
	ixip = ip->i_ipmnt->i_ipinodex;
	if (iptovaddr(ixip, 1, &saddr))
	{
		DPRINTF(("smap_delete(): iptovaddr() failed!\n"));
		ip->i_acl = 0;
		return(NULL);
	}

	IWRITE_LOCK(ixip);

	/* set up exception handling */
	if (rc = setjmpx(&jb))
		switch (rc)
		{
		    case ENOSPC:
		    case ENOMEM:
		    case EIO:
			goto out;
		    default:
			assert(0);
		}

	if (ip->i_acl)
	{
		int	off;

		off = ip->i_acl;
		acl = (struct acl *)(((char *)saddr) + off);

		/* update i_acl: replace the ondisk acl with incore acl 
		 * if reference is held on the inode, or clear it.
		 *
		 * i_acl should be cleared before dinode is written back
		 * to disk (both home and log) or iread() should clear it (!)
	 	 */
		if (ip->i_count > 1)
		{
			newacl = (struct acl *)malloc((uint)acl->acl_len);
			if (newacl)
				bcopy(acl, newacl, acl->acl_len);
			ip->i_acl = ACL_INCORE | ((int)newacl);
		}
		else
			ip->i_acl = 0;

		/* free security extension of the inode in .inodex
	 	 */
		DPRINTF(("sec_delete(): free ACL in \".inodex\"\n"));
		smap_free(ixip, saddr, off);
	}

	if ((ip->i_privflags & PCL_EXTENDED) && ip->i_privoffset)
	{
		/* free security extension of the inode in .inodex
	 	 */
		DPRINTF(("sec_delete(): free PCL in \".inodex\"\n"));
		smap_free(ixip, saddr, ip->i_privoffset);
	}

	clrjmpx(&jb);

out:
	ipundo(saddr);	/* release virtual address space */
	imark(ixip, IFSYNC);
	return(ixip);
}


/* 
 *	sec_free()
 *
 * free incore security resources assocaited 
 * with the specified inode;
 *
 * SERIALIZATION: called only by iclose() on i_count = 0 for i_nlink = 0
 *		  with IWRITE_LOCK on entry/exit.
 */
sec_free(ip)
struct	inode	*ip;
{
	/* if there is an incore ACL, free it... */
	if (ip->i_acl & ACL_INCORE)
	{
		ip->i_acl &= ~ACL_INCORE;
		if (ip->i_acl)
		{
			free((void *)(ip->i_acl));
			ip->i_acl = 0;
		}
	}
}


/* 
 *	acl_mapin()
 *
 * get addressibility to and check validity of the acl
 *
 * SERIALIZATION: called via USE_MODE() macro 
 * 		  from iaccess(), check_all(), check_any()
 *		  with inode locked
 */
acl_mapin(ip, acl, saddr)
struct inode	*ip;
struct acl	**acl;
char		**saddr;
{
	int	rc;
	struct	inode	*ixip;
	label_t	jb;	/* jump buf pointer */

	if (ip->i_acl & ACL_INCORE)
	{
		*acl = (struct acl *)(ip->i_acl & ~ACL_INCORE);
		if (!valid_acl(*acl))
			return(-1);
		else
			return(0);
	}

	ixip = ip->i_ipmnt->i_ipinodex;
	if (rc = iptovaddr(ixip, 1, saddr))
		return(rc);

	IWRITE_LOCK(ixip);

	if ((rc = setjmpx(&jb)) == 0)
	{
		*acl = (struct acl *)(*saddr + (int)(ip->i_acl));
		if (!valid_acl(*acl))
		{
			rc = -1;
			ipundo(*saddr);
		}
		clrjmpx(&jb);
		IWRITE_UNLOCK(ixip);
		return(rc);
	}

	IWRITE_UNLOCK(ixip);

	/* falling through here means we got a vmm error */
	switch (rc)
	{
	    case ENOSPC:
	    case ENOMEM:
	    case EIO:
		ipundo(*saddr);
		return(rc);
	    default:
		assert(0);
	}
}


/*
 *	acl_mapout()
 */
acl_mapout(ip, saddr)
struct inode	*ip;
char		*saddr;
{
	/* no longer need addressibility to the acl */
	if ((ip->i_acl & ACL_INCORE) == 0)
		ipundo(saddr);

	return(0);
}


/*
 * NAME:	valid_acl(acl)
 *
 * FUNCTION:	perform minimal correctness checking on an ACL:
 *			1) acl->acl_len >= ACL_SIZ
 *			2) acl->acl_len == sum of ace lengths
 *			3) each ace->ace_len == sum of ace's id lengths
 *
 * PARAMETERS:	acl	- a pointer to the access control list to
 *			  be checked
 *
 * RETURN :	1 if successful
 *		0 if ACL was found to be invalid
 *
 * SERIALIZATION: inode locked on entry/exit.
 */

valid_acl(acl)
struct acl	*acl;
{
	struct acl_entry   *ace;
	struct acl_entry   *acl_end;
	int	acl_len;	/* given ACL length */
	int	tot_acl_len;	/* calculated ACL length */

	acl_len = acl->acl_len;
	if (acl_len < ACL_SIZ)
	{
		DPRINTF(("valid_acl(): acl_len < ACL_SIZ\n"));
		return(0);
	}

	tot_acl_len = ACL_SIZ;

	/* for each acl entry ... */
	acl_end = acl_last(acl);
	for (ace = acl->acl_ext; ace < acl_end; ace = acl_nxt(ace))
	{
		struct ace_id	*id;
		struct ace_id	*id_end;
		int	ace_len;	/* given ACE length */
		int	tot_ace_len;	/* calculated ACE length */

		ace_len = ace->ace_len;
		if (ace_len < ACE_SIZ)
		{
			DPRINTF(("valid_acl(): ace_len < ACE_SIZ\n"));
			return(0);
		}
		tot_ace_len = ACE_SIZ;

		/* foreach id in the acl entry ... */
		id_end = id_last(ace);
		for (id = ace->ace_id; id < id_end; id = id_nxt(id))
		{
			if (id->id_len < ID_SIZ)
			{
				DPRINTF(("valid_acl(): id_len < ID_SIZ\n"));
				return(0);
			}
			tot_ace_len += id->id_len;
			if (tot_ace_len > ace_len)
			{
				DPRINTF(("valid_acl(): ace_len < sum(ids)\n"));
				return(0);
			}
		}
		if (tot_ace_len != ace_len)
		{
			DPRINTF(("valid_acl(): ace_len != sum(ids)\n"));
			return(0);
		}
		tot_acl_len += tot_ace_len;
		if (tot_acl_len > acl_len)
		{
			DPRINTF(("valid_acl(): acl_len < sum(aces)\n"));
			return(0);
		}
	}
	
	if (tot_acl_len != acl_len)
	{
		DPRINTF(("valid_acl(): acl_len != sum(aces)\n"));
		return(0);
	}

	return(1);		
}


/*
 *	smap_alloc_check()
 *
 * Check if we can allocate <n> bytes of space in ".inodex".
 * called only by i_setacl().acl_check().
 *
 * returns ENOSPC on failure 0 on success.
 */

smap_alloc_check(n)
int	n;			/* number of bytes required */
{
	int	lines;

	lines = btol(n);

	/* can't handle over 32 lines (a page) right now */
	if (lines > 32)
	{
		DPRINTF(("smap_alloc_check(): too big\n"));
		return(ENOSPC);
	}

	return(0);
}
