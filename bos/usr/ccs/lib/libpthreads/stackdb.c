static char sccsid[] = "@(#)22	1.1  src/bos/usr/ccs/lib/libpthreads/stackdb.c, libpth, bos411, 9428A410j 10/15/93 07:31:58";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	stackdb_fork_before
 *	stackdb_fork_after
 *	_pthread_stackdb_startup
 *	square_root
 *	increment_stack_count
 *	decrement_stack_count
 *	find_cluster
 *	cluster_check
 *	calculate_new_midpoint
 *	create_new_cluster
 *	destroy_cluster
 *	merge_cluster
 *	remove_cluster_tail
 *	add_stack_to_cluster
 *	remove_stack_from_cluster
 *	collapse_clusters
 *	calculate_root_limits
 *	check_for_cluster_size
 *	_add_stack
 *	_delete_stack
 *	stack_lookup
 *	_stack_self
 *	toggle_tracing
 *	dump_stack_stats
 *	dump_cluster
 *	dump_cluster_element
 *	dump_stack_element
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
 * File: stackdb.c
 *
 * This file contains the functions which manage the stack database used
 * to look up thread identity.
 *
 * The stack database is a linked list of clusters each of which contains
 * a linked list of stack descriptors. There are mid point pointers to both
 * the centre of the cluster list and to the centre of each stack list of
 * each cluster. This structure is always kept roughly square ie there are
 * the same number of clusters as there are stacks per cluster. If either
 * the number of clusters or the number of stacks in a cluster gets too big
 * or too small then the structure is shrunk or grown to keep it about square.
 * The search for a stack starts at the mid cluster and proceeds to one end.
 * In this way only half the clusters are searched. Once the correct cluster
 * is found the stack list is searched from the centre to one end and so only
 * half of the stacks in that list is searched.
 */
#include "internal.h"

/*
 * Local Definitions
 */
#define	BACKWARDS	0
#define	FORWARDS	1
#define	INITIAL_BASE	~0
#define	INITIAL_LIMIT	0


/*
 * Local Macros
 */
#define	elements_after_mid(cluster) \
		(cluster->elements / 2 + (cluster->mid_move > 0 ? 1 : 0))
#define	elements_before_mid(cluster) \
		(cluster->elements / 2 + (cluster->mid_move < 0 ? 1 : 0))

/*
 * Local types
 */
struct cluster {
	pthread_queue	link;
	unsigned long	base;
	unsigned long	limit;
	int		elements;
	int		mid_move;
	stk_t		mid_list;
	pthread_queue	stack_list;
};

typedef struct cluster cluster_t;

#ifdef STATISTICS
struct stack_stats {
	long	stack_creates;
	long	stack_destroys;
	long	cluster_creates;
	long	cluster_destroys;
	long	cluster_checks;
	long	cluster_splits;
	long	midpoint_moves;
	long	stack_adds;
	long	stack_removes;
	long	cluster_merges;
	long	collapses;
	long	cheap_collapses;
	long	medium_collapses;
	long	expensive_collapses;
	long	lookups;
	long	lookup_cluster;
	long	lookup_stack;
};
#endif

/*
 * Local Data
 */
private cluster_t	root_cluster;
private long		number_of_stacks;
private long		max_clusters;
private long		stacks_per_cluster;
private spinlock_t	stackdb_lock;
#ifdef DEBUG_PTH
private	int		stackdb_trace = 0;
#endif

#ifdef STATISTICS
private struct stack_stats	stats;
#endif


#ifdef STATISTICS
/*
 * Function:
 *	reset_stack_stats
 *
 * Description:
 *	Set the statistics to 0 when the process starts up.
 */
void
reset_stack_stats(void)
{
	stats.stack_creates = 0;
	stats.stack_destroys = 0;
	stats.cluster_creates = 0;
	stats.cluster_destroys = 0;
	stats.cluster_checks = 0;
	stats.cluster_splits = 0;
	stats.midpoint_moves = 0;
	stats.stack_adds = 0;
	stats.stack_removes = 0;
	stats.cluster_merges = 0;
	stats.collapses = 0;
	stats.cheap_collapses = 0;
	stats.medium_collapses = 0;
	stats.expensive_collapses = 0;
	stats.lookups = 0;
	stats.lookup_cluster = 0;
	stats.lookup_stack = 0;
}
#endif


/*
 * Function:
 *	stackdb_fork_before
 *
 * Description:
 *	Quiesce the stackdb subsystem prior to a fork.
 */
private void
stackdb_fork_before(void)
{
	_spin_lock(&stackdb_lock);
}


/*
 * Function:
 *	stackdb_fork_after
 *
 * Description:
 *	Release stackdb following a fork.
 */
private void
stackdb_fork_after(void)
{
	_spin_unlock(&stackdb_lock);
}


/*
 * Function:
 *	_pthread_stackdb_startup
 *
 * Description:
 *	Initialize the stack database. Clear the root cluster and reset the
 *	stats structure.
 *	Register the fork handlers.
 *	This function is called from pthread_init().
 */
void
_pthread_stackdb_startup(void)
{
	_spinlock_create(&stackdb_lock);
	queue_init(&root_cluster.link);
	root_cluster.base = INITIAL_BASE;
	root_cluster.limit = INITIAL_LIMIT;
	root_cluster.elements = 0;
	queue_init(&root_cluster.stack_list);
	root_cluster.mid_list = (stk_t)&root_cluster;
	root_cluster.mid_move = 0;
	number_of_stacks = 0;
#ifdef STATISTICS
	reset_stack_stats();
#endif

	if (pthread_atfork(stackdb_fork_before,
			      stackdb_fork_after,
			      stackdb_fork_after))
		INTERNAL_ERROR("_pthread_stackdb_startup");
}


/*
 * Function:
 *	square_root
 *
 * Parameters:
 *	val - the integer value you wish to take the root of
 *
 * Return value:
 *	The square root of val
 *
 * Description:
 *	Use a Maclaurin series factorization to work out the square root.
 *	The larger the value the more iterations needed to get the appropriate
 *	accuracy. This is here to remove a dependency on libm.
 */
private double
square_root(int val)
{
	double	an, an1, bn, root;
	int	i, loop;

	/* Calculate the square root of an integral value using
	 * an infinite product expansion.
	 */

	/* Calculate the number of product iterations needed for this
	 * number. This produces values that are accurate to at least
	 * 10 decimal places.
	 */
	loop = 6;
	for (i = val; i > 10; i /= 10)
		loop += 2;

	/* The algorithm works by factoring out the Maclaurin series.
	 * To calculate sqrt(1 + x) we let
	 *  a[0] = x;
	 *  a[n+1] = (a[n] * a[n]) / (4 * a[n] + 4)
	 *  b[n] = (2 * a[n] + 2) / (a[n] + 2)
	 *
	 * and sqrt(1 + x) = b[0] * b[1] * b[2] * ... b[n]
	 * where the value of n is determined by the level of accuracy
	 * required and the value of x.
	 */
	root = 1;
	an1 = (double)(val - 1);
	for (i = 0; i < loop; i++) {
		an = an1;
		bn = (2 * an + 2)/(an + 2);
		root = root * bn;
		an1 = (an * an)/(4 * an + 4);
	}
	return (root);
}


/*
 * Function:
 *	increment_stack_count
 *
 * Description:
 *	Increment the number of stacks in the database and calculate the
 *	new maximum number of clusters we need and the maximum number of
 *	stacks allowed in each cluster.
 */
private void
increment_stack_count(void)
{
	number_of_stacks++;
	max_clusters = 2 * (int)(square_root((number_of_stacks + 1)/2) + 0.5);

	stacks_per_cluster = (number_of_stacks + (max_clusters - 1)) /
				max_clusters;
}


/*
 * Function:
 *	decrement_stack_count
 *
 * Description:
 *	Decrement the number of stacks in the database and calculate the
 *	new maximum number of clusters we need and the maximum number of
 *	stacks allowed in each cluster.
 */
private void
decrement_stack_count(void)
{
	number_of_stacks--;
	max_clusters = 2 * (int)(square_root((number_of_stacks + 1)/2) + 0.5);

	stacks_per_cluster = (number_of_stacks + (max_clusters - 1)) /
				max_clusters;
}


/*
 * Function:
 *	find_cluster
 *
 * Parameters:
 *	stack - pointer to a stack structure containing the stack dimensions
 *
 * Return value:
 *	A pointer to a cluster to insert the stack into if one exists
 *	a pointer to the root cluster otherwise
 *
 * Description:
 *	Try to find a cluster in which this stack should be
 *	inserted into. Start the search at the mid point of the
 *	cluster list and search to the end or the beginning depending
 *	on the value of the stack. If the cluster list is empty, the root
 *	cluster is returned.
 */
private cluster_t *
find_cluster(stk_t stack)
{
	cluster_t	*mid;
	cluster_t	*end;
	cluster_t	*cluster;
	cluster_t	*next;
	int		search;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("find_cluster(stack = %x)\n", stack);
#endif

	/* Get the mid point in the root cluster and determine which way
	 * we have to search. If we strike it lucky that the stack should
	 * be in the mid cluster then return that.
	 */
	mid = (cluster_t *)root_cluster.mid_list;

	if (mid->base > stack->limit) {
		search = BACKWARDS;
		cluster = (cluster_t *)queue_prev(&mid->link);
	} else if (mid->limit < stack->base) {
		search = FORWARDS;
		cluster = mid;
	} else
		return (mid);

	/* It doesn't matter which way we search, the end is always at
	 * root.
	 */
	end = (cluster_t *)queue_end(&root_cluster.link);

	while (cluster != end) {

		/* If this cluster spans our stack then we are done.
		 */
		if ((cluster->base <= stack->base) &&
		    (cluster->limit >= stack->limit))
			return (cluster);

		/* If this stack falls between this cluster and the following
		 * cluster then return this cluster.
		 */
		next = (cluster_t *)queue_next(&cluster->link);
		if ((cluster->limit < stack->base) &&
		    ((next->base > stack->limit) || (next == end)))
			return (cluster);

		/* Get the next cluster to search
		 */
		if (search == BACKWARDS)
			cluster = (cluster_t *)queue_prev(&cluster->link);
		else
			cluster = (cluster_t *)queue_next(&cluster->link);
	}

	/* There are no clusters in the list, return the root cluster.
	 */
	return (&root_cluster);
}


#ifdef DEBUG_PTH
/*
 * Function:
 *	cluster_check
 *
 * Parameters:
 *	cluster - pointer to the cluster to check.
 *
 * Description:
 *	perform a consistency check on the cluster. Check that the number
 *	of elements the cluster claims to have are actually there.
 */
private void
cluster_check(cluster_t *cluster)
{
	cluster_t	*c;
	int		n;

	if (cluster == &root_cluster)
		return;

	for (c = (cluster_t *)queue_head(&cluster->stack_list), n = 0;
	     c != (cluster_t *)queue_end(&cluster->stack_list);
	     c = (cluster_t *)queue_next(&c->link))
		n++;

	if (n != cluster->elements) {
		dbgPrintf("counted %d elements, expected %d\n",
					n, cluster->elements);
		INTERNAL_ERROR("cluster_check");
	}
}
#endif


/*
 * Function:
 *	calculate_new_midpoint
 *
 * Parameters:
 *	cluster - the cluster to be checked
 *
 * Description:
 *	Move the pointer to the middle of the stack list to a new
 *	position after stacks have been removed/added to this cluster.
 *	For every multiple of 2 of the mid_move value, the mid pointer
 *	should be advanced/retarded depending on the sign.
 */
private void
calculate_new_midpoint(cluster_t *cluster)
{
#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("calculate_new_midpoint(cluster = %x)\n", cluster);

	cluster_check(cluster);		/* Paranoia */
#endif

	for (;;) {
		if (cluster->mid_move > 1) {

			/* Move the mid pointer forward
			 */
			cluster->mid_list = (stk_t)
				queue_next(&cluster->mid_list->link);
			cluster->mid_move -= 2;

		} else if (cluster->mid_move < -1) {

			/* Move the mid pointer backwards
			 */
			cluster->mid_list = (stk_t)
				queue_prev(&cluster->mid_list->link);
			cluster->mid_move += 2;

		} else {

			/* We have done all the moving we need to do
			 */
			break;
		}
#ifdef STATISTICS
		stats.midpoint_moves++;
#endif
	}

	/* Check the cluster still makes sense
	 */
	if ((cluster->mid_list == (stk_t)cluster) &&
	    (cluster->elements != 0))
		INTERNAL_ERROR("calculate_new_midpoint");
}


/*
 * Function:
 *	create_new_cluster
 *
 * Parameters:
 *	prev - the cluster in the chain to insert the new cluster after
 *
 * Return value:
 *	A pointer to the new cluster if created
 *	NULL if no memory for the cluster is available.
 *
 * Description:
 *	Create and initialize a new cluster and add it into the cluster
 *	list. If the list already contained clusters then calculate a new
 *	midpoint which may be necessary.
 */
private cluster_t *
create_new_cluster(cluster_t *prev)
{
	cluster_t	*cluster;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("create_new_cluster(prev = %x)\n", prev);
#endif

#ifdef STATISTICS
	stats.cluster_creates++;
#endif

	/* Allocate memory for the new cluster.
	 */
	cluster = (cluster_t *)malloc(sizeof(cluster_t));
	if (cluster == NULL)
		return (NULL);

	/* Initialize the new cluster and insert it into the cluster list.
	 */
	cluster->base = 0;
	cluster->limit = 0;
	cluster->elements = 0;
	queue_init(&cluster->stack_list);
	cluster->mid_list = (stk_t)cluster;
	cluster->mid_move = 0;
	queue_insert(&prev->link, &cluster->link);

	/* Check is this is the first cluster in the list, if so then
	 * set up the root cluster.
	 */
	if (root_cluster.elements == 0) {
		root_cluster.mid_move = 1;
		root_cluster.mid_list = (stk_t)cluster;
		root_cluster.elements++;

	} else {

		/* This is not the first cluster. Calculate the new mid_move
		 * value and check to see if we need a new mid point.
		 */
		if ((prev->limit < root_cluster.mid_list->base) ||
		    (prev == &root_cluster)) {
			/* we are adding before the mid point */
			root_cluster.mid_move--;
		} else {
			/* we are adding after the mid point */
			root_cluster.mid_move++;
		}
		root_cluster.elements++;
		calculate_new_midpoint(&root_cluster);
	}
	return (cluster);
}


/*
 * Function:
 *	destroy_cluster
 *
 * Parameters:
 *	cluster - remove an empty cluster from the cluster list
 *
 * Description:
 *	Remove a cluster from the cluster list. If the cluster is the mid point
 *	then we move the mid point before removal. Having removed the cluster
 *	we check to see if the mid point should be moved.
 */
private void
destroy_cluster(cluster_t * cluster)
{
#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("destroy_cluster(cluster = %x)\n", cluster);
#endif

#ifdef STATISTICS
	stats.cluster_destroys++;
#endif

	/* Don't allow non empty clusters to be removed.
	 */
	if (cluster->elements != 0)
		INTERNAL_ERROR("destroy_cluster");

	/* If we are trying to remove the mid point then we move the mid
	 * point forwards or backwards depending on the current value of
	 * mid_move. The idea is to move the mid point in the direction that
	 * would be correct after the cluster has been removed.
	 */
	if (cluster == (cluster_t *)root_cluster.mid_list) {
		if (root_cluster.mid_move <= 0) {
			root_cluster.mid_list = (stk_t)
				queue_prev(&root_cluster.mid_list->link);
			root_cluster.mid_move += 2;
		} else {
			root_cluster.mid_list = (stk_t)
				queue_next(&root_cluster.mid_list->link);
			root_cluster.mid_move -= 2;
		}
	}

	/* Check to see if we are removing before or after the mid point and
	 * adjust mid_move accordingly. We know that we are not removing the
	 * mid point so it has to be one or the other.
	 */
	if (cluster->limit < root_cluster.mid_list->base) {
		/* we are destroying before the mid point */
		root_cluster.mid_move++;
	} else {
		/* we are destroying after the mid point */
		root_cluster.mid_move--;
	}

	/* Actually remove the cluster and update the root cluster.
	 */
	queue_remove(&cluster->link);
	free(cluster);
	root_cluster.elements--;
	calculate_new_midpoint(&root_cluster);
}


/*
 * Function:
 *	merge_clusters
 *
 * Parameters:
 *	target - the cluster to get all the stacks
 *	source - the cluster to lose all the stacks
 *
 * Description:
 *	move all the stacks from the source cluster onto the target cluster
 *	keeping the target mid point valid. This is normally done when there
 *	are too many clusters (the number of stacks has been reduced) and we
 *	need to delete some clusters.
 */
private void
merge_clusters(cluster_t *target, cluster_t *source)
{
#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("merge_clusters(target = %x, source = %x)\n",
				target, source);
#endif

#ifdef STATISTICS
	stats.cluster_merges++;
#endif

	/* Find out whether the source stacks have to be added on the
	 * head or the tail of the target stack list.
	 */
	if (source->base > target->limit) {

		/* Add source to end of target.
		 */
		queue_merge(&target->stack_list, &source->stack_list);
		target->mid_move += source->elements;
		target->limit = source->limit;

		/* We can merge onto an empty cluster so check for this.
		 */
		if (target->elements == 0)
			target->base = source->base;

	} else {

		/* Add source to front of target.
		 */
		queue_merge(&source->stack_list, &target->stack_list);
		queue_move(&target->stack_list, &source->stack_list);
		target->mid_move -= source->elements;
		target->base = source->base;

		/* We can merge onto an empty cluster so check for this.
		 */
		if (target->elements == 0)
			target->limit = source->limit;
	}

	/* If the merge was onto an empty cluster then the mid list does
	 * not point at anything sensible so we have to set it up.
	 */
	if (target->elements == 0)
		target->mid_list = (stk_t)queue_head(&target->stack_list);
	target->elements += source->elements;
	source->elements = 0;

	/* Finally move the mid point down the list to take into account of
	 * the new stacks.
	 */
	calculate_new_midpoint(target);
}


/*
 * Function:
 *	remove_cluster_tail
 *
 * Parameters:
 *	cluster - the cluster to lose the end half of the stack list
 *	tail - the empty cluster to get the stacks
 *
 * Description:
 *	Remove all the stacks from the mid point to the end of the list
 *	from cluster and add it onto tail. Keep the mid point information
 *	valid for the cluster but we don't bother with the tail as this is
 *	normally a temporary cluster that will be merged with another cluster 
 *	immediately.
 */
private void
remove_cluster_tail(cluster_t *cluster, cluster_t *tail)
{
	stk_t	mid_prev = NULL;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("remove_cluster_tail(cluster = %x, tail = %x)\n",
					cluster, tail);
#endif

	/* Calculate the number of stacks being moved and adjust the mid point
	 * information.
	 */
	tail->elements = elements_after_mid(cluster);
	cluster->elements = elements_before_mid(cluster);
	cluster->mid_move = 2 - cluster->elements;

	/* Save the cluster limit information for both clusters.
	 */
	tail->base = cluster->mid_list->base;
	tail->limit = cluster->limit;
	if (cluster->elements != 0) {
		mid_prev = (stk_t)queue_prev(&cluster->mid_list->link);
		cluster->limit = mid_prev->limit;
	}

	/* Copy the stacks over in one go.
	 */
	queue_split(&cluster->stack_list, &tail->stack_list,
	                    &cluster->mid_list->link);

	/* Adjust the mid list pointer
	 */
	if (cluster->elements != 0) {
		cluster->mid_list = mid_prev;
		calculate_new_midpoint(cluster);
	}
}


/*
 * Function:
 *	add_stack_to_cluster
 *
 * Parameters:
 *	cluster - the target cluster to get the stack
 *	new_stack - the stack being added
 *
 * Description:
 *	Add a stack to a cluster which may be empty. The cluster limit
 *	information and mid point information have to be kept updated.
 */
private void
add_stack_to_cluster(cluster_t *cluster, stk_t new_stack)
{
	stk_t		stack;
	stk_t		next;
	stk_t		end;
	int		search;
	cluster_t	*new_cluster;
	cluster_t	cluster_tail;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("add_stack_to_cluster(cluster = %x, stack(%x, %x) = %x)\n",
			cluster, new_stack->base, new_stack->limit, new_stack);

	if (new_stack->limit == 0)
		INTERNAL_ERROR("add_stack_to_cluster");
#endif

#ifdef STATISTICS
	stats.stack_adds++;
#endif

	if (cluster->elements == 0) {

		/* This is the first stack in the cluster.
		 */
		queue_append(&cluster->stack_list, &new_stack->link);
		cluster->base = new_stack->base;
		cluster->limit = new_stack->limit;
		cluster->mid_move = 1;
		cluster->mid_list = new_stack;
		cluster->elements++;

	} else {

		/* Add the stack into the correct point in the cluster.
		 * start the search for the insertion point at the middle
		 * and go to one end.
		 */
		stack = cluster->mid_list;
		if (stack->base > new_stack->limit)
			search = BACKWARDS;
		else if (stack->limit < new_stack->base)
			search = FORWARDS;
		else	/* overlapping stacks */
			INTERNAL_ERROR("add_stack_to_cluster");
		end = (stk_t)queue_end(&cluster->stack_list);
		while (stack != end) {
			next = (stk_t)queue_next(&stack->link);

			/* Check to see if the stack falls inbetween this one
			 * and the next. If so break out of the loop.
			 */
			if ((new_stack->base > stack->limit) &&
			    ((new_stack->limit < next->base) || (next == end)))
				break;
			if (search == BACKWARDS)
				stack = (stk_t)queue_prev(&stack->link);
			else
				stack = next;
		}

		/* Insert in the correct position and adjust the cluster
		 * limit information.
		 */
		queue_insert(&stack->link, &new_stack->link);
		if (cluster->base > new_stack->base)
			cluster->base = new_stack->base;
		if (cluster->limit < new_stack->limit)
			cluster->limit = new_stack->limit;

		/* Adjust the mid point information and move the mid list
		 * pointer if necessary.
		 */
		cluster->mid_move += (search == BACKWARDS ? -1 : 1);
		cluster->elements++;
		calculate_new_midpoint(cluster);
	}

	/* If we have just exceeded the number of stacks per cluster allowed
	 * the create a new cluster and split the stack list between them.
	 */
	if (cluster->elements > stacks_per_cluster) {
		new_cluster = create_new_cluster(cluster);
		remove_cluster_tail(cluster, &cluster_tail);
		merge_clusters(new_cluster, &cluster_tail);
	}
}


private void
remove_stack_from_cluster(cluster_t *cluster, stk_t stack)
{
	stk_t	stack_tmp;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("remove_stack_from_cluster(cluster(%d) = %x, stack(%x, %x) = %x)\n",
			cluster->elements, cluster, stack->base, stack->limit, stack);
#endif

#ifdef STATISTICS
	stats.stack_removes++;
#endif

	if (stack == cluster->mid_list) {
		if (cluster->mid_move <= 0) {
			cluster->mid_list = (stk_t)
				queue_prev(&cluster->mid_list->link);
			cluster->mid_move += 2;
		} else {
			cluster->mid_list = (stk_t)
				queue_next(&cluster->mid_list->link);
			cluster->mid_move -= 2;
		}
	}
	if (stack->limit < cluster->mid_list->base) {
		/* we are removing before the mid point */
		cluster->mid_move++;
	} else {
		/* we are removing after the mid point */
		cluster->mid_move--;
	}
	queue_remove(&stack->link);
	cluster->elements--;
	if (cluster->elements != 0) {
		stack_tmp = (stk_t)queue_head(&cluster->stack_list);
		cluster->base = stack_tmp->base;
		stack_tmp = (stk_t)queue_tail(&cluster->stack_list);
		cluster->limit = stack_tmp->limit;
		calculate_new_midpoint(cluster);
	}
}


private void
collapse_clusters(void)
{
	cluster_t	*cluster;
	cluster_t	*prev;
	cluster_t	*next;
	cluster_t	cluster_tail;
	stk_t		stack;
	int		prev_can_take_half = FALSE;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("collapse_clusters()\n");
#endif

#ifdef STATISTICS
	stats.collapses++;
#endif

	cluster = (cluster_t *)queue_head(&root_cluster.link);

	while (root_cluster.elements > max_clusters) {
		next = (cluster_t *)queue_next(&cluster->link);
		if (next == (cluster_t *)queue_end(&root_cluster.link))
			break;

		if ((cluster->elements + next->elements) <= stacks_per_cluster) {
			merge_clusters(cluster, next);
			destroy_cluster(next);
#ifdef STATISTICS
			stats.cheap_collapses++;
#endif
			continue;
		}

		if (prev_can_take_half &&
		    ((next->elements + elements_after_mid(cluster))
						<= stacks_per_cluster)) {
			remove_cluster_tail(cluster, &cluster_tail);
			prev = (cluster_t *)queue_prev(&cluster->link);
			merge_clusters(prev, cluster);
			destroy_cluster(cluster);
			merge_clusters(next, &cluster_tail);
#ifdef STATISTICS
			stats.medium_collapses++;
#endif
		}

		if ((cluster->elements + elements_before_mid(next))
						<= stacks_per_cluster)
			prev_can_take_half = TRUE;
		else
			prev_can_take_half = FALSE;

		cluster = next;
	}

	if (root_cluster.elements <= max_clusters)
		return;

	/* There is no quick way to collapse the clusters.
	 */
#ifdef STATISTICS
	stats.expensive_collapses++;
#endif
	cluster = (cluster_t *)queue_head(&root_cluster.link);
	while (root_cluster.elements > max_clusters) {
		next = (cluster_t *)queue_next(&cluster->link);
		while (cluster->elements < stacks_per_cluster) {
			stack = (stk_t)queue_head(&next->stack_list);
			remove_stack_from_cluster(next, stack);
			add_stack_to_cluster(cluster, stack);
			if (next->elements == 0) {
				destroy_cluster(next);
				if (root_cluster.elements <= max_clusters)
					return;
				next = (cluster_t *)
					queue_next(&cluster->link);
			}
		}
		cluster = (cluster_t *)queue_next(&cluster->link);
	}
}


private void
calculate_root_limits(void)
{
	cluster_t	*cluster;

	if (root_cluster.elements == 0) {
		root_cluster.base = INITIAL_BASE;
		root_cluster.limit = INITIAL_LIMIT;
	} else {
		cluster = (cluster_t *)queue_head(&root_cluster.link);
		root_cluster.base = cluster->base;
		cluster = (cluster_t *)queue_tail(&root_cluster.link);
		root_cluster.limit = cluster->limit;
	}
}


private void
check_for_cluster_size(void)
{
	cluster_t	*cluster;
	cluster_t	*new_cluster;
	cluster_t	cluster_tail;

#ifdef DEBUG_PTH
	if (stackdb_trace)
		dbgPrintf("check_for_cluster_size()\n");
#endif

#ifdef STATISTICS
	stats.cluster_checks++;
#endif
	for (cluster = (cluster_t *)queue_head(&root_cluster.link);
	     cluster != (cluster_t *)queue_end(&root_cluster.link);
	     cluster = (cluster_t *)queue_next(&cluster->link)) {
		if (cluster->elements > stacks_per_cluster) {
			new_cluster = create_new_cluster(cluster);
			remove_cluster_tail(cluster, &cluster_tail);
			merge_clusters(new_cluster, &cluster_tail);
			cluster = new_cluster;
#ifdef STATISTICS
			stats.cluster_splits++;
#endif
		}
	}
}


void
_add_stack(stk_t new_stack)
{
	cluster_t	*cluster;

#ifdef STATISTICS
	stats.stack_creates++;
#endif

	_spin_lock(&stackdb_lock);
	increment_stack_count();
	cluster = find_cluster(new_stack);
	if (cluster == &root_cluster)
		cluster = create_new_cluster(cluster);
	add_stack_to_cluster(cluster, new_stack);
	calculate_root_limits();
	if (root_cluster.elements > max_clusters)
		collapse_clusters();
	_spin_unlock(&stackdb_lock);
}


void
_delete_stack(stk_t stack)
{
	cluster_t	*cluster;
	long		old_stacks_per_cluster;

#ifdef STATISTICS
	stats.stack_destroys++;
#endif

	_spin_lock(&stackdb_lock);
	old_stacks_per_cluster = stacks_per_cluster;
	decrement_stack_count();
	cluster = find_cluster(stack);
	remove_stack_from_cluster(cluster, stack);
	if (cluster->elements == 0)
		destroy_cluster(cluster);
	calculate_root_limits();
	if (stacks_per_cluster < old_stacks_per_cluster)
		check_for_cluster_size();
	if (root_cluster.elements > max_clusters)
		collapse_clusters();
	_spin_unlock(&stackdb_lock);
}


private stk_t
stack_lookup(long sp)
{
	cluster_t	*cluster;
	stk_t		stack;
	int		search;

#ifdef STATISTICS
	stats.lookups++;
#endif

	_spin_lock(&stackdb_lock);

	if ((sp < root_cluster.base) || (sp > root_cluster.limit)) {
		_spin_unlock(&stackdb_lock);
		return (NULL);
	}

	cluster = (cluster_t *)root_cluster.mid_list;
	if (sp < cluster->base)
		search = BACKWARDS;
	else
		search = FORWARDS;

	for (;;) {
#ifdef STATISTICS
		stats.lookup_cluster++;
#endif
		if ((sp >= cluster->base) && (sp <= cluster->limit))
			break;
		if (search == BACKWARDS)
			cluster = (cluster_t *)queue_prev(&cluster->link);
		else
			cluster = (cluster_t *)queue_next(&cluster->link);
		if (cluster == (cluster_t *)queue_end(&root_cluster.link)) {
			_spin_unlock(&stackdb_lock);
			return (NULL);
		}
	}

	stack = cluster->mid_list;
	if (sp < stack->base)
		search = BACKWARDS;
	else
		search = FORWARDS;
	for (;;) {
#ifdef STATISTICS
		stats.lookup_stack++;
#endif
		if ((sp >= stack->base) && (sp <= stack->limit)) {
			_spin_unlock(&stackdb_lock);
			return (stack);
		}
		if (search == BACKWARDS)
			stack = (stk_t)queue_prev(&stack->link);
		else
			stack = (stk_t)queue_next(&stack->link);

		if (stack == (stk_t)queue_end(&cluster->stack_list)) {
			_spin_unlock(&stackdb_lock);
			return (NULL);
		}
	}
}


stk_t
_stack_self(void)
{
	caddr_t	sp;
	stk_t	self;

	sp = (caddr_t)_get_stack_pointer();
	self = stack_lookup((long)sp);
	if (self == NULL) {
#ifdef DEBUG_PTH
		if (stackdb_trace)
			dump_cluster();
#endif
		INTERNAL_ERROR("_stack_self");
	}
	return (self);
}


#ifdef DEBUG_PTH

void
toggle_tracing(void)
{
	stackdb_trace = !stackdb_trace;
}


#ifdef STATISTICS
dump_stack_stats(void)
{
	dbgPrintf("Stack: Creates %d, Destroys %d\n",
		stats.stack_creates, stats.stack_destroys);
	dbgPrintf("Cluster: Creates %d, Destroys %d, Mid moves %d\n",
		stats.cluster_creates, stats.cluster_destroys,
		stats.midpoint_moves);
	dbgPrintf("Stack/Cluster: Adds %d, Removes %d, Merges %d\n",
		stats.stack_adds, stats.stack_removes, stats.cluster_merges);
	dbgPrintf("Collapses: Total %d, Cheap %d, Medium %d, Expensive %d\n",
		stats.collapses, stats.cheap_collapses, stats.medium_collapses,
		stats.expensive_collapses);
	dbgPrintf("Cluster: checks %d, splits %d\n",
		stats.cluster_checks, stats.cluster_splits);
	dbgPrintf("Lookups %d, cluster cost %5.2f, Stack cost %5.2f,"
		  " ave cost %5.2f\n",
		  stats.lookups,
		  (double)stats.lookup_cluster / (double)stats.lookups,
		  (double)stats.lookup_stack / (double)stats.lookups,
		  (double)(stats.lookup_cluster + stats.lookup_stack)
		   / (double)stats.lookups);
}
#endif


dump_cluster(void)
{
	cluster_t	*cluster;

	dbgPrintf("Stacks: %d  Max Clusters: %d  Max stacks per cluster: %d"
		  " root: %x\n",
		  number_of_stacks, max_clusters, stacks_per_cluster,
		  &root_cluster);
	dbgPrintf("Cluster root: ");
	dbgPrintf("base: %x, limit: %x, size: %d, mid_list: %x,"
		  " mid_move: %d\n\n",
		  root_cluster.base, root_cluster.limit, root_cluster.elements,
		  root_cluster.mid_list, root_cluster.mid_move);
	for (cluster = (cluster_t *)queue_head(&root_cluster.link);
	     cluster != (cluster_t *)queue_end(&root_cluster.link);
	     cluster = (cluster_t *)queue_next(&cluster->link))
		dump_cluster_element(cluster);
}


dump_cluster_element(cluster_t *cluster)
{
	stk_t	stack;

	dbgPrintf("Cluster %x:\n", cluster);
	dbgPrintf("size: %d, mid_list: %x, base: %x, limit: %x, mid_move: %d\n",
		  cluster->elements, cluster->mid_list,
		  cluster->base, cluster->limit, cluster->mid_move);

	for (stack = (stk_t)queue_head(&cluster->stack_list);
	     stack != (stk_t)queue_end(&cluster->stack_list);
	     stack = (stk_t)queue_next(&stack->link))
		dump_stack_element(stack);
}


dump_stack_element(stk_t stack)
{
	dbgPrintf("\tStack %x: ", stack);
	dbgPrintf("base: %5x, limit: %5x, vp: %x\n",
		stack->base, stack->limit, stack->vp);
}

#endif

