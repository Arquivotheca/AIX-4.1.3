#ifndef lint
static char sccsid[] = "@(#)59 1.1 src/bos/kernext/tty/ttydbg.c, sysxtty, bos411, 9428A410j 3/11/94 09:20:28";
#endif
/*
 *
 * COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
 *
 * FUNCTIONS:	tty_db_config, tty_db_register, tty_db_unregister, tty_db_open
 *		tty_db_close, tty_dmp, tty_print,
 *		tty_read_mem, tty_read_word, tty_db_init, tty_db_del,
 *		tty_db_modulelistadd, tty_db_ttylistadd
 *		tty_db_is_per_tty, tty_db_pmadd, tty_db_modptr, tty_db_modfind
 *		tty_db_cdt_ttyadd, tty_db_cdt_ttydel, tty_db_cdt_modadd,
 *		tty_db_cdt_moddel.
 *		tty_db_usage, tty_db_parse_line, tty_db_do_command,
 *		tty_db_disp_tty,
 *		tty_db_kdb_is_avail.
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/lockl.h>
#include "ttydbg.h"

/*
 * Functions declared in that file.
 */
/* Externals */
int		tty_db_config();
int		tty_db_register();
int		tty_db_unregister();
int		tty_db_open();
int		tty_db_close();

struct cdt_head	*tty_dmp();
void		tty_print();
int		tty_read_mem();
unsigned int	tty_read_word();

/* Internals */
static int	tty_db_init();
static int	tty_db_del();

static int	tty_db_modulelistadd();
static int	tty_db_ttylistadd();
static struct	per_tty *tty_db_is_per_tty();

static int	tty_db_pmadd();
static int	tty_db_modptr();
static int	tty_db_modfind();

static void	tty_db_cdt_ttyadd();
static void	tty_db_cdt_ttydel();
static void	tty_db_cdt_modadd();
static void	tty_db_cdt_moddel();

static void	tty_db_usage();
static int	tty_db_parse_line();
static int	tty_db_do_command();
static int	tty_db_disp_tty();

static int	tty_db_kdb_is_avail();

/*
 *	General tables : informations needed for dump, debug.
 */
struct	str_module_s *module_list;	/* general modules and drivers list */

struct	all_tty_s	*tty_list;	/* general tty list		    */

/*
 * declarations for configuration routine.
 */
static int ttydbg_count = 0;	/* config methods loads count		*/

lock_t ttydbg_conf_lock = LOCK_AVAIL; /* extension configuration lock.	*/

/*
 * Component dump table, declarations.
 */
struct	all_tty_s	**ttydbg_list, **ttydbg_list_end;
struct	str_module_s	**mod_list, **mod_list_end;

/* We can not use the cdt structure name of dump.h because it defines	*/
/* only 1 entry in the component dump table entries item cdt_entry[]	*/
#ifndef	DEBUG_TTYDBG
static
#endif	/* DEBUG_TTYDBG */
struct {
	struct cdt_head cdt_head;
	struct cdt_entry cdt_entry[4];
} str_tty_dmp = {
	{ DMP_MAGIC, "tty", sizeof(str_tty_dmp) },
	{
		{"tty_list", sizeof(ttydbg_list), &ttydbg_list, 0},
		{"tty_end", sizeof(ttydbg_list_end), &ttydbg_list_end, 0},
		{"mod_list", sizeof(mod_list), &mod_list, 0},
		{"mod_end", sizeof(mod_list_end), &mod_list_end, 0}
	}
  };

/*
 * lldb tty subcomand
 */
#define	VIRT	1

#define	VALC(c)		((c)>=0&&(c)<=256)
#define ISDIGIT(c)	(((c>='0') && (c<='9')))
#define ISALPHA(c)	(((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')))
#define ISXDIGIT(c)	((((c>='0') && (c<='9')) || ((c>='a') && (c<='f')) || \
			 ((c>='A') && (c<='F'))))
#define TOLOWER(c)	((((c>='A') && (c<='Z')) ? c + ('a' - 'A') : c))
#define	isdigit(c)	(VALC(c) ? ISDIGIT(c) : 0)
#define	isalpha(c)	(VALC(c) ? ISALPHA(c) : 0)

#define	TTY_SUBCOM	"tty"

extern void (* db_tty_dump_ptr)();

#ifdef	DEBUG_TTYDBG
char *name;
int maj;
int min;
int chan;

int vflag;
int oflag;
int dflag;
int lflag;
int eflag;
#else	/* DEBUG_TTYDBG */
static char *name;
static int maj;
static int min;
static int chan;

static int vflag;
static int oflag;
static int dflag;
static int lflag;
static int eflag;
#endif	/* DEBUG_TTYDBG */

/*
 * Kdb extended I/F used by kernext display modules
 * For db_printf, 1st parameter is the print format
 */
#define DB_READ_MEM  1
#define DB_READ_WORD 2
#define DB_READ_BYTE 3

#define db_printf                  brkpoint
#define db_read_mem(vad, lad, siz) brkpoint(DB_READ_MEM, vad, lad, siz)
#define db_read_word(vad)          brkpoint(DB_READ_WORD, vad)
#define db_read_byte(vad)          brkpoint(DB_READ_BYTE, vad)

#define STATIC_BREAK_TRAP	0x7C810808

/************************************************************************/
/************************************************************************/
/*	Routines called by external modules.                            */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_db_config()
 *
 * PARAMETERS:	-cmd is an int sent for configuration or unconfiguration.
 *		-uiop is a pointer on a uio structure sent by the caller.
 *
 * FUNCTION:	pinned or unpinned as necessary. For Configuration Init verifies
 *		that it is the first configurion initialized, if not do nothing.
 *		If it is the first, then allocate memory for the general module
 *		list and add the new tty dump function in the list maintained
 *		by the system.
 *		At unconfiguration time deletes the dump function, deallocate
 *		the general module list and unpins code.
 *
 * RETURNS:	EINVAL if the command is false.
 *		Any other error sent on pincode() or tty_db_init() calls.
 */
int
tty_db_config(int cmd, struct uio *uiop)
{
	int	error = 0;
	int	locked;
	
	locked = lockl(&ttydbg_conf_lock, LOCK_SHORT);
	switch (cmd) {
	case CFG_INIT:
		/*
		 * The work is to do just the first time we are called.
		 * Otherwise don't do anything.
		 */
		if (ttydbg_count == 0) {
			if ((error = pincode(tty_db_config)) != 0)
				break;
			/*
			 * pinned ok, allocate the module list.
			 * and allocate the tty_list.
			 */
			if ((error = tty_db_init()) != 0) {
				unpincode(tty_db_config);
				break;
			}
			/*
			 * Updates the component dump table str_tty_dmp.
			 * Initializes all values to 0 before the first call
			 * to the component dump table updates.
			 */
			ttydbg_list = ttydbg_list_end = 0;
			mod_list = mod_list_end = 0;
			tty_db_cdt_ttyadd(tty_list);
			tty_db_cdt_modadd(module_list);
			/*
			 * The component dum table is updated, the tty table and
			 * module table are allocated in memory, so we can add
			 * the dump routine for the system, and the number of
			 * configuration init is incremented.
			 */
			dmp_add(tty_dmp);
			++ttydbg_count;
			/* set the pointer to the print routine for lldb */
			db_tty_dump_ptr = tty_print;
		}
		break;	/* nothing to do */
	case CFG_TERM:
		/*
		 * Unload only if the count is 1.
		 * Otherwise nothing to do.
		 */
		if (ttydbg_count == 1) {
			/*
			 * Del the tty dump entry from the system.
			 */
			dmp_del(tty_dmp);
			/*
			 * Updates the values for component dump table.
			 * it is not important because tty_dmp couldn't be
			 * called now.
			 */
			tty_db_cdt_ttydel(tty_list);
			tty_db_cdt_moddel(module_list);
			/*
			 * Delete the module_list and the tty_list.
			 * The count is decremented, and the code unpinned.
			 */
			tty_db_del();
			--ttydbg_count;
			/* unset the pointer to the print routine for lldb */
			db_tty_dump_ptr = NULL;

			unpincode(tty_db_config);
		}
		break;
	default:
		error = EINVAL;
	}
	if (locked != LOCK_NEST)
		unlockl(&ttydbg_conf_lock);
	return(error);
}

/*
 * NAME:	tty_db_register()
 *
 * PARAMETERS:	p is a pointer to the str_module_conf structure to add in the 
 *		modules table.
 *
 * FUNCTION:	registers the str_module_conf structure sent by the modules or
 *		drivers in a general modules table.
 *		Called by the tty modules and drivers at their first 
 *		configuration time.
 *
 * RETURNS:	EINVAL if the table of modules has not been allocated by config.
 *		Otherwise all return codes sets by tty_db_modulelistadd().
 */
int
tty_db_register(p)
	struct	str_module_conf *p;
{
	if (!module_list)
		return(EINVAL);
	/*
	 * add this module in the general table if needed.
	 */
	return(tty_db_modulelistadd(p));
}

/*
 * NAME:	tty_db_unregister()
 *
 * PARAMETERS:	-p is the pointer to str_module_conf structure to delete
 *
 * FUNCTION:	deallocate the str_module_conf associated to this module from
 *		the general list of modules.
 *		Must be called by module or driver at unconfiguration time.
 *
 * RETURNS:	-1 if there is no general list module.
 *		EINVAL if the module is not in the list.
 *		0 if succeeded.
 */
int
tty_db_unregister(p)
	register struct str_module_conf *p;
{
	register struct str_module_s *ref, *last_ref;
	int	i, j, free_cnt, find;

	if (!module_list)
		return(-1);
	if (p->name[0] == '\0')
		return(EINVAL);
	find = 0;
	ref = last_ref = module_list;

	while (ref) {
		for (i = 0; i < STR_MODULE_CNT; i++) {
			if (ref->lists[i].name[0] == '\0')
				continue;		/* skip the hole */
			if (!(strncmp(ref->lists[i].name, p->name, FMNAMESZ))) {
				/*
				 * we have found the module in the table
				 */
				find++;
				bzero(&(ref->lists[i]), sizeof(struct 
							str_module_conf));
				if (ref != module_list) {
					/*
					 * Verifies if there is a table to free.
					 */
					free_cnt = 0;
					for (j = 0; j < STR_MODULE_CNT; j++)
						if (ref->lists[j].name[0] == '\0')
							free_cnt++;
					if (free_cnt == STR_MODULE_CNT) {
						/*
						 * The table is empty so frees
						 * the table and updates the
						 * corresponding fields in the
						 * component dump table.
						 */
						last_ref->next = ref->next;
						tty_db_cdt_moddel(ref);
						xmfree(ref, pinned_heap);
					}
				}
			}
		}
		last_ref = ref;
		ref = ref->next;
	}
	if (!find)
		return(EINVAL);
	return(0);
}

/*
 * NAME:	tty_db_open()
 *
 * PARAMETERS:	-ptr is a pointer to the per_tty structure to add in the list.
 *
 * FUNCTION:	for the first call we allocate the general list that will
 *		contained all ttys liste.
 *		Called by each tty module/driver first open to registered
 *		the per_tty structure sent.
 * 
 * RETURNS:	ENOMEM if not enough memory.
 *		Otherwise all return code sets by tty_db_ttylistadd().
 */
int
tty_db_open(p)
	register struct tty_to_reg *p;
{
	if (!tty_list) {
		/*
		 * That is the first open.
		 */
		if (!(tty_list = (struct all_tty_s *)xmalloc(PAGESIZE, (uint)0,
							pinned_heap)))
			return(ENOMEM);
		bzero(tty_list, PAGESIZE);
		tty_db_cdt_ttyadd(tty_list);
	}
	/*
	 * The tty table exists, so add the tty_to_reg structure in the table.
	 */
	return(tty_db_ttylistadd(p));
}
	
/*
 * NAME: tty_db_close()
 *
 * PARAMETERS:	-ptr is a pointer to the per_tty structure to delete.
 *
 * FUNCTION:	only called on the last close by the module and drivers.
 *		Unregisters one by one the pushed_modules structure from
 *		the per_tty structure, and finally for the last one deallocate
 *		all the per_ty structure from the list as nobody has anything
 *		to do with that tty now.
 *
 * RETURNS:	-1 if there is not general tty list.
 *		EINVAL if the module or driver is not in the list.
 *		0 if succeeded.
 */
int
tty_db_close(p)
	register struct tty_to_reg *p;
{
	register struct all_tty_s *ref, *last_ref;
	register struct per_tty *find;
	register int i, j, found, count, is_driver;

	if (!tty_list)
		return(-1);
	if (p->name[0] == '\0')
		return(EINVAL);

	found = count = is_driver = 0;
	ref = last_ref = tty_list;

	while (ref) {
		for (i = 0; i < STR_TTY_CNT; i++) {
			if (ref->lists[i].dev == p->dev) {
				/*
				 * The corresponding per_tty structure is
				 * found, look at if it is a driver.
				 */
				find = &(ref->lists[i]);
				found = 1;
				/* A driver hold the entry 0 in the mod_info
				 * item table of a per_tty structure.
				 */
				if (!strncmp(find->mod_info[0].mod_ptr->name, 
							p->name, FMNAMESZ))
					is_driver = 1;
				break;
			}
		}
		if (found == 1)
			break;
		last_ref = ref;
		ref = ref->next;
	}
	if (found == 0)		/* there is no per_tty structure in the table */
		return(EINVAL);	/* for that device.			      */
	else {
		if (is_driver == 1) {
			/*
			 * In a driver close case, the per_tty structure in the
			 * table can be deleted.
			 */
			bzero(find, sizeof(struct per_tty));
			/*
			 * Search how many per_tty structures are empty in that
			 * table if this is not the first tty table.
			 */
			if (ref != tty_list) {
				for (i = 0; i < STR_TTY_CNT; i++)
					if (ref->lists[i].ttyname[0] == '\0')
						count++;
				/*
				 * If all the table is empty, then frees it and
				 * updates the last_ref->next pointer to the
				 * next table if any, and updates the 
				 * corresponding pointers in the component dump
				 * table.
				 */
				if (count == STR_TTY_CNT) {
					last_ref->next = ref->next;
					tty_db_cdt_ttydel(ref);
					xmfree(ref, pinned_heap);
				}
			}
			return(0);
		} else {
			/*
			 * module close. Search the corresponding module 
			 * informations in the per_tty structure found and
			 * zeroed the fields.
			 */
			for (j = 1; j < MODULEMAX; j++) {
				if (find->mod_info[j].mod_ptr == NULL)
					continue;	/* skip the holes */
				if (!strncmp(find->mod_info[j].mod_ptr->name,
							p->name, FMNAMESZ)) {
					/*
					 * This is the good one.
					 */
					find->mod_info[j].mod_ptr = NULL;
					find->mod_info[j].private_data = NULL;
					return(0);
				}
			}
		}
		return(EINVAL);
	}		
}

/*
 * NAME:        tty_dmp()
 *
 * PARAMETERS:  no parameters.
 *
 * FUNCTION:    call by system dump, the functionnality is to search about
 *              all opened tty and to call all existing modules and drivers
 *              dump function pushed for that tty with the good pointer to the
 *              the good structure to dump in parameter.
 *
 * RETURNS:     always 0.
 */
struct cdt_head *
tty_dmp() 
{
        return(&str_tty_dmp);
}

/*
 * NAME:	tty_print()
 *
 * PARAMETERS:	the buffer from the command.
 *
 * FUNCTION:	lldb tty sub-command.
 *
 * RETURNS: 	none
 */
void
tty_print(char *argv)
{
	label_t tty_db_debug;

	if (!tty_db_parse_line((argv + strlen(TTY_SUBCOM))))
		return;

	if (setjmpx(&tty_db_debug))
		return;
	if (tty_db_do_command())
		return;
	clrjmpx(&tty_db_debug);
}

/*
 * NAME:	tty_read_mem()
 * 
 * PARAMETERS:	-addr is the address that the caller wants wants to read from
 *		-buffer is the user buffer used to collect the informations
 *		-size is the size to read in memory
 *
 * FUNCTION:	read in memory from addr to buffer size bytes
 *
 * RETURNS:	0  in case of success
 *		-1 if the read or copy failed
 */

int
tty_read_mem(void *addr, void *buffer, unsigned int size)
{
	int kdb_return = 0;

#ifdef DEBUG_TTYDBG
	if (tty_db_kdb_is_avail()) {
		kdb_return = db_read_mem(addr, buffer, size);
		if (kdb_return != size) {
			return(-1);
		} else {
			return(0);
		}
	}
	else {
		if (get_from_memory(addr, VIRT, buffer, size)) {
			return(0);
		} else {
			longjmpx();
		}
	}
#else	/* DEBUG_TTYDBG */
#ifdef _KDB
	kdb_return = db_read_mem(addr, buffer, size);
	if (kdb_return != size) {
		return(-1);
	} else {
		return(0);
	}
#else	/* _KDB */
	if (get_from_memory(addr, VIRT, buffer, size)) {
		return(0);
	} else {
		longjmpx();
	}
#endif	/* _KDB */
#endif	/* DEBUG_TTYDBG */
}


/*
 * NAME:	tty_read_word()
 * 
 * PARAMETERS:	-addr is the address to read from
 *
 * FUNCTION:	read 4 bytes from addr
 *
 * RETURNS:	4 byes readden
 */

unsigned int
tty_read_word(void *addr)
{
	int intermed;

#ifdef DEBUG_TTYDBG
	if (tty_db_kdb_is_avail()) {
		return(db_read_word(addr));
	}
	else {
		if (get_from_memory(addr, VIRT, &intermed, sizeof(intermed))) {
			return(intermed);
		} else {
			longjmpx();
		}
	}
#else	/* DEBUG_TTYDBG */
#ifdef	_KDB
	return(db_read_word(addr));
#else	/* _KDB */
	if (get_from_memory(addr, VIRT, &intermed, sizeof(intermed))) {
		return(intermed);
	} else {
		longjmpx();
	}
#endif	/* _KDB */
#endif	/* DEBUG_TTYDBG */
}

/************************************************************************/
/************************************************************************/
/*	Internal Routines.                                              */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_db_init()
 *
 * CALL:	internal to that module at configuration time.
 *
 * FUNCTION:	to allocate a table for 16 modules which one will be filled by
 *		each driver and module by the call to tty_db_register at their
 *		own configuration time, and to allocate a list of 53 ttys.
 *
 * RETURNS:	EINVAL the table of modules or the tty table is still existing.
 *		ENOMEM	not enough place in memory to allocate the module table
 *			or the tty table.
 *		0	allocations are succesfull.
 */
static int
tty_db_init()
{
	int	i, j;

	if (module_list || tty_list)
		return(EINVAL);
	else {
                /*
                 * It is the first time that tty_db_register is called.
                 */
                if (module_list = (struct str_module_s *)xmalloc(sizeof(
                                 struct str_module_s) , (uint)0, pinned_heap)) {
			bzero(module_list, sizeof(struct str_module_s));
                } else
                        return(ENOMEM); /* not enough place in memory   */
		if (tty_list = (struct all_tty_s *)xmalloc(PAGESIZE, (uint)0,
				 pinned_heap)) {
			bzero(tty_list, PAGESIZE);
		} else
			return(ENOMEM); /* not enough place in memory	*/
        }
	return(0);
}

/*
 * NAME:	tty_db_del()
 *
 * PARAMETERS:	no
 *
 * FUNCTION:	dealloc all that needs to be deallocate. All tty tables and the  *		all modules tables.
 *		Internal to this module at unconfig time.
 *
 * RETURNS:	For internal needs only.
 *		-1 if there is no modules table nor ttys table.
 */
static int
tty_db_del()
{
	int	i;
	register struct str_module_s *mod_to_free;
	register struct all_tty_s *tty_to_free;
	int	error = 1;

	if (!module_list)
		error = -1;
	else {
		while (mod_to_free = module_list->next) {
			/*
		 	 * Reorganizes the chain and frees memory for the next
		 	 * retrieved.
		 	 */
			module_list->next = mod_to_free->next;
			xmfree(mod_to_free, pinned_heap);
		}
		/*
	 	 * Now all modules and drivers are deleted from the list, so it 
		 * is time to deallocate the first list itself.
	 	 */
        	xmfree(module_list, pinned_heap);
	}

	if (!tty_list)
		error = -1;
	else {
		while (tty_to_free = tty_list->next) {
			/*
		 	 * Reorganizes the chain and frees memory for the next 
			 * tty list retrieved.
		 	 */
			tty_list->next = tty_to_free->next;
			xmfree(tty_to_free, pinned_heap);
		}
		/*
	 	 * Now, all ttys are deleted from the table, so deallocates the
	 	 * first table itself.
	 	 */
		xmfree(tty_list, pinned_heap);
	}
	return(error);
}

/*
 * NAME:	tty_db_modulelistadd()
 *
 * CALL:	internal to this module only.
 *
 * FUNCTION:	add the module informations in the general list of module, if
 *		this module/driver is not yet registered. Alloc memory as
 *		necessary.
 *
 * RETURN:	EEXIST if the module or driver is in the list yet.
 *		ENOMEM if there is not enough place to install the module
 *		       or driver in the list.
 *		0 if success.
 */
static int
tty_db_modulelistadd(p)
	register struct str_module_conf *p;
{
	register struct str_module_s *ref;
	register struct str_module_conf *first_free = NULL;
	int i, find;

	find = 0;
	if (!module_list)
		return(EINVAL);
	if (p->name[0] == '\0')
		return(EINVAL);
	ref = module_list;

	while (ref) {
		for (i = 0; i < STR_MODULE_CNT; i++) {
			if (ref->lists[i].name[0] == '\0') {
				if (!find) {
					/* 
					 * we have found the first free place in
					 * the table, so registered the location
					 */
					first_free = &(ref->lists[i]);
					find = 1;
				}
			} else if (!strncmp(ref->lists[i].name, p->name,
								FMNAMESZ)) {
				/* the module/driver exists in the table */
				return(EEXIST);
			}
		}
	/*
		if (find == 1)
			break;
	*/
		ref = ref->next;
	}
	if (!first_free) {
		/*
		 * There is no place left in all the existing tables, so
		 * tries to allocate in memory a new table.
		 */
		if (!(ref->next = (struct str_module_s *)xmalloc(sizeof(struct
								str_module_s),
								(uint)0,
								pinned_heap)))
			return(ENOMEM);
		bzero(ref->next, sizeof(struct str_module_s));
		first_free = &(ref->next->lists[0]);
		/*
		 * Updates the corresponding fields of the component dump table.
		 */
		tty_db_cdt_modadd(ref->next);
	}
	/*
	 * Copies the structure received in the table.
	 */
	bcopy(p, first_free, sizeof(struct str_module_conf));

	return(0);
}

/*
 * NAME:	tty_db_ttylistadd()
 *
 * CALL:	internal function to this module.
 *
 * FUNCTION:	adds a new opened tty in te list of opened tty.
 *
 * RETURN:	EINVAL if exists a per-tty structure without name or there is
 *		no corresponding per_tty structure for the module for that dev.
 *		ENOMEM if there is not enough memory.
 *		0 if success.
 *		Otherwise all return code sets by tty_db_pmadd().
 */
static int
tty_db_ttylistadd(p)
	register struct tty_to_reg *p;
{
	register struct per_tty *first;
	struct str_module_conf *mod_confp;
	int error;

	if ((error = tty_db_modfind(p->name, &mod_confp)) != 0)
		return(error);

	/*
	 * In tty streams, only the driver is used to know the tty name
	 */
	if ((p->ttyname[0] == 0) || (mod_confp->type != 'd')) {
		/*
		 * This is a module open, so there must be a corresponding
		 * per_tty structure in the table. Retrieves it and add the
		 * module informations in the table for the tty.
		 */
		if (first = tty_db_is_per_tty(p->dev))
			return(tty_db_pmadd(first, p));
		else
			return(EINVAL);
	}
	/*
	 * This is a driver open.
	 */
	if (first = tty_db_is_per_tty(p->dev))
		return(EINVAL);	/* the same tty is still registered	*/
	/*
	 * Filled the first free per_tty structure in the table with all
	 * informations given by the driver.
	 */
	return(tty_db_pmadd(NULL, p));
}

/*
 * NAME:	tty_db_is_per_tty()
 *
 * CALL:	internal to that module.	
 *
 * FUNCTION:	retrieve the per_tty structure from the general 
 *		list of ttys if there is an existing one for that dev.
 * RETURN:	a pointer on the per_tty structure if any.
 *		0 if there is no per_tty structure corresponding to that dev.
 */
static struct per_tty *
tty_db_is_per_tty(devnum)
	register dev_t devnum;
{
	register struct all_tty_s *ref;
	int i;
	
	ref = tty_list;
	while (ref) {
		for (i = 0; i < STR_TTY_CNT; i++) {
			if (ref->lists[i].dev == devnum)
				return(&(ref->lists[i])); /* the good one */
		}
		ref = ref->next;
	}
	/*
	 * There is no corresponding tty in the table.
	 */
	return(0);
}

/*
 * NAME:	tty_db_pmadd()
 *
 * CALL:	internal only.
 *
 * FUNCTION:	adds the module informations needed in the corresponding
 *		opened tty structure.
 *
 * RETURN:	EEXIST if the tty is in the list yet.
 *		ENOMEM if there is not enough memory.
 *		EINVAL if MODULEMAX is not great enough.
 *		0 if success.
 */
static int
tty_db_pmadd(exist, new_module)
	register struct per_tty *exist;
	register struct tty_to_reg *new_module;
{
	register struct all_tty_s *ref, *last_ref;
	register struct per_tty *first_free = NULL;
	struct str_module_conf *mod_confp;
	int i, j, find;
	int error;

	if (exist == NULL) {
		/*
		 * This is a driver.
		 */
		find = 0;
		if (!tty_list || !module_list)
			return(EINVAL);
		ref = last_ref = tty_list;
		if (new_module->ttyname[0] == '\0')
			return(EINVAL);	/* no device name from the driver. */
		while (ref) {
			for (i = 0; i < STR_TTY_CNT; i++) {
				if (ref->lists[i].ttyname[0] == '\0') {
					if (!find) {
						first_free = &(ref->lists[i]);
						find = 1;
					}
				} else if (!strncmp(ref->lists[i].ttyname,
					   new_module->ttyname, TTNAMEMAX))
						return(EEXIST);
			}
		/*
			if (find == 1)
				break;
		*/
			last_ref = ref;
			ref = ref->next;
		}
		/*
		 * If there is no free place in the table, then allocates a
		 * new table.
		 */
		if (!first_free) {
			if (!(last_ref->next = 
				(struct all_tty_s *)xmalloc(PAGESIZE, (uint)0,
								pinned_heap)))
				return(ENOMEM);
			bzero(last_ref->next, PAGESIZE);
			/*
			 * Updates the corresponding values in the component
			 * dump table.
			 */
			tty_db_cdt_ttyadd(last_ref->next);
			first_free = &(last_ref->next->lists[0]);
		}
		first_free->dev = new_module->dev;
		bcopy(new_module->ttyname, first_free->ttyname, TTNAMEMAX);
		first_free->mod_info[0].private_data = new_module->private_data;

		return(tty_db_modptr(first_free, new_module, 0));
	}
	/*
	 * This is a module open.
	 */
	if ((error = tty_db_modfind(new_module->name, &mod_confp)) != 0)
		return(error);

	if (mod_confp->type == 's') {
		/*
		 * This is the streamhead tty and there is a special entry
		 * for it in the per_tty structure.
		 */
		exist->mod_info[(MODULEMAX-1)].private_data = 
					new_module->private_data;
		return(tty_db_modptr(exist, new_module, (MODULEMAX - 1)));
	} else {
		/*
		 * This is the case of a normal module open.
		 */
		for (i = 1; i < MODULEMAX - 1; i++) {
			if (!exist->mod_info[i].mod_ptr){
				/*
				 * This is the location for that module
				 * informations.
				 */
				exist->mod_info[i].private_data =
						new_module->private_data;
				return(tty_db_modptr(exist, new_module, i));
			}
		}
		return(EINVAL); 
		/*
		 * EINVAL is set if MODULEMAX is not great enough.
		 */
	}
}

/*
 * NAME:	tty_db_modptr()
 *
 * CALL:	By tty_db_pmadd()
 *
 * FUNCTION:	For each module/driver in a per_tty entry structure, updates
 *		the pointer to the correct str_module_conf structure.
 *
 * RETURNS:	EINVAL if there is no module table, or if there is no
 *		corresponding str_module_conf structure for that module/driver
 *		in the module table.
 *		0 if it is ok.
 */
static int
tty_db_modptr(that_tty, new_module, number)
	register struct per_tty *that_tty;
	register struct tty_to_reg *new_module;
	register int	number;
{
	register struct str_module_s *mod_ref;
	register struct str_module_conf *mod_find = NULL;
	int i;

	if (!module_list)
		return(EINVAL);
	if (new_module->name[0] == '\0')
		return(EINVAL);

	mod_ref = module_list;
	while (mod_ref) {
		for (i = 0; i < STR_MODULE_CNT; i++) {
			if (mod_ref->lists[i].name[0] == '\0')
				continue;		/* skip the hole */
			if (!strncmp(mod_ref->lists[i].name,
				     new_module->name, FMNAMESZ)) {
				mod_find = &(mod_ref->lists[i]);
				break;
			}
		}
		if (mod_find)
			break;
		mod_ref = mod_ref->next;
	}
	if (mod_find)
		that_tty->mod_info[number].mod_ptr = mod_find;
	else
		return(EINVAL); /* there is no corresponding driver */
				/* in the module table.		    */
	return(0);
}

/*
 * NAME:	tty_db_modfind()
 *
 * FUNCTION:	search a module/driver using its name.
 *
 * RETURNS:	EINVAL if there is no module table, or if there is no
 *		corresponding str_module_conf structure for that module/driver
 *		in the module table.
 *		0 if it is ok.
 */
static int
tty_db_modfind(name, mod_findpp)
	register char *name;
	register struct str_module_conf **mod_findpp;
{
	register struct str_module_s *mod_ref;
	register struct str_module_conf *mod_find = NULL;
	int i;

	if (!module_list)
		return(EINVAL);
	if (name[0] == '\0')
		return(EINVAL);

	mod_ref = module_list;
	while (mod_ref) {
		for (i = 0; i < STR_MODULE_CNT; i++) {
			if (mod_ref->lists[i].name[0] == '\0')
				continue;		/* skip the hole */
			if (!strncmp(mod_ref->lists[i].name,
				     name, FMNAMESZ)) {
				mod_find = &(mod_ref->lists[i]);
				break;
			}
		}
		if (mod_find)
			break;
		mod_ref = mod_ref->next;
	}
	if (!mod_find)
		return(EINVAL); /* there is no corresponding driver */
				/* in the module table.		    */

	*mod_findpp = mod_find;

	return(0);
}

/*
 * NAME:	tty_db_cdt_ttyadd()
 *
 * CALL: 	Each time a new tty table is created.
 *
 * FUNCTION: 	Updates the values for the table of tty's table in the component
 *		dump table.
 *
 * RETURN:	no value returned.
 *
 */
static void 
tty_db_cdt_ttyadd(new)
	struct	all_tty_s	*new;
{
	all_ttyp_t *intermed;
	int	ocnt, osize, ncnt, nsize;

	osize = (ocnt = ttydbg_list_end - ttydbg_list) * sizeof(all_ttyp_t);
	/* ocnt + 1 : one more entry for this "new" pointer */
	nsize = (ncnt = ocnt + 1) * sizeof(all_ttyp_t);
	if (intermed = (all_ttyp_t *)xmalloc(nsize, (uint)0, pinned_heap)) {
		bzero(intermed, nsize);
		if (ttydbg_list) {
			bcopy(ttydbg_list, intermed, osize);
			xmfree(ttydbg_list, pinned_heap);
		}
		ttydbg_list = intermed;
		ttydbg_list_end = intermed + ncnt;

		intermed = ttydbg_list_end - 1;
		*intermed = new;
	}
}

/*
 * NAME:	tty_db_cdt_ttydel()
 *
 * CALL:	Each time a tty table is deleted.
 *
 * FUNCTION:	Update the values for the table of tty's table in the component
 *		dump table.
 *
 * RETURN:	no value returned.
 *
 */
static void
tty_db_cdt_ttydel(old)
	struct all_tty_s *old;
{
	all_ttyp_t	*intermed;

	for (intermed = ttydbg_list; intermed < ttydbg_list_end; intermed++)
		if (*intermed == old)
			*intermed = 0;
}

/*
 * NAME:	tty_db_cdt_modadd()
 *
 * CALL:	Each time a table of module is added.
 * 
 * FUNCTION:	Updates the values for the table of modules table in the 
 *		component dump table.
 *
 * RETURN:	no value returned.
 *
 */
static void
tty_db_cdt_modadd(new)
	struct	str_module_s	*new;
{
	str_modulep_t	*intermed;
	int	ocnt, osize, ncnt, nsize;

	osize = (ocnt = mod_list_end - mod_list) * sizeof(str_modulep_t);
	/* ocnt + 1 : one more entry for this "new" pointer */
	nsize = (ncnt = ocnt + 1) * sizeof(str_modulep_t);
	if (intermed = (str_modulep_t *)xmalloc(nsize, (uint)0, pinned_heap)) {
		bzero(intermed, nsize);
		if (mod_list) {
			bcopy(mod_list, intermed, osize);
			xmfree(mod_list, pinned_heap);
		}
		mod_list = intermed;
		mod_list_end = intermed + ncnt;

		intermed = mod_list_end - 1;
		*intermed = new;
	}
}

/*
 * NAME:	tty_db_cdt_moddel()
 *
 * CALL:	For each module table deleted.
 *
 * FUNCTION:	Updates the values for the table of modules table in the 
 *		component dump table.
 *
 * RETURN:	no value returned.
 *
 */
static void
tty_db_cdt_moddel(old)
	struct	str_module_s	*old;
{
	str_modulep_t	*intermed;

	for (intermed = mod_list; intermed < mod_list_end; intermed++)
		if (*intermed == old)
			*intermed = 0;
}

/*
 * NAME:	tty_db_usage()
 *
 * PARAMETERS:	none
 *
 * FUNCTION:	print the tty command usage.
 *
 * RETURNS: 	none
 */
static void
tty_db_usage()
{
	printf("Usage: tty  [-d | -l | -e] [-o] [-v] [name | [maj [min]]]\n");
	printf("       where : -d print only driver(s) informations\n");
	printf("               -l print only line discipline(s) informations\n");
	printf("               -e print informations about all module/driver\n");
	printf("               -o print informations about opened lines only\n");
	printf("               -v verbose, print all informations\n");
	printf("               name device name\n");
}

/*
 * NAME:	tty_db_parse_line()
 *
 * PARAMETERS:	the buffer from the command.
 *
 * FUNCTION:	tty command analysis.
 *
 * RETURNS: 1 on success, 0 otherwise
 */
static int
tty_db_parse_line(char *s)
{
	int parsed = 0;
	int len;

	/*
	 *	Global data must be cleaned on each tty command.
	 */
	name = NULL;
	maj = -1;
	min = -1;
	chan = -1;

	vflag = 0;
	oflag = 0;
	dflag = 0;
	lflag = 0;
	eflag = 0;

	while (*s && *s == ' ') ++s;
  
	if ((len = strlen(s)) && s[len-1] == '\n')
		s[len-1] = '\0';
  
	/* check for new user args -d -l -e -v -o */
	while (*s && *s == '-') {
		s++;
		switch (*s) {
		case 'd':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			dflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'l':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			lflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'e':
			if (lflag || eflag || dflag) {
				tty_db_usage();
				return(0);
			}
			eflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'v':
			vflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		case 'o':
			oflag = 1;
			do ++s;  while (*s && *s == ' ');
			parsed = 1;
			break;
		default:
			tty_db_usage();
			return(0);
		}
	}

	if (*s == 'o') {
	/* necessary because crash and pstat still call tty with 'o' arg as
	 * default.  it also indicates that there are no other args provided.
	 */
		oflag = 1;
		eflag = 1;  
		do ++s;  while (*s && *s == ' ');
		return(1);
	}
	/* tty name passed as arg */
	if (*s && isalpha(*s)) {
		name = s;
		/* no option given; set eflag */
		if (!lflag && !eflag && !dflag)
			eflag = 1;
		return(1);
	}
	/* maj/min numbers passed as args */
	name = 0;
	if (isdigit(*s)) {
		maj = 0;
		do maj = maj * 10 + *s++ - '0'; while (isdigit(*s));
		do ++s;  while (*s && *s == ' ');
		/* no option given; set eflag */
		if (!lflag && !eflag && !dflag)
			eflag = 1;
		parsed = 1;
	}
	if (isdigit(*s)) {
		min = 0;
		do min = min * 10 + *s++ - '0'; while (isdigit(*s));
		do ++s;  while (*s && *s == ' ');
		parsed = 1;
	}

	if (!parsed)
		tty_db_usage();  /* not sure this is possible */
	return(parsed);
}

/*
 * NAME:	tty_db_do_command()
 * 
 * PARAMETERS:	none, but use data set by tty_db_parse_line().
 *
 * FUNCTION:	perform the tty command.
 *		"1st level" = look through the general tty's table.
 *		Treats the -o option.
 *
 * RETURNS:	0   in case of success
 *		!=0 otherwise (return code of tty_db_disp_tty()).
 */
static int
tty_db_do_command()
{
	struct all_tty_s *refp;
	struct per_tty *per_ttyp;
	int ii, res = 0, once = 0;

	refp = tty_list;
	while (refp) {  
		for (ii = 0; ii < STR_TTY_CNT; ii++) {
			per_ttyp = &(refp->lists[ii]);	/* easier to handle */

			/* if the -o option was given - we only want to look at
			 * open ttys.  we know it's open by the presence of the 
			 * streamhead tty (strh_tty) in the push_modules array.
			 * the streamhead tty will always be at position
			 * MODULEMAX-1 in this array if it is there at all. to
			 * see if it is there check if mod_ptr == NULL. For
			 * safety, should also check for the module being of
			 * type 's', this will require reading the struct though
			 */

			if (oflag &&
			    per_ttyp->mod_info[MODULEMAX-1].mod_ptr == NULL)
				continue;

			/* there will always be something in the ttyname field
			 * if this per_tty is being used - at this point we
			 * have, i think, a vadr in the field, but i should be
			 * able to check for NULL before going off to do that
			 */
			if (per_ttyp->ttyname[0] != '\0') {
				/* the problem with this is that it continues to
				 * search for possible candidates to print even
				 * after it has printed the one it's looking
				 * for - in the case of name and maj/min 
				 */
				if ((name ?
				     once = !strcmp(name, per_ttyp->ttyname) :
				     (maj == -1 ||
				     (maj == major(per_ttyp->dev) &&
				     (min == -1 ||
				     (min == minor(per_ttyp->dev)))))) &&
				     (res = tty_db_disp_tty(per_ttyp))) {
					return(res);
				}
			}
			if (once)
				return(0);
		}

		refp = refp->next;
	}
	return(res);
}

/*
 * NAME:	tty_db_disp_tty()
 * 
 * PARAMETERS:	per_ttyp points to a per_tty structure.
 *
 * FUNCTION:	perform the tty command.
 *		"2nd level" = look through the per_tty structure.
 *		Treats the -d, -l -e options.
 *		The verbose flag -v , will be passed through to the module
 *		print routine.
 *
 * RETURNS:	0   in case of success
 *		!=0 otherwise (return code of the module print routine).
 */
static int
tty_db_disp_tty (struct per_tty *per_ttyp)
{
	struct str_module_conf *modp;
	void *priv;
	int ii, arg, found = 0, res = 0;

	arg = (vflag ? TTYDBG_ARG_V : 0);

	printf("\n/dev/%s (%d,%d)\n", per_ttyp->ttyname,
		major(per_ttyp->dev), minor(per_ttyp->dev));

	for (ii = MODULEMAX - 1; ii >= 0; ii--) {
		modp = per_ttyp->mod_info[ii].mod_ptr;	/* easier to handle */
		priv = per_ttyp->mod_info[ii].private_data;
		if (modp) {
			if (dflag) {
				if (modp->type == 'd') {
							/* driver only */
					printf("Module name: %s\n", modp->name);
					res = modp->print_funcptr(priv, arg);
					break;
				}
			}
			else if (lflag) {
				if (modp->type == 'l') {
							/* line discipline */
					printf("Module name: %s\n", modp->name);
					res = modp->print_funcptr(priv, arg);
					break;
				}
			}
			else {				/* all modules/driver */
				if (modp->name[0] == '\0') {
					continue;
				}
				printf("Module name: %s\n", modp->name);
				res = modp->print_funcptr(priv, arg);
			}
		}
	}
	return(res);
}

/*
 * NAME:	tty_db_kdb_is_avail()
 * 
 * PARAMETERS:	none
 *
 * FUNCTION:	test if kdb is present.
 *
 * RETURNS:	0   kdb is not present
 *		!=0 kdb is present
 */
static int
tty_db_kdb_is_avail()
{
	extern int brkpoint();
	int *ba_addr;

	/* Get the address of brkpoint.
	 * This is contained in the first word of the
	 * function descriptor
	 */
	ba_addr = (int *)*((int *)brkpoint);

	/* Check the TRAP code
	 */
	return (*ba_addr != STATIC_BREAK_TRAP);
}
