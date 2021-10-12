/* @(#)60 1.1 src/bos/kernext/tty/ttydbg.h, sysxtty, bos411, 9428A410j 3/11/94 09:20:37 */
/*
 *  
 * COMPONENT_NAME: (sysxtty)	ttydbg extension for tty debugging
 *  
 * FUNCTIONS: none
 *  
 * ORIGINS: 83
 *  
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef	_H_TTYDBG
#define	_H_TTYDBG

/*
 * General include for streams tty based modules and drivers debugging.
 */
#include <sys/dump.h>
#include <sys/str_tty.h>

typedef	struct	all_tty_s *all_ttyp_t;
typedef struct	str_module_s *str_modulep_t;
/*
 * Structures and defines for the management of the modules and driver tty.
 */
#define	STR_MODULE_CNT 16

struct	str_module_s {
	struct	str_module_conf	lists[STR_MODULE_CNT];
	struct	str_module_s	*next;
};

#endif	/* _H_TTYDBG	*/
