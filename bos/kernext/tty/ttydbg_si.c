#ifndef lint
static char sccsid[] = "@(#)27 1.1 src/bos/kernext/tty/ttydbg_si.c, sysxtty, bos411, 9428A410j 4/23/94 11:50:32";
#endif
/*
 *
 * COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
 *
 * FUNCTIONS:	tty_db_config.
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

/*
 * Functions declared in ttydbg_dbg.c.
 */
extern void	tty_print();

/*
 * Functions declared in ttydbg_reg.c.
 */
extern struct cdt_head	*tty_dmp();

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

extern int	tty_db_register();
extern int	tty_db_unregister();
extern int	tty_db_open();
extern int	tty_db_close();

extern int	tty_db_init();
extern int	tty_db_del();

extern void	tty_db_cdt_ttyadd();
extern void	tty_db_cdt_modadd();
extern void	tty_db_cdt_ttydel();
extern void	tty_db_cdt_moddel();

/*
 * Function pointers declared and exported by the debugger lldb.
 * (src/bos/kernel/db, dbkern.c and POWER/dbtty.c)
 */
extern int	(* tty_db_register_ptr)();
extern int	(* tty_db_unregister_ptr)();
extern int	(* tty_db_open_ptr)();
extern int	(* tty_db_close_ptr)();

extern void	(* db_tty_dump_ptr)();

/*
 * Bounds for pinned code : data declared in ttydbg_start.s and ttydbg_end.s.
 */
extern long pin_ttydbg_obj_start;
extern long pin_ttydbg_obj_end;
extern long pin_ttydbg_com_start;
extern long pin_ttydbg_com_end;

/*
 * Declarations for configuration routine.
 */
static int ttydbg_active = 0;	/* config methods : extension active or not */

lock_t ttydbg_conf_lock = LOCK_AVAIL; /* extension configuration lock.	*/

/*
 * Data declared in ttydbg_reg.c.
 */
extern struct str_module_s *module_list;
extern struct all_tty_s **ttydbg_list, **ttydbg_list_end;
extern struct str_module_s **mod_list, **mod_list_end;

/*
 * Data declared in lldb
 */
extern int dbg_pinned;

/************************************************************************/
/************************************************************************/
/*	Routines called by external modules.                            */
/************************************************************************/
/************************************************************************/

/*
 * NAME:	tty_db_config()
 *		This is the external entry point to the module.
 *		It is called to configure, and unconfigure the general table
 *		and some usefull data.
 *		It is defined in the Makefile in ttydbg_ENTRYPOINT and used
 *		by the binder with the "-e" option.
 *		The call to this function comes via the sysconfig with
 *		cmd==SYS_CFGDD/SYS_CFGKMOD system call.
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
 *		Any other error sent on tty_db_init() call.
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
		if (ttydbg_active == 0) {
			if (dbg_pinned) {
				/*
				 * The lldb debugger is active :
				 * pin the whole module
				 */
				if ((error = pincode(tty_db_config)) != 0)
					break;
			}
			else {
				/*
				 * The lldb debugger is not active :
				 * just pin the registration functions for
				 * crash command and the system dump.
				 */
				pin(&pin_ttydbg_obj_start,
				    (int)&pin_ttydbg_obj_end - (int)&pin_ttydbg_obj_start);
				pin(&pin_ttydbg_com_start,
				    (int)&pin_ttydbg_com_end - (int)&pin_ttydbg_com_start);
			}
			/*
			 * pinned ok, allocate the module list.
			 * and allocate the tty_list.
			 */
			if ((error = tty_db_init()) != 0) {
				if (dbg_pinned) {
					unpincode(tty_db_config);
					break;
				}
				else {
					unpin(&pin_ttydbg_obj_start,
					(int)&pin_ttydbg_obj_end - (int)&pin_ttydbg_obj_start);
					unpin(&pin_ttydbg_com_start,
					(int)&pin_ttydbg_com_end - (int)&pin_ttydbg_com_start);
				}
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
			++ttydbg_active;
			/* set the pointer to the print routine for lldb */
			db_tty_dump_ptr = tty_print;
			/*
			 * set the pointers to the registration routines
			 * for modules
			 */
			tty_db_register_ptr = tty_db_register;
			tty_db_unregister_ptr = tty_db_unregister;
			tty_db_open_ptr = tty_db_open;
			tty_db_close_ptr = tty_db_close;
		}
		break;	/* nothing to do */
	case CFG_TERM:
		/*
		 * Unload only if ttydbg_active is 1.
		 * Otherwise nothing to do.
		 */
		if (ttydbg_active == 1) {
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
			 * ttydbg_active is decremented, and the code unpinned.
			 */
			tty_db_del();
			--ttydbg_active;
			/* unset the pointer to the print routine for lldb */
			db_tty_dump_ptr = NULL;
			/*
			 * unset the pointers to the registration routines
			 * for modules
			 */
			tty_db_register_ptr = NULL;
			tty_db_unregister_ptr = NULL;
			tty_db_open_ptr = NULL;
			tty_db_close_ptr = NULL;

			if (dbg_pinned) {
				unpincode(tty_db_config);
			}
			else {
				unpin(&pin_ttydbg_obj_start,
				      (int)&pin_ttydbg_obj_end - (int)&pin_ttydbg_obj_start);
				unpin(&pin_ttydbg_com_start,
				      (int)&pin_ttydbg_com_end - (int)&pin_ttydbg_com_start);
			}
		}
		break;
	default:
		error = EINVAL;
	}
	if (locked != LOCK_NEST)
		unlockl(&ttydbg_conf_lock);
	return(error);
}
