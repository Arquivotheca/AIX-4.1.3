/* @(#)01	1.4  src/bos/kernext/aio/kernel_services.h, sysxaio, bos411, 9428A410j 3/1/94 09:35:56 */
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: kernel_services.h
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/types.h>
#include <sys/m_types.h>

pid_t	getpid(void);
void	closehadd(struct fs_hook *closeh);
void	fs_exechadd(struct fs_hook *fs_exech);
