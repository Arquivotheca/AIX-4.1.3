static char sccsid[] = "@(#)49  1.9  src/bos/kernel/db/POWER/dbtrace.c, sysdb, bos411, 9428A410j 4/11/94 07:12:09";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	trace_disp, disp_trchdr, disp_trcq, disp_channel,
 *		disp_hookid, disp_hooktype, fill_ebuffer, display_tracebuf
 *		read_entry, select_channel
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
  */

/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * LEVEL 2, 12 Years Bull Confidential Information
*/
#include <sys/types.h>
#include <sys/trchdr.h>
#include <sys/trcmacros.h>
#include "dbtrace.h"			/* magic # -> english keys      */
#include "pr_proc.h"			/* for Get_from_memory          */
#include "parse.h"                      /* parser structure             */
#include "vdberr.h"			/* Error message stuff		*/

#define	TRENTRIES	128		/* # trace entries to show	*/
#define RESTR_SIZE	5		/* # restrictions to allow	*/

/* macros for conversion of ascii to hex */
#define isdigit(c)      (((c>='0') && (c<='9')))
#define isxdigit(c)     ((((c>='0') && (c<='9')) || ((c>='a') && (c<='f')) || \
                          ((c>='A') && (c<='F'))))
#define tolower(c)      ((((c>='A') && (c<='Z')) ? c + ('a' - 'A') : c))

extern clrdsp();
extern char *getterm();
/*
extern struct trchdr trchdr[];
*/
extern struct trchdr *trchdr_ptr;

static ulong entries[TRENTRIES];
int trace_entrycount = 0;
int next_entry = 0; 
char *disp_hooktype ();
char *disp_hookid ();

/*
	defect 139984 : reading of the tid
*/
struct entry
{
	ulong hookword;			/* hookword, describes entry */
	ulong timestamp;		/* optional timestamp (check hw) */
	int   numargs;			/* number of words of trace data */
	ulong d[5];			/* trace data */
	ulong next;			/* ptr to next entry */
#ifdef _THREADS
	ulong tid;			/* tid: Thread Ident */
#endif
};

struct restrict
{
	ulong hook[RESTR_SIZE];		/* hook ids */
	ulong subhook[RESTR_SIZE];	/* optional subhook ids */
	ulong data[RESTR_SIZE];		/* trace data */
	char n_hook;
	char n_data;
	char valid_subhook[RESTR_SIZE];	/* flag indicating subhook is valid */
};

/*
 * NAME: trace_disp
 *
 * FUNCTION:  Display Kernel Trace Information 
 *
 *
 * NOTES:  This displays data in the kernel trace buffers.  Data is entered
 *	into these buffers via the shell command "trace", if the shell command
 *	has not been invoked prior to using the low-level debugger command
 *	then the trace buffers will be empty.
 *
 *	A search facility has been added to trace which allows specification of
 *	certain search criteria which will be used to restrict the set of
 *	displayed trace entries.  The command line to trace now has the form:
 *	   trace [-h] [hook[:subhook]]... [#data]... [-c channel]
 *
 *	-h displays the trace headers, which are trace driver data structures.
 *	-c selects the channel number from the command line, otherwise it will
 *	   be prompted for.
 *
 */
trace_disp(ps)
struct parse_out *ps;
{
	int i, val;
	char c;
	char *ptr;
	int channel;				/* channel to display */

	struct restrict criteria;		/* search criteria */
	int restricted;				/* number of restrictions */
	char found;				/* entry matching criteria */
	char show_header;			/* headers requested */
	char show_channel;			/* channel data requested */

	show_header = show_channel = 0;		/* init flags */
	criteria.n_hook = 0;			/* init counters */
	criteria.n_data = 0;

	for (i = 0; i < RESTR_SIZE; i++)
		criteria.valid_subhook[i] = 0;

	/*
		By default, trace prompts for and displays the data
		for one channel.
	*/

	if (ps -> num_tok == 0)
	{
		channel = select_channel ();
		show_channel++;
	}

	/* Re-parse the command line */

	for (i = 1; i <= ps -> num_tok; i++)
	{
		if (! strcmp (ps->token[i].sv, "-h"))
			show_header++;
		else if (! strcmp (ps->token[i].sv, "-c"))
		{
			show_channel++;

			if (i+1 > ps->num_tok)
			{
				printf ("missing operand\n");
				return (0);
			}

			if (ps->token[i+1].tflags & HEX_VALID)
				channel = ps->token[++i].hv;
			else
			{
			    vdbperr (ivd_text1, ps->token[i+1].sv, ivd_text2);
			    return (0);
			}
		}
		else if (ps -> token[i].sv[0] == '#')
		{
			val = 0;
			for (ptr = &ps->token[i].sv[1]; *ptr != '\0'; ptr++)
				if (isdigit (*ptr))
					val = 16*val + (*ptr - 0x30);
				else
					val = 16*val + (tolower (*ptr) - 0x57);
			criteria.data[criteria.n_data++] = val;
		}
		else
		{
			/* Save off the hook id */
			val = 0;
			for (ptr = ps->token[i].sv; (*ptr != '\0') &&
				(*ptr != ':'); ptr++)
			{
				if (isdigit (*ptr))
					val = 16*val + (*ptr - 0x30);
				else
					val = 16*val + (tolower (*ptr) - 0x57);
			}
			criteria.hook[criteria.n_hook++] = val;

			/* if an optional subhook id was used... */

			if (*ptr == ':')
			{
				val = 0;
				for (ptr++; *ptr != '\0'; ptr++)
				    if (isdigit (*ptr))
					val = 16*val + (*ptr - 0x30);
				    else
					val = 16*val + (tolower (*ptr) - 0x57);
				criteria.subhook[criteria.n_hook - 1] = val;
				criteria.valid_subhook[criteria.n_hook - 1] = 1;
			}
		}
	}

	if (show_header)
	{
		/* Display the trace headers */
		for (i = 0; i < TRC_NCHANNELS; i++)
		{
			disp_trchdr (i);
			if (debpg () == FALSE)
				break;
		}
	}

	if ((criteria.n_hook + criteria.n_data) &&
	    (! show_channel))
	{
		channel = select_channel ();
		show_channel++;
	}

	if (show_channel)
	{
		/* Display channel data */
		if ((channel < 0) || (channel > TRC_NCHANNELS))
		{
			printf ("Valid channels are: 0 - %d\n",
				TRC_NCHANNELS - 1);
			return (0);
		}
		disp_channel (channel, &criteria);
	}

	return (0);		/* don't leave debugger */
}

/*
 * NAME: select_channel
 *
 * FUNCTION: Prompt for and read in a trace channel number
 *
 * RETURNS: Channel number
 */

select_channel ()
{
	char selected = 0;
	char *ptr;
	int channel;

	while (! selected)
	{
		printf ("Trace channel[0 - %d]: ", TRC_NCHANNELS - 1);
		ptr = getterm ();
		channel = db_atoi (ptr);
		if ((channel >= 0) && (channel < TRC_NCHANNELS))
			selected++;
	}

	return (channel);
}

/*
 * NAME: disp_trchdr
 *
 * FUNCTION:  Display the trace header for the specified channel.
 *
 */

disp_trchdr (channel)
int channel;
{
	struct trchdr head;

	/* if (!Get_from_memory (&trchdr[channel], VIRT, &head, */
	/* Change from BULL */
	if (!Get_from_memory ((trchdr_ptr+channel), VIRT, &head,
		sizeof (struct trchdr)))
	{
		printf ("Trace header paged out\n");
		return;
	}

	printf ("Channel %d\n", channel);
	printf ("State: 0x%08x   MSR Save: 0x%08x  Lockword: 0x%08x\n",
		head.trc_state, head.trc_msr, head.trc_lockword);
	printf ("Sleep: 0x%08x   Mode:     0x%08x  FSM State: 0x%08x\n",
		head.trc_sleepword, head.trc_mode, head.trc_fsm);
	printf ("Channel: 0x%08x Wrap Cnt: 0x%08x\n",
		head.trc_channel, head.trc_wrapcount);
	printf ("Current Queue:\n");
	disp_trcq (&head.trc_currq);
	printf ("A Queue:\n");
	disp_trcq (&head.trc_Aq);
	printf ("B Queue:\n");
	disp_trcq (&head.trc_Bq);
}

/*
 * NAME: disp_trcq
 *
 * FUNCTION: Display the queue entry at the specified address
 *
 */

disp_trcq (addr)
struct trc_q *addr;
{
	struct trc_q q;

	if (!Get_from_memory (addr, VIRT, &q, sizeof (struct trc_q)))
	{
		printf ("Trace Queue header paged out\n");
		return;
	}

	printf ("   *Start:  0x%08x   *End: 0x%08x   *inptr: 0x%08x\n",
		q.q_start, q.q_end, q.q_inptr);
	printf ("   size: 0x%08x\n",q.q_size);
}

/*
 * NAME: disp_channel
 *
 * FUNCTION:  Display the contents of the specified channel, based on any
 *	      search criteria which was entered on the command line.
 *
 */

disp_channel (c, criteria)
int c;
struct restrict *criteria;
{
	int offset = 0;
	int type;
	int numtoshow;
	ulong address;
	int i;
	int index;
	struct entry entry;
	struct trchdr head;

	/* if (!Get_from_memory (&trchdr[c], VIRT, &head, */
	/* Change from BULL */
	if (!Get_from_memory ((trchdr_ptr+c), VIRT, &head,
		sizeof (struct trchdr)))
	{
		printf ("Trace header paged out\n");
		return;
	}

	fill_ebuffer (c, criteria);
	printf ("Trace Channel %d  (%d entries)\n", c, trace_entrycount);

	numtoshow = (trace_entrycount > TRENTRIES) ?
		TRENTRIES : trace_entrycount;

	for (i = numtoshow; i > 0; i--)
	{
		index = (next_entry - 1) - (numtoshow - i);
		if (index < 0)
			index += TRENTRIES;

		address = entries[index];
		read_entry (address, &entry);

		type = HKWDTOTYPE (entry.hookword);

		printf ("Current queue starts at 0x%08x and ends at 0x%08x\n",
			head.trc_start, head.trc_end);
		printf ("Current entry is #%d of %d at 0x%08x\n\n", i,
			numtoshow, address);

		printf ("   Hook ID: %s    Hook Type: %s\n",
			disp_hookid (HKWDTOHKID (entry.hookword)),
			disp_hooktype (HKWDTOTYPE (entry.hookword)));

/*
	defect 139984 : display the tid
*/
#ifdef _THREADS
		printf ("   ThreadIdent: 0x%08x\n", entry.tid);
#endif

#if 0
		/* Timestamps are useless in the debugger */
		if (ISTIMESTAMPED (entry.hookword))
			printf ("   Timestamp: 0x%08x\n", entry.timestamp);
		else
			printf ("\n");
#endif

		if (HKWDTOLEN(entry.hookword) != 0)
			printf ("   Subhook ID/HookData: 0x%04x\n",
				HKWDTOBLEN (entry.hookword));
		else
			printf ("   Data Length: 0x%04x bytes\n",
				HKWDTOBLEN (entry.hookword));

		for (index = 0; index < entry.numargs; index++)
			printf ("   D%d: 0x%08x\n", index, entry.d[index]);

		if ((type == HKTY_Vr) || (type == HKTY_VTr))
			printf ("   *Variable Length Buffer: 0x%08x\n",
				address + 2 * sizeof (ulong));

		if (i != 1)
		{
			if (debpg() == FALSE)
				break;
		}
		else
			printf ("\nEnd of Trace\n\n");
	}
}

/*
 * NAME: disp_hookid
 *
 * FUNCTION: Display a kernel trace hookword ID in ASCII
 *
 * RETURNS: NONE
 */

char *
disp_hookid (id)
int id;
{
	static buf[100];
	int i;
	int num_hookwords = sizeof (hkwd_key) / sizeof (hkwd_key[0]);

	for (i = 0; i < num_hookwords; i++)
	{
		if (id == HKWDTOHKID (hkwd_key[i].type))
		{
			sprintf (buf, "%s (0x%08x)",
				hkwd_key[i].typename, id);
			return ((char * ) buf);
		}
	}

	sprintf (buf, "Unknown (0x%08x)", id);
	return ((char *) buf);
}

/*
 * NAME: disp_hooktype
 *
 * FUNCTION: Display a kernel trace hook type in ASCII
 *
 * RETURNS:  NONE
 */

char *
disp_hooktype (type)
int type;
{
	static char buf[100];
	int i;
	int num_types = sizeof (hkty_key) / sizeof (hkty_key[0]);

	for (i = 0; i < num_types; i++)
	{
		if (type == HKWDTOTYPE (hkty_key[i].type))
		{
			sprintf (buf, "%s (0x%08x)", 
				hkty_key[i].typename, type);
			return (buf);
		}
	}

	sprintf (buf, "Unknown (0x%08x)", type);
	return (buf);
}

/*
 * NAME: fill_ebuffer
 *
 * FUNCTION: fill a table with pointers to the last TRENTRIES trace entries
 *
 * NOTES:
 *	In order to display the last TRENTRIES trace entries, a circular buffer
 *	is created and the address of the start of each trace entry is placed
 *	into the buffer.  When all the kernel trace buffers have been traversed
 *	this table will contain the last TRENTRIES entries.
 *
 * RETURNS:  NONE
 */

fill_ebuffer (c, criteria)
int c;					/* channel to fill from */
struct restrict *criteria;		/* search criteria */
{
	struct trchdr head;		/* trace header */

	trace_entrycount = 0;		/* number of trace entries seen */
	next_entry = 0;			/* location to store next entry */

	/* Load the trace header to get at the buffer pointers. */

	/* if (!Get_from_memory (&trchdr[c], VIRT, &head, */
	/* Change from BULL */
	if (!Get_from_memory ((trchdr_ptr+c), VIRT, &head,
		sizeof (struct trchdr)))
	{
		printf ("Trace header paged out\n");
		return;
	}

	/*
		The kernel trace facility uses two buffers which together
		are treated as a single circular buffer.  The two buffers,
		A and B, alternate logging trace events when the other
		becomes full.  Therefore in order to properly deal with
		this buffering scheme we need to traverse the OTHER buffer
		(filling our circular pointer table) before traversing the
		one currently used by the driver.  Then, in the event that
		the debuggers trace command is ran shortly after a wraparound
		of the kernel's trace buffer, the pointer table will be full.
	*/

	if (head.trc_start == head.trc_startA)
	{
		traverse_tracebuf (head.trc_startB, head.trc_inptrB, criteria);
		traverse_tracebuf (head.trc_startA, head.trc_inptr, criteria);
	}
	else
	{
		traverse_tracebuf (head.trc_startA, head.trc_inptrA, criteria);
		traverse_tracebuf (head.trc_startB, head.trc_inptr, criteria);
	}
}


/*
 * NAME:  traverse_tracebuf
 *
 * FUNCTION:  Enters a buffer's trace entries into a circular pointer table
 *
 * NOTES:
 *	Runs through one trace buffer and enters the address of each entry
 *	which matches the search criteria into a circular buffer.  Will be
 *	called for each of the buffers used by the kernel trace facility.
 *
 * RETURNS:  NONE
 */

traverse_tracebuf (address, buffer_end, criteria)
ulong address;
ulong buffer_end;
struct restrict *criteria;
{
	int type;
	struct entry entry;
	int restricted;				/* number of restrictions */
	int found, store;			/* flags */
	int x, y;				/* array indexes */

	restricted = criteria -> n_hook + criteria -> n_data;

	do
	{
		if (address >= buffer_end)
			return;

		if (next_entry == TRENTRIES)
			next_entry = 0;

		read_entry (address, &entry);

		if (restricted)
		{
			found = 0;
			store = 1;

			if (criteria -> n_hook)
			{
			    found = 0;
			    for (x = 0; x < criteria-> n_hook; x++)
			        if (HKWDTOHKID (entry.hookword) ==
				    criteria->hook[x])
					if (criteria->valid_subhook[x])
					{
					    if (criteria->subhook[x] ==
						(entry.hookword & 0xFFFF))
						    found++;
					}
					else
					    found++;
			    if (! found)
				store = 0;
			}

			/*
				This should handle variable length data
				as well.  Someday...
			 */

			if (criteria -> n_data)
			{
				found = 0;
				for (x = 0; x < criteria -> n_data; x++)
					for (y = 0; y < entry.numargs; y++)
						if (entry.d[y] ==
						    criteria -> data[x])
							found++;
				if (! found)
					store = 0;
			}

			if (store)
			{
				/* Place entry in buffer */
				entries[next_entry++] = address;
				trace_entrycount++;
			}
		}
		else
		{
			/* Place entry in buffer */
			entries[next_entry++] = address;
			trace_entrycount++;
		}

		address = entry.next;

		type = HKWDTOTYPE (entry.hookword);

		if ((type < 0) || (type > HKWDTOTYPE(HKTY_LAST)))
		{
			printf ("Bad type (0x%x) -- trace terminated\n", type);
			return;
		}
	} while ((type >= 0) && (type <= HKWDTOTYPE(HKTY_LAST)));
}

/*
 * NAME:  read_entry
 *
 * FUNCTION:  Reads a trace entry in from one of the trace buffers
 *
 * NOTES:  This is a "safe" means of accessing the trace buffers so that
 *	in the future should the trace buffers become unpinned we will not
 *	have to worry about accessing unpinned memory (and possible page faults)
 *
 * RETURNS: NONE
 */

read_entry (address, entry)
ulong address;
struct entry *entry;
{
	int type;
	int offset;

	entry -> numargs = 0;
	if (!Get_from_memory (address, VIRT, &entry -> hookword,
		sizeof (ulong)))
	{
		printf ("Trace entry paged out\n");
		return;
	}
	type = HKWDTOTYPE (entry -> hookword);

	if ((type < 0) || (type > HKWDTOTYPE (HKTY_LAST)))
		return;

	if (HKWDTOLEN(entry -> hookword) != 0)
		offset = HKWDTOLEN(entry -> hookword) * sizeof (ulong);
	else
	{
		/*
			Variable length trace entry:
				hookword + 1 word of data + variable length
				buffer
		 */

		offset = 2 * sizeof (ulong) +
			HKWDTOWLEN (entry -> hookword) * sizeof (ulong);
	}

	/*
		Possibly timestamped (add an extra word)
	 */
	if (ISTIMESTAMPED (entry -> hookword))
		offset += sizeof (ulong);
/*
	defect 139984 : reading of the tid
*/
#ifdef _THREADS
	offset += sizeof (ulong);


	if (ISTIMESTAMPED (entry -> hookword)) {
		if (!Get_from_memory ((int) address + offset - sizeof (ulong),
			VIRT, &entry -> timestamp, sizeof (ulong)))
		{
			printf ("Timestamp paged out\n");
			return;
		}
		if (!Get_from_memory ((int) address + offset - 2*sizeof (ulong),
			VIRT, &entry -> tid, sizeof (ulong)))
		{
			printf ("Thread Ident paged out\n");
			return;
		}
	}
	else
		if (!Get_from_memory ((int) address + offset - sizeof (ulong),
			VIRT, &entry -> tid, sizeof (ulong)))
		{
			printf ("Thread Ident paged out\n");
			return;
		}

#else
	if (ISTIMESTAMPED (entry -> hookword))
		if (!Get_from_memory ((int) address + offset - sizeof (ulong),
			VIRT, &entry -> timestamp, sizeof (ulong)))
		{
			printf ("Timestamp paged out\n");
			return;
		}
#endif
	/*
		Extract event data fields.  These begin at the second longword
		of the event.
	*/
	switch (type)
	{
		case HKTY_Lr:
		case HKTY_LTr:
		case HKTY_Vr:
		case HKTY_VTr:
			if (!Get_from_memory((int) address + 1 * sizeof (ulong),
				VIRT, &entry -> d[0], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}

			entry -> numargs = 1;
			break;
		case HKTY_Gr:
		case HKTY_GTr:
			if (!Get_from_memory((int) address + 1 * sizeof (ulong),
				VIRT, &entry -> d[0], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}
			if (!Get_from_memory((int) address + 2 * sizeof (ulong),
				VIRT, &entry -> d[1], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}
			if (!Get_from_memory((int) address + 3 * sizeof (ulong),
				VIRT, &entry -> d[2], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}
			if (!Get_from_memory((int) address + 4 * sizeof (ulong),
				VIRT, &entry -> d[3], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}
			if (!Get_from_memory((int) address + 5 * sizeof (ulong),
				VIRT, &entry -> d[4], sizeof (ulong)))
			{
				printf ("Trace data paged out\n");
				return;
			}
			entry -> numargs = 5;
			break;
		default:
			entry -> numargs = 0;
			break;
	}

	entry -> next = (int) address + offset;
}
