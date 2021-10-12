static char sccsid[] = "@(#)13	1.1  src/bos/usr/ccs/lib/libdiag/min/dmenustub.c, libdiag, bos411, 9428A410j 4/8/94 13:08:09";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	This module contains all the stubbed functions of
 *		dmenu.c. These stubbed functions will be used to
 *		create a special libdiag that is independent from
 *		libasl, libcur and libcurses and will be used solely
 *		in the boot install environment.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <values.h>
#include "diag/diago.h"
#include "diag/diag.h"
#include "nl_types.h"
#include "dcda_msg.h"
#include <sys/signal.h>


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* FUNCTION PROTOTYPES */
long dmenu(long, nl_catd, struct msglist [],
	   long, long, ASL_SCR_TYPE *, ASL_SCR_INFO *);
long diag_display(long, nl_catd, struct msglist [],
	 	  long, long, ASL_SCR_TYPE *, ASL_SCR_INFO *);
long diag_display_menu(long, long, char *[], int, int);
long diag__msg(long, int, nl_catd, unsigned short, unsigned short, va_list);
long diag_msg_nw(long, nl_catd, unsigned short, unsigned short, ...);
long diag_msg(long, nl_catd, unsigned short, unsigned short, ...);
long diag_asl_execute(char *, char *, int *);
int diag_execute(char *, char *[], int *);
char *mlcprintf(char *, ...);
long diag_emsg(nl_catd, unsigned short, unsigned short, ...);
long diag_tmsg(nl_catd, unsigned short, unsigned short, ...);
char *diag_cat_gets(nl_catd, int, int);
char *diag_device_gets(nl_catd, int, int, char *);
nl_catd diag_catopen(char *, int);
nl_catd diag_device_catopen(char *, int);
long diag_asl_msg(char *, ...);

/* EXTERNAL FUNCTIONS */
extern char *getenv(char *);

/* GLOBAL VARIABLES */
ASL_SCR_TYPE diag_menu_defaults = DM_TYPE_DEFAULTS;
ASL_SCR_TYPE dm_default = DM_TYPE_DEFAULTS;
ASL_SCR_TYPE dm_menutype;
char *diag_caterror = "** CATALOG ERROR **";
char *default_catdir = DIAGNOSTICS;
char *def_device_cat = "devices.cat";

#define DIAG_CATERROR diag_caterror


long dmenu(
	long		menu_number,
	nl_catd		cat_fdes,
	struct msglist	msglist[],
	long		proctype,
	long		screen_type,
	ASL_SCR_TYPE	*menutype,
	ASL_SCR_INFO	*menuinfo)
{

	return(0);
}


long diag_display(
	long		menu_number,
	nl_catd		cat_fdes,
	struct msglist	msglist[],
	long		proctype,
	long		screen_type,
	ASL_SCR_TYPE	*menutype,
	ASL_SCR_INFO	*menuinfo)
{

	return(0);
}	/* end of diag_display() */


/* Display commonly used menus/menu goals */
long diag_display_menu(
	long	msgid,
	long	mnum,
	char	*substitution[],
	int	lcount,
	int	lerrors)
{
	return( 0 );

}


long diag__msg(
	long	menu_number,
	int	wait,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	va_list ap)
{

	return(0); /*?*/
}					/* end of diag__msg() */

long diag_msg_nw(
	long menu_number,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	return(0);
}

long diag_msg(
	long menu_number,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	return(0);
}

long diag_asl_execute(
	char *command,			/* first  arg for execv */
	char *options,			/* second arg for execv */
	int *exit_status)		/* exit status from wait */
{
  return(0);
  }


/*
 * FUNCTION:
 *      Executes another program. Uses execv() system call.
 *	Similar to diag_asl_execute(), but is not dependent
 *	upon asl initialization.
 *
 * RETURNS:
 *	0 	-- command executed.
 *	-1	-- error occured.
 */
int diag_execute(
	char *command,			/* first  arg for execv  */
	char *options[],		/* second arg for execv  */
	int  *exit_status)		/* exit status from wait */
{


  int pid1, pid2;	/*  process IDs */
  int ret = 0;		/* return code */
  int savmask;

	/* pid=0 indicates child process */
	if ((pid1 = fork()) == 0)
	  {
	  execv(command, options);

	  /* execv() should never return */
	  exit(-1);
	  }

	/* now parent again */
	savmask = sigblock(1 << (SIGINT-1));
	pid2 = wait(exit_status);
	(void) sigsetmask(savmask);

	if (pid1 == -1) ret = -1;	/* fork failed */
	if (pid2 == -1) ret = -1;	/* wait() failed */
	if (pid2 != pid1) ret = -1;	/* waited for wrong child */

	/* if child is killed or abend's */
	if (*exit_status & 0xFF) ret = -1;
	if (((*exit_status >> 8) & 0xFF) == 0xFF) ret = -1;
	return ret;
}

long diag_asl_clear_screen(void)
{
	return(0);
}

char *mlcprintf(char *format, ...)
{
	return(0);
}

ASL_RC
diag_help()
{
	/* put some help message here */
	return(0);
}

#undef asl_init
long diag_asl_init(char *name)
{
	return(0);
}


 long
diag_asl_read (screen_code,wait, buf)
ASL_SCREEN_CODE screen_code;
int        	wait;  	/* if TRUE wait for at least 1 char. before returning */
char            *buf;	/* buffer for returning input characters */
{
	return (0);
}

#undef asl_beep
long diag_asl_beep(void)
{
	return(0L);
}

#undef asl_quit
long diag_asl_quit(name)
char *name;
{
	return(0);
}

long diag_emsg(
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	return(0);
}

long diag_hmsg(
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	char *str)
{
  long rc;

	rc = diag_emsg(cat_fdes, setid, msgid, (int )str);
	return(rc);
}

long diag_tmsg(
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	return(0);
}

char *diag_cat_gets(
	nl_catd	fdes,
	int setno,
	int msgno)
{

	nl_catd new_fdes = 0;
	nl_catd vendor_fdes = 0;
	char	*temp_ptr, *temp2;
	char	new_path[256];
	char	vendor_path[256];
	int	diag_source;
	int	len;

	if (0 >= (int )fdes)
	  {
	  temp_ptr = (char *)malloc(strlen(DIAG_CATERROR) + 1);
	  strcpy(temp_ptr, DIAG_CATERROR);
	  return(temp_ptr);
	  }

	temp_ptr = catgets(fdes, setno, msgno, DIAG_CATERROR);
	/* if message not found - search another catalog */
	if (DIAG_CATERROR == temp_ptr)
	  {
	  if ((temp2 = getenv("DIAGNOSTICS")) == (char *)NULL)
	    sprintf(new_path, "%s/catalog/default/%s",
		    default_catdir, fdes->_name);
	  else
	    sprintf(new_path, "%s/catalog/default/%s",
		    temp2, fdes->_name);
	  new_fdes = catopen(new_path, NL_CAT_LOCALE);
	  if (new_fdes > (nl_catd)0)
	    {
	    temp_ptr = catgets(new_fdes, setno, msgno, DIAG_CATERROR);
	    if(temp_ptr != DIAG_CATERROR)
	    /* message found in default directory */
	      {
	        len = strlen(temp_ptr);		/* Copy the catalog msg */
	        temp2 = (char *)malloc(len + 1);/* to a new memory block*/
	        strcpy(temp2, temp_ptr);	/* so we can close the  */
	        catclose(new_fdes);		/* default catalog.	*/
         	temp_ptr = temp2;
	      } else {
	    /* If the message is still not found, and the message catalog */
	    /* being searched is dcda.cat, then go one step further to    */
	    /* search in vendor.cat.					  */
	    if (!strcmp(fdes->_name, MF_DCDA))
		{
		sprintf(vendor_path, "%s/catalog/default/%s", default_catdir,
			"vendor.cat");
		vendor_fdes = catopen(vendor_path, NL_CAT_LOCALE);
		if (vendor_fdes > (nl_catd)0)
		   {
		   temp_ptr = catgets(vendor_fdes, setno, msgno, DIAG_CATERROR);
		   len = strlen(temp_ptr);
		   temp2 = (char *)malloc(len +1);
		   strcpy(temp2, temp_ptr);
		   catclose(vendor_fdes);
		   temp_ptr = temp2;
		   }
		}
	    else {
	    /* See if running off CDROM, from the hard file. */
	    /* If so, then try the alternate diagnostic	     */
	    /* directory for message from supplemental device*/

	  	if ((temp2 = getenv("DIAG_SOURCE")) != (char *)NULL)
			{
			diag_source=atoi(temp2);
			if(diag_source==IPLSOURCE_CDROM)
	  			if ((temp2 = getenv("ALT_DIAG_DIR")) == 
						(char *)NULL)
				    sprintf(new_path, "%s/catalog/default/%s",
		    			default_catdir, fdes->_name);
	  			else
				    sprintf(new_path, "%s/catalog/default/%s",
		    				temp2, fdes->_name);
				new_fdes = catopen(new_path, NL_CAT_LOCALE);
				if (new_fdes > (nl_catd)0){
	    				temp_ptr = catgets(new_fdes, setno, 
						msgno, DIAG_CATERROR);
		   			len = strlen(temp_ptr);
					temp2 = (char *)malloc(len +1);
					strcpy(temp2, temp_ptr);
					catclose(new_fdes);
					temp_ptr = temp2;
				}	
			}
		}
	    }
	  }
	}
	return(temp_ptr);
}


char *diag_device_gets(
	nl_catd	fdes,
	int setno,
	int msgno,
	char *def_str)
{
	return((char *)NULL);
}

nl_catd diag_catopen(
	char *filename,
	int open_flag)
{

  char *temp2;
  char new_path[256];
  int	oflag;
  nl_catd fdes;

	oflag=NL_CAT_LOCALE;
	if(open_flag != 0)
		oflag=open_flag;

	fdes = catopen(filename, oflag);
	if ((-1 == (int )fdes) && (*filename != '/'))
	  {
	  if ((temp2 = getenv("DIAGNOSTICS")) == (char *)NULL)
	    sprintf(new_path, "%s/catalog/default/%s",
		    default_catdir, filename);
	  else
	    sprintf(new_path, "%s/catalog/default/%s",
		    temp2, filename);
	  fdes = catopen(new_path, oflag);
	  }

	return(fdes);
}

nl_catd diag_device_catopen(
	char *filename,
	int open_flag)
{

	return(0);
}

long diag_asl_msg(char *m1, ...)
{

	return(0L);

}
