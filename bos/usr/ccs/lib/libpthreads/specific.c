static char sccsid[] = "@(#)19	1.16  src/bos/usr/ccs/lib/libpthreads/specific.c, libpth, bos41J, 9515A_all 4/12/95 10:42:57";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	specific_fork_before
 *	specific_fork_after
 *	_pthread_specific_startup
 *	pthread_key_create
 *	__key_create_internal
 *	pthread_key_delete
 *	_pthread_setspecific
 *	pthread_setspecific
 *	pthread_getspecific
 *	_pthread_getspecific_addr
 *	_specific_data_setup_initial
 *	_specific_data_cleanup
 *	pthread_Seterrno
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
 * File: specific.c
 *
 * This file contains all the functions associated with the management of
 * thread specific data. Thread specific data is held in a page of memory
 * beyond the red zone of the stack.
 */

#include "internal.h"

extern void *_errno_hdl;
extern int _pthread_getspecific_addr(pthread_key_t, void **);
extern pthread_queue   __dbx_known_pthreads;

/*
 * Local Variables
 */
private	spinlock_t	specific_lock;
private	unsigned int	next_key;
private unsigned int    high_key;
private unsigned int    low_key;
private unsigned int    last_key;
private specific_key_t	*key_table;
private int             *key_alloc_table;

#define       existing_pthreads __dbx_known_pthreads

/*
 * Function:
 *	specific_fork_before
 *
 * Description:
 *	Quiesce the specific data subsystem. Do not allow any key creation
 *	during a fork.
 */
private void
specific_fork_before(void)
{
	_spin_lock(&specific_lock);
}


/*
 * Function:
 *	specific_fork_after
 *
 * Description:
 *	Unlock key creation after a fork.
 *	Note: The table is still valid in the child.
 */
private void
specific_fork_after(void)
{
	_spin_unlock(&specific_lock);
}


/*
 * Function:
 *	_pthread_specific_startup
 *
 * Description:
 *	Initialize the key table and lock.
 *	Register the fork handlers.
 *	This function is called from pthread_init().
 */
int
_pthread_specific_startup(void)
{
int     i;
	/* Allocate a key table for the process. This defines whether a
	 * key is valid or not. Mark the next free key to be the start
	 * of this table and initialize the lock that protects all this.
	 */
	if (!(key_table = (specific_key_t *)_pmalloc(KEYTABLE_SIZE))) {
		LAST_INTERNAL_ERROR("_pthread_specific_startup : ENOMEM");
	}

	memset((void *)key_table, 0, KEYTABLE_SIZE);

	if (!(key_alloc_table = (int *)_pmalloc(PTHREAD_DATAKEYS_MAX *
						sizeof(int)))) {
		LAST_INTERNAL_ERROR("_pthread_specific_startup : ENOMEM");
	}

	 for (i=0; i<PTHREAD_DATAKEYS_MAX; i++) {
                key_alloc_table[i] = 0;
        }

	next_key = 0;
	high_key = 0;
	low_key = 0;
	last_key = APTHREAD_DATAKEYS_MAX - 1;
	_spinlock_create(&specific_lock);

	if (pthread_atfork(specific_fork_before,
			      specific_fork_after,
			      specific_fork_after))
		INTERNAL_ERROR("_pthread_specific_startup");
	return (0);
}

/*******************************************************************************

 key_alloc_table management


 <--------------------- APTHREAD_DATAKEYS_MAX elements ----------------------->


 <------------------ PTHREAD_DATAKEYS_MAX elements -----------><-intern.keys ->

  ____________________________________________________________________________ 
 |   |   |   |   |   |   |   |   |                |   |   |   |   |   |   |   |
 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 |................| 0 | 1 | 0 | 1 | 1 | 1 | 1 |
 |___|___|___|___|___|___|___|___|________________|___|___|___|___|___|___|___|
 ^           ^                                        ^       ^
 |           |                                        |       |
 |           |                                        |       |
 |           |                                        |       |
 |           |                                        |       |
 |           |                                        |       |
next_key  low_key                                 high_key last_key

last_key is the mark of the 4 pre-allocated keys (__key_create_internal)

After the pthread_key_delete of the 3 first keys,
On the next pthread_key_create next_key is the first available key to allocate
                               low_key is the first allocated key
                               high_key is the last allocated key

after the pthread_key_create key_alloc_table becomes:
  ____________________________________________________________________________ 
 |   |   |   |   |   |   |   |   |                |   |   |   |   |   |   |   |
 | 1 | 0 | 0 | 1 | 1 | 1 | 1 | 0 |................| 0 | 1 | 0 | 1 | 1 | 1 | 1 |
 |___|___|___|___|___|___|___|___|________________|___|___|___|___|___|___|___|
 ^   ^                                                ^       ^
 |   |                                                |       |
 |   |                                                |       |
 |   |                                                |       |
 |   |                                                |       |
 |  next_key                                          |       |
low_key                                          high_key last_key

After two other pthread_key_create:
  ____________________________________________________________________________ 
 |   |   |   |   |   |   |   |   |                |   |   |   |   |   |   |   |
 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 |................| 0 | 1 | 0 | 1 | 1 | 1 | 1 |
 |___|___|___|___|___|___|___|___|________________|___|___|___|___|___|___|___|
 ^                           ^                        ^       ^
 |                           |                        |       |
 |                           |                        |       |
 |                           |                        |       |
 |                           |                        |       |
 |                           |                        |       |
low_key                   next_key               high_key last_key

*******************************************************************************/

/*
 * Function:
 *	pthread_key_create
 *
 * Parameters:
 *	key_ptr	- pointer to a place to store the newly created key
 *	destructor - function to call for any data when the thread dies.
 *
 * Return value:
 *	0	Success, key_ptr is set with the key id
 *	EINVAL	if the pointer passed is an invalid pointer
 *	EAGAIN	Insufficient memory exists to create the key.
 *
 * Description:
 *	Allocate the next entry out of the key table. Mark it
 *	as allocated and move the next key pointer on. Key ids
 *	are small integers which are used as an index into the key array.
 */
int
pthread_key_create(pthread_key_t *key_ptr, void (*destructor)(void *))
{
int     i;

	/* Check we have somewhere to store the new key id.
	 */
	if (key_ptr == NULL) {
		return (EINVAL);
	}

	/* Try to allocate the next empty key slot.
	 */
	_spin_lock(&specific_lock);
	if (next_key == -1) {
		_spin_unlock(&specific_lock);
		return (EAGAIN);
	}

	if (next_key > high_key) high_key = next_key;
	if (next_key < low_key) low_key = next_key;
	*key_ptr = next_key;

	/* We have to determine the next free key for the next key_create
        */

        key_alloc_table [next_key] = 1;
        next_key = -1;
        for (i=*key_ptr + 1; i<PTHREAD_DATAKEYS_MAX; i++) {
                if (key_alloc_table [i] == 0) {
                        next_key = i;
                        break;
                }
        }

	/* We have got the new key id. Mark the slot as allocated and
	 * remember the destructor function (which may be NULL).
	 */
	key_table[*key_ptr].flags = KEY_ALLOCATED;
	key_table[*key_ptr].destructor = destructor;
	_spin_unlock(&specific_lock);
	return (0);
}

/*
 * Function:
 *	__key_create_internal
 *
 * Parameters:
 *	key_ptr	- pointer to a place to store the newly created key
 *	destructor - function to call for any data when the thread dies.
 *
 * Return value:
 *	0	Success, key_ptr is set with the key id
 *	EINVAL	if the pointer passed is an invalid pointer
 *
 * Description:
 *	Allocate the next entry out of the key table. Mark it
 *	as allocated and move the next key pointer on. Key ids
 *	are small integers which are used as an index into the key array.
 */
int
__key_create_internal(pthread_key_t *key_ptr, void (*destructor)(void *))
{
	/* Check we have somewhere to store the new key id.
	 */
	if (key_ptr == NULL) {
                INTERNAL_ERROR("__key_create_internal, key NULL");
	}

	/* Try to allocate the next empty key slot.
	 */
	_spin_lock(&specific_lock);
	if (last_key == next_key) {
		_spin_unlock(&specific_lock);
                INTERNAL_ERROR("__key_create_internal");
	}

	*key_ptr = last_key--;

	/* We have got the new key id. Mark the slot as allocated and
	 * remember the destructor function (which may be NULL).
	 */
	key_table[*key_ptr].flags = KEY_ALLOCATED;
	key_table[*key_ptr].destructor = destructor;
	_spin_unlock(&specific_lock);
	return (0);
}

/*
 * Function:
 *      pthread_key_delete
 *
 * Parameters:
 *      key     - the id of the specific data to delet
 *
 * Return value:
 *      0       Success, key_ptr is set with the key id
 *      EINVAL  The key value is invalid
 *      EBUSY	Thread specific data is still associated with the key.
 *
 * Description:
 *      Deletes a thread-specific data key previously returned by
 *      pthread_key_create().
 */
int
pthread_key_delete(pthread_key_t key)
{
        pthread_d       thread;
	int             *ptelem;
	int             adjust;
	int             i;
        specific_data_t *data;
			/* pthread queue structures size in int */
	int off = sizeof(pthread_queue) / sizeof(int); 


        if ((key < NULL) || (key >= PTHREAD_DATAKEYS_MAX)) {
                return (EINVAL);
        }

        _spin_lock(&specific_lock);

        if (key_table[key].flags != KEY_ALLOCATED) {
                _spin_unlock(&specific_lock);
                return (EINVAL);
        }

        for (
                ptelem = (int *)queue_next(&existing_pthreads),
		adjust = (int)(ptelem - off),
		thread = (pthread_d)adjust;

                ptelem != (int *)&existing_pthreads;

                ptelem = (int *)queue_next(&thread->DBXlink),
		adjust = (int)(ptelem - off),
		thread = (pthread_d)adjust ) {

                if (!(thread->state & PTHREAD_RETURNED)) {
                        data = &thread->specific_data[key];
                        if ( data->value != NULL) {
                                _spin_unlock(&specific_lock);
                                return (EBUSY);
                        }
                }
        }

        key_table[key].flags = KEY_FREE;
        key_table[key].destructor = NULL;
	key_alloc_table [key] = 0;

	if (key < next_key) next_key = key;
	/* if the key deleted is the high_key we have to determine which is the
	 * new available high_key
	 */

	if (key == high_key) {
                for (i=high_key - 1; ((i >= low_key) && (i >=0)); i--) {
                        if (key_alloc_table [i] == 1) {
                                high_key = i;
                                break;
                        }
                }
                if (i < 0) high_key = next_key;
        }

	/* if the key deleted is the low_key we have to determine which is the
	 * new available low_key
	 */

	if (key == low_key) {
		for (i=low_key + 1; ((i <= high_key) && 
				     (i <=PTHREAD_DATAKEYS_MAX)); i++) {
			if (key_alloc_table [i] == 1) {
				low_key = i;
				break;
			}
		}
		if (i > PTHREAD_DATAKEYS_MAX) low_key = next_key;
	}

        _spin_unlock(&specific_lock);
        return (0);
}


/*
 * Function:
 *	_pthread_setspecific
 *
 * Parameters:
 *	self	- the calling thread
 *	key	- the id of the specific data to set
 *	value	- the new data to associate with that key
 *
 * Return value:
 *	0	Success, the data is set in the per thread area
 *
 * Description:
 *	internal function.
 *	put the data passed into the thread specific
 *	data pointed to by the thread structure.
 */
int
_pthread_setspecific(pthread_d self, pthread_key_t key, const void *value)
{
	specific_data_t	*data;
	specific_key_t	*key_status;

	 _spin_lock(&specific_lock);

        key_status = &key_table[key];
        _spin_unlock(&specific_lock);

	/* Mark the data as being valid.
	 */
	data = &self->specific_data[key];
	data->value = value;
	data->flags |= SPECIFIC_DATA_SET;
	return (0);
}


/*
 * Function:
 *	pthread_setspecific
 *
 * Parameters:
 *	key	- the id of the specific data to set
 *	value	- the new data to associate with that key
 *
 * Return value:
 *	0	Success, the data is set in the per thread area
 *	EINVAL	If the key is outside the key table
 *		The key is not yet allocated
 *
 * Description:
 *	Check that the key lies within the table and that is has been
 *	allocated. If so then put the data passed into the thread specific
 *	data pointed to by the thread structure.
 */
int
pthread_setspecific(pthread_key_t key, const void *value)
{
	pthread_d	self;
	specific_data_t	*data;
	specific_key_t	*key_status;

	if ((key < NULL) || (key >= APTHREAD_DATAKEYS_MAX)) {
		return (EINVAL);
	}

	 _spin_lock(&specific_lock);

        key_status = &key_table[key];
        if (key_status->flags != KEY_ALLOCATED) {
                _spin_unlock(&specific_lock);
                return (EINVAL);
        }
        _spin_unlock(&specific_lock);

	/* The key is find, now set the data. Mark the data as being valid.
	 */
	self = pthread_id_lookup(pthread_self());
	data = &self->specific_data[key];
	data->value = value;
	data->flags |= SPECIFIC_DATA_SET;
	return (0);
}


/*
 * Function:
 *	pthread_getspecific
 *
 * Parameters:
 *	key	- the id of the specific data to get
 *
 * Return value:
 *      success, the data associated with that key
 *	if no thread-specific data value is associated with key then the value
 *		 NULL is returned.
 *	
 * Description:
 *	Ensure the key and place to store the data are valid. If they are
 *	the specific data is returned if a set_specific has already
 *	happened. If the data has not already been set then NULL is returned
 *	but the call returns successfully.
 */
void *
pthread_getspecific(pthread_key_t key)
{
	pthread_d	self;
	specific_data_t	*data;
	specific_key_t	*key_status;

	if ((key < NULL) || (key >= APTHREAD_DATAKEYS_MAX)) {
                return (NULL);
        }

	_spin_lock(&specific_lock);

        if (key_table[key].flags != KEY_ALLOCATED) {
                _spin_unlock(&specific_lock);
                return (NULL);
        }
        _spin_unlock(&specific_lock);


	/* Find the data area. Return NULL if the data has not been previously
	 * set. The call is still considered a success.
	 */
	self = pthread_id_lookup(pthread_self());
	data = &self->specific_data[key];

	if (data->flags & SPECIFIC_DATA_SET)
		return ((void*)data->value);
	else
                return (NULL);
}


/*
 * Function:
 *	_pthread_getspecific_addr
 *
 * Parameters:
 *	key	- the id of the specific data
 *	addr	- the place to store the address of the data
 *
 * Return value:
 *	0	success, the address is stored in 'addr'
 *	EINVAL	The key is out of the key table
 *		The pointer 'addr' is not a valid pointer
 *		The key has within the table but unallocated
 *
 * Description:
 *	Ensure the key and place to store the address are valid.
 *	If they are, the address of the specific data is stored in addr.
 */
int
_pthread_getspecific_addr(pthread_key_t key, void **addr)
{
#ifdef errno
#undef errno
#endif /*errno*/
	extern int      errno;

	pthread_d	self;
	specific_data_t	*data;
	specific_key_t	*key_status;

        /* Check the key is within the table and the pointer we have
         * been given is OK.
         */
        if ((key >= APTHREAD_DATAKEYS_MAX) || (addr == NULL)) {
                return (EINVAL);
        }

	/* Get the key descriptor and check that this key has been allocated.
	 */
	key_status = &key_table[key];

	if (!(key_status->flags & KEY_ALLOCATED)) {
		return (EINVAL);
	}

	/* Find the data area.
	 * Set the data to NULL if it has not been previously set.
	 */
	self = pthread_id_lookup(pthread_self());
	if ((key == (pthread_key_t)_errno_hdl) && 
				(self->flags & PTHREAD_INITIAL_THREAD)) {
		*addr = &errno;
	} else {
		data = &self->specific_data[key];
		*addr = &data->value;
	}
	return (0);
}


/*
 * Function:
 *	_specific_data_setup_initial
 *
 * Parameters:
 *	thread	- The new thread that is being created
 *
 * Description:
 *	create the specific data area for initial thread.
 */
void
_specific_data_setup_initial(pthread_d thread)
{
	/*
	 * The PTHREAD_DATA page is used for thread specific data.
	 * Set the pointer in the pthread structure to point at this area.
	 */
	thread->specific_data = 
		(specific_data_t *) thread->vp->specific_data_address;
	memset((void *)thread->specific_data, 0, PTHREAD_DATA);
}


/*
 * Function:
 *	_specific_data_cleanup
 *
 * Parameters:
 *	thread	- the thread that is about to exit
 *
 * Description:
 *	For every valid key, call the destructor function on every piece
 *	of set, non-null data.
 */
void
_specific_data_cleanup(pthread_d thread)
{
	pthread_key_t	key;
	specific_data_t	*data;
	specific_key_t	*key_status;
	int             i;
	unsigned int    loc_high_key;
	unsigned int    loc_low_key;

	/* Look at all allocated keys.
	 */
	_spin_lock(&specific_lock);
	loc_high_key = high_key;
	loc_low_key = low_key;
	_spin_unlock(&specific_lock);

	for (i=loc_low_key; i<= loc_high_key; i++) {
                if (key_alloc_table [i] == 1) {
                        key = (pthread_key_t)i;
                        data = &thread->specific_data[key];
                        key_status = &key_table[key];

		/* Only call the destructor if there is one, the thread had
		 * set some data and that data was not NULL.
		 */
			if ((data->flags & SPECIFIC_DATA_SET) &&
		    	(key_status->destructor != NILFUNC(void)) &&
		    	(data->value != NULL))
				(*(key_status->destructor))((void*)data->value);
		}
	}
}

pthread_Seterrno(int error)
{
int     *addr;
int     rc;

        rc = _pthread_getspecific_addr((pthread_key_t) _errno_hdl, (void*)&addr)
;
        if (rc) {
                INTERNAL_ERROR("cannot set per-thread errno");
        }
        else
                *addr = error;
        return (error);
}
