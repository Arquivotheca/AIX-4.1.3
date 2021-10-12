static char sccsid[] = "@(#)10  1.23  src/bos/diag/util/uscsi/uscsi.c, dsauscsi, bos411, 9431A411a 7/26/94 17:29:16";
/*
 *   COMPONENT_NAME: DSAUSCSI
 *
 *   FUNCTIONS: analyse_result
 *		copy_text
 *		display_adapters
 *		genexit
 *		get_address
 *		get_scsi_addr
 *		get_max_scsi_id
 *		int_handler
 *		main
 *		open_scsi_adapter
 *		resource_desc
 *		send_inq_command
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* #define DEBUGSCSI */

#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include <nl_types.h>
#include <cf.h>
#include <odmi.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/scdisk.h>
#include <sys/scsi.h>

#include "uscsi_msg.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/diag_exit.h"

#define menuIdSAIntro			0x802070
#define menuIdAdapterSelectMsg		0x802071
#define menuIdScsiIdPrompt		0x802073
#define menuIdAdapterDevSameAddr	0x802075
#define menuIdInquUserConfirm		0x802076
#define menuIdInquInProcess		0x802077
#define menuIdInqOK			0x802078
#define menuIdInquNoResponse		0x802079
#define menuIdInquBusError		0x802180
#define menuIdSciostartError		0x802181
#define menuIdInquIOError		0x802184
#define menuIdInquDevBusy		0x802185
#define menuIdAdapterOpenError		0x802186
#define menuIdAdapterAddrNotFound	0x802187
#define menuIdScsiDevNoneFound		0x802188

#define INQUIRY_BUF_LEN			0xFF

enum {  exitOK, exitDiagAslInitErr, exitDiagCatopenErr, exitGetCuDvListErr,
	exitDisplayAdaptErr, exitAdapterAddrNotFound, exitAdapterOpenErr,
	exitAdaptConnwhereErr };
	

/* GLOBAL VARIABLES     */
char		*devname;		/* device name used in open cmd */
char		dname[] = "/dev/";
char		device_name[NAMESIZE];
nl_catd         cat_fdes;		/* catalog file descriptor */
struct CuDv	*adapt_list;
struct listinfo	obj_info;
int             sciostart_rc;		/* SCIOSTART status */
int             saved_adap_fdes = -1;	/* saved variables to close */
int             saved_scsiaddr = -1;	/* device on interupt */
static ASL_SCR_TYPE ndmenutype = DM_TYPE_DEFAULTS; /* default menu type for */
                                                    /* non dialogue type menu */
static int	init_config_state = -1;  
uchar		lun_id = 0;		/* lun id is hardcoded */
uchar		max_scsi_id;		/* max scsi id the adapt supports */
#ifdef DEBUGSCSI
FILE		*fdebug;
#endif

/* CALLED FUNCTIONS     */
extern nl_catd		diag_catopen(char *, int);

/*  */
/* NAME: main
 *
 * FUNCTION: SCSI BUS Service Aid
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This unit provides a means to diagnose a SCSI Bus problem.
 *      Selection' Menu and controls execution of the response. It performs
 *      the following functions:
 *      1.  Display operating description.
 *      2.  Displays list of scsi adapters.
 *      3.  Prompts for SCSI Address to test.
 *      4.  Issue SCSI Inquiry command.
 *      5.  Analyse results.
 *      5.  Displays the results for the user.
 *
 * RETURNS: 0 - no error
 *          1 - error
 */

main(argc, argv)
int     argc;
char    *argv;
{
	int		get_address();
	int		display_adapters();
	void		analyse_result();
	void 		resource_desc();
	void		copy_text();
	int		tu_test();
	int		get_scsi_addr();
	int		send_inq_command();
	uchar		get_max_scsi_id(char *);

        register int	rc;
	int		adap_fdes;	/* adapter file descriptor */
        int   		scsi_address;
        long 		key_hit = DIAG_ASL_OK;
	void		int_handler(int);
        struct sigaction act;
	char		*criteria="name like [vs]*csi* and chgstatus!=3";
	struct CuDv	*adapt_used;
	int             adapt_num;

        static struct	msglist menuSAIntro[] = {
				{ FDSAID_SET1,  MENU_SA_INTRO, },
				(int)NULL
	};

        static struct	msglist menuScsiDevNoneFound[] = {
				{ FDSAID_SET1,  MENU_SCSI_DEV_NONE_FOUND, },
				(int)NULL
	};

        static ASL_SCR_INFO      scrSAIntro[ DIAG_NUM_ENTRIES(menuSAIntro) ];
        static ASL_SCR_INFO      scrScsiDevNoneFound[ DIAG_NUM_ENTRIES(menuScsiDevNoneFound) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;


	setlocale(LC_ALL,"");
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

	if (diag_asl_init("NO_TYPE_AHEAD") == -1)
		genexit(exitDiagAslInitErr);
        if ((cat_fdes = diag_catopen(MF_USCSI, 0)) == -1)
		genexit(exitDiagCatopenErr);

        init_dgodm();

#ifdef DEBUGSCSI
fdebug = fopen("/tmp/uscsi.trace","a");
fprintf(fdebug,"***** SCSI Bus SA Start *****\n");
fprintf(fdebug,"%d:\tEINVAL=%d EIO=%d ETIMEDOUT=%d ENOCONNECT=%d ENODEV=%d\n", __LINE__, EINVAL, EIO, ETIMEDOUT, ENOCONNECT, ENODEV);
fflush(fdebug);
#endif

        /*
          display SCSI Bus Service Aid Description Frame
        */
        rc = diag_display(menuIdSAIntro,cat_fdes, menuSAIntro, DIAG_IO,
		ASL_DIAG_NO_KEYS_ENTER_SC,
		&menutype, scrSAIntro);
        if (rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL)
                genexit(exitOK);

        /* get list of all devices to be included in diag test selection */
        adapt_list = get_CuDv_list(CuDv_CLASS,criteria,&obj_info,MAX_EXPECT,2);
        if ((adapt_list == (struct CuDv *) -1)){
                genexit(exitGetCuDvListErr);
        }
	if ((adapt_list == (struct CuDv *) NULL)){
		rc = diag_display(menuIdScsiDevNoneFound, cat_fdes,
			menuScsiDevNoneFound, DIAG_IO,
			ASL_DIAG_NO_KEYS_ENTER_SC,
			&menutype, scrScsiDevNoneFound);
                genexit(exitOK);
	}
	adapt_num = obj_info.num;
        /*
         * Loop here until user wants to quit
         */
        while ((key_hit != DIAG_ASL_CANCEL) &&
                (key_hit != DIAG_ASL_EXIT)) {

		/* reset global variables */
		sciostart_rc = saved_adap_fdes = -1;
	
        	/*
         	 * prompt for the adapter
        	 */
        	rc = display_adapters(adapt_num, &adapt_used);
		if (rc == -1)
                	genexit(exitDisplayAdaptErr);
       		if (rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL)
                	genexit(exitOK);

        	adap_fdes = saved_adap_fdes = open_scsi_adapter(adapt_used);

		if ((max_scsi_id = get_max_scsi_id(adapt_used->PdDvLn_Lvalue)) == -1)
			genexit(exitAdaptConnwhereErr);

		while((rc != DIAG_ASL_EXIT) && (rc != DIAG_ASL_CANCEL)){
                	/*
                  	 * prompt for scsi address to test
	       		 */
                	rc = get_address(adapt_used, &scsi_address);
			saved_scsiaddr = scsi_address;
                	if (rc == DIAG_ASL_CANCEL)
                        	break;
			if (rc == DIAG_ASL_EXIT)
				genexit(exitOK);
                	/*
		  	 * start adapter, send inquiry then stop adapter
			 */
               		rc = send_inq_command(adap_fdes, scsi_address);
		}
                /*
		 * check if user wants to quit
		 */
		if (rc == DIAG_ASL_EXIT)
			key_hit = rc;
		else
               		key_hit = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,
				 NULL, NULL);
		close(adap_fdes);
		saved_adap_fdes = -1;
        } /* endwhile */

        genexit(exitOK);

} /* endmain */
/*  */
/* NAME: display_adapters
 *
 * FUNCTION: Display all the scsi adapters menu.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */

int
display_adapters(int adapt_num, struct CuDv **adapt_used)
{
	void		resource_desc();
        register int	index;
        register int	line = 0;
        int             rc;
        char            *string;
        char            *free_string;
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /*
	  allocate space for adapt_num 
	*/
        resinfo = (ASL_SCR_INFO *) calloc(adapt_num+2,sizeof(ASL_SCR_INFO));
        if (resinfo == (ASL_SCR_INFO *) -1)
                return(-1);

        free_string = string = (char *)malloc(adapt_num*132);
        if (string == (char *) -1)
                return(-1);
        /*
	  set the title line in the array
	*/
        resinfo[line++].text=(char *)diag_cat_gets(cat_fdes, FDSAID_SET1,
			MENU_ADAPTER_SELECT_MSG);
        /*
	  get the description
        */
        for (index=0; index < adapt_num; index++)  {
                resource_desc(string,adapt_list+index);
                resinfo[line].text = string;
                resinfo[line].non_select = ASL_NO;
                string = string + strlen(string)+1;
                line++;
        }
        /*
	  finally add the last line
	*/
        resinfo[line].text = (char *)diag_cat_gets(cat_fdes, FDSAID_SET1,
			MENU_ADAPTER_SELECT_PROMPT);
        /*
          now display menu
        */
        restype.max_index = line;
        restype.cur_index = 1;
        rc = diag_display(menuIdAdapterSelectMsg, cat_fdes, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC,
			&restype, resinfo);
        if (rc == DIAG_ASL_COMMIT)
                *adapt_used = adapt_list+DIAG_ITEM_SELECTED(restype)-1;
        free (free_string);
        free (resinfo);
        return (rc);
}
/*   */
/*
 * NAME: resource_desc
 *
 * FUNCTION: Build a text string describing the resource and its location.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *      This function takes as input a line type which indicates the type
 *      of display line to build.
 *
 * RETURNS: 0
 */

void
resource_desc(string, device)
char		*string;
struct	CuDv	*device;
{
        char    *tmp;
	char	*device_text;

        sprintf(string, "%-16s %-16.16s ",
                        device->name,
                        device->location);

        tmp = string + strlen(string);
	device_text=get_dev_desc(device->name);
        copy_text(strlen(string), tmp, device_text);
	free(device_text);
}

/*   */
/*
 * NAME: copy_text
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

void
copy_text(string_length, buffer, text)
int     string_length;            /* current length of text already in buffer */
char    *buffer;                  /* buffer to copy text into     */
char    *text;                    /* text to be copied            */
{
        int     space_count;
        int     char_positions;
        char    *tmp;
        char    *next_text;

        /*
          determine if length of text string will fit on one line
        */
        char_positions = LINE_LENGTH - string_length;
        if (char_positions < strlen(text))  {
                /*
	          dont break the line in the middle of a word
		*/
                if (text[char_positions] != ' ' && text[char_positions+1] != ' ')
                        while(--char_positions)
                                if (text[char_positions] == ' ')
                                        break;
                if (char_positions == 0)
                        char_positions = LINE_LENGTH - string_length;

                strncpy(buffer, text, char_positions+1);
                tmp = buffer + char_positions + 1;
                *tmp++ = '\n';
                next_text = text + char_positions+1;
                while (*next_text == ' ')      /* remove any leading blanks */
                        next_text++;
                space_count = string_length;
                while (space_count--)
                        *tmp++ = ' ';
                copy_text(string_length, tmp, next_text);
        }
        else
                sprintf(buffer, "%s", text);
}
/*  */
/*
 * NAME: get_address
 *
 * FUNCTION: Present dialog box and prompt for scsi address to test
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */
int
get_address(struct CuDv *adapt_used, int *scsiaddr)
{
	int 		rc;
	int 		line = 0;
	int		adapter_addr;
	int		max_scsi;
	char		entered_slot[] = {"0"};
	struct		listinfo cinfo;
	ASL_SCR_INFO    *dialog_info;
	static	ASL_SCR_TYPE	menuTypeScsiIdPrompt = DM_TYPE_DEFAULTS;
	static	ASL_SCR_TYPE	menuTypeAdapterDevSameAddr = DM_TYPE_DEFAULTS;
	static	char	*available_slots[] = { "0,1,2,3,4,5,6,7",
				"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15"};

	static struct msglist menuAdapterDevSameAddr[] = {
			{ FDSAID_SET1,  MENU_ADAPTER_DEV_SAME_ADDR, },
			{ FDSAID_SET1,  MSG_PRESS_ENTER, },
			(int)NULL
	};

	static ASL_SCR_INFO scrAdapterDevSameAddr[ DIAG_NUM_ENTRIES(menuAdapterDevSameAddr) ];


        /* allocate space for at least 5 entries */
        dialog_info = (ASL_SCR_INFO *) calloc(5, sizeof(ASL_SCR_INFO));
        if (dialog_info == (ASL_SCR_INFO *) NULL)
                return(-1);

        /* set the title line in the array   */
        dialog_info[line++].text = diag_cat_gets(cat_fdes,FDSAID_SET1,MENU_SCSI_SA_TITLE);

	/* slot prompt */
        dialog_info[line].text = diag_cat_gets(cat_fdes, FDSAID_SET1, MENU_SCSI_ID_PROMPT);
	dialog_info[line].op_type = ASL_RING_ENTRY;
	dialog_info[line].entry_type = ASL_NUM_ENTRY;
	dialog_info[line].required = ASL_YES;

	dialog_info[line].disp_values = available_slots[max_scsi_id/8];

	dialog_info[line].data_value = entered_slot;
	dialog_info[line].entry_size = 2;
	line++;
	
        dialog_info[line].text = diag_cat_gets(cat_fdes, FDSAID_SET1, MENU_SCSI_ID_DEFAULT);
	menuTypeScsiIdPrompt.max_index = line ;

	adapter_addr = get_scsi_addr(adapt_used);

	while (1) {
		rc = diag_display(menuIdScsiIdPrompt, cat_fdes, NULL,DIAG_IO,
			ASL_DIAG_DIALOGUE_SC, &menuTypeScsiIdPrompt, dialog_info);
	        if (rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL)
			return(rc);
		if (rc == DIAG_ASL_COMMIT) {
			*scsiaddr = (int)strtoul(dialog_info[1].data_value,
						(char **) NULL, 0);
			if (*scsiaddr != adapter_addr)
				break;
			rc = diag_display(menuIdAdapterDevSameAddr, cat_fdes,
				menuAdapterDevSameAddr, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC,
				&menuTypeAdapterDevSameAddr, scrAdapterDevSameAddr);
	        	if (rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL)
				return(rc);
		}
	}
	return (rc);
}

/*  */
/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
        diag_asl_clear_screen();
#ifdef DEBUGSCSI
	fprintf(fdebug, "\tinterupt received...\n");
#endif
        genexit(exitOK);
}

/*  */
/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

genexit(exitcode)
int exitcode;
{
	int rc;

        diag_asl_quit();
        catclose(cat_fdes);

#ifdef DEBUGSCSI
	fprintf(fdebug,"\tExiting...\n\tadap_fdes=%d scsiaddr=%d sciostart_rc=%d\n", saved_adap_fdes, saved_scsiaddr, sciostart_rc);
#endif

	if (sciostart_rc == 0) {
		rc = ioctl(saved_adap_fdes, SCIOSTOP, IDLUN(saved_scsiaddr, lun_id));

#ifdef DEBUGSCSI
	fprintf(fdebug,"\tSCIOSTOP: rc = %d\n", rc);
#endif

	}

	if (saved_adap_fdes >= 0){
        	rc = close(saved_adap_fdes);

#ifdef DEBUGSCSI
	fprintf(fdebug,"\tclose: rc = %d\n", rc);
#endif

	}

	if (init_config_state >= 0) {
		rc = initial_state(init_config_state, device_name);
		if (exitcode == 0)
			exitcode = rc;
	}

	if ((adapt_list != -1) && (adapt_list != NULL))
		odm_free_list(adapt_list, &obj_info);

        exit(exitcode);
}
/*  */
/*
 * NAME: get_scsi_addr()
 *
 * FUNCTION: Gets the scsi address for the adapter chosen by the user
 *           from the CuAt structure found in the odm data base.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
 */

int
get_scsi_addr(struct CuDv *adapt_used)
{
        struct CuAt             *cuat;        /* ODM Customized device struct */
	int			how_many;
	int			rc;
        static struct msglist menuAdapterAddrNotFound[] = {
                        { FDSAID_SET1,  MENU_ADAPTER_ADDR_NOT_FOUND, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
	static ASL_SCR_INFO scrAdapterAddrNotFound[ DIAG_NUM_ENTRIES(menuAdapterAddrNotFound) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        /*
          Search ODM Customized Device Attribute File
        */
        cuat = (struct CuAt *)getattr(adapt_used->name, "id", FALSE,
			&how_many);
	if (cuat == NULL){
                rc = diag_display( menuIdAdapterAddrNotFound, cat_fdes, menuAdapterAddrNotFound, DIAG_IO,
                                   ASL_DIAG_KEYS_ENTER_SC, &menutype,
                                   scrAdapterAddrNotFound);
		genexit(exitAdapterAddrNotFound);
	}

        return(strtoul(cuat->value, (char **) NULL, 0));
} /* endfunction get_scsi_addr */

/*  */
/*
 * NAME: open_scsi_adapter()
 *
 * FUNCTION: Opens the scsi adapter to prepare to send a test unit ready
 *           command
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
 */

int
open_scsi_adapter(struct CuDv *adapt_used)
{
        int     adap_fdes;
	int 	rc;
	int	errno_rc;
        char    *devname;
        char    dname[] = "/dev/";

        static struct msglist menuAdapterOpenError[] = {
                        { FDSAID_SET1,  MENU_ADAPTER_OPEN_ERROR, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
	static ASL_SCR_INFO scrAdapterOpenError[ DIAG_NUM_ENTRIES(menuAdapterOpenError) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        /*
	  create the adapter device name to open
	*/
        devname = (char *) malloc(NAMESIZE + (strlen(dname)+1));
        if (devname != NULL)
                strcpy(devname, dname);
	strcpy(device_name, adapt_used->name);
        strcat(devname, adapt_used->name);
	init_config_state = configure_device(device_name);
	if (init_config_state == -1) {
                rc = diag_display(menuIdAdapterOpenError, cat_fdes,
				menuAdapterOpenError, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrAdapterOpenError);
		genexit(exitAdapterOpenErr);
	}

        adap_fdes = open(devname, O_RDWR, NULL, NULL);
	errno_rc = errno;
        if (adap_fdes >= 0)
                return(adap_fdes);
        else {
                rc = diag_display(menuIdAdapterOpenError, cat_fdes,
				menuAdapterOpenError, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrAdapterOpenError);
                genexit(exitAdapterOpenErr);
	}
}
/*  */
/*
 * NAME: send_inq_command()
 *
 * FUNCTION: Set up adapter. Sends SCIOSTART, then SCIOINQU, then SCIOSTOP
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
 */

int
send_inq_command(adap_fdes, scsiaddr)
int adap_fdes;
int scsiaddr;
{
        int		rc;
        int		errno_rc = -99;	/* value returned by errno lib function */
        char		msgstr[512];	/* substitution string for ASL */
        struct	sc_inquiry     sc_inqu;	/* this struct contains parameters used
                                         * by the ioctl sending the SCIOINQU cmd
                                         */
	char		inqu_buf[INQUIRY_BUF_LEN+1];

                       /* set up struct containing messages displayed on menu */
        static struct msglist menuInquUserConfirm[] = {
                        { FDSAID_SET1,  MENU_INQU_USER_CONFIRM, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static struct msglist menuSciostartError[] = {
                        { FDSAID_SET1,  MENU_SCIOSTART_ERROR, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static struct msglist menuInquDevBusy[] = {
                        { FDSAID_SET1,  MENU_INQU_DEV_BUSY, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
	static struct msglist menuInquInProcess [] = {
			{ FDSAID_SET1, MENU_INQU_IN_PROCESS, },
			(int)NULL
	};
        static ASL_SCR_INFO scrInquUserConfirm[ DIAG_NUM_ENTRIES(menuInquUserConfirm) ];
        static ASL_SCR_INFO scrSciostartError[ DIAG_NUM_ENTRIES(menuSciostartError) ];
	static ASL_SCR_INFO scrInquDevBusy[ DIAG_NUM_ENTRIES(menuInquDevBusy) ];
	static ASL_SCR_INFO scrInquInProcess[DIAG_NUM_ENTRIES(menuInquInProcess) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

	/*
	  In future release, need to figure out lun_id.
	  For now, it can only be 0.
	*/
        sciostart_rc = ioctl(adap_fdes, SCIOSTART, IDLUN(scsiaddr, lun_id));
	errno_rc = errno;

#ifdef DEBUGSCSI
fprintf(fdebug, "%d:\tSCIOSTART: scsiaddr=%d lun_id=%d adap_fdes=%d\n", __LINE__, scsiaddr, lun_id, adap_fdes);
fprintf(fdebug, "\tsciostart_rc=%d errno=%d\n", sciostart_rc, errno_rc);
fflush(fdebug);
#endif
	    
        if (sciostart_rc != 0) {
		switch (errno_rc){
		case EINVAL:
                	rc = diag_display(menuIdInquDevBusy, cat_fdes,
				menuInquDevBusy, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrInquDevBusy);
			break;
		default:
			rc = diag_display(menuIdSciostartError, cat_fdes,
				menuSciostartError, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrSciostartError);
			break;
		}
	} else {
        /*
	  get set to send inquiry command
	*/
		memset(&sc_inqu, 0, sizeof(sc_inqu));
                sc_inqu.scsi_id = scsiaddr; 
                sc_inqu.lun_id = lun_id;              
                sc_inqu.get_extended = (uchar)NULL; 
                sc_inqu.inquiry_len = INQUIRY_BUF_LEN;
                sc_inqu.inquiry_ptr = inqu_buf;
                sc_inqu.flags = (uchar)NULL;
                /*
		  Put scsi address in variable then display screen
		*/
                rc = diag_display(menuIdInquUserConfirm, cat_fdes,
				menuInquUserConfirm, DIAG_MSGONLY,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrInquUserConfirm);
                sprintf(msgstr, scrInquUserConfirm[0].text, scsiaddr);
                free(scrInquUserConfirm[0].text);
                scrInquUserConfirm[0].text = (char *) malloc(strlen(msgstr)+1);
                strcpy (scrInquUserConfirm[0].text, msgstr);
                rc = diag_display(menuIdInquUserConfirm, cat_fdes, NULL, DIAG_IO,
                                   ASL_DIAG_KEYS_ENTER_SC, &menutype,
                                   scrInquUserConfirm);
		diag_asl_clear_screen();
		if (rc != DIAG_ASL_ENTER){
                	errno_rc = ioctl(adap_fdes, SCIOSTOP,
				 IDLUN(scsiaddr, lun_id));
			sciostart_rc = -1;
			return(rc == DIAG_ASL_EXIT ? rc : DIAG_ASL_ENTER);
		}
                rc = diag_display(menuIdInquInProcess, cat_fdes,
			menuInquInProcess, DIAG_IO,
			ASL_DIAG_OUTPUT_LEAVE_SC, &menutype,
			scrInquInProcess);
                 /*
	           issue inquiry cmd
		 */

#ifdef DEBUGSCSI_TEXT
fprintf(fdebug, "%d:\t==== Before 1st SCIOINQU ====\n", __LINE__);
prt_sc_inquiry(&sc_inqu);
fflush(fdebug);
#endif

                 rc = ioctl(adap_fdes, SCIOINQU, &sc_inqu);
                 errno_rc = errno;

#ifdef DEBUGSCSI
fprintf(fdebug, "%d:\t==== After 1st SCIOINQU ====\n", __LINE__);
#ifdef DEBUGSCSI_TEXT
prt_sc_inquiry(&sc_inqu);
#endif
fprintf(fdebug, "\trc= %d errno=%d\n", rc, errno_rc);
fflush(fdebug);
#endif

                 if (rc == -1){
			sleep(3);
                        /*
			  issue inquiry cmd again
			*/
			switch (errno_rc){
			case EIO:
			case ETIMEDOUT:
               		         rc = ioctl(adap_fdes, SCIOINQU, &sc_inqu);
                        	 errno_rc = errno;
				 break;
			case ENOCONNECT:
				 sc_inqu.flags = SC_ASYNC;
               		         rc = ioctl(adap_fdes, SCIOINQU, &sc_inqu);
                        	 errno_rc = errno;
				 break;
			default:
				 break;
			}	

#ifdef DEBUGSCSI
fprintf(fdebug, "%d:\t==== Before analysis ====\n", __LINE__);
#ifdef DEBUGSCSI_TEXT
prt_sc_inquiry(&sc_inqu);
#endif
fprintf(fdebug, "\trc= %d errno=%d\n", rc, errno_rc);
fflush(fdebug);
#endif

		} 
		analyse_result(rc, errno_rc, &sc_inqu);
		/*
		stop path to scsi device
		*/
		rc = ioctl(adap_fdes, SCIOSTOP, IDLUN(scsiaddr, lun_id));
		sciostart_rc = -1;	/* turn off this flag */
        }
        return(DIAG_ASL_ENTER);
}
/*  */
/*
 * NAME: analyse_result()
 *
 * FUNCTION: Analyses the return value from the send_inq_command function.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
 */

void
analyse_result(inqu_rc, errno_rc, sc_inqu)
int	inqu_rc;
int	errno_rc;
struct	sc_inquiry  *sc_inqu;
{
        int		rc;		/* return status */

        /* set up struct containing messages displayed on menus */
        static struct msglist menuInquOK[] = {
                        { FDSAID_SET1,  MENU_INQU_OK, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static struct msglist menuInquNoResponse[] = {
                        { FDSAID_SET1,  MENU_INQU_NO_RESPONSE, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static struct msglist menuInquBusError[] = {
                        { FDSAID_SET1,  MENU_INQU_BUS_ERROR, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static struct msglist menuInquIOError[] = {
                        { FDSAID_SET1,  menuIdInquIOError, },
                        { FDSAID_SET1,  MSG_PRESS_ENTER, },
                        (int)NULL
        };
        static ASL_SCR_INFO scrInquOK[ DIAG_NUM_ENTRIES(menuInquOK) ];
        static ASL_SCR_INFO scrInquNoResponse[ DIAG_NUM_ENTRIES(menuInquNoResponse) ];
        static ASL_SCR_INFO InquBusError[ DIAG_NUM_ENTRIES(menuInquBusError) ];
        static ASL_SCR_INFO scrInqIOError[ DIAG_NUM_ENTRIES(menuInquIOError) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        switch (inqu_rc) {
        case 0:
                rc = diag_display(menuIdInqOK, cat_fdes, menuInquOK,
				DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrInquOK);
                break;
        case -1:
                switch (errno_rc) {
                case ENOCONNECT:
                        rc = diag_display(menuIdInquBusError, cat_fdes,
				menuInquBusError, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				InquBusError);
                        break;
                case ENODEV:
                case EINVAL:
                case ETIMEDOUT:
                        rc = diag_display(menuIdInquNoResponse, cat_fdes,
				menuInquNoResponse, DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC, &menutype,
				scrInquNoResponse);
                        break;
                case EIO:
                default:
			rc = diag_display(menuIdInquIOError, cat_fdes,
					menuInquIOError, DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC,
					&menutype, scrInqIOError);
			break;
                }
                break;
        default:
                break;
        }
} /* endfunction analyse_result */

#ifdef DEBUGSCSI_TEXT
/*
 |
*/
prt_sc_inquiry(struct sc_inquiry *sc_inqu)
{
	int	i;

	fprintf(fdebug, "scsi_id=%d lun=%d \n", sc_inqu->scsi_id, sc_inqu->lun_id);
	for (i=0; i<INQUIRY_BUF_LEN; i++){
		fprintf(fdebug, "%c", (sc_inqu->inquiry_ptr)+i);
		if ((i+1)%80 == 0)
			fprintf(fdebug, "\n");
	}
	fprintf(fdebug, "\n");
}
#endif

/* ^L */
/*
 * NAME: get_max_scsi_id()
 *
 * FUNCTION: get the maximum scsi id that a device supports
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
*/
uchar
get_max_scsi_id(char *uniquetype)
{
	uchar		maxScsiId = 0;
	uchar		scsiId;
	uchar		lunId;
	int		i;
	char		criteria[80];
	struct PdCn	*pdcn;
	struct listinfo	pdcn_info;

	sprintf(criteria, "uniquetype = %s", uniquetype);
	pdcn = get_PdCn_list(PdCn_CLASS, criteria, &pdcn_info, 16, 2);
	if (pdcn == (struct PdCn *) -1)
		return (-1);

	for (i=0; i< pdcn_info.num; i++){
		if (diag_get_sid_lun((pdcn+i)->connwhere,&scsiId,&lunId)==-1){
			maxScsiId = -1;
			break;
		}
		if (maxScsiId < scsiId)
			maxScsiId = scsiId;
	}

	odm_free_list(pdcn, &pdcn_info);

	return(maxScsiId);
}
