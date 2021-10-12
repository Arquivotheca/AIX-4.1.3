#ifndef lint
static char sccsid[] = "@(#)26 1.1 src/bos/kernext/tty/ttydbg_reg.c, sysxtty, bos411, 9428A410j 4/23/94 11:50:14";
#endif
/*
 *
 * COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
 *
 * FUNCTIONS:	tty_db_register, tty_db_unregister,
 *		tty_db_open, tty_db_close,
 *		tty_dmp,
 *		tty_db_init, tty_db_del,
 *		tty_db_modulelistadd, tty_db_ttylistadd
 *		tty_db_is_per_tty, tty_db_pmadd, tty_db_modptr, tty_db_modfind
 *		tty_db_cdt_ttyadd, tty_db_cdt_ttydel,
 *		tty_db_cdt_modadd, tty_db_cdt_moddel.
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
/*
 * For now the registration macros and functions have the same name, so
 * discard registration macros defined in str_tty.h included by ttydbg.h
 * In a further release these macros should be defined (and used) with
 * names in all uppercase.
 */
#undef		tty_db_register
#undef		tty_db_unregister
#undef		tty_db_open
#undef		tty_db_close

/* Externals */
struct cdt_head	*tty_dmp();
/*
 * For the pse (streams frame work) loading purpose, only the symbol names of
 * registration functions are usefull (in str_tty.c). Then the debugger lldb
 * declares and exports the corresponding function pointers. These pointers
 * will be initialized with the right function addresses by the ttydbg
 * extension when its tty_db_config routine will be called with cmd==CFG_INIT.
 * Thus, the 4 following registration functions are accessed through pointers
 * and the macros defined in str_tty.h provide that facility.
 */
int		tty_db_register();
int		tty_db_unregister();
int		tty_db_open();
int		tty_db_close();

int		tty_db_init();
int		tty_db_del();

void		tty_db_cdt_ttyadd();
void		tty_db_cdt_ttydel();
void		tty_db_cdt_modadd();
void		tty_db_cdt_moddel();

/* Internals */

static int	tty_db_modulelistadd();
static int	tty_db_ttylistadd();
static struct	per_tty *tty_db_is_per_tty();

static int	tty_db_pmadd();
static int	tty_db_modptr();
static int	tty_db_modfind();

/*
 *	General tables : informations needed for dump, debug.
 */
struct	str_module_s *module_list;	/* general modules and drivers list */

struct	all_tty_s	*tty_list;	/* general tty list		    */

/*
 * Component dump table, declarations.
 */
struct	all_tty_s	**ttydbg_list, **ttydbg_list_end;
struct	str_module_s	**mod_list, **mod_list_end;

/* We can not use the cdt structure name of dump.h because it defines	*/
/* only 1 entry in the component dump table entries item cdt_entry[]	*/
#ifndef	_KDB
static
#endif	/* _KDB */
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
 * pre-defined print functions (AIX modules).
 */
extern int scxma_print();
extern int ldtty_print();
extern int slion_print();
extern int nls_print();
extern int pty_print();
extern int srs_print();
extern int sptr_print();
extern int sh_ttydb_print();

int pdpf[LAST_PDPF];

/************************************************************************/
/************************************************************************/
/*	Internal Routines but                                           */
/*	called by external modules through a function pointer           */
/************************************************************************/
/************************************************************************/

/*
 * NAME:        tty_dmp()
 *
 * PARAMETERS:  no parameters.
 *
 * FUNCTION:    Returns the component dump table address.
 *		called by system dump functionnality.
 *		The address of this function is provided to or removed from
 *		system dump using the dmp_add() or dmp_del() routines.
 *
 * RETURNS:     always 0.
 */
struct cdt_head *
tty_dmp() 
{
        return(&str_tty_dmp);
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
						he_free(ref);
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
		if (!(tty_list = (struct all_tty_s *)he_alloc(
					sizeof(struct all_tty_s), BPRI_MED)))
						
			return(ENOMEM);
		bzero(tty_list, sizeof(struct all_tty_s));
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
					he_free(ref);
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

/************************************************************************/
/************************************************************************/
/*	Internal Routines.                                              */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_db_init()
 *
 * CALL:	External to that module at configuration time.
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
int
tty_db_init()
{
	int	i, j;

	if (module_list || tty_list)
		return(EINVAL);
	else {
                /*
                 * It is the first time that tty_db_register is called.
                 */
                if (module_list = (struct str_module_s *)he_alloc(
                                 sizeof(struct str_module_s) , BPRI_MED)) {
			bzero(module_list, sizeof(struct str_module_s));
                } else
                        return(ENOMEM); /* not enough place in memory   */
		if (tty_list = (struct all_tty_s *)he_alloc(
					sizeof(struct all_tty_s), BPRI_MED)) {
			bzero(tty_list, sizeof(struct all_tty_s));
		} else
			return(ENOMEM); /* not enough place in memory	*/
        }
	/*
	 * Initialization of pre-defined print functions table
	 */
	pdpf[CXMA_PDPF] = scxma_print;
	pdpf[LDTERM_PDPF] = ldtty_print;
	pdpf[LION_PDPF] = slion_print;
	pdpf[NLS_PDPF] = nls_print;
	pdpf[PTY_PDPF] = pty_print;
	pdpf[RS_PDPF] = srs_print;
	pdpf[SPTR_PDPF] = sptr_print;
	pdpf[STR_TTY_PDPF] = sh_ttydb_print;

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
int
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
			he_free(mod_to_free);
		}
		/*
	 	 * Now all modules and drivers are deleted from the list, so it 
		 * is time to deallocate the first list itself.
	 	 */
        	he_free(module_list);
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
			he_free(tty_to_free);
		}
		/*
	 	 * Now, all ttys are deleted from the table, so deallocates the
	 	 * first table itself.
	 	 */
		he_free(tty_list);
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
		if (!(ref->next = (struct str_module_s *)he_alloc(
					sizeof(struct str_module_s),BPRI_MED)))
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
	switch((int)p->print_funcptr) {
	case CXMA_PDPF:
	case LDTERM_PDPF:
	case LION_PDPF:
	case NLS_PDPF:
	case PTY_PDPF:
	case RS_PDPF:
	case SPTR_PDPF:
	case STR_TTY_PDPF:
		first_free->print_funcptr = pdpf[(int)p->print_funcptr];
		break;
	}

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
				(struct all_tty_s *)he_alloc(
					sizeof(struct all_tty_s),BPRI_MED)))
				return(ENOMEM);
			bzero(last_ref->next, sizeof(struct all_tty_s));
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
void 
tty_db_cdt_ttyadd(new)
	struct	all_tty_s	*new;
{
	all_ttyp_t *intermed;
	int	ocnt, osize, ncnt, nsize;

	osize = (ocnt = ttydbg_list_end - ttydbg_list) * sizeof(all_ttyp_t);
	/* ocnt + 1 : one more entry for this "new" pointer */
	nsize = (ncnt = ocnt + 1) * sizeof(all_ttyp_t);
	if (intermed = (all_ttyp_t *)he_alloc(nsize, BPRI_MED)) {
		bzero(intermed, nsize);
		if (ttydbg_list) {
			bcopy(ttydbg_list, intermed, osize);
			he_free(ttydbg_list);
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
void
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
void
tty_db_cdt_modadd(new)
	struct	str_module_s	*new;
{
	str_modulep_t	*intermed;
	int	ocnt, osize, ncnt, nsize;

	osize = (ocnt = mod_list_end - mod_list) * sizeof(str_modulep_t);
	/* ocnt + 1 : one more entry for this "new" pointer */
	nsize = (ncnt = ocnt + 1) * sizeof(str_modulep_t);
	if (intermed = (str_modulep_t *)he_alloc(nsize, BPRI_MED)) {
		bzero(intermed, nsize);
		if (mod_list) {
			bcopy(mod_list, intermed, osize);
			he_free(mod_list);
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
void
tty_db_cdt_moddel(old)
	struct	str_module_s	*old;
{
	str_modulep_t	*intermed;

	for (intermed = mod_list; intermed < mod_list_end; intermed++)
		if (*intermed == old)
			*intermed = 0;
}

