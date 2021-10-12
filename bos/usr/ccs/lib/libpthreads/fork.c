static char sccsid[] = "@(#)08	1.8  src/bos/usr/ccs/lib/libpthreads/fork.c, libpth, bos412, 9445C412b 11/2/94 12:02:34";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	pthread_atfork
 *	pthread_atfork_np
 *	pthread_atfork_prepare
 *	pthread_atfork_parent
 *	pthread_atfork_child
 *	_pthread_fork_startup
 *	
 * ORIGINS:  71, 83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * Implementation of the Unix fork() call.
 * This is necessary to handle internal locks and data before and after
 * the call.
 */

#include "internal.h"

/*
 * Global Variables
 */
extern	void	(*_atfork_prefork_routine)();
extern	void	(*_atfork_postfork_routine)();
extern	void	(*_atfork_child_routine)();


/*
 * Local Types
 */
pthread_queue		atfork_table;

/*
 * Local Variables
 */
private spinlock_t	atfork_lock;


/*
 * Function:
 *	pthread_atfork
 *
 * Return value:
 *	0 Success
 *	ENOMEM Insufficient table space exits to record the handler addresses.
 *
 * Description:
 *	Entry point to atfork functions.
 *	Register functions for pre and post fork handling.
 */
int
pthread_atfork(void (*prepare)(), void (*parent)(), void (*child)())
{
	struct atfork	*entry;

	entry = (struct atfork *)_pmalloc(sizeof(struct atfork));
	if (entry == NULL)
		return (ENOMEM);
	_spin_lock(&atfork_lock);
	queue_append(&atfork_table, &entry->link);
	entry->flag = 0;
	entry->prepare = prepare;
	entry->parent = parent;
	entry->child = child;
	_spin_unlock(&atfork_lock);
	return (0);
}

/*
 * Function:
 *	pthread_atfork_np
 *
 * Return value:
 *	0 Success
 *	ENOMEM Insufficient table space exits to record the handler addresses.
 *
 * Description:
 *	Entry point to atfork functions.
 *	Register functions for pre and post fork handling.
 *	This function is different of pthread_atfork(), it permits to
 *	pass a parameter to functions for pre and post fork handling.
 *	This function is not portable.
 */
int
pthread_atfork_np(void *userstate, void (*prepare)(), void (*parent)(), void (*child)())
{
	struct atfork	*entry;

	entry = (struct atfork *)_pmalloc(sizeof(struct atfork));
	if (entry == NULL)
		return (ENOMEM);
	_spin_lock(&atfork_lock);
	queue_append(&atfork_table, &entry->link);
	entry->flag = 1;
	entry->userstate = userstate;
	entry->prepare = prepare;
	entry->parent = parent;
	entry->child = child;
	_spin_unlock(&atfork_lock);
	return (0);
}


/* fork() handlers
 * The current scheme is that handlers are registered in a LIFO queue.
 *
 * Before fork() the execution order is by queue order, that is each
 * handler does its stuff and calls the next handler [pthreads late].
 * After fork() the order is reversed, that is each handler calls the
 * next and then does its stuff [pthreads early].
 */

/*
 * Function:
 *	pthread_atfork_prepare 
 *
 * Description:
 *	Called before the actual fork.
 */
private void
pthread_atfork_prepare()
{
	struct atfork	*entry;

	/* Call pthread handlers (in reverse order).
	 */
	_spin_lock(&atfork_lock);
	for (entry = (struct atfork *)queue_tail(&atfork_table);
	     entry != (struct atfork *)queue_end(&atfork_table);
	     entry = (struct atfork *)queue_prev(&entry->link))
        	if (entry->prepare)
            		if (entry->flag)
                		(*entry->prepare)(entry->userstate);
            		else
                		(*entry->prepare)();
}


/*
 * Function:
 *	pthread_atfork_parent 
 *
 * Description:
 *	Called after the fork, in the parent process.
 */
private void
pthread_atfork_parent()
{
	struct atfork	*entry;

	/* Call pthread parent handlers (in FIFO order). 
	 */
	for (entry = (struct atfork *)queue_head(&atfork_table);
	     entry != (struct atfork *)queue_end(&atfork_table);
	     entry = (struct atfork *)queue_next(&entry->link))
        	if (entry->parent)
            		if (entry->flag)
                		(*entry->parent)(entry->userstate);
            		else
                		(*entry->parent)();
	_spin_unlock(&atfork_lock);
}


/*
 * Function:
 *	pthread_atfork_child 
 *
 * Description:
 *	Called after the fork, in the child process.
 */
private void
pthread_atfork_child()
{
	struct atfork	*entry;

	/* Call pthread child handlers (in FIFO order).
	 * we unlock at begining because there is just one thread.
	 */
	_spin_unlock(&atfork_lock);
	for (entry = (struct atfork *)queue_head(&atfork_table);
	     entry != (struct atfork *)queue_end(&atfork_table);
	     entry = (struct atfork *)queue_next(&entry->link))
        	if (entry->child)
            		if (entry->flag)
                		(*entry->child)(entry->userstate);
            		else
                		(*entry->child)();
}


/*
 * Function:
 *	_pthread_fork_startup
 *
 * Description:
 *	This function is called by pthread_init() at startup time to
 *	set up the fork() handling semantics.
 */
void
_pthread_fork_startup(void)
{
	/* Install the pthread handlers.
	 */
	_atfork_prefork_routine = pthread_atfork_prepare;
	_atfork_postfork_routine = pthread_atfork_parent;
	_atfork_child_routine = pthread_atfork_child;

	_spinlock_create(&atfork_lock);

	queue_init (&atfork_table);
}
