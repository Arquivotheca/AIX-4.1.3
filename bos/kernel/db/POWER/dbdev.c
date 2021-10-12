static char sccsid[] = "@(#)52	1.12  src/bos/kernel/db/POWER/dbdev.c, sysdb, bos411, 9435B411a 8/30/94 17:04:16";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *		drivers(ps) 
 *		devsw_inited ()
 *		find_closest_driver (address)
 *		get_deventry(slot, print)
 *		closest_entry (devswptr, address)
 *		get_addressname (slot, address)
 *		display_map(ps)
 *		is_same(string,addr)
 *		get_fa(func_desc)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include "pr_proc.h"			/* defines for Get_from_mem() */
#include "parse.h"			/* defines for Get_from_mem() */
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/xcoff.h>
#include "../ldr/ld_data.h"    /* This looks bad, but what is a better choice */

char display_messages;

extern struct devsw *devsw, *get_deventry();
extern char *getterm(), *get_addressname ();
unsigned long closest_entry ();

/*
 * NAME: 	drivers
 * FUNCTION: 	prints devsw table 
 * RETURNS:	0 always
 */
drivers(ps) 
struct parse_out *ps;
{
	register int i;
	int print_it=1;

	/*
		If there is a token (command line argument), then
		determine if token is a dev/switch slot, or an address.

		Otherwise, print every slot in the dev/switch table which
		is in use.

		Slots are decimals less than the number of dev/switch slots.
 		Addresses are valid hex constants greater than the number
		of dev/switch slots.
	*/


	if (! devsw_inited())
	{
		printf ("The devsw table has not been initialized yet...\n");
		return 0;
	}

	if(ps->num_tok >= 1)		/* is there a token? */
	{
		if ((ps -> token[1].tflags & DEC_VALID) &&
		    (ps -> token[1].dv < DEVCNT))
 		{
			print_it = 1;		/* is a slot, so print it */
			display_messages = TRUE;
			get_deventry(ps->token[1].dv, print_it);
			return 0;
		}
		else if ((ps -> token[1].tflags & HEX_VALID) &&
		         (ps -> token[1].hv >= DEVCNT))
		{
			print_it = 0;		/* is an address, dont print */
			find_closest_driver (ps -> token[1].hv);
		}
		else
		{
			printf ("Usage: dev [major device number: 0 - %u]\n",
				DEVCNT - 1);
			return 0;
		}

		return 0;
	}

	/* print every slot in the devsw table which is in use */
	display_messages = FALSE;
	for(i=0; i<DEVCNT; i++)
	{
		if(get_deventry(i,print_it))
			if(print_it)
			{
				/** Page Here **/
				if (debpg() == FALSE)
					return 0;
			}
	}
	return 0;
}

/*
 * NAME:	devsw_inited
 * FUNCTION:	Determines if the devsw table contains valid data
 * RETURNS:	1 if the devsw contains valid data, 0 otherwise
 *
 */

devsw_inited ()
{
	static struct devsw mydevsw;
	unsigned long addr;
	int slot;

	/* if any of the first 10 slots are valid, then assume table is ok */
	for (slot = 0; slot < 10; slot++)
	{
		addr = (unsigned long) (devsw + slot);

		if(!Get_from_memory(addr, VIRT, &mydevsw, sizeof(struct devsw)))
		{
			printf("devsw table paged out of memory!\n");
			return 0;	
		}

		if(DEV_DEFINED & mydevsw.d_opts)
			return 1;
	}

	return 0;
}

/*
 * NAME:	find_closest_driver
 * FUNCTION:	displays data on closest driver previous to passed address
 * RETURNS:	1 if a driver was found before address, 0 if none was found.
 *
 */

find_closest_driver (address)
unsigned long address;
{
	unsigned long	best_addr = 0;		/* best guess found so far */
	int		best_slot = 0;
	unsigned long	tmp;
	int		slot;
	char		*name;

	/* locate the dev/switch slot which is closest to the passed address */
	/* best_addr will be 0 if no driver was found. */
	display_messages = FALSE;
	for (slot = 0; slot < DEVCNT; slot++)
	{
		tmp = closest_entry (get_deventry (slot, 0), address);
		
		if ((tmp > best_addr) && (tmp <= address))
		{
			best_addr = tmp;
			best_slot = slot;
		}
	}

	if (best_addr == 0)
	{
		printf ("No drivers are located before %x\n", address);
		return (0);
	}
	else
	{
		get_deventry (best_slot, 1);	/* print dev/sw entry */
		name = get_addressname (best_slot, address);
		if (name != NULL)
		{
			printf ("Entry point: slot %d, %s\n", best_slot, name);
			return (1);
		}
		else
		{
			printf ("No entry point at %x\n", address);
			return (0);
		}
	}
}

/*
 * NAME: 	get_deventry
 * FUNCTION: 	looks at the devsw slot requested 
 * RETURNS:	pointer to 1 slot in the devsw[] table
 *		or NULL is slot not in use (and thus not printed).
 */
struct devsw *
get_deventry(slot, print)
int slot;	/* devsw slot to inspect */
int print;	/* true if we want this slot printed, and slot is in use */
{
	static struct devsw mydevsw;

	ulong addr;
	ulong get_fa();

	if (slot > DEVCNT)
	{
		printf ("invalid slot %d passed to get_deventry\n", slot);
		return NULL;
	}

	addr = (ulong) (devsw + slot);

	if(!Get_from_memory(addr, VIRT, &mydevsw, sizeof(struct devsw))) {
		if (display_messages)	
			printf("devsw table paged out of memory!\n");
		return NULL;	
	}
	
	if(!(DEV_DEFINED & mydevsw.d_opts))
	{
		if (print)
		{
			if (display_messages)
				printf ("Slot not used\n");
		}
		return NULL;	/* this slot not used */
	}

	if(!print)
		return &mydevsw;

	printf("MAJ#%03u       Open        Close       Read        Write\n",
		slot);
	printf("   func desc  0x%08x  0x%08x  0x%08x  0x%08x\n", mydevsw.d_open,
		mydevsw.d_close,mydevsw.d_read,mydevsw.d_write);
	printf("   func addr  0x%08x  0x%08x  0x%08x  0x%08x\n", 
		get_fa(mydevsw.d_open), get_fa(mydevsw.d_close),
		get_fa(mydevsw.d_read), get_fa(mydevsw.d_write));
	printf("              Ioctl       Strategy    Tty         Select\n");
	printf("   func desc  0x%08x  0x%08x  0x%08x  0x%08x\n", 
		mydevsw.d_ioctl,
		mydevsw.d_strategy, mydevsw.d_ttys, mydevsw.d_select);
	printf("   func addr  0x%08x  0x%08x              0x%08x\n", 
		get_fa(mydevsw.d_ioctl), get_fa(mydevsw.d_strategy),
		 get_fa(mydevsw.d_select));
	printf("              Config      Print       Dump        Mpx\n");
	printf("   func desc  0x%08x  0x%08x  0x%08x  0x%08x\n", 
		mydevsw.d_config,
		mydevsw.d_print, mydevsw.d_dump, mydevsw.d_mpx);
	printf("   func addr  0x%08x  0x%08x  0x%08x  0x%08x\n", 
		get_fa(mydevsw.d_config), get_fa(mydevsw.d_print),
		get_fa(mydevsw.d_dump), get_fa(mydevsw.d_mpx));
	printf("              Revoke      Dsdptr      Selptr      Opts\n");
	printf("   func desc  0x%08x  0x%08x  0x%08x  0x%08x\n", 
		mydevsw.d_revoke,
		mydevsw.d_dsdptr, mydevsw.d_selptr, mydevsw.d_opts);
	printf("   func addr  0x%08x \n", 
		get_fa(mydevsw.d_revoke));
	printf("\n");

	return &mydevsw;
}

/*
 * NAME: 	closest_entry
 * FUNCTION: 	finds closest entry point in passed devsw entry
 * RETURNS:	address of closest entry point, or NULL if no
 *		entry points were before passed address.
 *
 */

unsigned long
closest_entry (devswptr, address)
struct devsw *devswptr;
unsigned long address;
{
	int i;
	unsigned long best_addr = 0;
	unsigned long tmp_addr = 0;
	static struct devsw mydevsw;

	/* addresses of driver entry-points */
	static struct entry
	{
		ulong *entry;
		char *name;
	} entrypoint[] =
	{
		(ulong*) &mydevsw.d_open,	"Open",
		(ulong*) &mydevsw.d_close,	"Close",
		(ulong*) &mydevsw.d_read,	"Read",
		(ulong*) &mydevsw.d_write,	"Write",
		(ulong*) &mydevsw.d_ioctl,	"Ioctl",
		(ulong*) &mydevsw.d_strategy,	"Strategy",
		(ulong*) &mydevsw.d_ttys,	"Tty",
		(ulong*) &mydevsw.d_select,	"Select",
		(ulong*) &mydevsw.d_config,	"Config",
		(ulong*) &mydevsw.d_print,	"Print",
		(ulong*) &mydevsw.d_dump,	"Dump",
		(ulong*) &mydevsw.d_mpx,		"Mpx",
		(ulong*) &mydevsw.d_revoke,	"Revoke",
	};
	if (devswptr == NULL)
		return (0);

	/* load local copy of devsw table entry */
	for (i = 0; i < sizeof (struct entry); i++)
		((char *) &mydevsw)[i] = ((char *) devswptr)[i];

	for (i = 0; i < (sizeof (entrypoint) / sizeof (struct entry)); i++)
	{
		tmp_addr = get_fa (*(entrypoint[i].entry));

		if ((tmp_addr > best_addr) && (tmp_addr <= address))
		{
			best_addr = tmp_addr;
		}
	}
	return best_addr;
}

/*
 * NAME: 	get_address_name
 * FUNCTION: 	returns name of passed driver entry point
 * RETURNS:	pointer to name of passed driver entry point or NULL if
 *		an invalid address is passed in.
 *
 */

char *
get_addressname (slot, address)
int slot;			/* devsw slot to inspect */
unsigned long address;		/* address of entry point */
{
	static struct devsw mydevsw;
	ulong addr;
	ulong get_fa();
	unsigned long	best_addr = 0;
	char		*best_name = NULL;
	unsigned long	tmp_addr;
	int i;

	static struct entry
	{
		ulong *entry;
		char *name;
	} entrypoint[] =
	{
		(ulong *) &mydevsw.d_open,	"Open",
		(ulong *) &mydevsw.d_close,	"Close",
		(ulong *) &mydevsw.d_read,	"Read",
		(ulong *) &mydevsw.d_write,	"Write",
		(ulong *) &mydevsw.d_ioctl,	"Ioctl",
		(ulong *) &mydevsw.d_strategy,	"Strategy",
		(ulong *) &mydevsw.d_ttys,	"Tty",
		(ulong *) &mydevsw.d_select,	"Select",
		(ulong *) &mydevsw.d_config,	"Config",
		(ulong *) &mydevsw.d_print,	"Print",
		(ulong *) &mydevsw.d_dump,	"Dump",
		(ulong *) &mydevsw.d_mpx,		"Mpx",
		(ulong *) &mydevsw.d_revoke,	"Revoke",
	};

	addr = (ulong) (devsw + slot);

	if(!Get_from_memory(addr, VIRT, &mydevsw, sizeof(struct devsw))) {
		printf("devsw table paged out of memory!\n");
		return NULL;	
	}
	
	if(!(DEV_DEFINED & mydevsw.d_opts))
		return NULL;	/* this slot not used */

	for (i = 0; i < (sizeof (entrypoint) / sizeof (struct entry)); i++)
	{
		tmp_addr = get_fa (*entrypoint[i].entry);

		if ((tmp_addr > best_addr) && (tmp_addr <= address))
		{
			best_addr = tmp_addr;
			best_name = entrypoint[i].name;
		}
	}
	return best_name;
}	


/*
* NAME: display_map
*
* FUNCTION: this routine will call the function to display the kernel
*	loaded exports, kernel exports, system call exports
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

display_map(ps)
struct parse_out *ps;
{
#define BUFSIZ 30
	struct loader_entry	*le, l;
	struct loader_exports	lexp;
	struct exp_index	*in, *intmp,e;
	int			i,cnt,lines=1;
	int			print=0;	/* set to 1 to print whole map*/
	char			buf[BUFSIZ+1];	/* holds current sym name */
	char			symname[BUFSIZ+1]; /* current sym name match */
	char			*srchstr = (char *)NULL;/* sym name */
				/*if print==srchstr==null:searching by address*/
	int			exact_addr=0; /* true if exact address match */
	ulong			addr=0;	 /* addr of sym we're searching for */

	/* if the corresponding error messages have been issued, don't repeat */
	char			expidxerr = 0;		/* export index */
	char			symnameerr = 0;		/* symbol name */

	if(!Get_from_memory(&kernel_anchor.la_loadlist, VIRT, &le,
		sizeof(kernel_anchor.la_loadlist))){
		printf("Loadlist pointer not found\n");
		return 0;	
	    }
	if(le  == NULL) {
		printf("Loadlist not found\n");
		return 0;
	}
	if(ps->num_tok == 0)
		print++;
	else if(!(ps->token[1].tflags & HEX_VALID)) {
		srchstr = ps->token[1].sv;
		is_same(NULL,0); /* init this routines buffers */
	}

	do {
	    if(!Get_from_memory(le, VIRT, &l, sizeof(struct loader_entry))){
		printf("Trouble reading loadlist at address 0x%08x\n",
			le);
		return 0;	
	    }
	    le = &l;
	    if(print) {
		printf(
		"\nFile %s: text 0x%08x size 0x%08x, data 0x%08x size 0x%08x\n",
			le->le_filename,le->le_file,le->le_filesize,
			le->le_data,le->le_datasize);
		printf("Loadlist at 0x%08x has %u depends\n",le,le->le_ndepend);
		/* for(ledep=le->le_depend,i=0;i < le->le_ndepend; i++,ledep++)
			printf("0x%08x ", *ledep); */
		lines += 2;
	    }
	    if (le->le_exports == NULL)
		continue;
	    if(!Get_from_memory(le->le_exports,VIRT,&lexp,
		sizeof(struct loader_exports))){
		printf("Trouble reading loader exports at address 0x%08x\n",
			le->le_exports);
		return 0;	
	    }
	    in = lexp.indexes;
	    if(print) {
	    	printf("Number of exports: %u\n",lexp.numexports);
	        lines++;
	    }
	    for (i = 0; i < lexp.numexports; i++, in++) {
		cnt = 0;
		intmp = in;
		do {
			if(!Get_from_memory(intmp, VIRT, &e,
				sizeof (struct exp_index)))
			{
			   if (!expidxerr)
			   {
			      printf ("Trouble reading export index at 0x%08x\n",
					in);
			      expidxerr++;
			   }
			   break;
			}
			intmp = &e;
			if(intmp->sym == NULL)
				continue;
			if(!Get_from_memory(intmp->sym, VIRT, buf, 
				(intmp->syml<BUFSIZ)?intmp->syml:BUFSIZ))
			{
				if (! symnameerr)
				{
				  printf("Trouble reading sym name at 0x%08x\n",
					intmp->sym);
				   symnameerr++;
				}
				break;
			}
		     	buf[(intmp->syml<BUFSIZ)?intmp->syml:BUFSIZ]='\0';
			if(print){
				if(++cnt>3) {
					printf("\n    ");
					cnt = 0;
					lines++;
				}
		     		printf("%s:0x%08x    ",buf,intmp->exp_location);
			}
			else {	/* searching */
				if(srchstr) { 	/* searching to match string */
					if(!strcmp(srchstr,buf)) {
					    if(is_same(buf,intmp->exp_location))
						; /* do nothing */
					    else
		     				printf("%s:0x%08x\n",buf,
						intmp->exp_location);
					}
				}
				else {		/* searching for address */
				       if((ulong) intmp->exp_location<=ps->token[1].hv
					 && (ulong) intmp->exp_location>addr) {
						addr = (ulong) intmp->exp_location;
						strcpy(symname,buf);
						if (addr == ps->token[1].hv) {
						  printf("Exact match found: %s\n",symname);
						  exact_addr = 1;
						}
					}
				}
			}
		} while ((intmp = intmp->next_index) != NULL);
		if(print) {
			lines++;
			printf("\n");
		}
		if(lines>=16) {
			if (debpg() == FALSE)
			   	return(0);
			lines = 1;
		}
	    }
		if(lines>=16) {
			if (debpg() == FALSE)
				return(0);
			lines = 1;
		}
	} while ((le = le->le_next) != NULL);

	if(!print && !srchstr) { 	/* searching to match address */
		if(!addr)
			printf("No symbol found less than or equal to 0x%08x\n",
				ps->token[1].hv);
		else if (!exact_addr)
			printf("Closest symbol found: %s at 0x%08x\n",symname,
				addr);
	}

	return 0;
}

/* keep record of what is printed 
 * and return TRUE if already seen
 */
int
is_same(string,addr)
char *string;
ulong addr;
{
#define BUFMAX 5
#define LENMAX 20
	static char buf[BUFMAX][LENMAX];
	static ulong add[BUFMAX];
	int i;

	if((string==NULL) && (addr==0)) {
		for(i=0;i<BUFMAX;i++)
			buf[i][0] = '\0';
		return 1;
	}

	for(i=0;i<BUFMAX;i++) {
		if(buf[i][0]=='\0'){
			strncpy(buf[i],string,LENMAX);
			add[i] = addr;
			break;
		}
		if(!strncmp(buf[i],string,LENMAX) && addr==add[i])
			return TRUE;
	}
	return FALSE;
}


/*	take a func decriptor and 
 *	return the address of the function
 */
ulong
get_fa(func_desc)
ulong func_desc;
{
	ulong func_addr;

	if(!Get_from_memory(func_desc, VIRT, &func_addr, sizeof(func_addr))) 
		return -1;	

	return func_addr;
}
