/* @(#)06	1.14  src/bos/kernel/ipc/ipcspace.h, sysipc, bos411, 9428A410j 11/25/93 09:18:25 */

#ifndef _H_IPCSPACE
#define _H_IPCSPACE

/*
 * COMPONENT_NAME: (SYSIPC) IPC size deffintions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
      
#define MSGMAX 65535		/* max message size 			*/
#define MSGMNB 65535		/* max number of bytes on a queue	*/
#define MSGMNI 4096		/* max number of message queue IDs. 	*/
				/* Must be a power of 2			*/
#define MSGMNM 8192		/* Maximium number of message per queue */

#define SEMMNI 4096		/* max number of semaphoer IDs. Must be	*/
				/* a power of 2				*/
#define SEMMSL 65535		/* max number of semaphores per ID	*/
#define SEMOPM 1024		/* max number of operations per semop call
				 * If you change this rember that semops
				 * are copied onto the kernel stack in
				 * the semop system call.  Making SEMOPM
				 * too large will overflow the kernel stack
				 */
#define SEMUME 1024		/* max number of undo entries per proc	*/
#define SEMVMX 32767		/* semaphore maximum value		*/
#define SEMAEM 16384		/* adjust on exit max value		*/

#define SHMMAX SEGSIZE		/* max shared memory segment size	*/
#define SHMMIN 1		/* min shared memory segment size	*/
#define SHMMNI 4096		/* number of shared memory IDs. Must be */
				/* a power of 2				*/

#endif



