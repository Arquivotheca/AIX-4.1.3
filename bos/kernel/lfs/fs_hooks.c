static char sccsid[] = "@(#)91	1.3  src/bos/kernel/lfs/fs_hooks.c, syslfs, bos411, 9428A410j 8/27/93 16:20:36";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: fs_hook_add
 *
 * ORIGINS: 27
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

#include "sys/types.h"
#include "sys/fs_hooks.h"
#include "sys/fs_locks.h"

struct fs_hook *closeh_anchor = NULL;
struct fs_hook *fs_exech_anchor = NULL;

void
fs_hookadd(struct fs_hook *fsh, int cmd)
{
	struct fs_hook **fsh_anchorp;
	struct fs_hook	*current, *prev, *tmpfsh;

	/* arg check */
	if (!fsh)
		return;

	switch (cmd) {
	      case FS_EXECH_ADD:
		fsh_anchorp = &fs_exech_anchor;
		break;
	      case FS_CLOSEH_ADD:
		fsh_anchorp = &closeh_anchor;
		break;
	      default:
		return;
	}

	/* 
	 * If hook is already in list, remove it.
	 */
remloop:
	tmpfsh = fsh;
	for (prev=NULL, current = *fsh_anchorp;
	     current;
	     prev = current, current = current->next)
	 	if (current == fsh) {
			if (prev)
				if (!compare_and_swap(&prev->next,
						      &tmpfsh, current->next))
					goto remloop;
			else
				if (!compare_and_swap(fsh_anchorp,
						      &tmpfsh, current->next))
					goto remloop;
			break;
		}

	/* 
	 * Add hook at head of list.
	 */
	fsh->next = *fsh_anchorp;
	while (!compare_and_swap(fsh_anchorp, &fsh->next, fsh));
}
