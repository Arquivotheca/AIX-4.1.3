static char sccsid[] = "@(#)64 1.32  src/bos/usr/lib/methods/common/findcons.c, sysxcons, bos411, 9428A410j 94/07/01 17:11:01";
/*
 * COMPONENT_NAME: (CFGMETHODS) System Configuration Console Finder 
 *
 *   FUNCTIONS: MSGSTR
 *              dstruct
 *              findcons
 *              freeall
 *              get_disp_devs
 *              get_num_lfts
 *              poll_displays
 *              setterm
 *              write_lft_prompt
 *              write_prompts
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:        findcons
 *
 * FUNCTION:
 *		This subroutine queries all display devices to determine
 *		which one will be the default for output and input during the
 *		installation process.  It does this by displaying a message on
 *		each terminal device and polling each until one responds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *			The findcons subroutine is called from the
 *			 console config routine.
 *
 */

#define _KERNSYS
#define _RSPC
#include <sys/systemcfg.h>
#undef _RSPC
#undef _KERNSYS


#include <stdio.h>
#include <sys/lft_ioctl.h>
#include <sys/errno.h>
#include <termio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <nl_types.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/mdio.h>

#include "console_msg.h"
extern nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_CONSOLE, Num, Str)

/* system keylock defines */
#define KEYMASK 0x00000003
#define SECURE  1
#define SERVICE 2
#define NORMAL  3
/* servmode flag defines */
#define SERVMODE 0x1	/* system in SERVICE mode flag */
#define CONSDEFINED 0x2 /* console is currently defined */

#define MAX_LFTS 12             /* Max supported physical displays (F1-F12) */
int num_lfts;                   /* contains the total number of lft's */

extern int setleds();
extern int servmode;		/* service mode flag from cfgcon */ 
int	term_choice;		/* terminal number chosen as console */

char  console_physdisp[16]="";  /* console physical display name */
struct {
	char name[16];
} physdisp_lft[MAX_LFTS];       /* names of lft displays */
char console_path[32]="";       /* pathname of default console device */
int	runtime;		/* runtime execution mode for LIC prompt */

/*
	The following structure is a doubly linked list that will contain
	the device name of all tty's and lft displays on the system.
*/

struct terminals {
	int		fd;		/* file descriptor to open device */
	char	dispdev[16];            /* device name of tty/lft */
	struct	terminals *next;        /* pointer to next struct */
	struct	terminals *last;        /* pointer to last struct */
} *topterm;

char    * getenv();

int     totdisps;                      /* Total number of displays           */
char    strbuf[800];

/*
 *     findcons():
 *     Open all available tty/lft devices
 *     Write message to all tty/lft devices
 *     Wait for input from any tty/lft device
 *     Copy pathname of chosen device to console_path
 *     If lft responded, copy physical display id to console_physdisp
 */
int 
findcons()
{
	int	fildes;   	/* file descriptor of standard out */
	int	count;    	/* loop index */
	int	retcode = 0;   	/* return value of findcons */
	int	rc;            	/* intermediate return values */
	int	default_lft;   	/* the number associated with lft */
	struct	PdAt	*pdatp; /* pointer to Predefined */
	struct terminals *curterm;	/* used to traverse linked list */
	struct listinfo stat_info;
	char crit[128];

	/* check on  RUNTIME flag */
     	sprintf(crit,PHASE1_DISALLOWED);
	pdatp = (struct PdAt *)odm_get_list(PdAt_CLASS,crit,&stat_info,1,1);
	if (((int)pdatp != -1) && ((int)pdatp != NULL))
		runtime = TRUE;
	else
		runtime = FALSE;

	/* Determine the total number of lft displays available.  */
	num_lfts = get_num_lfts();

	/* Construct a linked list of all available display devices.  */
	retcode = get_disp_devs();

#ifdef _DEBUG
        dstruct(topterm);
#endif

	/* Traverse the linked list of terminal names, write a prompt.  */
	if (retcode == 0)
		retcode = write_prompts();

	/* Poll input from all display devices.  */
	if ((retcode == 0) && (servmode != -1))
		poll_displays();

	/* Close all terminals and get ready to get out of here */
	for(curterm = topterm;curterm != NULL; curterm = curterm->next)
		close(curterm->fd);

	/* Free all of the nodes in the linked list. */
	freeall(topterm);

	return ( retcode );
} /* findcons */



/*
   freeall() is a recursive function which frees all of the nodes of a linked
             list by recursively traversing the list to the end and then by
             freeing the nodes from the end backwards.
*/

freeall(top)
struct terminals *top;
{
#ifdef _DEBUG
        printf("freeall\n");
#endif
	if ( top != NULL )
	{
		freeall(top->next);
		free(top);
	}
}


/*
   get_disp_devs:  Constructs a linked list of available display devices.
*/

int
get_disp_devs()
{
	struct terminals *curterm, *nexterm; /* used to create linked list */
	struct CuDv cudv;		     /* Pointer to query results   */
	struct CuAt *cuatp;		/* Pointer to query results   */
	struct objlistinfo cuat_info,cudv_cinfo; /* Results of search stats  */
	int	rc;
	int i;
	char  tbuf[80];			/* Temp buffer to build args  */

#ifdef _DEBUG
        printf("get_disp_devs\n");
#endif

	topterm = NULL;                 /* No list yet set top to NULL    */

	/* initialize ODM */

	odm_initialize();

	if (num_lfts)		/* were any found in get_num_lfts? */
	{
		/* look for an lft defined in the database */
		rc = odm_get_first(CuDv_CLASS, "name = 'lft0'", &cudv);
		if ( rc < 0 )
		{
			fprintf(stderr,MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"));
			return(4);
		}

		/* LFT found on system. */
		if ((rc > 0) && (cudv.status == AVAILABLE))
		{
			nexterm = (struct terminals *) malloc(sizeof(
					struct terminals));

			if ( nexterm == NULL )
			{
				fprintf(stderr,MSGSTR(CFCONE07,"cfgcon: a system failure has occurred (malloc failure) \n"));
				return(4);
			}

		if ( topterm == NULL )		/* First record in the chain */
			{
				topterm = nexterm;	/* Point topterm to chain    */
				nexterm->last = NULL;	/* Set top of chain pointer  */
			}
		else
			{
				curterm->next = nexterm; /* Set pointer to last  */
				nexterm->last = curterm; /* Set pointer to next  */
			}

			/*
			  Make new record current, put lft onto the list and
			  ensure last is NULL.
			*/
		curterm = nexterm;
		strcpy(curterm->dispdev, "lft0");
		curterm->next = NULL;

		}
	}

	/*
	  Check ODM database for "altcons" attributes. Check if these
	  devices are AVAILABLE, if so then include
	  it in the list of candidates.
	*/

	cuatp = (struct CuAt *)odm_get_list( CuAt_CLASS,
		"name = 'sys0' and attribute = 'altcons'",
                 &cuat_info,1,1);
	if (cuatp == (struct CuAt *) -1)
	{
		fprintf(stderr,MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"));
		return(4);
	}

#ifdef _DEBUG
                printf("cuat_info.num = %d\n", cuat_info.num);
#endif
	/*
	  For all the ttys for whom "altcons" attribute is set, check
	  if the device is avilable then add it to the list.
	  Remove the "altcons" attribute from the database.
	*/

	for( i = 1; i <= cuat_info.num; i++ )
	{ 
		sprintf(tbuf, "name = '%s'",
			 cuatp->value);
		cuatp++;
#ifdef _DEBUG
	printf("criteria = %s\n", tbuf);

#endif

		rc = odm_get_first(CuDv_CLASS, tbuf, &cudv);
		if ( rc < 0 )
		{
			fprintf(stderr,MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"));
			return(4);
		}

		/* tty was found on current serial adapter. */

		if ( rc != NULL && cudv.status == AVAILABLE)
		{
			nexterm = (struct terminals *) malloc(sizeof
					(struct terminals));
			if (nexterm == NULL)
			{
				fprintf(stderr, "malloc failed.\n");
				return(5);
			}

			if(topterm == NULL)
			{
				topterm = nexterm;
				nexterm->last = NULL;
			}
			else
			{
				curterm->next = nexterm;
				nexterm->last = curterm;
			}

			curterm = nexterm;
			strcpy(curterm->dispdev, cudv.name);
			curterm->next = NULL;
#ifdef _DEBUG
	printf("cudv->name = %s\n", cudv.name);
#endif
			/* Remove this attribute from the CuAt db*/
			sprintf(tbuf, 
				"name = 'sys0' AND attribute = 'altcons' AND value = '%s'", cudv.name);

			if (odm_rm_obj(CuAt_CLASS, tbuf) < 0) {
				fprintf(stderr,MSGSTR(CFCONE01,"cfgcon: failure accessing the device database (ODM) \n"));
				return(6);
			}
		}

	}  /* for */

	/* All done with odm */
	odm_terminate();
	return (0);
}


/*
	get_num_lfts:  query lft and return number of lft displays attached.
*/
get_num_lfts()
{
	int i, fd, rc;
	int num=0;
	lft_query_t lftq;
	lft_disp_query_t lftdq;
	lft_disp_info_t lft_disps[MAX_LFTS];

	setsid();
	if( (fd = open("/dev/lft0", O_RDWR | O_NDELAY)) > 0 )
	{
		rc = ioctl(fd, LFT_QUERY_LFT, &lftq);
		if (rc) {
#ifdef _DEBUG
	fprintf(stderr,
		"ioctl LFT_QUERY_LFT failed with return code %d and errno %d\n",
			rc, errno);
#endif
			close(fd);
			return(num);
		}
		num = (lftq.number_of_displays > MAX_LFTS) ? 
		    MAX_LFTS : lftq.number_of_displays;
	}
	else
	{
#ifdef _DEBUG
		fprintf(stderr,
		  "open of /dev/lft0 failed with error %d\n", errno);
#endif
		return(num);
	}
	
	if (num)
	{
	/*
	  There is an lft, so fill the physdisp_lft[] array with all
	  of the physical display names of the attached lft displays.
	*/
		lftdq.num_of_disps = num;	
		lftdq.lft_disp = &lft_disps[0];
		rc = ioctl(fd, LFT_QUERY_DISP, &lftdq);
		if (rc)
		{
#ifdef _DEBUG
	fprintf(stderr,
		"ioctl LFT_QUERY_DISP failed with rc %d and errno %d\n",
			rc, errno);
#endif
			close(fd);
			return(0);
		}
		for (i=0; i<num; i++)
			strcpy(physdisp_lft[i].name,
			    lft_disps[i].disp_name);
	}
	close(fd);
#ifdef _DEBUG
        printf("number of lft's = %d\n", num);
	for (i=0; i<num; i++)
	    printf("display %d name: %s\n", i, physdisp_lft[i].name);
#endif
	return(num);
}


/*
   poll_displays:  Read from all terminal devices.  When one returns input,
                   assign pointer to that device name and break.
*/
int
poll_displays()
{
	char	*format;
	char	fbuf[3];		/* function key buffer */
	int		notdone;   /* binary flag to specify end of loop */
	int		rc;
	int		i, j, tty_choice;
	int		timeout = 30; /* 30 second timeout for console select*/
	MACH_DD_IO mddRecord;
	ulong currkey, startkey;
	int fdmdd = -1;			/* file descriptor for /dev/nvram */

	struct terminals *curterm;
	char  *clear_screen = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	char  *lic1prompt, *lic2prompt;

 	lic1prompt = MSGSTR(CFGCLIC1,"\n\r HARDWARE SYSTEM MICROCODE\n\r Licensed Internal Code - Property of IBM \n\r (C) Copyright IBM Corp. 1990, 1994.\n\r All rights reserved.\n\n\r US Government Users Restricted Rights -");

 	lic2prompt = MSGSTR(CFGCLIC2,"\n\r Use, duplication or disclosure restricted\n\r by GSA ADP Schedule Contract with IBM Corp.\n\n\n\r");

#ifdef _DEBUG
printf("poll_displays\n");
printf("service mode = %d \n",servmode);
#endif
	notdone = TRUE;
	setleds (0xA31);
	/* if service mode then open bus driver to sense position of keylock */
	if (servmode & SERVMODE)
                if ((fdmdd = open("/dev/nvram", O_RDONLY)) != -1)
                {      
                        mddRecord.md_incr = MV_WORD;
                        mddRecord.md_size = 1;
                        mddRecord.md_data = (uchar *)&startkey;
#ifdef _DEBUG
printf("nvram device open successful \n");
#endif
                        if (ioctl(fdmdd, MIOGETKEY, &mddRecord) == -1)
                        {
                                /* ioctl() failed */
#ifdef _DEBUG
printf("MIOGETKEY ioctl failed\n");
#endif
                                close(fdmdd);
                                fdmdd = -1;
                        }
                        else
                        {
#ifdef _DEBUG
printf("start key position = %d \n", startkey);
#endif
                                startkey &= KEYMASK;
                        }
                }

	if (num_lfts == 0) 
		tty_choice = 1;
	else
		tty_choice = num_lfts;

	while ( notdone )
	{
		/* if we were in service mode at boot time and the current
		   keylock switch position has been changed while we were 
		   polling for displays, then set no console flag and 
		   exit loop
		 */
                if (fdmdd != -1)
                {
                        mddRecord.md_incr = MV_WORD;
                        mddRecord.md_size = 1;
                        mddRecord.md_data = (uchar *)&currkey;
                        if ((ioctl(fdmdd, MIOGETKEY, &mddRecord) != -1))
                        {
                                currkey &= KEYMASK;
                                if (currkey != startkey)
                                {
#ifdef _DEBUG
printf("key position changed \n");
#endif
                                        setleds (0xA34);
                                        servmode = -1;
                                        break;
                                }
                        }
                }
		for( curterm = topterm, i = tty_choice;
		    curterm != NULL && notdone;
			curterm = curterm->next, i++ )
		{
			strbuf[0] = '\0';
			rc = read( curterm->fd, strbuf, 79);
			if (rc > 0 && strbuf[0] != '\n') 
			{
				strbuf[rc] = '\0';
#ifdef _DEBUG
		{
			char *ptr;
			ptr = strbuf;
			while(*ptr)
				printf(" %02x",*ptr++);
		}
#endif
				if (strcmp(curterm->dispdev, "lft0") == 0) 
				{
					if ((sscanf(strbuf,
						"\033[%dq",&term_choice)==1)
						&& (term_choice >0) &&      
						(term_choice <= num_lfts))
					{	
#ifdef _DEBUG
                printf("lft term_choice = %d\n", term_choice);
#endif
						notdone = FALSE;
						strcpy(console_physdisp,
						  physdisp_lft[term_choice-1].name);
						strcpy(console_path,
						  curterm->dispdev);
					}	
				}	
				else /* async terminal */    
				{     
					for (j = 0 ; j < 79; j++)
						if (strbuf[j] < '0' ||
						    strbuf[j] > '9')
					{
						strbuf[j] = '\0';
						break;
					}
					term_choice = atoi(strbuf);
					if (term_choice == i)
					{
#ifdef _DEBUG
                printf("tty term_choice = %d\n", term_choice);
#endif
						notdone = FALSE;
						strcpy(console_path,
						    curterm->dispdev);
				/* turn echo on for maintenance shell */
						setterm(curterm->fd,
						    ECHO);
					}	
				}	
			}	     
#ifdef _DEBUG		else
			  if (rc == -1) {
			    printf("read %d failed with errno %d\n", 
				curterm->fd, errno);
			    notdone = FALSE; /* exit now */
			  }
#endif
		} /* end of for all terminals */ 

/* if all terminals have been polled then sleep for 1 second before 
   repolling. If console defined then only poll for timeout times and the
   let the system continue by setting a -1 in servmode and exiting 
 */
		if (notdone == TRUE)
		{
			sleep(1); /* sleep for 1 second before looping */
			if (((--timeout)== 0) && (servmode & CONSDEFINED)
					 && !(servmode & SERVMODE))
			{
				servmode = -1;
				notdone = FALSE;
				setleds (0xA34);
			}
		}
			
	}	/* end of while not done*/

	if (fdmdd != -1)
		close(fdmdd);

	/* clear the messages from screens */
	for( curterm = topterm; curterm != NULL;
	    curterm = curterm->next)
	{
		tcflush(curterm->fd, TCIOFLUSH);
		if (strcmp(curterm->dispdev, "lft0") == 0)
		{
			for ( i = 0; i < num_lfts; i++ )
			{
				ioctl(curterm->fd,
				  LFT_SET_DFLT_DISP, 
				  physdisp_lft[i].name);
				WriteTO(curterm->fd, clear_screen, 25);

			/* put out Licensed Microcode Copyright if not runtime */
			/* only write to selected display */
				if ((runtime == FALSE) && 
                                    (!(__rspc())) &&
				    (strcmp(physdisp_lft[i].name,
				     console_physdisp) == 0))
				{
					rc = WriteTO (curterm->fd,lic1prompt,
						strlen(lic1prompt));
					rc = WriteTO (curterm->fd,lic2prompt,
						strlen(lic2prompt));
				}
			}
		}
		else
		{
			WriteTO(curterm->fd, clear_screen, 25);

			/* put out Licensed Microcode Copyright if not runtime */
			/* only write to selected display */
			if ((runtime == FALSE) &&
                            (!(__rspc())) &&
                            (strcmp(curterm->dispdev, console_path) == 0))
			{
				rc = WriteTO (curterm->fd,lic1prompt,
					strlen(lic1prompt));
				rc = WriteTO (curterm->fd,lic2prompt,
					strlen(lic2prompt));
			}
		}
	}
	return(0);
}



/*
  write_prompts:  Traverse the linked list of terminal names, write a prompt.
*/
int
write_prompts()
{

	char *ttyprompt;
	int	rc;
	int	fildes;
	struct terminals *curterm;
	FILE  *fptr;
	int rc1;

#ifdef _DEBUG
printf("write_prompts\n");
#endif

	totdisps = 0;


	ttyprompt = MSGSTR(CFGCTTY,"\n\n\
\r ******* Please define the System Console. *******\n\n\r\
Type a %d and press Enter to use this terminal as the\n\
  system console.\n\
Typ een %d en druk op Enter om deze terminal als de\n\
  systeemconsole to gebruiken.\n\
Skriv tallet %d og trykk paa Enter for aa bruke denne\n\
  terminalen som systemkonsoll.\n\
Pour definir ce terminal comme console systeme, appuyez\n\
  sur %d puis sur Entree.\n\
Taste %d und anschliessend die Eingabetaste druecken, um\n\
  diese Datenstation als Systemkonsole zu verwenden.\n\
Premere il tasto %d ed Invio per usare questo terminal\n\
  come console.\n\
Escriba %d y pulse Intro para utilizar esta terminal como\n\
  consola del sistema.\n\
Tryck paa %d och sedan paa Enter om du vill att den haer\n\
  terminalen ska vara systemkonsol.\n\n\n\n\n");

	for(curterm = topterm; curterm != NULL; curterm = curterm->next)
	{
		sprintf(strbuf, "/dev/%s", curterm->dispdev);
		if( (curterm->fd = open(strbuf, O_RDWR|O_NDELAY) ) < 0)
		{
			/* If can't open, then drop from the list  */
#ifdef _DEBUG
                printf("open of %s failed with rc %d and errno %d\n", curterm->dispdev,
						curterm->fd, errno);
#endif
			if( curterm->last != NULL )
				(curterm->last)->next = curterm->next;
			else
				topterm = curterm->next;
			if(curterm->next != NULL)
				(curterm->next)->last = curterm->last;
			free(curterm);
			continue;
		}

		if( ((strcmp(curterm->dispdev, "lft0") == 0)) )
		{
			setterm(curterm->fd, ECHO);
			write_lft_prompt(curterm->fd);  
		}
		else /* must be tty */
		{
			/* output message to press enter */
			setterm(curterm->fd,0); /* no echo */
			sleep(1); /* wait for slow terminals */
			++totdisps;
			sprintf(strbuf, ttyprompt, totdisps,totdisps,
				totdisps,totdisps,totdisps,totdisps,
				totdisps,totdisps);
			rc = WriteTO (curterm->fd, strbuf,
				 strlen(strbuf)); 
		}
#ifdef _DEBUG
printf("strbuf = %s\n", strbuf);
dstruct(curterm);
#endif
	}  /* end for */

	if( totdisps == 0 )
	{
		fprintf(stderr,MSGSTR(CFCONE06,"cfgcon: console is not defined and no candidate terminals were detected \n"));
		servmode = -1;		/* indicate no terminals found */
	}
	return(0);
}


/*
   write_lft_prompt:  output prompt to all lft displays.
*/
int
write_lft_prompt( int fildes)
{
	char 	*lftprompt;
	int     count;
	int     rc;


#ifdef _DEBUG
printf("write_lft_prompt\n");
#endif

	for( count = 0; count < num_lfts; count++)
	{
#ifdef _DEBUG
printf("count=%d  set default display=%s\n",
  count,physdisp_lft[count].name);
#endif
		if( ( rc = ioctl(fildes, LFT_SET_DFLT_DISP, 
		    physdisp_lft[count].name )) < 0 )
		{
			fprintf(stderr,MSGSTR(CFCONE04,"cfgcon: failed to set physical display on LFT \n"));
			continue;
		}
		lftprompt = MSGSTR(CFGCHFT," \n\n\r ******* Please define the System Console. *******");
		rc = WriteTO (fildes, lftprompt, strlen(lftprompt));
#ifdef _DEBUG
if (rc < 0)
  printf("write(%d, %s, %d) failed with errno %d\n",
	fildes, lftprompt, strlen(lftprompt), errno);
#endif
		lftprompt = MSGSTR(CFGCHFT1,"\n\n\
Type the F%d key and press Enter to use this display as\n\
  the system console.\n\
Druk op de toets F%d en daarna op Enter om dit\n\
  beeldstation als de systeemconsole te gebruiken.\n\
Trykk paa tasten F%d og deretter paa Enter for aa bruke\n\
  denne skjermen som systemkonsoll.\n\
Pour definir ce terminal comme console systeme, appuyez\n\
  sur la touche F%d puis sur Entree.\n\
Taste F%d und anschliessend die Eingabetaste druecken,\n\
  um diese Anzeige als Systemkonsole zu verwenden.\n\
Premere il tasto F%d ed Invio per usare questo terminale\n\
  come console per il sistema.\n\
Pulse la tecla F%d y pulse Intro para utilizar esta\n\
  pantalla como consola del sistema.\n\
Tryck paa F%d och sedan paa Enter om du vill att den haer\n\
  skaermen ska vara systemkonsol.\n\n\n");
		++totdisps;
		sprintf(strbuf, lftprompt, totdisps,totdisps,totdisps,
			totdisps,totdisps,totdisps,totdisps,totdisps);
		rc = WriteTO (fildes, strbuf, strlen(strbuf));
		lftprompt =MSGSTR(CFGCHFT2,""); 
		rc = WriteTO (fildes, lftprompt, strlen(lftprompt));
	}
	return(0);
} 

/*
   setterm: setup terminal for input/output
*/

int
setterm( int fd , int flag)
{
	struct	termios	tt;

	tcgetattr(fd, &tt);	/* ignore errors */
	tt.c_cflag |= CLOCAL;
	if (flag != CLOCAL)
	/* if not just set CLOCAL then do full default setup */
	{
		tt.c_iflag = BRKINT|ISTRIP|ICRNL|IXON|IXANY;
		tt.c_oflag = OPOST|ONLCR|TAB3;
		if (flag != ECHO)
			tt.c_lflag = ISIG|ICANON;
		else
			tt.c_lflag = ISIG|ICANON|ECHO|ECHOE|ECHOK;
		tt.c_cc[VINTR] = CINTR;
		tt.c_cc[VQUIT] = CQUIT;
		tt.c_cc[VERASE] = CERASE;
		tt.c_cc[VKILL] = CKILL;
		tt.c_cc[VEOF] = CEOF;
		tt.c_cc[VEOL] = CNUL;
		tt.c_cc[VEOL2] = CNUL;
	}
	tcsetattr(fd, TCSANOW, &tt);	/* ignore errors */
	return(0);
}


WriteTO(int fd, char *buf, int len)	/* Loop until all written */
{					/* Need this for writes to tty opened with O_NDELAY */
	int rc;				/* Times out if write does not complete */
	int timeout=40;
	int olen = len;

	while (timeout && len) {
		rc = write(fd, buf, len);
		if ((rc == -1) && (errno != EAGAIN))
			return rc;
		len -= rc;
		if (len) {
			usleep(100000);		/* delay 1/10 second */
			buf += rc;
			timeout--;		/* timeout */
		}
	}
	return(olen - len);
}


#ifdef _DEBUG
/**************************************************************************
 Dump terminal structures for debugging 
 **************************************************************************/

dstruct(tp)
struct terminals        *tp;
{
	struct terminals        *thisterm;

	thisterm = topterm;
	while(thisterm !=  NULL)
	{
		printf("fd = %d  ",thisterm->fd);
		printf("ddev = %s  ",thisterm->dispdev);
		printf("Next = %X  ",thisterm->next);
		printf("Last = %X \n",thisterm->last);
		thisterm = thisterm->next;
	}
}
#endif


