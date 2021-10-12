static char sccsid[] = "@(#)82	1.4  src/bos/diag/util/ueth/ethstw/stiluenet.c, dsaueth, bos41J, 9523B_all 6/6/95 16:15:23";
/*
 *   COMPONENT_NAME: DSAUETH
 *
 *   FUNCTIONS: clean_up
 *		disp_menu
 *		main
 *		process_rc
 *		riser_card_determination
 *		set_timer
 *		timeout
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include "diag/diago.h"
#include "uenet_msg.h"
#include "uenet.h"
#include <locale.h>
#include <sys/signal.h>
#include "tu_type.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#define                NOT_CONFIG      -1
#define                NO_ERROR        0
int	riser_card_determination();
void	set_timer ();

extern int exectu();
int setnum = 1;

struct msglist Dlist[] = {
        { 1, DESCRIP_TITLE , },
        { 1, DESCRIP, },
        { 1, NO_ERR , },
        { 1, ADAP_ERR , },
        { 1, SYS_ER , },
        { 1, UNID_ERR , },
        { 1, ADAP_BUSY , },
        { 1, ENTER , },
        NULL
};


struct msglist NAlist[] = {
        { 1, RESULTS, },
        { 1, NO_ADAPS, },
        NULL
};

struct msglist Rlist[] = {
        { 1, RESULTS , },
        { 1, NO_ERR, },
        { 1, ENTER, },
        NULL
};

ASL_SCR_TYPE enttype = DM_TYPE_DEFAULTS;

int	filedes=0;
nl_catd	catd;
int	adap_num;
char	device_name[64];
char	location[32];

TUTYPE stileth_tucb;

int	riser_card;
int     alarm_timeout=FALSE;
struct  sigaction  invec;       /* interrupt handler structure  */



struct  sigaction  invec;       /* interrupt handler structure  */
char    msgstr[256];
char    msgstr1[256];
char    option[256];
char    *new_out, *new_err;
int     diag_device_configured=FALSE;
int     set_device_to_diagnose = FALSE;


/*************************************************************************/



main (argc, argv)
int	argc;
char	*argv[];
{
	char	devname[32];
	int	rc=0;

	setlocale (LC_ALL, "");
        catd = (nl_catd) diag_catopen(MF_UENET, 0);
        diag_asl_init("NO_TYPE_AHEAD");
        init_dgodm();

	stileth_tucb.mdd_fd = -1;
	strcpy (device_name, argv[1]);
	strcpy (location, argv[2]);
	sprintf(devname,"/dev/%s/D",device_name);

	unconfigure_lpp_device();

	set_timer ();
	stileth_tucb.mdd_fd = open ("/dev/bus0", O_RDWR);	
	filedes = open(devname, O_RDWR);
	alarm (0);
	if ((filedes == -1) || ( alarm_timeout) || (stileth_tucb.mdd_fd == -1))
	{
		disp_menu( ADAP_BUSY);
	}
	else
	{
		riser_card = riser_card_determination ();
		switch (riser_card)
		{
 			case TWISTED_PAIR_RISER_CARD:

				disp_menu (STANDBY);
 				/* run test 0x10 */
				set_timer ();
       				stileth_tucb.header.tu = 0x10;
       				rc = exectu(filedes, &stileth_tucb);
				alarm(0);
                       		break;

               		case THICK_RISER_CARD: 
				disp_menu (STANDBY);
				set_timer ();
       				stileth_tucb.header.tu = 8;
       				rc = exectu(filedes, &stileth_tucb);
       				alarm(0);
				break;

			case THIN_RISER_CARD:
				disp_menu (STANDBY);
				set_timer ();
		 		stileth_tucb.header.tu = 9;
       				rc = exectu(filedes, &stileth_tucb);
       				alarm(0);
				break;
			default:
				rc = RISERCARD_ERR;
				break;
		}
		process_rc (rc);
	}
	clean_up ();

	
}
/***************************************************************************/
/*
 * NAME: disp_menu
 *
 * FUNCTION: handles menu displays
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: rc = user input from menu displayed
 */

int disp_menu(msgnum)

int msgnum;
{
   
	int 	rc = 0; 	/* return code from a function */
	char 	msgstr[512]; 
	int 	num_adaps;

	switch(msgnum)
	{
		case DESCRIP:
			rc = diag_display(DESCRIP_NUM, catd, Dlist, DIAG_IO,
			    ASL_DIAG_NO_KEYS_ENTER_SC, NULL, NULL);
			break;
		case STANDBY:
			rc = diag_msg_nw(STANDBY_NUM, catd, setnum, STANDBY,
			    device_name, location);
			break;

		case ADAP_ERR:	/* adapter error 	*/
			Rlist[1].msgid = ADAP_ERR;
			break;

		case TX_TIME:	/* transmit timeout	*/
			Rlist[1].msgid = TX_TIME;
			break;

		case SYS_ER:	
			Rlist[1].msgid = SYS_ER;
			break;

		case UNID_ERR:	/* An unidentified error	*/
			Rlist[1].msgid = UNID_ERR;
			break;

		case ADAP_BUSY:	/* Cannot open device device is busy	*/
			Rlist[1].msgid = ADAP_BUSY;
			break;
	}

	switch(msgnum)
	{
		case NO_ERR:
		case ADAP_ERR:
		case TX_TIME:
		case SYS_ER:
		case UNID_ERR:
		case ADAP_BUSY:
			rc = diag_display(RESULTS_NUM, catd, Rlist, DIAG_IO,
			    ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
			break;
	}

	switch(rc)
	{
		case ASL_EXIT:
		case ASL_CANCEL:
			clean_up();
			break;
		case ASL_COMMIT:
			switch(msgnum)
			{
				case ACTION:
					rc = DIAG_ITEM_SELECTED(enttype);
					adap_num = rc - 1;	
					break;
				default:
					break;
			}
			break;
	}

	return(rc);
}
/****************************************************************
* NAME: clean_up
*
* FUNCTION: Cleanup before exit SA ; closing catalog file ;
*           quiting asl; restore the database to the initial
*           state
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/

clean_up()
{
	int result;

	if (stileth_tucb.mdd_fd != -1)
		close (stileth_tucb.mdd_fd);
	if (filedes > 0)
		close(filedes);
	configure_lpp_device();
	term_dgodm();
	diag_asl_quit(NULL);
	catclose(catd);
	exit (0);
}
/************************************************************************/ 
process_rc(rc)
int rc;
{
	if (alarm_timeout )
	{
		disp_menu (TX_TIME);
		return (0);
	}
	switch(rc)
	{
		case 0:
			disp_menu(NO_ERR);
			break;


		case TX_INCOMPLETE:
		case NO_COLLISION:

			disp_menu(TX_TIME);
			break;

		case WRAP_WR_ERR:
		case WRAP_RD_ERR:
		case WRAP_CMP_ERR:
		case BAD_PCK_S_ERR:
		case ENT_CFG_ERR:
		case ENT_POS_ERR:
		case WRAP_NODATA:
		case START_ERR:
		case SPOS3_RD_ERR:
		case SPOS6_WR_ERR:
		case SPOS7_WR_ERR:
		case RISERCARD_ERR:
		case IO6_RD_ERR:
		case BADFUSE:
		case CIO_QUERY_ERR:
		case CONNECTOR_ERR:
		case STAT_NOT_AVAIL:
	
			disp_menu (ADAP_ERR);
			break;

		default:
			disp_menu(UNID_ERR);
			break;
	}
}
/****************************************************************
* NAME: riser_card_determination
*
* FUNCTION: Run riser card type test
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  set the global variable riser_card
*
* RETURNS:
*       None
****************************************************************/

int  riser_card_determination()
{
	int rc = 0;

        /* determine riser card type */
        stileth_tucb.header.mfg = 0;
        stileth_tucb.header.loop = 1;
        stileth_tucb.header.tu = 0x11;

        riser_card = exectu(filedes, &stileth_tucb) ;

        switch( riser_card) 
	{
		case TWISTED_PAIR_RISER_CARD:
                case THICK_RISER_CARD: 
		case THIN_RISER_CARD:
                        break;
                case BADFUSE:/* bad fuse of thick */
                case IO6_RD_ERR:
                case RISERCARD_ERR: 
		default:
			riser_card = -1;
			break;
        }
	return (riser_card);

}
/*
 * NAME: timeout()
 *
 * FUNCTION: Designed to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void timeout(int sig)
{
        alarm(0);
        alarm_timeout = TRUE;
}

/*
 * NAME: set_timer ()
 *
 * FUNCTION: Designed to set up alarm timer
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void set_timer()
{
        invec.sa_handler = timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(60);
}

/*---------------------------------------------------------------------------
  * NAME: configure_lpp_device ()
  *
  * FUNCTION: configure_lpp_device ()
  *         Unload the diagnostic device driver
  *         setting the device to the DEFINE state (No matter what state
  *         it is in before running the test. We do not want left it in
  *         DIAGNOSE state after running diagnostics
  *         clean_up will restore the AVAILABLE state if it is so before
  *         running the test
  *
  *
  * EXECUTION ENVIRONMENT:
  *
  *      Environment must be a diagnostics environment wich is described
  *      in Diagnostics subsystem CAS.
  *
  * RETURNS: NONE
  *--------------------------------------------------------------------------*/
int   configure_lpp_device()
{
	char    criteria[128];
	int     result;

	/* Unconfigure diagnostics device. Unload the device from system */
	if(diag_device_configured)
	{
		/* UCFG is defined as 1                 */
		sprintf (option," -l %s -f 1", device_name);

		strcpy (criteria,"/usr/lib/methods/cfgddeth");

		result = invoke_method(criteria,option, &new_out,&new_err);
		free(new_out);
		free(new_err);
		if (result !=  NO_ERROR)
		{
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
				DIAG_DEVICE_CANNOT_UNCONFIGURED),
				device_name, location);
			diag_asl_msg ("%s\n", msgstr);
			return (-1);
		}
	}

	/* Setting the device state to original state */
        if (set_device_to_diagnose)
        {
                result = diagex_initial_state( device_name );
                if (result == NO_ERROR)
                {
                        set_device_to_diagnose= FALSE;
                }
                else
                {
                       sprintf(msgstr, (char *)diag_cat_gets (
                          catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
				device_name, location);
                        diag_asl_msg ("%s\n", msgstr);
                        return (-1);
                }
        }
 }
/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device 				*/
/*      Description:						*/
/*      Return          : 0 Good                                */
/*                        -1 BAD                                */
/*--------------------------------------------------------------*/
int   unconfigure_lpp_device()
{
	char    criteria[128];
       	int     result;

        result = diagex_cfg_state ( device_name );

        switch (result) {
                case 2:
                        sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
					device_name, location);
                        diag_asl_msg ("%s\n", msgstr);
                        clean_up();
                        break;
                case 3: sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
				device_name, location);
                        diag_asl_msg ("%s\n", msgstr);
                        clean_up();
                        break;
        }
        set_device_to_diagnose= TRUE;

       /* The diagnostics device driver needs to be loaded into the system */

       /* CFG is defined as 0  */
       sprintf (option," -l %s -f 0", device_name);

       strcpy (criteria,"/usr/lib/methods/cfgddeth");
       result = invoke_method(criteria,option, &new_out,&new_err);
       if (result == NO_ERROR)
       {
               diag_device_configured=TRUE;
       }
       else
       {
               sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                         DIAG_DEVICE_CANNOT_CONFIGURED),
			device_name, location);
               diag_asl_msg ("%s\n", msgstr);
               clean_up();
       }

       return (0);
 }
