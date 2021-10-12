static char sccsid[] = "@(#)67	1.5  src/bos/usr/ccs/lib/libc/sysconf.c, libcenv, bos411, 9428A410j 3/4/94 11:38:47";
/*
 * COMPONENT_NAME: LIBCENV
 *
 * FUNCTIONS: sysconf
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

#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <errno.h>
#include <userpw.h>
#include <sys/var.h>
#include <sys/sysconfig.h>
#include "sysconf.h"

static int i;	/* the array index to the current macro name being evaluated */

/*
 * NAME: sysconf
 *
 * FUNCTION: The sysconf() function provides a method for the application to 
 *	determine the current value of a configurable system limit or option.
 *      The 'name' argument represents the system variable to be queried.
 *	These variables are found in <limits.h> or <unistd.h>.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 * 	If 'name' is an invalid value, sysconf() returns a -1; otherwise,
 *      the sysconf() function will return the current variable value on 
 * 	the system.
 *
 * ERRORS:
 *	EINVAL		The value of the 'name' argument is invalid.
 *
 */
long
sysconf(int name)
{
	i = name;
	if (i < 0 || i >= sizeof(values)/sizeof(values[0])) {
		errno = EINVAL;
		return (-1);
	}
	if (values[i].func)
		return (*(values[i].func))();
	else
		return (values[i].limit);
}
		

/*
 * NAME: num_cpus
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 *	Returns the number of processors on line
 */

long
num_cpus () 
{
	struct var cpus;

	sysconfig(SYS_GETPARMS, &cpus, sizeof(struct var));
	return(cpus.v_ncpus);
}

/*
 * NAME: num_cpus_cfg
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 *	Returns the number of processors configured
 */

long
num_cpus_cfg () 
{
	struct var cpus;

	sysconfig(SYS_GETPARMS, &cpus, sizeof(struct var));
	return(cpus.v_ncpus_cfg);
}

/*
 * NAME: child_max
 *
 * FUNCTION: 
 * 	Since root has no limitation of the number of child processes, 
 *	child_max is used to determine the number of process table slots
 *	as the returned number for CHILD_MAX.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 *	the numebr of process table slots.
 */

long
child_max () 
{
	struct var childmax;

	if (geteuid() == 0)
		return(NPROC);
	else {
		sysconfig(SYS_GETPARMS, &childmax, sizeof(struct var));
		return(childmax.v_maxup);
	}	
}


/*
 * NAME: def_chk
 *
 * FUNCTION: 
 * 	For the optional supported features, they may be defined as -1 
 * 	in the unistd.h to specify its never been support. If the 
 * 	symbol is defined to be not -1, the feature is always supported
 *	in the implementation.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 *	TRUE when the symbol is defined to non -1.
 *	-1 when the symbol is defined to -1.
 */

long
def_chk()
{
	if (values[i].limit == -1)
		return (-1);	/* never support this feature */
	else
		return (TRUE);	/* always support this feature */
}


/*
 * NAME: nodef_chk
 *
 * FUNCTION: 
 *	When a optional supported feature is not defined in the unistd.h,
 *	sysconf() has to query the information through the AIX ODM data
 *	base. The ODMPATH is /usr/lib/objrepos. The object class concerned
 *	here is "lpp". The "state" of the lpp is BROKEN, DEINSTALLING, 
 *	and DEINSTLLED when it corresponding values are 7, 8, and 9.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 *	TRUE when the optional feature has been intalled successfully and 
 *	available.
 *	-1 in all other cases.
 */

long 
nodef_chk (void)
{
	char *command;
	FILE *fp;
	int c;

	switch (i)
	{
		case _SC_2_C_DEV:	/* C Development Util. */
			command = "/bin/odmget -q \"name='xlccmp.obj' and state != 7 and state != 8 and state != 9\" lpp";
			break;
		case _SC_2_FORT_DEV:	/* Fortran Development Util. */
			command = "/bin/odmget -q \"name='xlfcmp.obj' and state != 7 and state != 8 and state != 9\" lpp";
			break;
		case _SC_2_FORT_RUN:	/* Fortran Run Time Util. */
			command = "/bin/odmget -q \"name='xlfrte.obj' and state != 7 and state != 8 and state != 9\" lpp";
			break;
		case _SC_2_SW_DEV:	/* Software Development Util. */
		case _SC_2_LOCALEDEF:	/* Supports creation of locale */
			command = "/bin/odmget -q \"name='bosadt.bosadt.obj' and state != 7 and state != 8 and state != 9\" lpp";
			break;
		default:
			return (-1);
	}
	system ("set ODMDIR=/usr/lib/objrepos");
	if ((fp = popen (command,"r")) && (fgetc(fp) != EOF))
			return (TRUE);
	return(-1);
}
