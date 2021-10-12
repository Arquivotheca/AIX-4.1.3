static char sccsid[] = "@(#)56	1.23  src/bos/kernel/lfs/pathname.c, syslfs, bos411, 9431A411a 8/3/94 16:27:32";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: pn_alloc, pn_get, pn_getchar, pn_set
 *            pn_combine, pn_getcomponent, pn_skipslash
 *            pn_free, copyname
 *
 * ORIGINS: 24, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* 87/01/18 (#)vfs_pathname.c	1.7 NFSSRC */
/* (#)vfs_pathname.c 1.1 86/09/25 SMI	*/


#include <sys/limits.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/pathname.h>
#include <sys/shm.h>
#include <sys/malloc.h>
#include <sys/fs_locks.h>

/*
 * Pathname utilities.
 *
 * In translating file names we copy each argument file
 * name into a pathname structure where we operate on it.
 * Each pathname structure can hold PATH_MAX+1 characters
 * including a terminating null, and operations here support
 * allocating and freeing pathname structures, fetching
 * strings from user space, getting the next character from
 * a pathname, combining two pathnames (used in symbolic
 * link processing), and peeling off the first component
 * of a pathname.
 */

#ifdef notneeded
static char *pn_freelist;
int pn_bufcount;
Simple_lock	pn_lock;	/* Pathname free list lock */

/*
 * Allocate contents of pathname structure.
 * Structure itself is typically automatic
 * variable in calling routine for convenience.
 */
int
pn_alloc(pnp)
	register struct pathname *pnp;
{
	/*
	 * The pathname lock is held across xmalloc, which may block.
	 * There may be some advantage to releasing the lock before
	 * calling malloc, but until we encounter evidence that it
	 * is causing problems, we won't unlock.  The effects of 
	 * unlocking are difficult to predict, other than it will
	 * probably cause more buffers to be allocated.
	 */
	PN_LOCK();
	if (pn_freelist) {
		pnp->pn_buf = pn_freelist;
		pn_freelist = *(char **) pnp->pn_buf;
	} else {
		pnp->pn_buf = xmalloc(PATH_MAX+1, 2, kernel_heap);
		pn_bufcount++;
	}

	/*
	 * We have to make sure the memory was actually allocated.
	 * The xmalloc may have failed.
	 */
	if (pnp->pn_buf == NULL)
	{
		PN_UNLOCK();
		return ENOMEM;
	}

	PN_UNLOCK();
	pnp->pn_path = (char *)pnp->pn_buf;
	pnp->pn_pathlen = 0;
	return 0;
}
#endif

/*
 * Pull a pathname from user or kernel space
 */
int
pn_get(str, seg, pnp)
	char *str;
	int seg;
	register struct pathname *pnp;
{
	uint len;
	extern int copyinstr(), copystr();
	int rc;

	/* no fetch protect on low memory, so we do it here, partially */
	if (!str) return(EFAULT);

	/* set the path pointer */
	pnp->pn_path = pnp->pn_buf;

	/* run the appropriate copy function */
	if (seg == SYS)
		rc = copystr(str,pnp->pn_path,PATH_MAX+1,&len);
	else
		rc = copyinstr(str,pnp->pn_path,PATH_MAX+1,&len);

	if (rc == 0)
		pnp->pn_pathlen = len-1; /* pathlen excludes the NULL byte */
	else
		if (rc == E2BIG)
			rc = ENAMETOOLONG;
	return rc;
}

#ifdef notneeded
/*
 * Get next character from a path.
 * Return null at end forever.
 */
pn_getchar(pnp)
	register struct pathname *pnp;
{
	if (pnp->pn_pathlen == 0)
		return (0);
	pnp->pn_pathlen--;
	return (*pnp->pn_path++);
}

#endif /* notneeded */

/*
 * Set pathname to argument string.
 */
pn_set(pnp, path)
	register struct pathname *pnp;
	register char *path;
{
	pnp->pn_path = pnp->pn_buf;
	pnp->pn_pathlen = strlen(path);		/* don't count null byte */
	bcopy(path, pnp->pn_path, pnp->pn_pathlen + 1);
	return 0;
}

/*
 * Combine two argument pathnames by putting
 * second argument before first in first's buffer,
 * and freeing second argument.
 * This isn't very general: it is designed specifically
 * for symbolic link processing.
 */
pn_combine(pnp, sympnp)
	register struct pathname *pnp;
	register struct pathname *sympnp;
{

	if (pnp->pn_pathlen + sympnp->pn_pathlen >= PATH_MAX+1)
		return (ENAMETOOLONG);
	ovbcopy(pnp->pn_path, pnp->pn_buf + sympnp->pn_pathlen,
	    (uint)pnp->pn_pathlen);
	bcopy(sympnp->pn_path, pnp->pn_buf, (uint)sympnp->pn_pathlen);
	pnp->pn_pathlen += sympnp->pn_pathlen;
	pnp->pn_path = pnp->pn_buf;
	return (0);
}

/*
 * Strip next component off a pathname and leave in
 * component buffer which will have room for
 * comp_len bytes and a null terminator character.
 */
pn_getcomponent(pnp, component, comp_len, act_lenp)
	register struct pathname *pnp;
	register char *component;
	int comp_len;	/* length of *component */
	int *act_lenp;	/* actual length copied into *component 
				(excludes the NULL char) */
{
	register char *cp;
	register int l;
	register int n;
	register char c;

	cp = pnp->pn_path;
	l = pnp->pn_pathlen;
	n = comp_len;
	while ((l > 0) && (*cp != '/')) {
		if (--n < 0)
			return(ENAMETOOLONG);
		c = *cp++;
		*component++ = c;
		--l;
	}
	*act_lenp = pnp->pn_pathlen - l;
	pnp->pn_path = cp;
	pnp->pn_pathlen = l;
	*component = 0;
	return (0);
}

/*
 * skip over consecutive slashes in the pathname
 */
void
pn_skipslash(pnp)
	register struct pathname *pnp;
{
	while ((pnp->pn_pathlen != 0) && (*pnp->pn_path == '/')) {
		pnp->pn_path++;
		pnp->pn_pathlen--;
	}
}

#ifdef notneeded
/*
 * Free pathname resources.
 */
void
pn_free(pnp)
	register struct pathname *pnp;
{
	if (pnp->pn_buf) {
		/* kmem_free((caddr_t)pnp->pn_buf, (uint)PATH_MAX+1); */
		PN_LOCK();
		*(char **) pnp->pn_buf = pn_freelist;
		pn_freelist = pnp->pn_buf;
		PN_UNLOCK();
		pnp->pn_buf = 0;
	}
}
#endif

/* this routine was retrieved from the release 5 direnter.c and is left
   here because it is called several places in the kernel */

copyname(s1, s2, n)
register char *s1, *s2;
register int n;
{
	register char *end = s1+n;

	while (s1 < end) {
		if (((*s1 = *s2++) == '\0') || (*s1 == '/'))
		    while (s1 < end)
			*s1++ = '\0';
		s1++;
	}

	return((int)--s2);
}

/*
 * NAME:        strip_slash
 *
 * FUNCTION:    Remove trailing slashes from pathname.
 *              Used to remove trailing slashes from the initial pathname
 *              provided by system call and to remove trailing slashes after
 *              pn_combine()'ing a symlink to the working pathname.
 *              The latter results when a symlink is made as follows:
 *                      ln -s some_dirname/ some_slnk_name
 *
 * PARAMETERS:
 *              struct pathname *pnp    - pathname to strip of trailing /'s
 *
 * RETURN:
 *              return the number of slashes removed
 */
int
strip_slash (struct pathname    *pnp)
{
        int     slash_cnt = 0;

        while (pnp->pn_pathlen > 1)
        {
                if (pnp->pn_path[pnp->pn_pathlen-1] == '/')
                {
                        pnp->pn_path[--pnp->pn_pathlen] = '\0';
                        slash_cnt++;
                }
                else
                        break;
        }
        return slash_cnt;
}
