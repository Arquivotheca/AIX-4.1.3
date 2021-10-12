/*  @(#)27	1.12.1.7  src/bos/kernel/sys/ptrace.h, sysproc, bos411, 9428A410j 12/16/93 12:04:35 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 3, 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_PTRACE
#define _H_PTRACE

#include <sys/thread.h>

/*
 * Request parameters for the ptrace() system call.  The child process must
 * issue the PT_TRACE_ME ptrace request to set the child's trace flag that
 * causes the child to be left in a stopped state upon receipt of a signal.
 * The parent process can examine and modify the child process's memory
 * image using the ptrace() call when the child is in the stopped state.
 * Also, the parent process can cause the child process to either terminate
 * or continue, with the possibility of ignoring the signal that caused it
 * to stop.
 */

#define	PT_TRACE_ME	0	/* used ONLY by child process to be traced */
#define	PT_READ_I	1	/* read child instruction address space */
#define	PT_READ_D	2	/* read child data address space */
#define	PT_WRITE_I	4	/* write child instruction address space */
#define	PT_WRITE_D	5	/* write child data address space */
#define	PT_CONTINUE	7	/* continue execution */
#define	PT_KILL		8	/* terminate execution */
#define	PT_STEP		9	/* execute one or more instructions */	

#define	PT_READ_GPR	11	/* read general purpose register */
#define	PT_READ_FPR	12	/* read floating point register */

#define	PT_WRITE_GPR	14	/* write general purpose register */
#define	PT_WRITE_FPR	15	/* write floating point register */

#define PT_READ_BLOCK	17	/* read block of data ptrace(17) */
#define PT_WRITE_BLOCK	19	/* write block of data ptrace(19) */

#define PT_ATTACH	30	/* attach to a process */
#define PT_DETACH 	31	/* detach a proc to let it keep running */
#define PT_REGSET 	32	/* return entire register set to caller */
#define PT_REATT  	33	/* reattach debugger to proc */
#define PT_LDINFO   	34	/* return loaded program file descriptor/info */
#define PT_MULTI  	35	/* set/clear multi-processing */
#define PT_NEXT		36	/* step over instruction, not implemented in */
			 	/* AIX ptrace (only for kdbx kernel debugger) */

#define PTT_CONTINUE	50	/* continue execution of one or more threads */
#define PTT_STEP	51	/* execute one or more instr. for a thread */
#define PTT_READ_SPRS	52	/* read thread's special purpose registers */
#define PTT_WRITE_SPRS	53	/* write thread's special purpose registers */
#define PTT_READ_GPRS	54	/* read thread's general purpose registers */
#define PTT_WRITE_GPRS	55	/* write thread's general purpose registers */
#define PTT_READ_FPRS	56	/* read thread's floating point registers */
#define PTT_WRITE_FPRS	57	/* write thread's floating point registers */

/* maximum bytes transferred with read/write via the ptrace() system call */
#define	IPCDATA		1024

#ifdef _NO_PROTO
int	ptrace();
#else /* _NO_PROTO */
int	ptrace(int request, int id, int *address, int data, int *buffer);
#endif /* _NO_PROTO */

struct ptsprs {
	unsigned long	pt_iar;		/* instruction address register	*/
	unsigned long	pt_msr;		/* machine state register	*/
	unsigned long	pt_cr;		/* condition register		*/
	unsigned long	pt_lr;		/* link register		*/
	unsigned long	pt_ctr;		/* count register		*/
	unsigned long	pt_xer;		/* fixed point exception	*/
	unsigned long	pt_mq;		/* multiply/quotient register	*/
	unsigned long	pt_reserved_0;	/* reserved			*/
	unsigned long	pt_fpscr;	/* floating point status reg	*/
	char		pt_reserved_1;	/* reserved			*/
	char            pt_reserved_2;	/* reserved			*/
	char            pt_reserved_3[2];/* reserved			*/
	unsigned long	pt_reserved_4[5];/* reserved			*/
	void		*pt_reserved_5;	/* reserved			*/
	unsigned long	pt_reserved_6;	/* reserved			*/
        unsigned long	pt_reserved_7;	/* reserved			*/
	unsigned long	pt_reserved_8;	/* reserved			*/
	unsigned long	pt_reserved_9;	/* reserved			*/
	unsigned long	pt_fpscrx;	/* software extension to fpscr  */
};

struct ptthreads
{
	tid_t	th[MAXTHREADS];
};

#ifdef _KERNEL

#define	IPCBSZ		(sizeof (double))
#define	MAXLDINFO	(128*1024)

struct ptipc {
	int		 ip_flag;	/* flags defined below */
	struct proc	*ip_dbp;	/* debugger process */
	int		 ip_event;	/* event used for ptrace/procxmt IPC */
	int		 ip_id;		/* ptrace(2) id parmameter */
	int		 ip_req;	/* ptrace(2) request parameter */
	int		*ip_addr;	/* ptrace(2) address parameter */
	int		 ip_data;	/* ptrace(2) data parameter */
	char		*ip_blkdata;	/* pointer to IPCDATA sized block or */
	char		 ip_buf[IPCBSZ];/* ... here if !(ip_flag & IPCBLKDATA)*/
};

/* ip_flag */
#define	IPCBUSY		0x00000001
#define	IPCWANTED	0x00000002
#define	IPCPTRACE	0x00000004
#define	IPCBLKDATA	0x00000008

#endif /* _KERNEL */

#endif /* _H_PTRACE */
