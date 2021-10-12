/* @(#)01	1.1  src/bos/kernel/sys/mpc.h, sysios, bos411, 9428A410j 7/27/93 21:02:21 */
#ifndef _H_MPC
#define _H_MPC
/*
 * COMPONENT_NAME: (SYSIOS) Inter Processor Communication header file
 *
 * ORIGINS: 83
 *
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/processor.h>

struct mpc_reg {
	void  (*func)(void);
	uint  pri;
};

/*
 * Global mpc data.
 */
extern
struct mpc_reg mpc_reg_array[sizeof(int)*NBBY];		/* registered mpc services array   */

/*
 * cpuid for broadcast
 */
#define MPC_BROADCAST	((cpu_t)-1)

/*
 * mpc message type
 */
typedef uint mpc_msg_t;



#ifndef _NO_PROTO
mpc_msg_t mpc_register(
	uint priority,
	void (*function)(void));

void mpc_send(
	     cpu_t cpuid,
	     mpc_msg_t msg);

#else /* not _NO_PROTO */

mpc_msg_t mpc_register();
void mpc_send();

#endif /* not _NO_PROTO */

#endif /* _H_MPC */
