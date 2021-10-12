static char sccsid[] = "@(#)26        1.9  src/bos/usr/bin/copydump/copydumpmenu.c, cmddump, bos411, 9436E411a 9/9/94 14:30:56";
/*
 * COMPONENT_NAME: Menu for manual copy of dump
 *
 * FUNCTIONS:  main (copydumpmenu)
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
#include <unistd.h>
 */

/*              include file for message texts          */
#include <stdio.h>
#include <fcntl.h>
#include <odmi.h>
#include <string.h>
#include <locale.h>
#include <sys/dump.h>
#include <sys/utsname.h>
#include <sys/cfgodm.h>
#include <limits.h>
#include <nl_types.h>
#include <sys/ioctl.h>
#include <BosMenus_msg.h>
#include <Menus.h>
#include <sys/lft_ioctl.h>
#include <sys/wait.h>
#include "copydump.h"
#define SYSDUMP "/dev/sysdump"

#define ITEM_LEN 80
#define DEVICE_START 13
#define MAX_NUM_DEV (20 - DEVICE_START)
nl_catd catfd;
#define MSGSTR(n,s) catgets(catfd,MS_COPYDUMP,n,s)

#define CLEARLFT "^[[H^[[J\n"
#define CLEARTTY "\012\012\012\012\012\012\012\012\012\012\012\012"

struct dumpinfo dmp;                    /* dump information */

struct Menu CopydumpMenu;
struct Menu StatusScreen;
struct MessageBox msgbox;
	
char device[10][ITEM_LEN];    /* enough for 10 devices */
char Menu_head[600];
char Device_head[ITEM_LEN];
char Menu_item[11][ITEM_LEN];    /* enough for 10 lines*/
char status_str [ITEM_LEN];
char wait_str [ITEM_LEN];
char buf [256];
int device_index = 0;
int bosinst_flag = TRUE;
int lft = 0;
int copydump_called = FALSE;

struct Menu *CopydumpMenuDriver();

extern int errno;
extern int odmerrno;
#define TAPE_DEVICE "tape/scsi/*"
#define DISK_DEVICE "diskette/siofd/*"
/*
 * NAME: main copydumpmenu
 *
 * FUNCTION:
 *	Provides a memu to the user for copying the system dump
 * 	to a tape or diskette device. 
 *	This routine is called at
 *	1) system boot time if a previous attempt to copy the 
 *	   dump (via save_core) failed and if the forcecopydump
 * 	   attribute is set to TRUE.
 *
 *	2) Service time via Install and Maintenance medial.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called with root authority
 *
 */

main(argc,argv)
int argc; 
char *argv[];


{
	char *values;
	char *m = NULL;
        char crit[MAX_ODMI_CRIT];
	int ngot;
	struct CuDv *cudv ;       /* Customized device list */
	struct PdDv *pddv ;       /* Predefined device list */
	char   *devtype;
	struct listinfo stat_info;
	struct listinfo PdDv_stat;
	nl_catd catd2;
	int index=1;
	int i;
	int fd;
	int rv;


	/***************************************************************
	 Set up for ILS
	 ***************************************************************/

	setlocale(LC_ALL,"");
        catfd = catopen(MF_BOSMENUS,0);
	
	/***************************************************************
	 Check to see if it is invoked from rc.boot
	 Only the first argument is checked, the rest of them is ignored.
	 ***************************************************************/

	if (( argc > 1 ) && ( strcmp(argv[1],"-b") == 0 ) )
		bosinst_flag = FALSE;

	/***********************************************
	  We need to see if the console is an lft or tty
          so we know how to handle clearing the screen.
	  Only do this when called from rc.boot, because
	  otherwise bosinstall handles it for us.		
        ************************************************/
	if (!bosinst_flag)
		check_console_type();

	/***************************************************************
	 Get the dump information from NVRAM and check for previous dump
	 ***************************************************************/

	if ( rv = getdumpinfo(&dmp))
	{
	  /* i.e unable to get the info from NVRAM via /dev/sysdump
	   * return.
	   * bosinst will display the error message.  We should not
	   * encounter this error if this routine is invoked from
	   * bosboot.
	  */
	  return(1);
	}
	else
	{
	  if ((dmp.dm_size <= 0 ) ||
		(dmp.dm_devicename[0] == '\0'))
	  {
	   /*
	   *DisplayError("No previous dump exists\n");
	   * bosinst will display the error message.  We should not
	   * encounter this error if this routine is invoked from
	   * bosboot.
	   */
	    return(2);
	  }
	}
	/***************************************************************
	 Check for the location code to see if the dump device is a
	 logical volume. Only allowing copy if it is.
	 ***************************************************************/
	if ( dmp.dm_filehandle[0] == '\0' )
	  {
	   /*
	   * DisplayError("Dump is not in a logical volume. No copy is allowed\n");
	   * bosinst will display the error message.  We should not
	   * encounter this error if this routine is invoked from
	   * bosboot.
	   */
	    return(3);
	  }
	/***************************************************************
	 Get ready to display the menu.
	 ***************************************************************/
 	
	if ( odm_initialize () < 0 )
	{    
	  /*
	   * DisplayError("Unable to initialize ODM.\n");
	   * bosinst will display the error message.  We should not
	   * * encounter this error if this routine is invoked from
	   * bosboot.
	  */
	  return(4);
	}

	/***************************************************************
	 Initialize the menu title.
	 ***************************************************************/
	for ( i= 0; i<24; i++)
	  {
	     *Menu_item[i] = NULL;
	  }

	sprintf(Menu_head,
		MSGSTR(CAT_CD_HEAD,COPY_DUMP_HEADING),
		dmp.dm_size,dmp.dm_devicename);

	sprintf(Device_head,
		MSGSTR(CAT_HEAD2,
		"           Device Type                   Path Name"));

	/* Check for tape/diskette devices that the dump can be copied to */
	i = 0;

 	/* Check for tape devices */
        sprintf(crit,"PdDvLn LIKE '%s'",TAPE_DEVICE);
        cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,3,1);

	ngot = 0;
        while( (ngot < stat_info.num)   && &(cudv[ngot]) && (device_index < MAX_NUM_DEV) )
            {
		/* if device_index >= 7 we run out of room on the screen.
		   We assume there should not be more than 7 external backup
		   devices per machine.  If this assumption is incorrect,
		   we need to use the multiple screen.   
                */
		/* if status == 0 then device is defined but not available.
		   if status == 1 then device is defined and available.
		   In the bosinst environment, we should only check for
		   the available device since we won't be able to configure
		   the defined device. On the other hand, in bosboot env
		   we should be able to configure the defined devices.
		*/
		if (cudv[ngot].status == 1 || ( !bosinst_flag && (cudv[ngot].status == 0)))
	 	  {
		    strcpy(device[device_index++],cudv[ngot].name);
		    /* now we need to get the name of the device from catalog */
		    sprintf(crit,"uniquetype = '%s'",cudv[ngot].PdDvLn_Lvalue);
		    pddv = (struct PdDv *)odm_get_list(PdDv_CLASS,crit,&PdDv_stat,1,1);

		    /* open the catalog */
		    catd2 = catopen((char *)&(pddv->catalog),NL_CAT_LOCALE);
		    /* get the device name.  If unable to get the device
			name from the catalog then use the unique name
		 	that is in the PdDv data base
		    */
		    devtype = catgets(catd2,pddv->setno,pddv->msgno,
				      cudv[ngot].PdDvLn_Lvalue);
		    sprintf(Menu_item[i],
		    	"    %2d  %-30s   /dev/%s",index++,
			devtype,cudv[ngot].name);
		    i++;
		    /* close the message catalog */
		    catclose(catd2); 
		  }
		ngot++;
	    }
	/* Check for diskette devices */
        sprintf(crit,"PdDvLn LIKE '%s'",DISK_DEVICE);
        cudv = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,3,1);
	ngot = 0;
	while( (ngot < stat_info.num)   && &(cudv[ngot]) && (device_index < MAX_NUM_DEV) )
	{
		/* if status == 0 then device is defined but not available.
		   if status == 1 then device is defined and available.
		   In the bosinst environment, we should only check for
		   the available device since we won't be able to configure
		   the defined device. On the other hand, in bosboot env
		   we should be able to configure the defined devices.
		*/
		if (cudv[ngot].status == 1 || ( !bosinst_flag && (cudv[ngot].status == 0)))
		  { 
		    strcpy(device[device_index++],cudv[ngot].name);
		    sprintf(crit,"uniquetype = %s",cudv[ngot].PdDvLn_Lvalue);
		    pddv = (struct PdDv *)odm_get_list(PdDv_CLASS,crit,&PdDv_stat,1,1);
		    /* open the catalog */
		    catd2 = catopen(pddv->catalog,NL_CAT_LOCALE);
		    /* get the device name.  If unable to get the device
			name from the catalog then use the unique name
		 	that is in the PdDv data base
		    */
		    devtype = catgets(catd2,pddv->setno,pddv->msgno,
		                      cudv[ngot].PdDvLn_Lvalue);
		    sprintf(Menu_item[i],
		            "    %2d  %-30s   /dev/%s",index++,
		            devtype,cudv[ngot].name);
		    i++;
		    /* close the message catalog */
		    catclose(catd2);
		  }
		ngot++;
	}
	
	sprintf(Menu_item[7],
			MSGSTR(CAT_HELP,
			"    %2d  Help ?"),88);
	if (bosinst_flag)
		sprintf(Menu_item[8],
			MSGSTR(CAT_EXIT1,
			"    %2d  Previous Menu"),99);
	else
		sprintf(Menu_item[8],
			MSGSTR(CAT_EXIT2,
			"    %2d  Exit"),99);
	sprintf(Menu_item[10],
			MSGSTR(CAT_CHOICE,
			">>> Choice"),1);
	
	parse(&CopydumpMenu,0,&Menu_head,10);
	
	CopydumpMenu.Text[11] = (char *)&Device_head;

	for (i = 0; i < 11; i++) 
	{
	  CopydumpMenu.Text[DEVICE_START + i] = (char *)&Menu_item[i];
	}

	CopydumpMenu.driver = CopydumpMenuDriver;
	CopydumpMenu.preformat = 0;
	CopydumpMenu.Length = 24;
	CopydumpMenu.DefaultLine = 13;
	CopydumpMenu.Animate = 0;
	CopydumpMenu.MultipleSelect = 0;

	odm_terminate();
	if (!bosinst_flag)
		clear_screen();
 	DisplayMenu(&CopydumpMenu);
	return(0);
}


struct Menu *CopydumpMenuDriver(menu,input)
struct Menu *menu;
int *input;
{
	int rv;
	struct dumpinfo dump;
	int dumpfd;

	menu->Message = 0;  /* blank out the message box */

	if (*input == 99 )
		{
	  	dumpfd = open(SYSDUMP, 0);
	  	if (dumpfd < 0)
	  		{ 
	    		perror("copydumpmenu: open /dev/sysdump");
	    		return (0);
	    		}
	  	bzero(&dump, sizeof(dump));
	  	if ( ioctl(dumpfd, DMP_IOCSTAT2, &dump) < 0 )
	    		perror("copydumpmenu: ioctl(DMP_IOCSTAT2)");
	  	close(dumpfd);
	  	return (0);
	  	}

	if (*input == 88 )
	{
		if (!bosinst_flag)
			clear_screen();
		DisplayHelp(COPY_DUMP_HELP,CAT_CD_HELP);
		if (!bosinst_flag)
			clear_screen();
		DisplayMenu(menu);
		return(0);
	}

	if ((*input > 0) && (*input <= device_index))
		    {
		       	  sprintf(buf,"/usr/bin/copydump %s %d %s /dev/%s %d %d",dmp.dm_devicename,
				dmp.dm_size,dmp.dm_filehandle, &device[((*input) - 1)],
				bosinst_flag, copydump_called);
	
		       /* display the progress screen */
        	       StatusScreen.driver = NULL;
        	       StatusScreen.preformat = 0;
        	       StatusScreen.Length = 15;
        	       StatusScreen.DefaultLine = -1;
        	       StatusScreen.Animate = 1;
        	       StatusScreen.MultipleSelect = 0;

		       StatusScreen.AnimationString = "|/-\\";

		       sprintf(status_str,MSGSTR(STATUS_STR,
			"Copy in Progress from %s to /dev/%s\n"),
			dmp.dm_devicename,&device[((*input) - 1)]);

		       StatusScreen.Text[10] = (char *)&status_str;
		
		       sprintf(wait_str,MSGSTR(WAIT_STR,
			"Please Wait..."));
		
		       StatusScreen.Text[12] = (char *)&wait_str; 

		       if (!bosinst_flag)
		       	      clear_screen();	
		       DisplayMenu(&StatusScreen);

		       /****************************************************
			We need to make sure that we are the session leader 
			so that pax can handle console input and output.
		       ****************************************************/
	               setsid();
		       open("/dev/console",O_RDWR);

		       rv = system(buf);

		       /****************************************************
 	               Get the real exit status of the copydump command 
		       using the WEXITSTATUS macro defined in wait.h.
		       ****************************************************/
		       rv = WEXITSTATUS(rv);
			
		       /****************************************************
			We need to know if we call the copydump command more
		        than once from these menus in the bosinstall  
			environment.  This way we know whether or not to 
			import the rootvg and vary it on.	
		       ****************************************************/
		       if (bosinst_flag)	
		       		copydump_called = TRUE;	

		       StopAnim();
		       switch (rv) {
				case COPY_SUCCESS:
				  sprintf(buf, MSGSTR(CD_SUCCESS,
					"System Dump has been copied to /dev/%s.\n"),
					&device[((*input) - 1)]);
				  strcat(buf,MSGSTR(TAR_FORMAT,
					"After reboot, use the tar command to extract the dump.\n"));
				  break;
				case E_ODMINIT:
				  sprintf(buf, MSGSTR(ODMINIT_ERR,
					"Unable to intialize ODM.\n"));
				  break;
				case E_ODMGET:
				  sprintf(buf, MSGSTR(CANT_GET_PVNAME,
					"Unable to get the physical volume namefor\nthe dump device.\n"));
				  break;
				case E_IMPORTVG:
				  sprintf(buf, MSGSTR(CANT_IMPORTVG,
					"Unable to import the physical volume for\nthe dump device.\n"));
				  break;
				case E_VARYONVG:
				  sprintf(buf, MSGSTR(CANT_VARYONVG,
					"Unable to vary on the rootvg volume group\nto access the dump device.\n"));
			  	  break;
				case E_OPEN:
				  sprintf(buf, MSGSTR(OPEN_ERR,
					"Unable to open the dump device to read.\n"));
				  break;
				case E_LSEEK:
				  sprintf(buf, MSGSTR(LSEEK_ERR,
					"Unable to move the file pointer for the dump device.\n"));
				  break;
				case E_READ:
				  sprintf(buf, MSGSTR(READ_ERR,
					"Unable to read from the dump device.\n"));
				  break;
				
				case E_INVALID_DUMP:
				  sprintf(buf, MSGSTR(INVALID_DUMP,
					"No valid dump in the dump device.\n"));
				  break;
				case E_GET_DEVNAME:
				  sprintf(buf, MSGSTR(CANT_GET_DEVNAME,
					"Unable to get the device information for\nthe output device /dev/%s.\n"),
					&device[((*input) - 1)]);
				  break;
				case E_MKDEV:
				  sprintf(buf, MSGSTR(MKDEV_ERR,
					"Unable to configure the output device /dev/%s.\n"),
					&device[((*input) - 1)]);
				  break;
				default:
				  sprintf(buf,MSGSTR(COPY_ERR,
				  	"Fail to copy the system dump to the output device\n/dev/%s.\n"),
					&device[((*input) - 1)]);
				  sleep(3); /* sleep 5 seconds before display the previous menu */
				  break;
			
		       }
		       msgbox.Text = (char *)&buf;
		       menu->Message = &msgbox;
		       menu->DefaultLine = DEVICE_START + ((*input) - 1);
			if (!bosinst_flag)
		       		clear_screen();
		       DisplayMenu(menu);
		       return(rv);
	  	    }
	else
	      	    {
		       sprintf(buf,MSGSTR(CAT_INV_SEL,"Invalid Choice.  Please try again\n"));
		       msgbox.Text = (char *)&buf;
		       menu->Message = &msgbox;
			if (!bosinst_flag)
		       		clear_screen();
		       DisplayMenu(menu);
		       return(0);
	      	    }
}
/*
 * NAME: getdumpinfo
 *
 * FUNCTION: get dump infomation from the kernel by calling ioctl
 */
getdumpinfo(struct dumpinfo *dmp)
{
     int fd;

     fd=open(SYSDUMP,0);
     if (fd < 0 )
       {
          return(-1);
       }
     if (ioctl(fd,DMP_IOCINFO,dmp) == -1) {
          close(fd);
          return(-1);
       }
     close(fd);
     return(0);
}

/*
 * NAME: clear_screen
 *
 * FUNCTION: Clear the screen before displaying each new menu. 
 *	     This only needs to be done when we are NOT called
 *	     from bosinstall.  When called from bosinstall, the
 *	     clear is done for us.	
 */
clear_screen()
{
       /* This will write 24 blank lines to the screen.  */
	if (!lft)
		{
		fprintf(stdout,CLEARTTY);
		fprintf(stdout,CLEARTTY);
		}	
}

/*
 * check_console_type: Checks to see if console is tty or lft. 
 *             Sets global variable lft to true if it's an lft.
 */
int check_console_type()
{
  int		fd;
  int		rc = 1;
  lft_query_t	argptr;

	/*
	 * Open default console for read/write
	 */
	if( (fd = open ("/dev/console", O_RDWR)) < 0)
	{
	/* Assume it's a tty. */
		lft = 0;
		return;
	}

	/*
	 * The ioctl will fail if this is an async terminal.
	 */
	if( (ioctl(fd, LFT_QUERY_LFT, &argptr)) != -1)
	{
	/* it's an lft */
		lft = 1;
	}
	else
	{
	/* it's a tty */
		lft = 0;
	}

	close (fd);
}
