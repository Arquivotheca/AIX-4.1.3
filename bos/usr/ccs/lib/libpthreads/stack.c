static char sccsid[] = "@(#)21	1.12  src/bos/usr/ccs/lib/libpthreads/stack.c, libpth, bos41J, 9513A_all 3/28/95 08:56:16";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_get_stack_pointer
 *	setup_stack
 *	alloc_initial_stack
 *	stack_fork_before
 *	stack_fork_after
 *	_pthread_stack_startup
 *	new_stack
 *	_alloc_stack
 *	_dealloc_stack
 * 
 * ORIGINS:  71  83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: stack.c
 *
 * The functions in this file perform all the stack management and stack
 * related functions. 
 *
 * Stacks are (currently) attached to the vps executing on them. This happens
 * to be OK as there is a 1:1 mapping of pthreads to vps. Stacks have two
 * extra pages allocated for them. The first is the red-zone, which is both
 * read and write protected for overflow detection. The second is for thread
 * specific data. The initial stack (ie the stack for the thread that calls
 * main() is treated differently in that it is not deallocated in the same way.
 * It may be reused safely if the main() thread calls pthread_exit().
 * Default stack = stack 96K + redzone 4K + cleanup stack 24K
 */

#include "internal.h"
#include <sys/mman.h>


/*
 * Local Macros
 */
#define PAGE_ROUND_DOWN(b)	((b) & ~(__page_sizeM1))
#define PAGE_ROUND_UP(b)	PAGE_ROUND_DOWN((b) + __page_sizeM1)

/*
 * Local Variables
 */
private spinlock_t	initial_stack_lock;
private caddr_t	initial_stack_base;
private size_t		initial_stack_size;
private	int		initial_stack_allocated;

/*
 * Exported Variables
 */
size_t	_pthread_default_stack_size;


/*
 * Function:
 *	_get_stack_pointer
 *
 * Return value:
 *	This function returns a value just a little way up the stack from
 *	the callers frame.
 *
 * Description:
 *	Return the address of an automatic variable that will be on the
 *	stack. We know it will not be in a register as we take its address.
 *
 */
caddr_t
_get_stack_pointer(void)
{
	auto int	x;

	return ((caddr_t)&x);
}


/*
 * Function:
 *	setup_stack
 *
 * Parameters:
 *	vp	- the vp structure we are attaching this stack to
 *	base	- base address of the new stack
 *	size	- the size of the new stack
 *
 * Description:
 *	Add the base/range description of the stack to the vp data and
 *	protect the red zone.
 */
private void
setup_stack(vp_t vp, caddr_t base, size_t size)
{
	int res;
	caddr_t redzone;

	/* Save the stack dimensions away ignoring the red zone.
	 */
	vp->stack.size = size - vp->cancel_stack_size - RED_ZONE_SIZE;

	vp->stack.base = (long)base;
	vp->stack.limit = (long)base + size - sizeof(void *);

	/* Bind the stack to the vp.
	 */
	vp->stack.vp = vp;

	/* Make the red zones untouchable if it is not the initial stack.
	 * Two red zones must be defined:
	 * first one for the cancel stack (its size is taken in the cancel
	 *		stack area.
	 * second one for the user stack (it is out of the user stack).
	 */
#ifdef MMAP
	if (!vp->flags & VP_INITIAL_STACK) {
	    redzone = (caddr_t)vp->stack.base;
            res = mprotect (redzone, RED_ZONE_SIZE, PROT_NONE);
	    if (res == -1)
		INTERNAL_ERROR("setup_stack");

	    redzone = (caddr_t)vp->stack.base + vp->cancel_stack_size;
            res = mprotect (redzone, RED_ZONE_SIZE, PROT_NONE);
	    if (res == -1)
		INTERNAL_ERROR("setup_stack");
	}
#endif
}


/*
 * Function:
 *	alloc_initial_stack
 *
 * Parameters:
 *	vp	- The vp needing a new stack
 *	size	- the minimum size of the new stack
 *
 * Return value:
 *	TRUE	The initial stack was allocated to this vp
 *	FALSE	otherwise
 *
 * Description:
 *	The initial stack allocation is simply controlled by a global
 *	flag (protected by a spin lock). If the flag is true and the
 *	initial flag is at least the requested size then we grab it.
 */
private int
alloc_initial_stack(vp_t vp, size_t size)
{
	/* There is no point in looking if it is free if it isn't big enough.
	 */
	if (size > initial_stack_size)
		return (FALSE);

	/* Take the lock and see if the stack is free.
	 */
	_spin_lock(&initial_stack_lock);
	if (initial_stack_allocated) {
		_spin_unlock(&initial_stack_lock);
		return (FALSE);
	}

	/* The stack is free and it is big enough. Mark it as taken and
	 * drop the lock.
	 */
	initial_stack_allocated = TRUE;
	_spin_unlock(&initial_stack_lock);

	/* Tell the world that we have the initial stack.
	 */
	vp->flags |= VP_INITIAL_STACK;

	/* Attach this stack to our vp and return success.
	 */
	vp->cancel_stack_size = initial_stack_size / 4;
	setup_stack(vp, initial_stack_base, initial_stack_size);
	return (TRUE);
}


/*
 * Function:
 *	stack_fork_before
 *
 * Description:
 *	Quiesce the stack allocation prior to a fork.
 */
private void
stack_fork_before(void)
{
	_spin_lock(&initial_stack_lock);
}


/*
 * Function:
 *	stack_fork_after
 *
 * Description:
 *	Allow stack allocations to start after a fork.
 */
private void
stack_fork_after(void)
{
	_spin_unlock(&initial_stack_lock);
}


/*
 * Function:
 *	_pthread_stack_startup
 *
 * Parameters:
 *	vp	- The initial vp
 *
 * Description:
 *	This function is called by pthread_init() at startup time to
 *	initialize the stack data and to allocate the initial stack
 *	to the initial vp.
 *	Register the fork handlers.
 *	This function is called from pthread_init().
 */
void
_pthread_stack_startup(vp_t vp)
{
	struct rlimit	limits;
	caddr_t	sp;
	size_t	_pthread_default_initial_stack_size;

	/*  default stack size.
	 */
	_pthread_default_stack_size = PTHREAD_STACK_MIN;

	/* Find the default initial stack size. Use the maximum break size from
	 * getrlimit().  */
	if (getrlimit(RLIMIT_STACK, &limits) != 0) {
		perror("getrlimit");
		INTERNAL_ERROR("_stack_startup");
	}

	/* Remember the default size, round it up to the nearest page
	 */
	_pthread_default_initial_stack_size = PAGE_ROUND_UP(limits.rlim_cur);

	/* Find out where the initial stack is. We mark the base as being
	 * beyond the current point so we don't trample on the process entry
	 * information when we reallocate it to another thread. We do need
	 * to remember the real information too though.
	 * stack_direction = STACK_DOWN
	 */
	sp = _get_stack_pointer();
	initial_stack_base = (caddr_t)((unsigned int)sp & ~(_pthread_default_initial_stack_size - 1));
	initial_stack_size = PAGE_ROUND_DOWN(sp - initial_stack_base);
	initial_stack_allocated = FALSE;
	_spinlock_create(&initial_stack_lock);

	/* Now allocate the initial stack to the initial vp so it looks
	 * like any other vp.
	 */
	alloc_initial_stack(vp, initial_stack_size);

	if (pthread_atfork(stack_fork_before,
			      stack_fork_after,
			      stack_fork_after))
		INTERNAL_ERROR("_pthread_stack_setup");
}


/*
 * Function:
 *	new_stack
 *
 * Parameters:
 *	vp	- the vp in need of a new stack
 *	size	- The size of the stack to be created
 *
 * Description:
 *	Allocate a new stack for a vp. This function is called if a new vp
 *	is being created from scratch or a vp is having a larger stack created.
 */
private int
new_stack(vp_t vp, size_t size)
{
	caddr_t	base;

        		/* stack = 24k +4K + 96K */
#ifdef MMAP
        base = mmap ((caddr_t)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 
			-1, 0);
#else
	base = (caddr_t)malloc(size);
	if (base == NULL) base = (caddr_t)(-1);
#endif

	if (base == (caddr_t)-1) {
	        /* Running out of space isn't necessarily fatal.
                */
                if (errno == ENOMEM) {
                       return (FALSE); /*  EAGAIN  POSIX 1003.4a */
                }
		INTERNAL_ERROR("new_stack");
	}
	setup_stack(vp, (caddr_t)base, size);
	return (TRUE);
}


/*
 * Function:
 *	_alloc_stack
 *
 * Parameters:
 *	vp	- the vp needing the new stack
 *	size	- the minimum size of this new stack
 *
 * Return value:
 *	TRUE	if the new stack is allocated
 *
 * Description:
 *	Try to allocate the initial stack for the vp. If we can't then just
 *	allocate a new one from scratch.
 */
int
_alloc_stack(vp_t vp, size_t size)
{
	/* Make sure the requested stack is at least a sensible size capable
	 * of supporting a couple of stack frames, then add the size of the
	 * red zone.
	 * The size of the cleanup stack must be also added.
	if (size < PTHREAD_STACK_MIN)
		size = PTHREAD_STACK_MIN;
	 */

	size += vp->cancel_stack_size + RED_ZONE_SIZE;

	/* Initial stack was not available so just create a new one.
	 */
	return (new_stack(vp, size));
}


/*
 * Function:
 *	_dealloc_stack
 *
 * Parameters:
 *	vp	- the vp to have the stack removed
 *
 * Description:
 *	Throw away the stack attached to this vp. If it is the initial stack
 *	then note the stack is free otherwise vm_deallocate it. Set the vp
 *	data to zero to stop is from deallocting twice by mistake.
 */
void
_dealloc_stack(vp_t vp)
{
	int res;
	caddr_t addr;
	size_t len;


	if (vp->flags & VP_INITIAL_STACK) {

		/* The vp has the initial stack, free it and mark the vp
		 * to show it has gone.
		 */
		_spin_lock(&initial_stack_lock);
		initial_stack_allocated = FALSE;
		_spin_unlock(&initial_stack_lock);
		vp->flags &= ~VP_INITIAL_STACK;

	} else {

		/* Free ordinary stacks using vm_deallocate.
		 */
 		addr = (caddr_t) vp->stack.base;
                len = vp->stack.size + vp->cancel_stack_size + RED_ZONE_SIZE;
#ifdef MMAP
                res = munmap( addr, len);
		if (res == -1) {
			INTERNAL_ERROR("_dealloc_stack");
		}
#else
		free(addr);
#endif
	}

	/* Mark the vp as having no stack.
	 */
	vp->stack.base = 0;
	vp->stack.limit = 0;
	vp->stack.size = 0;
}

