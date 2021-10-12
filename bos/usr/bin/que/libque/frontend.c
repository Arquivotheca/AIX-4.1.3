static char sccsid[] = "@(#)63	1.14  src/bos/usr/bin/que/libque/frontend.c, cmdque, bos411, 9428A410j 3/30/94 10:02:18";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: xpndlist, is_purenum, exe_enq, filenameonly
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <IN/standard.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "common.h"
#include "frontend.h"

#include "libq_msg.h"

extern boolean palladium_inst;
 
/*====EXPAND A LIST OF ITEMS IN AN ARGUMENT STRING */
xpndlist(xl_arg,xl_index,xl_outargs,xl_outprfx)
char	*xl_arg;
int	*xl_index;
char	**xl_outargs;
char	*xl_outprfx;
{
	char	*xl_value;

	while(*xl_arg != '\0')
	{
		/*----Skip past all commas, spaces */
		xl_value = xl_arg;
		while(*xl_value == '\"' ||
		      *xl_value == ' '  ||
		      *xl_value == ',')
			xl_value++;
		if(*xl_value == '\0')
			break;

		/*----Set pointer, go to end of string, and set a '\0' */
		xl_arg = xl_value;
		while(*xl_arg != '\0' &&
		      *xl_arg != '\"' &&
		      *xl_arg != ' '  &&
		      *xl_arg != ',')
			xl_arg++;
		if(*xl_arg != '\0')
			*(xl_arg++) = '\0';

		/*----Send new argument to output parmeter list */
		xl_outargs[(*xl_index)++] = xl_outprfx;
		xl_outargs[(*xl_index)++] = xl_value;
	}
	return(0);

}

/*====DETERMINE IF STRING IS A NUMBER WITHOUT ANY OTHER CHARACTERS */
int is_purenum(ip_str)
char	*ip_str;
{
	register int	ip;		/* index into input string */
	int		status = 1;	/* exit status of this routine */
	char		*rhs;		/* right-hand side of value */

	/*----Check each character of string */
	for(ip = 0; ip_str[ip] != '\0'; ip++)

		/*----Return if a char is not a number */
		if(!isdigit((int)ip_str[ip])) {
			if(palladium_inst && (rhs = strchr(ip_str, ':'))) {
				for(ip = 0; rhs[ip] != '\0'; ip++)
					if(!isdigit((int)rhs[ip])) {
						status = 0;
						break;
					}
			}
			else
				status = 0;
			break;
		}

	/*----Made it thru loop, it is a pure number */
	return(status);
}

/*====EXECUTE ENQ WITH XLATED PARAMETERS */
exe_enq(ee_args)
char *ee_args[];
{
	int		ee_waitstat = 0;
	int		ee_fresult;
	extern char	*progname;

	if(ee_fresult = fork())
	{
		/*----Parent: wait for child to finish */
		if(ee_fresult == -1)	
			syserr((int)EXITFATAL,GETMESG(MSGFERR,QMSGFERR),progname);
		waitpid(ee_fresult,&ee_waitstat,0);
		if(ee_waitstat != 0)
			qexit((int)EXITFATAL);/* enq should have reported its problem */
	}
	else
	{
		/*----Child: exec ENQ with new parms */
		execvp(ENQPATH,ee_args);

		/*----Exec failed if reached here */
		syserr((int)EXITFATAL,GETMESG(MSGXERR,QMSGXERR),progname,ENQPATH);
	}
	return(0);
}

/*====EXTRACT THE FILE NAME FROM A COMPLETE PATH STRING */
char *filenameonly(fn_path)
char *fn_path;
{
	char		*fn_nameptr;
	unsigned	fn;
        
	/*----Search for end of string */
	for(fn_nameptr = fn_path, fn = 0;
            *(fn_path + fn) != '\0';
	    fn++);

	/*----Search backwards for slash */
	while(fn && (*(fn_path + (fn - 1)) != '/'))
		fn--;

	/*----return pointer to char after backslash */
	return(fn_path + fn);
}
