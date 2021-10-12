static char sccsid[] = "@(#)50  1.7  src/bos/kernext/cat/cat_cdt.c, sysxcat, bos411, 9428A410j 2/22/94 16:50:37";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: cat_cdt_func(), cat_add_cdt(), cat_del_cdt()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define FNUM 1

#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/malloc.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <errno.h>

#include "catdd.h"

extern int caglobal_lock;	/* Global Lock Variable */
extern struct ca *caheader; /* List of adapter structures */
cdt_t catcdt;			/* Component Dump Table */


/*****************************************************************************
** NAME:	cat_cdt_func
**
** FUNCTION:	Returns a pointer to the driver's dump routine.
**
** EXECUTION ENVIRONMENT:	Can be called from the process environment only.
**
** NOTES:
**    Input:
**		nothing
**    Output:
**		pointer to the driver's dump routine
**    Called From:
**		kernel panic() code
**    Calls:
**		nothing
**
** RETURNS:	pointer to the driver's dump routine
**
*****************************************************************************/
cdt_t *
cat_cdt_func(int phase)
{
	struct ca *ca;
	ulong bus;
	int i;

	CATDEBUG(("cat_cdt_func()\n"));
	/*
	** Take a snapshot of the uCode's status
	** area, notification queue and command queue.
	*/
	if (phase == 1) {
		for (ca = caheader; ca; ca = ca->next) {
			bus = CAT_MEM_ATT;
			CAT_READ(bus, CDT_FIFO_OFFSET, ca->fifos_cdt, CDT_FIFO_SIZE);
			BUSMEM_DET(bus);
			/* ignore PIO error at this point... */
		}
	}

	return &catcdt;
} /* cat_cdt_func() */


/*****************************************************************************
** NAME:	cat_add_cdt
**
** FUNCTION: 	add an entry to the component dump table
**
** EXECUTION ENVIRONMENT:	Can be called from the process environment only.
**
** NOTES:
**    Input:
**		label, area and length to be dumped at panic time
**    Output:
**		nothing
**    Called From:
**		catconfig()
**		others ???
**    Calls:
**		nothing
**
** RETURNS:  nothing
**
*****************************************************************************/
void
cat_add_cdt(
	register char *name, 		/* label string for area dumped */
	register char *ptr, 		/* area to be dumped */
	register int len)		/* amount of data to be dumped */
{
	struct cdt_entry temp_entry;
	int	num_elems;

	CATDEBUG(("cat_add_cdt()\n"));

	strncpy(temp_entry.d_name, name, sizeof(temp_entry.d_name));
	temp_entry.d_len = len;
	temp_entry.d_ptr = ptr;
	temp_entry.d_xmemdp = 0;

	if( catcdt.header._cdt_len == 0 )
		catcdt.header._cdt_len = sizeof catcdt.header;

	num_elems = (catcdt.header._cdt_len - sizeof(catcdt.header)) / 
	    sizeof(struct cdt_entry );
	if (num_elems < MAX_CDT_ELEMS) {
		catcdt.entry[num_elems] = temp_entry;
		catcdt.header._cdt_len += sizeof(struct cdt_entry );
	}

	return;
} /* cat_add_cdt() */


/*****************************************************************************
** NAME:	cat_del_cdt
**
** FUNCTION: 	delete an entry from the component dump table
**
** EXECUTION ENVIRONMENT:	Can be called from the process environment only.
**
** NOTES:
**    Input:
**		label, area, and length to be removed from dump table
**    Output:
**		nothing
**    Called From:
**		catclose()
**		others ???
**    Calls:
**		bzero()
**
** RETURNS:  nothing
**
*****************************************************************************/
void
cat_del_cdt(
register char *name, 	/* label string for area dumped */
register char *ptr, 	/* area to be dumped */
register int len)		/* amount of data to be dumped */
{
	struct cdt_entry temp_entry;
	int	num_elems;
	int	ndx;

	CATDEBUG(("cat_del_cdt()\n"));

	strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
	temp_entry.d_len = len;
	temp_entry.d_ptr = ptr;
	temp_entry.d_xmemdp = 0;

	num_elems = (catcdt.header._cdt_len - sizeof(catcdt.header)) 
		/ sizeof(struct cdt_entry );

	/*
	** find the element in the array (match only the memory pointer)
	*/
	for (ndx = 0; (ndx < num_elems) && 
		(temp_entry.d_ptr != catcdt.entry[ndx].d_ptr); ndx++) {
		;
	}

	/*
	** re-pack the array to remove the element if it is there
	*/
	if (ndx < num_elems) {
		for (ndx++; ndx < num_elems; ndx++)
			catcdt.entry[ndx-1] = catcdt.entry[ndx];
		bzero(&catcdt.entry[ndx-1], sizeof(struct cdt_entry ));
		catcdt.header._cdt_len -= sizeof(struct cdt_entry );
	}

	return;
} /* cat_del_cdt() */
