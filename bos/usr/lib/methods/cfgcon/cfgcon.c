static char sccsid[] = "@(#)25  1.14.1.9  src/bos/usr/lib/methods/cfgcon/cfgcon.c, sysxcons, bos411, 9428A410j 12/16/93 12:22:20";
/*
 * COMPONENT_NAME: CFGMETHODS  CONSOLE configuration routine
 *
 * FUNCTIONS: main, make_special_file, errexit
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	This is the console device config routine. The basic flow
	of this routine is:

	Read default console device from ODM database. The value will be
	contained in ODM as a Customized Attribute for the system object.

	If a value is found in ODM then
	1) run "console finder" routine to find value for console device
	2) put this value in the ODM for next time

	Set default console device via a sysconfig() call to console
	device driver.

		Create /dev/console special file if necessary.
*/

#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <fcntl.h>
#include <termio.h>
#include <stdio.h>
#include <cf.h>
#include <strings.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/console.h>
#include <sys/audit.h>
#include <sys/lft_ioctl.h>
#include <locale.h>

#include "console_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_CONSOLE, Num, Str)

#define CONMAJOR 4
#define CONMINOR 0
/* defines for servmode variable */
#define SERVMODE 0x1	/* system in SERVICE mode flag */
#define CONSDEFINED 0x2 /* console is currently defined */
#define MKDEV	"/usr/sbin/mkdev"

/* external functions */

extern int findcons();
extern int setterm();
extern setleds();
int servmode;				/* service mode flag */
extern	char console_path[];		/* console device pathname */
extern	char console_physdisp[];	/* lft physical display name */
extern	int term_choice;		/* lft physical display number */

int exit_on_fail = 0;	/* don't find console if not already defined in ODM */
int set_con_null = 0;	/* set console back to NULL */
int cfgcondev(),ucfgcondev();

/*
  Set permissions for special file
*/

#define FPERM	S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define MODE 	FPERM | S_IFCHR

/* command strings for inittab file updates */
#define ONSTRING	"\"cons:0123456789:respawn:/usr/sbin/getty /dev/console\""
#define OFFSTRING	"\"cons:0123456789:off:/usr/sbin/getty /dev/console\""

char	*devpath = "/dev/console";	/* pathname of special file */

/*
 * NAME: main
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */


main( argc, argv)
int argc;
char *argv[];
{

	struct cfg_dd cfg;	/* system sysconfig command structure */
	struct cons_config con_cfg;	/* console sysconfig parameter */
	char	errstring[128];		/* used for error messages */
	char	crit[128];		/* search criteria */
	char	temp[128];
	struct CuAt *cuat;	/* customized attribute object pointer */
	struct CuDv cudv;	/* customized device object storage */
	struct PdDv pddv;	/* predefined device object storage */
	struct PdAt pdat;	/* predefined attribute object storage */
	struct Class cudv_class;	/* Class object */
	dev_t	devno;			/* device number for config_dd */
	long	majorno, minorno;	/* devno of /dev/console */
	struct	stat stat_buf;		/* used for stat() system call */
	int	rc,c,rc1;			/* return codes */
	int find_console;		/* boolean variable */
	int	fildes;			/* file descriptor for open() */
	int	fildes2;		/* file descriptor for open() */
	char  *outp, *errp;	/* stdout and stderr for odm_run_methods */
	int i, login, consfile;
	struct objlistinfo cuat_info;	/* result of search stats */


	setlocale(LC_ALL, "");
	catd = catopen(MF_CONSOLE, NL_CAT_LOCALE);
	consfile = FALSE; 	/* default to console is not a file */
	login = TRUE;		/* default to login requested on console */

	while( (c= getopt(argc, argv,"cu")) != EOF ) {
		switch (c) {
		  case 'c' :	if( set_con_null) exit(1);
				exit_on_fail = 1; break; /* -c and -u only valid for phase 1 */
		  case 'u' :	if( exit_on_fail) exit(1);	
				set_con_null = 1; break; 
		  default: ;		/* do nothing for other flags - the way it used to be. */
		}
	}

	/*
	   Start up odm.
	*/
	if ((rc = odm_initialize()) < 0)
	{
		fprintf(stderr,MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"));
		exit(E_ODMINIT);
	}

	/* if exit_on_fail or set_con_null is set, make sure it is not runtime */ 
	if( exit_on_fail || set_con_null ) {
		if( (rc = odm_get_first(PdAt_CLASS,PHASE1_DISALLOWED, &pdat)) != NULL )
			exit(1);	
	}

	/*
	  Get default console attribute.
	 */
	if (( cuat = getattr("sys0", "conslogin", FALSE, &rc ))
	    == NULL )
		errexit(MSGSTR(CFCONE02,"cfgcon: failure accessing the sys0 object in ODM \n"),E_NOCuOBJ);
	if (strcmp(cuat->value, "disable") == 0)
		login = FALSE;

/* get system boot keylock position (service or normal) and set mode */
	if (( cuat = getattr("sys0", "keylock", FALSE, &rc )) != NULL )
		if (strcmp(cuat->value, "normal") == 0)
			servmode = 0;
		else
			servmode = SERVMODE;

	/*
	  Get the console device pathname from the ODM. The console
	  device pathname is a customized attribute belonging to
	  the system object.
	  The getattr() routine searches for the attribute first
	  in the customized attribute class and if nothing is found
	  there, then searches the predefined attribute class.
	*/

	if (( cuat = getattr("sys0", "syscons", FALSE, &rc )) == NULL )
		errexit(MSGSTR(CFCONE02,"cfgcon: failure accessing the sys0 object in ODM \n"),E_NOCuOBJ);

#ifdef CFGDBG
printf("syscons attribute: %s\n", cuat->value);
#endif

	/*
	  "Console" attribute was found. Now see if the value returned
	  was a non-empty string. If non-empty, see if the status of
	  this object in the database is marked AVAILABLE. Since the
	  value is a pathname, must strip off the "/dev/" portion of
	  the string.
	  If not AVAILABLE, run "console finder" to assign a default
	  console.

	  If the value returned was an empty string, must also run
	  "console finder".
	*/

	find_console = TRUE;
	if ( strlen(cuat->value) != 0 )
	{
		servmode |= CONSDEFINED;  /* set console defined flag */
		/*
		  Check to see if the file is a special file.
		  If login is not enabled and the file does not exist or is
		  not a character special file, then do not need to
		  check ODM for AVAILABLE status or perform "console finder"
		  routine.
		*/

		rc = stat(cuat->value, &stat_buf);
		if ( (((rc == 0) && !S_ISCHR(stat_buf.st_mode)) ||
			((rc != 0) && (errno == ENOENT))) && !login )
		{	/* not a character special file or doesn't exist */
			strcpy(console_path, cuat->value);
			find_console = FALSE;
			consfile = TRUE;  /* assume console is a file */
			setleds(0xA34);
			if( exit_on_fail || set_con_null ) exit(1);
		}
		else if ((rc == 0) || ((rc!=0) && (errno == ENOENT)) )
		{
			sprintf(crit, "name = '%s'", cuat->value + 5);
			if( (rc = odm_get_first(CuDv_CLASS, crit, &cudv)) == -1 )
				errexit(MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"),E_NOCuDv);

			/*  If no lft or tty info in CuDv and 
			    exit_on_fail is set, don't proceed */	

			if ( !rc && (exit_on_fail || set_con_null) ) exit(1);  
			
			if ( rc && set_con_null) {  /* set console to null and unconfig lft */
				if( cudv.status == AVAILABLE )
				    ucfgcondev( &cudv ); 
				else
				    exit(0);	/* already unconfigured */
			}

			if ( rc && cudv.status != AVAILABLE ) /* config defined device */
			{
				rc1 = cfgcondev( &cudv, cuat ) ;
				if ( rc1 && exit_on_fail ) exit(1);
				if ( !rc1 )  {
				    find_console = FALSE;	
				    rc = odm_get_first(CuDv_CLASS, crit, &cudv);
				}
			}


			if ( ((rc == 0) && !login) || (cudv.status ==
				 AVAILABLE))
			{
				strcpy(console_path, cuat->value);
				find_console = FALSE;
				if ( strncmp("lft", cuat->value + 5, 3) == 0 )
					setleds (0xA32);
				else
					setleds (0xA33);
			}
		}
	}
	else 
	    if (set_con_null) exit(0); /* no console defined, quit */


	if ( find_console == TRUE )
	{
		/*
		  The routine findcons() will drive the process of finding
		  a terminal which will be the next default display.
		*/

#ifdef CFGDBG
printf("service mode = %d\n", servmode);
#endif
		rc = findcons();
#ifdef CFGDBG
printf("service mode = %d\n", servmode);
#endif
		if ( rc != 0 )
		{
			errexit(MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"),E_NOCuOBJ);
		}
		if (servmode != -1)
		{	/* console finder found a console */
#ifdef CFGDBG
printf("console_path = /dev/%s\n", console_path);
printf("console_physdisp = %s\n", console_physdisp);
#endif
			sprintf(cuat->value, "/dev/%s", console_path);
			strcpy(console_path, cuat->value);
			if ( strncmp("lft", cuat->value + 5, 3) == 0 )
				setleds(0xA32); 	/* show lft selected */
			else
				setleds (0xA33);	/* show tty selected */

			if ((rc = putattr(cuat)) < 0 )
				errexit(NULL,E_ODMUPDATE);

			/*
			  If the console choice was one of the lft displays,
			  setup the default display attribute and
			  notify the lft driver.
			  */

			if ( strncmp("lft", cuat->value + 5, 3) == 0 )
			{
				if (( cuat = getattr("lft0", "default_disp",
						FALSE, &rc )) == NULL )
					errexit(NULL,E_ODMGET);
				strcpy(cuat->value, console_physdisp);
				if ((rc = putattr(cuat)) < 0 )
					errexit(NULL,E_ODMUPDATE);
				fildes = open("/dev/lft0", O_RDWR);
				if ( fildes > 0 )
				{
					rc = ioctl(fildes, LFT_SET_DFLT_DISP,
						console_physdisp);
					close (fildes);
				}
				if ( rc < 0 || fildes < 0)
					fprintf(stderr,MSGSTR(CFCONE04,"cfgcon: failed to set physical display on LFT \n"));
			}
		}
	}	/* end of find_console == TRUE */


	majorno = CONMAJOR;
	minorno = CONMINOR;
	devno = makedev(majorno, minorno);
	/*
	  Make special file for the console.
	*/

	make_special_file(devno);

	/*
	  Now send the default console pathname down to the console
	  device driver via a sysconfig() call.
	*/

	if (servmode != -1)
	{
		rc = stat(console_path, &stat_buf);
		/* if console selection is a tty device then set
		   CLOCAL mode to prevent block when console is a tty that
		   is broken or disconnected or off
		 */
		fildes = -1;
		if ( (rc == 0) && S_ISCHR(stat_buf.st_mode))
		{
#ifdef CFGDBG
printf("setting tty modes for:  %s\n", console_path);
#endif
			if((fildes = open(console_path, O_WRONLY|O_NDELAY))
			     >= 0) {
				 if (isatty(fildes))
				    rc = setterm(fildes,CLOCAL);
				 else
				    close(fildes);	/* printer */
			}
		}
		cfg.kmid = 0;
		cfg.devno = devno;
		cfg.cmd = CONSOLE_CFG;
		con_cfg.cmd = CONSETDFLT;
		cfg.ddsptr = (char *)&con_cfg;
		con_cfg.path = (char *)console_path;
		cfg.ddslen = sizeof(con_cfg);

		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1)
			fprintf(stderr,MSGSTR(CFCONE05,"cfgcon: switch command to console driver failed \n"));
		/*
		  Now activate the default console.
		 */

		con_cfg.cmd = CONS_ACTDFLT;
		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1)
			fprintf(stderr,MSGSTR(CFCONE05,"cfgcon: switch command to console driver failed \n"));
		/* turn of possible debug mode */
		con_cfg.cmd = CONS_NODEBUG;
		sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd ));
		if (fildes != -1)
		{
			if((fildes2 = open("/dev/console", O_WRONLY)) >= 0)
				close(fildes2);
			close(fildes);
#ifdef CFGDBG
printf("closing console device  %s\n", console_path);
#endif
		}
	}


	/*
	  If console was found then setup inittab to support correct login
	  state otherwise disable login on console.
	*/
		/**********************************
		  Check the login attribute value
		 **********************************/
		if ( (login== FALSE) || (consfile == TRUE) || (servmode == -1))
		{
			odm_run_method("chitab",OFFSTRING,&outp,&errp);
		}
		else
		{
			odm_run_method("chitab",ONSTRING,&outp,
				&errp);
		}
	/*
	  Check ODM database for "tmpcons" attributes. If it exists, then
	  compare value to the name about to be sent to device driver. If
	  it matches, remove from database. Otherwise, unconfig , undefine,
	  and remove it from database. Please note there can be more than
	  one ttys with "tmpcons" attribute set. One of them will be
	  selected as console.
	*/

	cuat = (struct CuAt *)odm_get_list(CuAt_CLASS,
		"name = 'sys0' AND attribute = 'tmpcons'", &cuat_info,1,1);

#ifdef CFGDBG
printf("tmpcons attribute is = %s\n", cuat->value);
#endif
	if (cuat == (struct CuAt *)-1)
		errexit(MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"),E_ODMDELETE);

	for (i=1; (i<=cuat_info.num) && (servmode != -1); i++)
	{

		if ( strcmp(console_path + 5, cuat->value) == 0 )
		{
		/*
		  For a match, just remove this object from database.
		*/
#ifdef CFGDBG
printf("tmpcons matched - deleted \n");
#endif
			sprintf(crit,
			  "name = 'sys0' AND attribute = 'tmpcons' AND value = '%s'", cuat->value);
			if (odm_rm_obj(CuAt_CLASS, crit) < 0)
				errexit(MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"),E_ODMDELETE);
		}
		else
		{
			/*
			  No match found. Get customized device for this object
			  from ODM.
			*/
			sprintf(crit, "name = '%s'", cuat->value);
			rc = odm_get_first(CuDv_CLASS, crit, &cudv);
			if ( rc == -1 )
				errexit(NULL,E_NOCuDv);
			if ( rc != NULL )
			{
#ifdef CFGDBG
printf("cudv.name = %s\n", cudv.name);
#endif
				/*
				  Get predefined device for this object
				  from ODM.
				*/

				sprintf(crit, "uniquetype = '%s'",
					cudv.PdDvLn_Lvalue);
				rc = odm_get_first(PdDv_CLASS, crit, &pddv);
				if ( rc == -1 )
					errexit(NULL,E_NOPdDv);

				if ( rc != NULL )
				{
					/*
					  If device is configured, unconfigure
					  and undefine it. Otherwise,
					  just undefine it.
					*/
					sprintf(crit, "-l %s", cuat->value);
					if ( cudv.status == AVAILABLE )
					{
#ifdef CFGDBG
printf("unconfiguring device \n");
#endif
						rc = odm_run_method(
							pddv.Unconfigure, crit,
							&outp, &errp);
						/* Don't want to quit if method is
						   not found */
						/* if ( rc != 0 )
							errexit(NULL,
								E_ODMRUNMETHOD); */
						rc = odm_run_method(
							pddv.Undefine, crit,
							&outp, &errp);
					}
					else
					{
#ifdef CFGDBG
printf("undefining device \n");
#endif
						rc = odm_run_method(
							pddv.Undefine, crit,
							&outp, &errp);
					}
				/* if temporary console device was deleted
				   then remove attribute, otherwise leave */
					sprintf(crit, "name = '%s'",
						 cuat->value);
					rc = odm_get_first(CuDv_CLASS, crit,
						 &cudv);
					if ( rc == NULL )
					{
#ifdef CFGDBG
printf("tty removed - deleting tmpcons \n");
#endif
						sprintf(crit, "name = 'sys0' AND attribute = 'tmpcons' AND value = '%s'", cuat->value);
						if (odm_rm_obj(CuAt_CLASS,
							 crit) < 0)
							errexit(MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"),E_ODMDELETE);
					}
				}
			}

		}	/* No match found. */
		cuat++;
	} /* end for */

	/*
	  Terminate ODM and exit with success.
	*/
	odm_terminate();
	if( servmode == -1)
		exit(E_NODETECT);
	else
		exit(E_OK);

}


/*
 * NAME: make_special_file
 *
 * FUNCTION: Routine creating the console special file in /dev directory.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *   0 - success
 *  -1 - failure
 */
int
make_special_file(devno)
dev_t devno;				/* major/minor number */
{

	struct  stat buf;

	if (mknod(devpath,MODE,devno))
	{
		/*
		  See what error was.
		*/
		if (errno != EEXIST)
			return(1);		/* error other than "does not exist" */

		if (stat(devpath,&buf))
			return(2);		/* cannot perform stat call */

		if ( major(buf.st_rdev) == major(devno) &&
			minor(buf.st_rdev) == minor(devno) )
		{
			/*
			  major/minor #s are same--leave special file alone.
			*/
			return(0);
		}

		/*
		  Unlink special file name.
		*/
		if (unlink(devpath))
			return(3);

		/*
		  Try mknod again.
	 	*/
		if ( mknod(devpath, MODE, devno) )
		{
			return(4);
		}
	}

	chmod(devpath, MODE);

	return(0);
}


errexit(errstring,errcode)
char	*errstring;
char	errcode;
{
	if (errstring != NULL)
		fprintf(stderr, errstring);

	/*
	  Terminate ODM.
	*/
	odm_terminate();

	exit(errcode);
}

cfgcondev( cudv,cuat)
struct CuDv *cudv;
struct CuAt *cuat;
{
	char criteria[128];
	char cmd[128];
	char param[128];
	struct CuAt *cuat_disp; 
	struct CuDv cudv_kbd;
	struct PdAt *pdat_kbd;
	struct PdDv pddv;
	char  *outp;		/* stdout for odm_run_methods */
	int  rc, i;
	int kbd_found=0;
	struct objlistinfo pdat_info;	/* result of search stats */

       strcpy(cmd,MKDEV);
       if( strncmp("lft",cudv->name,3) == 0 )  /* lft console */
       {
		if( ( cuat_disp = (struct CuAt *) getattr(cudv->name,"default_disp",FALSE, &rc) ) == NULL )
			return( -1 );
#ifdef CFGDBG
printf(" Default display=%s \n",
cuat_disp->value);
#endif
		/* configure display adapter */
		sprintf(param," -l %s -R",cuat_disp->value);
		if( rc = odm_run_method(cmd,param,&outp,NULL)) 
			return(rc);
#ifdef CFGDBG
printf("display configured: %s %s\n", cmd,param);
#endif
		fprintf(stdout,"%s\n",cuat_disp->value);

	/* Look for the keyboard.  First, get a list of keyboards from PdAt */
	/* with attribute=sys_kbd, then look in CuDv for each uniquetype    */
	/* and try to configure.  If config fails, try the next one.	    */

		pdat_kbd = (struct PdAt *)odm_get_list(PdAt_CLASS, 
		    "attribute='sys_kbd'",&pdat_info,1,1);
		for (i=0; i<pdat_info.num; i++) {
			sprintf(criteria, "PdDvLn='%s'", pdat_kbd++->uniquetype);
			if( (rc = odm_get_first(CuDv_CLASS, criteria, &cudv_kbd)) < 0)
				return(-1);
			if (rc) {	/* try to configure keyboard */
#ifdef CFGDBG
printf("try to configure keyboard: %s \n", cudv_kbd.name);
#endif
				sprintf(param," -l %s -R",cudv_kbd.name);
				if( 0 == odm_run_method(cmd,param,&outp,NULL) ) {
					kbd_found++;
					break;
				}
			}
		}
		if (!kbd_found) return(-1);
#ifdef CFGDBG
printf("keyboard configured: %s %s\n", cmd,param);
#endif
		sprintf(param," -l %s -R", cudv->name);
       }
       else {
		/*  tty	- configure the parent first since mkdev on tty directly will update inittab. */
		sprintf(param," -l %s -R ",cudv->parent);
		if( rc = odm_run_method(cmd,param,&outp,NULL) ) 
			return(rc);
#ifdef CFGDBG
printf("tty parent configured: %s %s\n", cmd,param);
#endif
		sprintf(criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue);
	        if( odm_get_first(PdDv_CLASS, criteria, &pddv) <= 0)
			return(-1);
		sprintf(cmd,"%s ",pddv.Configure);
		sprintf(param," -l %s -1",cudv->name);
		fprintf(stdout,"%s\n",cudv->name);
	}

	if( rc = odm_run_method(cmd,param,&outp,NULL)) {
#ifdef CFGDBG
printf("console device configure failed:%s %s\n%s\n",
	cmd, param, outp);
#endif
			return(rc);
	}
#ifdef CFGDBG
printf("console device configured:%s %s\n",cmd, param);
#endif
	return(0);	
}

ucfgcondev (cudv)
struct CuDv *cudv;
{
	struct cfg_dd cfg;	/* system sysconfig command structure */
	struct cons_config con_cfg;	/* console sysconfig parameter */
	int    rc;
	char   cmd[128], criteria[128], param[128];
	struct PdDv pddv;

	cfg.kmid = 0;
	cfg.devno = makedev(CONMAJOR, CONMINOR);
	cfg.cmd = CONSOLE_CFG;
	cfg.ddsptr = (char *)&con_cfg;
	cfg.ddslen = sizeof(con_cfg);

	/* Set console to NULL */
	con_cfg.cmd = CONS_NULL;
	if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1)  {
		rc = errno;
		fprintf(stderr,MSGSTR(CFCONE05,"cfgcon: switch command to console driver failed \n"));
        }

	/*  Re-enable the debug mode */
	con_cfg.cmd = CONS_DEBUG;
	rc = sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )); 

	/* Now run unconfig method if it is lft so that newroot would not hang */	
	if( ! strncmp(cudv->name,"lft",3)) {
		sprintf(criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue);
        	if( (rc = odm_get_first(PdDv_CLASS, criteria, &pddv)) <= 0 )
		exit(1);
		sprintf(cmd,"%s ",pddv.Unconfigure);
		sprintf(param,"-l %s", cudv->name);
#ifdef CFGDBG
printf("console unconfigured: %s %s\n",cmd,param);
#endif
		rc = odm_run_method(cmd,param,NULL,NULL);
	}
#ifdef CFGDBG
printf("ucfgcondev exit=%d\n",rc);
#endif
	exit(rc);
}
