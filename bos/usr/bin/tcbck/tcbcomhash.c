static char sccsid[] = "@(#)45	1.1.1.1  src/bos/usr/bin/tcbck/tcbcomhash.c, cmdsadm, bos411, 9428A410j 5/18/94 10:24:00";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: hash_key, hash_add, hash_find
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file contains code which implements a hash table search and
 * management facility.  The standard C library routines require the
 * size of the table be computed in advance.  This facility permits
 * a smaller table to be initially allocated and then overflow buckets
 * used as the table continues to grow.
 */

#define	HASHSIZE	4096
#define	HASHINCR	8
#define	HASHMASK	(HASHSIZE-1)

/*
 * Hash table entry.  Each entry either points directly to
 * a data value, or to an array of buckets.
 */

struct	hash_table {
	int	h_cnt;
	int	h_bucket;
	char	*h_key;
	char	*h_data;
};

/*
 * Hash bucket structure.  The bucket contains the hash key
 * and the data.
 */

struct	hash_bucket {
	char	*hb_key;
	char	*hb_data;
};

static	struct	hash_table *hash_table;

/*
 * NAME: hash_key
 *                                                                    
 * FUNCTION: convert the null terminated string into a hash index
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.
 *                                                                   
 * NOTES:
 *	Randomizes the bits in a character string very thoroughly.  Good
 *	randomization makes for a good hash key.
 *
 * RETURNS: hash key created from character string.
 */  

int
hash_key (key)
char	*key;		/* NUL terminated character string index            */
{
	unsigned short	hash; /* Hash value for key                         */

	for (hash = 0;*key;key++)
		hash = ((hash << 3) | (hash >> 13)) ^ *key;

	return hash & HASHMASK;
}

/*
 * NAME:	hash_add
 *                                                                    
 * FUNCTION:	Add hash key and associated data to hash table
 *                                                                    
 * NOTES:
 *	The data type of the data parameter is (char *), but the
 *	object being stored may be any type provided it is
 *	suitably cast.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS:	Zero on success, non-zero otherwise.
 */  

int
hash_add (key, data)
char	*key;		/* Character string index for object                */
char	*data;		/* Pointer to object to be stored                   */
{
	struct	hash_bucket *bucket; /* Hash bucket in case of collision    */
	struct	hash_table *table; /* Entry in hash table for this key      */

	/*
	 * Allocate the initial hash table if it doesn't exist already
	 */

	if (! hash_table) {
		hash_table = (struct hash_table *)
			malloc (HASHSIZE * sizeof (struct hash_table));

		if (! hash_table)
			return -1;

		memset (hash_table, 0, sizeof *hash_table * HASHSIZE);
	}

	/*
	 * Create the hash key and find the entry in the table to be
	 * used.
	 */

	table = hash_table + hash_key (key);

	/*
	 * If this is a new entry, initialize the count, and store the
	 * key and data in the table entry itself.
	 */

	if (table->h_cnt == 0) {
		table->h_cnt = 1;
		table->h_data = data;
		table->h_key = key;
		table->h_bucket = 0;
	} else if (table->h_cnt == 1) {

	/*
	 * If there is only one entry here, we copy the values in the
	 * table into a bucket, then copy the new values into a bucket
	 * store the two buckets in the table entry.  All of the
	 * appropriate entries in the table need to be initialized as
	 * well.
	 */

		table->h_cnt = 2;
		table->h_bucket = HASHINCR;

		bucket = (struct hash_bucket *)
			malloc (HASHINCR * sizeof (struct hash_bucket));

		if (! bucket) {
			perror ("malloc");
			return 0;
		}
		bucket[0].hb_data = table->h_data;
		bucket[0].hb_key = table->h_key;

		bucket[1].hb_data = data;
		bucket[1].hb_key = key;
		table->h_data = (char *) bucket;
	} else {

	/*
	 * With two or more elements in the bucket already, the bucket
	 * size needs to be checked and the bucked increased in size if
	 * needed.  The key and data are simply stored in the new
	 * bucket.
	 */

		table->h_cnt++;
		if (table->h_cnt > table->h_bucket) {
			table->h_bucket += HASHINCR;
			bucket = (struct hash_bucket *)
				realloc (table->h_data,
					table->h_bucket * sizeof *bucket);
			table->h_data = (char *) bucket;
			if (! bucket) {
				perror ("realloc");
				return -1;
			}
		}
		bucket = (struct hash_bucket *) table->h_data;

		bucket[table->h_cnt - 1].hb_data = data;
		bucket[table->h_cnt - 1].hb_key = key;
		table->h_data = (char *) bucket;
	}
	return 0;
}

/*
 * NAME: hash_find
 *                                                                    
 * FUNCTION: Search hash table for object and return associated data
 *                                                                    
 * NOTES:
 *	The return value must be suitably cast before using.
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.
 *                                                                   
 * RETURNS: Pointer to object, or NULL if not found.
 */  

char *
hash_find (key)
char	*key;		/* NUL terminated character string to search for   */
{
	int	i;		/* Loop counter for searching buckets      */
	struct	hash_table *table; /* Pointer to entry for hash key        */
	struct	hash_bucket *bucket; /* Hash bucket for multiple items     */

	/* 
	 * If there is no hash table, it can't be found!
	 */

	if (! hash_table)
		return 0;

	/*
	 * Hash the key and find the table containing the desired
	 * entry.
	 */

	table = hash_table + hash_key (key);

	/*
	 * No entries in that portion of the table, so the key can
	 * not exist.
	 */

	if (table->h_cnt == 0)
		return 0;

	/*
	 * If there is only one entry, compare the key against the
	 * key in the table.  If there is a match, return the data.
	 * Otherwise just return NULL.
	 */

	if (table->h_cnt == 1)
		if (strcmp (table->h_key, key) == 0)
			return table->h_data;
		else
			return 0;

	/*
	 * The h_data pointer points to an array of buckets.  Find
	 * the bucket array and loop through it comparing against
	 * the keys in each bucket.
	 */

	bucket = (struct hash_bucket *) table->h_data;

	for (i = 0;i < table->h_cnt;i++)
		if (strcmp (bucket[i].hb_key, key) == 0)
			return bucket[i].hb_data;

	return 0;
}

/*
 * NAME: hash_del
 *
 * FUNCTION: Delete the hash table entry associated with the key.
 *
 * NOTES:
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process.
 *
 * RETURNS: Function does not return a value.
 */

void
hash_del (key)
char    *key;           /* NUL terminated character string to search for   */
{
        int     i;              /* Loop counter for searching buckets      */
        struct  hash_table *table; /* Pointer to entry for hash key        */
        struct  hash_bucket *bucket; /* Hash bucket for multiple items     */

        /*
         * If there is no hash table, can't delete entry.
         */

        if (! hash_table)
                return;

        /*
         * Hash the key and find the table containing the desired
         * entry.
         */

        table = hash_table + hash_key (key);

        /*
         * No entries in that portion of the table, so nothing to delete.
         */

        if (table->h_cnt == 0)
                return;

        /*
         * If there is only one entry, compare the key against the
         * key in the table.  If there is a match, delete the entry.
         */

        if (table->h_cnt == 1) {
                if (strcmp (table->h_key, key) == 0)
                        table->h_cnt = 0;
                return;
        }

        /*
         * The h_data pointer points to an array of buckets.  Find
         * the bucket array and loop through it comparing against
         * the keys in each bucket.
         */

        bucket = (struct hash_bucket *) table->h_data;

        for (i = 0; i < table->h_cnt; i++) {
                if (strcmp (bucket[i].hb_key, key) == 0)
                        bucket[i] = bucket[--table->h_cnt];
                else
                        continue;

                if (table->h_cnt == 1) {
                        table->h_data = bucket[0].hb_data;
                        table->h_key = bucket[0].hb_key;
                        table->h_bucket = 0;
                        free (bucket);
                }
        }
}
