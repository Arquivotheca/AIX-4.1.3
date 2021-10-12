static char sccsid[] = "@(#)03	1.6.1.2  src/bos/diag/util/ucfgvpd/ucfgvpd.c, dsauchgvpd, bos411, 9428A410j 2/12/93 17:15:44";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS: 	main
 *		display_cfg
 *		alter_vpd
 *		int_handler
 *		genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <locale.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/class_def.h"	/* object class data structures	*/ 
#include "ucfgvpd_msg.h"

#define UCFGVPD_MENU	0x802030
#define UDSPCFG_MENU	0x802031
#define UDSPVPD_MENU	0x802032
#define UALTVPD_MENU	0x802034
#define UCHGCFG_MENU	0x802035
#define UDSPLPP_MENU	0x802036


/* Global variables */
nl_catd		fdes;		/* catalog file descriptor	*/
char 		*outbuf;
char 		*errbuf;
int		exenvflg;
int		diskette_based;

/* Extern variables */
extern ASL_SCR_TYPE dm_menutype;
extern nl_catd diag_catopen(char *, int);

/* EXTERNAL FUNCTIONS */
extern char	*diag_cat_gets();

/* FUNCTION PROTOTYPES */
int display_cfg(int);
int alter_vpd(void);
int display_lpp(void);
void int_handler(int);
void genexit(int);

/*  */
/* NAME: main
 *
 * FUNCTION: Display or Print Configuration or VPD Menu
 * 
 * NOTES: This unit displays the 'Display or Print Configuration or VPD
 *	Selection' Menu and controls execution of the response. It performs
 *	the following functions:
 *	1.  Initialize the ODM.
 *	2.  Display menu and wait for response. 
 *	3.  Call appropriate function according to response.
 *
 * RETURNS: 0
 *
 */

main(int argc, char *argv[])
{
	int 	status = -1;
	int	selection;
	char	searchstring[80];
	struct sigaction act;
	static struct msglist menulist[] = {
		{ MSET, MTITLE },
		{ MSET, MOPT1 },
		{ MSET, MOPT2 },
		{ MSET, MOPT3 },
		{ MSET, MOPT4 },
		{ MSET, MOPT5 },
		{ MSET, MLASTLINE },
		{ (int )NULL, (int )NULL}
	};

	setlocale(LC_ALL,"");

	/* set up interrupt handler	*/
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);

	/* catalog name is the program name with .cat extension */
	diag_asl_init("default");
	fdes = diag_catopen(MF_UCFGVPD,0);
	init_dgodm();

	exenvflg = ipl_mode(&diskette_based);
	if (diskette_based)
	  {
	  menulist[4].msgid = MLASTLINE;
	  menulist[5].setid = (int )NULL;
	  menulist[5].msgid = (int )NULL;
	  }
	else
	  {
	  menulist[4].msgid = MOPT4;
	  menulist[5].setid = MSET;
	  menulist[5].msgid = MOPT5;
	  }

	status = DIAG_ASL_OK;
	while( status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT )  {

		status = diag_display(UCFGVPD_MENU, fdes, menulist, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( status == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch ( selection = DIAG_ITEM_SELECTED(dm_menutype)) {
				case 1 :
				case 2 :
					display_cfg(selection);
					break;
				case 3 :
					alter_vpd();
					break;
				case 4 :
					cfg_change();
					break;
				case 5 :
					display_lpp();
					break;
			}
		}
	}
	genexit(0);
}

/*  */ 
/*
 * NAME: display_cfg
 *                                                                    
 * FUNCTION: Display the device information about all devices in
 *		system configuration.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: This function invokes the command 'lscfg' to display the
 *	  device infomation and/or vpd data.
 *
 * RETURNS:
 *	DIAG_ASL_CANCEL : 
 *	DIAG_ASL_EXIT   :
 *	-1		: error invoking lscfg
 */  

int display_cfg(int selection)
{
	int 		rc;
	int		menu;
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	switch (selection)
	  {
	  case 1:
	    menu = UDSPCFG_MENU;
	    rc = odm_run_method(LSCFG, "", &outbuf, &errbuf );
	    break;
	  case 2:
	    menu = UDSPVPD_MENU;
	    rc = odm_run_method(LSCFG, "-v", &outbuf, &errbuf );
	    break;
	}

	/* Return error if command failed OR
	   if no information was retrieved. */
	if ((rc == -1) || (outbuf == (char *)NULL)) return(-1);

	/* allocate space for 3 entries */
	menuinfo = (ASL_SCR_INFO *) calloc( 3, sizeof(ASL_SCR_INFO) );

	/* put in the title and instruction line */
	menuinfo[0].text = (char *) strtok(outbuf, "\t");
	*( (char *)strrchr(menuinfo[0].text, '\n') ) = '\0';

	/* put in the list of devices */
	menuinfo[1].text = outbuf + strlen(menuinfo[0].text) + 3;
	/* remove the last newline */
	*( (char *)strrchr(menuinfo[1].text, '\n') ) = '\0';

	/* add in the last line */
	menuinfo[2].text = diag_cat_gets( fdes, MSET, MUCFGVPD);

	menutype.max_index = 2;
	rc = diag_display( menu, fdes, NULL, DIAG_IO,
			   ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   &menutype, menuinfo);
	free(outbuf);
	free(menuinfo);
	return(rc);
}

/*  */ 
/* NAME: alter_vpd  
 *
 * FUNCTION: Display and alter the VPD for individual resources installed.
 * 
 * NOTES: This unit performs the following functions:
 *	1. Invoke lscfg to get all installed resources
 *	2. Display screen and wait for response.
 *      3. Display VPD data for selected resource.
 *	4. Accept new VPD data.
 *	5. Save new VPD data in appropriate object. 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	DIAG_ASL_CANCEL : 
 *	DIAG_ASL_EXIT   :
 *	-1		: error invoking lscfg
 *
 */

int alter_vpd(void)
{
	int 		rc = -1;
	int		entry = 0;
	char		dname[NAMESIZE];
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	/* invoke lscfg to get all the information */
	rc = odm_run_method(LSCFG, "", &outbuf, &errbuf );
	if ((rc == -1) || (outbuf == (char *)NULL)) return(-1);

	/* allocate space for 256 entries, this should be enough */
	menuinfo = (ASL_SCR_INFO *) calloc( 1, 256*sizeof(ASL_SCR_INFO) );

	/* put in the title and instruction line */
	menuinfo[entry++].text = diag_cat_gets( fdes, MSET_DSP_VPD, DSPTITLE);
	/* put in the list of devices */
	outbuf = (char *) strchr(outbuf, '\t' ) - 1;
	strtok(outbuf, "\n");
	outbuf = NULL;
	for ( ; entry < 255 ; entry++) {
		menuinfo[entry].text = (char *) strtok(outbuf, "\n");
		if ( menuinfo[entry].text == NULL )
			break;
		menuinfo[entry].text[0] = ' ';
		if(menuinfo[entry].text[strlen(menuinfo[entry].text)+1]==' ') {
			menuinfo[entry].text[strlen(menuinfo[entry].text)]='\n';
			strtok(outbuf, "\n");
		}
	}
	/* put in the last line */
	menuinfo[entry].text = diag_cat_gets( fdes, MSET_DSP_VPD, DSPLASTLINE);

	menutype.max_index = entry; 
	rc = DIAG_ASL_OK;
	while ( rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT ) {
		rc = diag_display( UALTVPD_MENU, fdes, NULL, DIAG_IO,
			   ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   &menutype, menuinfo);
		if ( rc == DIAG_ASL_COMMIT ) {
			sscanf ( menuinfo[DIAG_ITEM_SELECTED(menutype)].text,
				 "%s", dname );
			dsp_alt_vpd ( dname ); 
		}
	}
	return(rc);
}

/*  */ 
/*
 * NAME: display_lpp
 *                                                                    
 * FUNCTION: Display information about installed software
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This should describe the execution environment for this
 *	procedure. For example, does it execute under a process,
 *	interrupt handler, or both. Can it page fault. How is
 *	it serialized.
 *                                                                   
 * NOTES: This function invokes the 'lslpp' command
 *	  It is not available from diskette package.
 *
 * RETURNS:
 *	DIAG_ASL_CANCEL : 
 *	DIAG_ASL_EXIT   :
 *	-1		: error invoking lslpp
 */  
int display_lpp(void)
{
	char		*temp;
	int 		rc;
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	if (diskette_based == DIAG_TRUE)
	  return(0);
	else
	  rc = odm_run_method(LSLPP, "-h", &outbuf, &errbuf );

	/* Return error if command failed OR
	   if no information was retrieved. */
	if ((rc == -1) || (outbuf == (char *)NULL)) return(-1);

	/* allocate space for 3 entries */
	menuinfo = (ASL_SCR_INFO *) calloc(3, sizeof(ASL_SCR_INFO));

	/* Add title to menu */
	menuinfo[0].text = diag_cat_gets(fdes, MSET_DSP_LPP, LPPTITLE);

	/* put in the list of devices */
	menuinfo[1].text = outbuf;

	/* add in the last line */
	menuinfo[2].text = diag_cat_gets(fdes, MSET_DSP_LPP, LPPLASTLINE);

	menutype.max_index = 2;
	rc = diag_display( UDSPLPP_MENU, fdes, NULL, DIAG_IO,
			   ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   &menutype, menuinfo);
	free(outbuf);
	free(menuinfo);
	return(rc);
}

/*  */ 
/*
 * NAME: int_handler
 *                                                                    
 * FUNCTION: Perform general clean up on receipt on an interrupt
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */  

void int_handler(int sig)
{
  struct sigaction act;

	/* if segmentation violation , clean up before core dump 
	if ( sig == SIGSEGV ) {
		act.sa_handler = SIG_DFL;
		sigaction(SIGSEGV, &act, (struct sigaction *)NULL);
		odm_terminate();
		diag_asl_quit();
		catclose(fdes);
		return(0);
	}		*/
	genexit(1);
}


/*
 * NAME: genexit
 *                                                                    
 * FUNCTION: Perform general clean up, and then exit with the status code.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * RETURNS: NONE
 *
 */  

void genexit(int exitcode)
{
	odm_terminate();
	diag_asl_quit();
	catclose(fdes);
	exit(exitcode);
}
