static char sccsid[] = "@(#)36	1.2.3.7  src/bos/usr/ccs/lib/libdiag/dmenu.c, libdiag, bos41J, 9509A_all 2/21/95 09:24:15";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	dmenu
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
	int j;
	int rc=0;
	char	*temp_ptr;
	char	menu_name[81];
	nl_catd	dcda_fdes;

	menuinfo[0].data_value = menu_name;
	if (menu_number == (long )NULL)
		*menuinfo[0].data_value = '\0';
	else
		sprintf(menuinfo[0].data_value, "%X", menu_number);

	if (NULL != msglist)
	{
		for (j=0; 0 != msglist[j].msgid; j++)
		{
			temp_ptr = diag_cat_gets(cat_fdes, msglist[j].setid,
					msglist[j].msgid);
			menuinfo[j].text = (char*)malloc(1+strlen(temp_ptr));
			if(menuinfo[j].text == (char *) 0) rc =1;
			else strcpy(menuinfo[j].text, temp_ptr);
		}
		menutype->max_index = j-1;
		menutype->cur_index = 1;
	}

	switch(proctype)
	{
	case DIAG_IO:
		if ((long )NULL == screen_type)
			screen_type = menutype-> screen_code;

		switch(screen_type)
		{
		case ASL_DIAG_OUTPUT_LEAVE_SC:
		case ASL_DIAG_LEAVE_NO_KEYS_SC:
		case ASL_DIAG_NO_KEYS_ENTER_SC:
		case ASL_DIAG_ENTER_SC:
		case ASL_DIAG_ENTER_HELP_SC:
		case ASL_DIAG_DIALOGUE_SC:
		case ASL_DIAG_DIALOGUE_HELP_SC:
		case ASL_DIAG_DIALOGUE_LIST_SC:
		case ASL_DIAG_LIST_CANCEL_EXIT_SC:
		case ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC:
		case ASL_DIAG_LIST_COMMIT_SC:
		case ASL_DIAG_LIST_COMMIT_HELP_SC:
		case ASL_DIAG_KEYS_ENTER_SC:
			menutype-> screen_code = screen_type;
			break;
		default:
			diag_asl_msg("Invalid screen_type for diag_display(), or dmenu().\nMenu number:%s",
					(int )menuinfo[0].data_value);
			break;						/* defensive */
		}
		break;
	case DIAG_MSGONLY:
		return((rc == 0) ? DIAG_ASL_OK : DIAG_ASL_FAIL);
	default:
		diag_asl_msg("Invalid proctype for diag_display(), or dmenu().\nMenu number:%s",
				(int )menuinfo[0].data_value);
		return(DIAG_ASL_ARGS2);
	}

	rc = asl_screen(menutype, menuinfo);

	if ((screen_type != ASL_DIAG_OUTPUT_LEAVE_SC) && 
	    (screen_type != ASL_DIAG_LEAVE_NO_KEYS_SC))
	  {
	  dcda_fdes = diag_catopen(MF_DCDA, 0);
	  temp_ptr = diag_cat_gets(dcda_fdes, DC_MSG_SET_1, DC_PROCESSING_MSG);
	  asl_note(ASL_INFORM_MSG, ASL_NO_LOG, temp_ptr);
	  catclose(dcda_fdes);
	  }

	return(rc);
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
	int jf = 0;
	int k;
	int rc;

	if( NULL == menuinfo )
	{
		if( NULL == msglist )
			return(DIAG_ASL_ARGS1);

		if( DIAG_MSGONLY == proctype )
			return(DIAG_ASL_ARGS2);

		for(jf=0; 0 != msglist[jf].msgid; jf++)
		;
		menuinfo = (ASL_SCR_INFO*)
			calloc( jf*sizeof(ASL_SCR_INFO), 1);
		if(  (ASL_SCR_INFO*) -1 == menuinfo  )
			return(DIAG_MALLOCFAILED);
	}
	if( NULL == menutype )
	{
		if( NULL == msglist )
			return(DIAG_ASL_ARGS1);

		dm_menutype = dm_default;
		menutype    = &dm_menutype;
		if( jf > 0 )
			k = jf;
		else
			for(k=0; 0 != msglist[k].msgid; k++)
			;
		menutype->max_index = k-1;
		menutype->cur_index = 1;
	}
	
	rc = dmenu(menu_number, cat_fdes, msglist, proctype,
		   screen_type, menutype, menuinfo);


	return(rc);
}	/* end of diag_display() */


/* Display commonly used menus/menu goals */
long diag_display_menu(
	long	msgid,
	long	mnum,
	char	*substitution[],
	int	lcount,
	int	lerrors)
{
	nl_catd	dcda_fdes;
	static	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;
	char	*temp_ptr;
	char	temp_buff[1024];
	int	rc;
	long	menu_option = ASL_DIAG_OUTPUT_LEAVE_SC;

	dcda_fdes = diag_catopen(MF_DCDA, 0);
	menuinfo = (ASL_SCR_INFO *)calloc(1,sizeof(ASL_SCR_INFO));
	temp_ptr = diag_cat_gets(dcda_fdes, DA_COMMON_MSG_SET_47,
			msgid);
	switch(msgid){
	case	NO_HOT_KEY:
		sprintf(temp_buff, temp_ptr);
		menu_option = ASL_DIAG_KEYS_ENTER_SC;
		break;
	case	CUSTOMER_TESTING_MENU:
	case	ADVANCED_TESTING_MENU:
		sprintf(temp_buff, temp_ptr, substitution[0],
			substitution[1], substitution[2]);
		break;
	case	LOOPMODE_TESTING_MENU:
		sprintf(temp_buff, temp_ptr,substitution[0],
			substitution[1], substitution[2], lcount, lerrors);
		break;
	case	NO_MICROCODE_MENU:
	case	NO_DIAGMICROCODE_MENU:
	case	NO_DDFILE_MENU:
		sprintf(temp_buff, temp_ptr, mnum, substitution[0],
				substitution[1], substitution[1]);
		menugoal(temp_buff);
		rc = ASL_ENTER;
		return( rc );
	case	DEVICE_INITIAL_STATE_FAILURE:
		sprintf(temp_buff, temp_ptr ,mnum, substitution[0],
				substitution[0]);
		menugoal(temp_buff);
		rc = ASL_ENTER;
		return( rc );
	default:
		return(-1);
	}
	menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
	strcpy(menuinfo[0].text, temp_buff);
	menutype.max_index=0;
	rc = dmenu(mnum, dcda_fdes, NULL, DIAG_IO,
		menu_option, &menutype, menuinfo);

	catclose(dcda_fdes);
	return( rc );

}


long diag__msg(
	long	menu_number,
	int	wait,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	va_list ap)
{
	char	templine[2000];
	static  ASL_SCR_TYPE menutype;
	static  ASL_SCR_INFO init_menuinfo = {" "};
	static  ASL_SCR_INFO menuinfo[3];
	char	*text;
	int	rc;
	char	*temp_ptr;
	char	menu_name[81];

	menutype = dm_default;
	text = diag_cat_gets(cat_fdes, setid, msgid);
	rc = vsprintf(templine, text, ap);
	menuinfo[0].text = templine;
	menuinfo[1].text = " ";

	menutype.max_index = 1;
	menutype.cur_index = 1;

	menutype.screen_code = ASL_DIAG_OUTPUT_LEAVE_SC;

	menuinfo[0].data_value = menu_name;
	if (menu_number == (long )NULL)
		*menuinfo[0].data_value = '\0';
	else
		sprintf(menuinfo[0].data_value, "%X", menu_number);

	rc = asl_screen(&menutype, menuinfo);
	if( ASL_OK == rc  &&  TRUE == wait )
		rc = asl_read(ASL_DIAG_NO_KEYS_ENTER_SC, TRUE, NULL);
	if( TRUE == wait )
		asl_clear_screen();

	return(rc); /*?*/
}					/* end of diag__msg() */

long diag_msg_nw(
	long menu_number,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	int	rc;
	va_list ap;

	va_start(ap, msgid);
	rc = diag__msg(menu_number, FALSE, cat_fdes, setid, msgid, ap);
	return(rc);
}

long diag_msg(
	long menu_number,
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	int	rc;
	va_list ap;

	va_start(ap, msgid);
	rc = diag__msg(menu_number, TRUE, cat_fdes, setid, msgid, ap);
	return(rc);
}

long diag_asl_execute(
	char *command,			/* first  arg for execv */
	char *options,			/* second arg for execv */
	int *exit_status)		/* exit status from wait */
{
  int	rc;
  int	signal;

  /********************************************* 
  *  diag_asl_execute() will return (-1) 
  *  if any of the following are TRUE:
  *  1)  asl_execute() returns ASL_FAIL (=-1)
  *	 which indicates that the fork() failed.
  *  2)  The executing program in the child
  *	 process was stopped
  *  3)  The exec() failed, and the child 
  *	 process exited with (-1).
  **********************************************/
  rc = 0;
  if (asl_execute(command, options, exit_status) < 0) rc = -1;
  if (*exit_status & 0xFF) rc = -1;
  signal = *exit_status;
  if (((signal >> 8) & 0xFF) == 0xFF) rc = -1; 
  return(rc);
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
	return(asl_clear_screen());
}

char *mlcprintf(char *format, ...)
{
	char	templine[2000];
	char	*m_ptr;
	int	rc;
	va_list ap;

	va_start(ap, format);
	rc = vsprintf(templine, format, ap);
	m_ptr = (char*)malloc( 1+strlen(templine) );
	strcpy(m_ptr, templine);
	return(m_ptr);
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
  int	rc;

	if (!strcmp(name, "NO_TYPE_AHEAD"))
	  rc = asl_init((FILE *) NULL,		/* No screen log file	*/
			FALSE,			/* trace_log OFF	*/
			FALSE,			/* verbose_log OFF	*/
			FALSE,			/* debug_log OFF	*/
			FALSE, 			/* filter_flag OFF	*/
			(ASL_RC *) diag_help);	/* Fake help routine	*/
	else
	  rc = asl_init((FILE *) NULL,		/* No screen log file	*/
			FALSE,			/* trace_log OFF	*/
			FALSE,			/* verbose_log OFF	*/
			FALSE,			/* debug_log OFF	*/
			TRUE, 			/* filter_flag ON	*/
			(ASL_RC *) diag_help);	/* Fake help routine	*/
	return(rc);
}


 long
diag_asl_read (screen_code,wait, buf)
ASL_SCREEN_CODE screen_code;
int        	wait;  	/* if TRUE wait for at least 1 char. before returning */
char            *buf;	/* buffer for returning input characters */
{
	return asl_read(screen_code,wait, buf);
}

#undef asl_beep
long diag_asl_beep(void)
{
	asl_beep();
	return(0L);
}

#undef asl_quit
long diag_asl_quit(name)
char *name;
{
	return(asl_quit() );
}

long diag_emsg(
	nl_catd	cat_fdes,
	unsigned short setid,
	unsigned short msgid,
	...)
{
	char		*text;
	char		*templine;
	int		rc;
	ASL_SCR_TYPE	hcmd_type;
	ASL_SCR_INFO	hmenu_info[3];
	va_list		ap;

	(void)memset(&hcmd_type,  0, sizeof(hcmd_type));
	(void)memset(hmenu_info, 0, sizeof(hmenu_info));
	text = diag_cat_gets(cat_fdes, setid, msgid);

	/* malloc enough memory for the catalog message	*/
	/* Estimate using the length of the text string */
	/* plus some for expansion of printf() formats.	*/
	templine = (char *)malloc(strlen(text) + 500);
	va_start(ap, msgid);
	rc = vsprintf(templine, text, ap);
	hmenu_info[0].text = " ";
	hmenu_info[1].text = templine;
	hmenu_info[2].text = " ";
	hcmd_type.screen_code = ASL_DIAG_DIALOGUE_HELP_SC;
	hcmd_type.max_index = 2;
	hcmd_type.cur_index = 1;
	rc = asl_screen(&hcmd_type, hmenu_info);
	free(templine);
	return(rc);
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
	char	*text;
	char	templine[2000];
	int	rc;
	va_list ap;

	text = diag_cat_gets(cat_fdes, setid, msgid);
	va_start(ap, msgid);
	rc = vsprintf(templine, text, ap);
	return(asl_note(ASL_INFORM_MSG,ASL_NO_LOG,templine));
}

char *diag_cat_gets(
	nl_catd	fdes,
	int setno,
	int msgno)
{
	nl_catd new_fdes = 0;
	nl_catd vendor_fdes = 0;
	char	*temp_ptr, *temp2, *tptr;
	char	fullpath_name[256];
	char	catfilename[256];
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

	/* if the full path is in the catalog file name then */
	/* strip its path, else take the name as is.         */
	if(strchr(fdes->_name, '/') == (char *)NULL)
		strcpy(catfilename, fdes->_name);
	else
	{
		strcpy(fullpath_name, fdes->_name);
		tptr = (char *)strtok(fullpath_name, "/");
		strcpy(catfilename, tptr);
		do
		{
			tptr = (char *)strtok('\0', "/");
			if(tptr) strcpy(catfilename, tptr);
		} while (tptr);
	}
	temp_ptr = catgets(fdes, setno, msgno, DIAG_CATERROR);
	/* if message not found - search another catalog */
	if (DIAG_CATERROR == temp_ptr)
	  {
	  if ((temp2 = getenv("DIAGNOSTICS")) == (char *)NULL)
	    sprintf(new_path, "%s/catalog/default/%s",
		    default_catdir, catfilename);
	  else
	    sprintf(new_path, "%s/catalog/default/%s",
		    temp2, catfilename);
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
	    if (!strcmp(catfilename, MF_DCDA))
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
		    			default_catdir, catfilename);
	  			else
				    sprintf(new_path, "%s/catalog/default/%s",
		    				temp2, catfilename);
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
  nl_catd new_fdes = 0;
  char	*temp_ptr, *temp2;
  char	*tptr=(char *)NULL;
  char  catfilename[256];
  char	new_path[512], fullpath_name[512];
  int	len;

	if (0 >= (int )fdes)
	  {
	  temp_ptr = (char *)malloc(strlen(def_str) + 1);
	  strcpy(temp_ptr, def_str);
	  return(temp_ptr);
	  }

	temp_ptr = catgets(fdes, setno, msgno, def_str);
	/* if message not found - search another catalog */
	if (!strcmp(def_str, temp_ptr))
	  {
	  if(strchr(fdes->_name, '/') == (char *)NULL)
		strcpy(catfilename, fdes->_name);
	  else
	    {
	        strcpy(fullpath_name, fdes->_name);
	  	tptr = (char *)strtok(fullpath_name, "/");
		strcpy(catfilename, tptr);
		do
		{
			tptr = (char *)strtok('\0', "/");
			if(tptr) strcpy(catfilename, tptr);
		} while (tptr);
            }
	  sprintf(new_path, "%s/%s", CFGMETHODSDIR, catfilename);
	  new_fdes = catopen(new_path, NL_CAT_LOCALE);
	  if (new_fdes > (nl_catd)0)
	    {
	    temp_ptr = catgets(new_fdes, setno, msgno, def_str);
	    len = strlen(temp_ptr);		/* Copy the catalog msg */
	    temp2 = (char *)malloc(len + 1);	/* to a new memory block*/
	    strcpy(temp2, temp_ptr);		/* so we can close the  */
	    catclose(new_fdes);			/* default catalog.	*/
	    temp_ptr = temp2;
	    }
	  }
	return(temp_ptr);
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
  char new_path[256];
  nl_catd fdes;
  int	oflag;

	oflag=NL_CAT_LOCALE;
	if(open_flag != 0)
		oflag=open_flag;

	fdes = catopen(filename, oflag);
	if ((-1 == (int )fdes) && (*filename != '/'))
	  {
	  sprintf(new_path, "%s/%s", CFGMETHODSDIR, filename);
	  fdes = catopen(new_path, oflag);
	  }

	return(fdes);
}

long diag_asl_msg(char *m1, ...)
{
	ASL_SCR_TYPE	hcmd_type;
	ASL_SCR_INFO	hmenu_info[3];
	char	templine[2000];
	int	rc;
	va_list ap;
	

	(void)memset(&hcmd_type,  0, sizeof(hcmd_type));
	(void)memset(hmenu_info, 0, sizeof(hmenu_info));
	va_start(ap, m1);
	rc = vsprintf(templine, m1, ap);
	hmenu_info[0].text = " ";
	hmenu_info[1].text = templine;
	hmenu_info[2].text = " ";
	hcmd_type.screen_code = ASL_DIAG_DIALOGUE_HELP_SC;
	hcmd_type.max_index = 2;
	hcmd_type.cur_index = 1;
	return(asl_screen(&hcmd_type, hmenu_info));

	/*return(asl_vnote(ASL_MSG,ASL_NO_LOG,m1,&ap));*/
}
