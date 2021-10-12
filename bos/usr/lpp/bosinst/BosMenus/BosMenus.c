static char sccsid[] = "@(#) 88 1.52 src/bos/usr/lpp/bosinst/BosMenus/BosMenus.c, bosinst, bos41J, 9519A_all 95/05/04 17:30:43";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS:  - format and driver functions for BOS menus
 *    BOSLangDrvr
 *    BOSLangPre
 *    BOSmainDrvr
 *    BOSmainPre
 *    ConfirmDrvr
 *    ConfirmPre
 *    NLSMenuDrvr
 *    NLSMenuPre
 *    NLSCultureDrvr
 *    NLSCulturePre
 *    NLSMsgTransDrvr
 *    NLSMsgTransPre
 *    NLSKeyboardDrvr
 *    NLSKeyboardPre
 *    NLSKeyboardIDDrvr
 *    NLSKeyboardIDPre
 *    ChgMethodDrvr
 *    ChgMethodPre
 *    CfgSupplementalDrvr
 *    CfgSupplementalPre
 *    ChgRootVGDrvr
 *    ChgRootVGPre
 *    MaintenanceDrvr
 *    MaintenancePre
 *    MksysbTapeDrvr
 *    MksysbTapePre
 *    MksysbConfirmDrvr
 *    MksysbConfirmPre
 *    MksysbChoiceDrvr
 *    MksysbChoicePre
 *    MksysbMapDrvr
 *    MksysbMapPre
 *    RVGWarnPre
 *    RVGWarnDrvr
 *    ShellInfoDrvr
 *    ShellInfoPre
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* BosMenus.c
 *
 * Description: Display the BOSinstall menus
 *	Default values are read from bosinst.data and image.data.  Settings
 *	selected by the user are updated in these files.
 *
 *      Individual menus are displayed by the DisplayMenu function.  
 *      Each menu (struct Menu) is defined in BosMenus.h.  A menu structure
 *      includes a preformat function pointer and a driver function pointer.
 *      DisplayMenu calls the preformat function, waits for user input,
 *	then calls the driver. It is the responsibility of the driver to
 *      determine what menu should be called next.  This file contains all 
 *	the preformat and driver routines.  Naming convention: xxxxPre 
 *	denotes a preformat routine, xxxxDrvr denotes a driver routine.
 *
 * Exit status:
 *	0 - success
 *	1 - internal error occured
 *
 *
 * Environment:
 *	BOSINSTDATA - bosinst.data path
 *	IMAGEDATA - image.data path
 *	TARGETVGS - full path of available disks file
 *	KEYBOARD_CLASS - type of keyboard (101, 102, 106, ascii)
 *	ERRORTEXT - error message passed into BosMenus
 *	ERRORNUM - messsage number of above message
 *	...
 */
#include <stdlib.h>
#include <string.h>
#include <lvm.h>
#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include <sys/stat.h>
#include <termios.h>
#include "ILS.h"
#include "BosInstTrans.h"
#include "BosInst.h"
#include "ImageDat.h"
#include "Menus.h"
#include "BosMenus_msg.h"
#include "BosMenus.h"
#include "BosDisks.h"

/* Some global things */
char *boottype;				/* boot type (numeric)           */
int InstallMethod = PRESERVATION;	/* Install Method                */
int migrateok = 0;			/* migrate install ok flag       */
int preserveok = 0;			/* preservation install ok flag  */
char *keyboard;				/* keyborad class                */
char *turbo;				/* turbo flag                    */
int DoNim;				/* network install flag          */
char *Lang;				/* Language environment		 */
int DoLang;				/* Language select flag          */
int MaintenancePath = 0;		/* Mksysb maintenance path       */

nl_catd catfd;				/* BosMneus message catalog fd   */
struct BosInst BosData;			/* bosinst.data file             */
struct imageDat ImageData;		/* image.data file               */

struct CC  *odm_cc;			/* clutural convention class ptr */
struct MESSAGES *odm_msg;		/* message class ptr             */
struct KEYBOARD *odm_kb;		/* default keyboard              */
struct BosCC *cc, *firstLang;		/* clutural convention class ptr */
struct BosCC *cc_default;		/* default cc (BIfied)           */
struct BosMESSAGES *firstmsg;		/* message class  (BIfied)r      */
struct BosMESSAGES *msg_default;	/* default message  (BIfied)     */
struct BosMESSAGES *msgtrans;		/* message translation  (BIfied) */
struct BosKEYBOARD *kb, *firstkb;	/* keyboard class ptr, first ptr */
struct BosKEYBOARD *kb_default;		/* default keyboard              */
struct KEYBOARD *kb_types[5];		/* default keyboard types        */
struct 
{                                       /* structure defining availble   */
    char *locale;                       /*  install languages            */
    struct MESSAGES *msg;
    char *line1;
    char *line2;
    int index;
} BosLangs[50];
int Customize;                  	/* customize menu choice         */
int maps = 0;				/* maps valid                    */
static struct listinfo CCListInfo;	/* odm list info                 */
static struct listinfo MSGListInfo;	/* odm list info                 */
static struct listinfo KEYListInfo;	/* odm list info                 */
struct disk_info *head =  0; 		/* head of disk list    	 */
int screen_first;			/* 1st visible disk on a screen  */
int defaulted = -1;			/* index to default disk         */
char disk_text[14][80];			/* disk selection text buffer  	*/
struct disk_info *visible_disks[12];	/* disks beign displayed        */
struct disk_info *diskList[255];	/* all disks                    */
int numberDisks = 0;			/* number of disks */
extern struct disk_info *diskList[];	/* array of disk info ptrs       */
char ILSText[14][80];			/* ILS selections            */
int DiskIndexValid;			/* disk indexes valid indicator  */
char *TapeDevices[9];			/* tape install device choices   */

static char *strmethods[3] = 		/* installation methods */
{"Preservation              ",
 "New and Complete Overwrite",
 "Migration                 ",
};

/* VG status - use in ChgDisk amnd MksysbChoice */
static char *vgstat[] = 
{
    "not in any vg",
    "rootvg       ",
    "other vg     "
};

static char *yesno[] = 
{
    "No ",
    "Yes"
};

/* invalid choice error box */
static struct MessageBox InvalidChoice = 
{
    "\n    Invalid Choice.  Please try again\n\n",
    BS_INVALID		/* ILS mesg num */
};
/* Overwrite warning box */
static struct MessageBox OverwriteWarn = 
{
    BM_OVERWRITEWARN,		/* string */
    BS_OVERWRITEWARN		/* ILS mesg num */
};
/* non-Overwrite warning box */
static struct MessageBox NonOverwriteWarn = 
{
    BM_NONOVERWRITEWARN,
    BS_NONOVERWRITEWARN
};
static struct MessageBox NoSpace =
{ "\n    Selected disk does not have enough space\n", BS_NOSPACE};
static struct MessageBox NoDisk =
{ "\n    You must select at least one bootable disk.\n", BS_NODISK};
static struct MessageBox NoDiskSelected =
{ "\n    No disks have been selected.\n", BS_NODISK_SELECT};
static struct MessageBox NoILS =
{ "\n    Unable to open ILS database,\n\
    Language selection Not Available\n",0};

char	*curLevel;		/* current level of AIX. 	 	*/
				/* <version>.<level>.<release>		*/
char	*BOSMENU_LANG;		/* Bos menu language from       	*/
				/* previous execution of BosMenus	*/
static int cc_defaulted;    /* flag indicating ple was not initialy set */
static int msg_defaulted;   /* flag indicating ple was not initialy set */
static int kb_defaulted;    /* flag indicating ple was not initialy set */
static char setlang[20];


/* function prototypes */
int read_bosinstdata(void);
int read_imagedata(void);
struct disk_info *readTargetVGS(void);
struct Menu *DisplayMenu(struct Menu *);
/*---------------------------------------------------------------------------
 * NAME: main
 *
 * FUNCTION: main for BosMenus
 *
 * EXECUTION ENVIRONMENT:
 *      The BosMenus process is started by bi_main
 *      BosMenus runs in the RAM filesystem.
 *
 * NOTES:
 *   read the data files
 *   get some environment variables and determine what menu to call first
 *   go into infinite loop displaying menus
 *
 * DATA STRUCTURES:
 *   reads BosData 
 *   reads ImageData
 *   read disk_info structure (head points to list)
 *   sets InstallMethod, boottype, keyboard, migrateok, preserveok
 *
 * RETURNS: none
 */
void main(int argc, char *argv[])
{
    struct Menu *mptr;			/* menu pointer         */
    struct disk_info *dip;		/* disk info ptr             */
    struct stat statbuf;		/* stat buffer (nim path)    */
    

    /* ignore signals 1,2,3, 15 */
    signal(1, SIG_IGN);
    signal(2, SIG_IGN);
    signal(3, SIG_IGN);
    signal(15, SIG_IGN);

    /* current level of AIX passed as an argument */
    curLevel = argv[1];

    /* Bos menus language from previous execution of BosMenus */
    if(argc > 2)
    {
        BOSMENU_LANG = argv[2];
    }
    else
    {
        BOSMENU_LANG= NULL;
    }

    /* read the bosinst.data, image.data, and targetvgs files */
    read_bosinstdata();
    read_imagedata();
    head = readTargetVGS();

    /* Determine which menu to display first from the environment vaiables.
     */
    boottype = getenv("BOOTTYPE");
    turbo = getenv("BOS_FORMAT");
    keyboard = getenv("KEYBOARD_CLASS");       
    Lang = getenv("LANG");

    if (!keyboard)
	keyboard = "ASCII";

    /* if the Bos menu language was chosen in a previous execution 
     * and is not C, initialize BosMenus for this language
     */
    mptr = NULL;
    if(BOSMENU_LANG != NULL && strcmp(BOSMENU_LANG,"C"))
    {
        struct Class *classp;               /* ODM class pointer       */
        char *classname = "MESSAGES";       /* ODM classnmae           */
        char criteria[256];                 /* search criteria         */
        char catpath[50];
  
        sprintf(criteria,"locale=%s AND bosinst_translated=y",BOSMENU_LANG);
        odm_initialize();
        classp = odm_mount_class(classname);
        odm_msg = odm_get_list(classp, criteria, &MSGListInfo, 50, 1);

        if (((int)odm_msg != -1) && (odm_msg != NULL))
        {
            sprintf(catpath, "/usr/lib/nls/msg/%s/BosMenus.cat",BOSMENU_LANG);
            catfd = catopen(catpath, NL_CAT_LOCALE);
            if((int)catfd != -1)
            {
                /* If there is no language set in bosinst.data, use this
                 * choice as the default
                 */
                if (!*BosData.locale.CULTURAL_CONVENTION)
                {
                    cc_defaulted = 1;
                    strcpy(BosData.locale.CULTURAL_CONVENTION,BOSMENU_LANG);
                }
                if (!*BosData.locale.MESSAGES)
                {
                    msg_defaulted = 1;
                    strcpy(BosData.locale.MESSAGES,BOSMENU_LANG);
                }
                if (!*BosData.locale.KEYBOARD)
                {
                    kb_defaulted = 1;
                    strcpy(BosData.locale.KEYBOARD,BOSMENU_LANG);
                }
                strcpy(BosData.locale.BOSINST_LANG,BOSMENU_LANG);
                sprintf(setlang, "LANG=%s",BOSMENU_LANG);
                putenv(setlang);

                /* translate the invalid choice message */
                InvalidChoice.Text =
                    catgets(catfd, BI, InvalidChoice.MsgNum, InvalidChoice.Text);
                mptr = &BosMain;
                DoLang = 1;
            }
            odm_free_list(odm_msg,&MSGListInfo);
        }
        odm_terminate();
    }
    if(mptr == NULL) 
    {
        mptr = &BOSLang;
    }

    /* Depending on what the vg status is, zap menu items as follows:
     *     3 - Migration: only available if current level is 3.2 or 4.1
     *     2 - Preservation: only available if there is a rootvg
     */

    for (dip = head; dip; dip = dip->next)
    {
	if ( (strcmp(dip->level, "3.2") >= 0) && (strcmp(dip->level, curLevel) < 0) )
        {
	    migrateok = 1;
        }
        else
	{
            /* Not migratable. If default disk and install method is
             * migrate, set preservation. */
            if (!strcmp(BosData.targets->HDISKNAME,dip->name) &&
                    !strcmp(BosData.control_flow.INSTALL_METHOD,"migrate"))
            {
                    /* reset default method to preservation */
                    strcpy(BosData.control_flow.INSTALL_METHOD,  "preserve");
            }
        }
	
	if (strcmp(dip->level,"0.0"))
        {
	    preserveok = 1;
        }
        else
        {
            /* Not rootvg. If default disk, set overwrite. */
            if (!strcmp(BosData.targets->HDISKNAME,dip->name))
            {
                    /* reset default method to overwrite */
                    strcpy(BosData.control_flow.INSTALL_METHOD,  "overwrite");
            }
        }
    }

    if (!strcmp(BosData.control_flow.INSTALL_METHOD, "overwrite"))
	InstallMethod = OVERWRITE;
    else if(!strcmp(BosData.control_flow.INSTALL_METHOD, "preserve"))
	InstallMethod = PRESERVATION;
    else 
	InstallMethod = MIGRATION;


    /* validate the install method choices */
    if ((InstallMethod == MIGRATION) && !migrateok)
	InstallMethod = PRESERVATION;

    if ((InstallMethod == PRESERVATION) && !preserveok)
	InstallMethod = OVERWRITE;


    /* Loop until the cows come home */
    while ( (mptr = DisplayMenu(mptr)));

}

/*----------------------------------------------------------------------------
 * NAME: BOSLangDrvr
 *
 * FUNCTION: Determine what langauge the wants to see during installation.
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus
 *
 * NOTES: If the bosinst.data file didn't have a default for cultural 
 *      convention, langagem or keyboard, this language is used for a defualt.
 *     
 * DATA STRUCTURES: sets catfd 
 *
 * RETURNS: next menu - BOSmain
 */
struct Menu *BOSLangDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int i;		/* loop index */
    char catpath[50];

    /* Display help is requested */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_03, FIG_03);
	return menu;
    }

    if (*input == -1)
    {
	/* An error occurred during preformat, ILS not avialable */
	BosMain.Message = &NoILS;
	return &BosMain;
    }

    /* find the selected entry */
    for (i=0; i < MSGListInfo.num; i++)
    {
	if (BosLangs[i].index == *input)
	{
	    /* this is the one. 
	     * Set the locale to this one and return BosMain
	     */

	    sprintf(catpath, "/usr/lib/nls/msg/%s/BosMenus.cat", 
		    BosLangs[i].msg->locale);
	    if (catfd)
		catclose (catfd);
	    catfd = catopen(catpath, NL_CAT_LOCALE);

	    /* If there is no language set in bosinst.data, use this
	     * choice as the default
	     */
	    if (!*BosData.locale.CULTURAL_CONVENTION || cc_defaulted)
	    {
		cc_defaulted = 1;
		strcpy(BosData.locale.CULTURAL_CONVENTION, BosLangs[i].msg->locale);
	    }
	    if (!*BosData.locale.MESSAGES || msg_defaulted)
	    {
		msg_defaulted = 1;
		strcpy(BosData.locale.MESSAGES, BosLangs[i].msg->locale);
	    }
	    if (!*BosData.locale.KEYBOARD || kb_defaulted)
	    {
		kb_defaulted = 1;
		strcpy(BosData.locale.KEYBOARD, BosLangs[i].msg->locale);
	    }

	    strcpy(BosData.locale.BOSINST_LANG, BosLangs[i].msg->locale); 
	    sprintf(setlang, "LANG=%s", BosLangs[i].msg->locale);
	    putenv(setlang);
            write_bosinstdata();

	    /* free the message stuff, no longer needed */
	    odm_free_list(msgtrans->msg, &MSGListInfo);
	    msgtrans = 0;

	    /* translate the invalid choice message */
	    InvalidChoice.Text = 
		catgets(catfd, BI, InvalidChoice.MsgNum, InvalidChoice.Text);
	    return &BosMain;
	}
    }

    /* Invalid choice */
    bibeep();
    return menu;

}

/*----------------------------------------------------------------------------
 * NAME: BOSLangPre
 *
 * FUNCTION: preformat the lanaguage selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus
 *
 * NOTES:
 *    Get the ILS message structures from the ODM database which have
 *    bosinst_translated == y.  Then to be sure, travese the list and
 *    get the "type nnn for somelanguage during install" message from
 *    each message catalog. If the message is not found, don't display
 *    a default message
 *     
 * DATA STRUCTURES: 
 *    sets DoLang
 *    updates menu structure
 *
 * RETURNS: 1
 */

int BOSLangPre( struct Menu *menu)
{
    struct MESSAGES *msgp;		/* ODM message structure          */
    struct BosMESSAGES *bmsgp;		/* exteneded ODM structure        */
    struct Class *classp;		/* ODM class pointer              */
    char *classname = "MESSAGES";	/* ODM classnmae                  */
    char criteria[256];			/* search criteria                */
    char *ptr, *trans;			/* text manipulation ptrs         */
    static char *langstr = 		/* default language string        */
	"     %d Type %d and press Enter to have English during install.";
    int i, j, k;			/* loop indexes                   */

    DoLang = 1;
    strcpy(criteria, "bosinst_translated=y");

    odm_initialize();
    classp = odm_mount_class(classname);
    odm_msg = odm_get_list(classp, criteria, &MSGListInfo, 50, 1);

    if ((int) odm_msg == -1)
	return -1;

    msgtrans = malloc (MSGListInfo.num * sizeof(struct BosMESSAGES));
    if (!msgtrans)
	exit(2);

    firstmsg = msgtrans;
    menu->DefaultLine = 1;
    
    /* traverse the list, assign the menu select numbers */
    k = 0;
    msgp = odm_msg;
    for (bmsgp = msgtrans, i=1, j = 0; j < MSGListInfo.num; msgp++, j++)
    {
	nl_catd fd;
	char catpath[50];

	ptr = catpath;
	sprintf(catpath, "/usr/lib/nls/msg/%s/BosMenus.cat", msgp->locale);
	fd = catopen(catpath, NL_CAT_LOCALE);

        /* if catopen failed and it's the C locale, provide a default
	 * string.  Other locales don't deserve a default string.
	 */
	trans = catgets(fd, 1, BOSINST, "");

	/* If this didn't work, there is no catalog as advertised by the ILS
	 * database; therefore do not allow it as a choice for selection. 
	 */
	if (!*trans)
	    continue;


	/* allocate space for the strings */
	ptr = malloc(strlen(trans)+6);
	if (!ptr)
	    exit(2);

	/* assign the menu tag number */
	bmsgp->msg = msgp;
	bmsgp->menu_index = i++;
	if ((i % 10) == 0) i +=2;
	sprintf(ptr, trans, bmsgp->menu_index, bmsgp->menu_index);

	
	/* Parse the string for a multi-line translation:
	 * Find the last '\n', replace it w/ NULL.
	 **/
	BosLangs[k].msg =  msgp;
	BosLangs[k].index = bmsgp->menu_index;
	BosLangs[k].line1 = ptr;
	BosLangs[k].line2 = 0;

	ptr = strchr(ptr, '\n');
	if (ptr)
	{
	    *ptr++ = '\0';
	    BosLangs[k].line2 = ptr;
	    ptr = strchr(ptr, '\n');
	    if (ptr)
	    {
		/* blow away a trailing newline if its there */
		*ptr++ = '\0';
	    }
	}

	
        catclose(fd);
	k++; 	/* index through valid boslangs */

    }

    /* If nothing was found, put C as English up there */
    if (i == 1)
    {
	ptr = malloc(strlen(langstr)+6);
	if (!ptr)
	    exit(2);

	sprintf(ptr, langstr, 1, 1);
	BosLangs[0].index = 1;
	BosLangs[0].line1 = ptr;
	BosLangs[0].line2 = 0;
    }


    /* load the appropriate page */
    for (i=0, j=1; (i < 10) && (i < MSGListInfo.num); i++, j++)
    {
	if (!BosLangs[i].line1) break;

        /* is this the default ? */
	if (!strcmp(BosLangs[i].msg->locale, BosData.locale.BOSINST_LANG))
	    menu->DefaultLine = j;
	menu->Text[j] = BosLangs[i].line1;
	if (BosLangs[i].line2)
	{
            j++;
	    menu->Text[j] = BosLangs[i].line2;
	}
    }
    return 1;
}
/*----------------------------------------------------------------------------
 * NAME: BOSmainPre
 *
 * FUNCTION: preformat welcome screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu 
 *
 * NOTES:
 *      Only formats the menus 1 time
 *      If DoLang is not set, the 'BIM_PREV  previous menu' is removed
 *     
 * DATA STRUCTURES: 
 *
 * RETURNS: 1
 */
int BOSmainPre( struct Menu *menu)
{
    static char *inst = 0;	/* instruction multi-line          */
    static struct MessageBox MessageBox;       /* Message box          */
    char *errortext;			/* environment error text */
    char *ptr;			/* text ptr                        */
    int i;

    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* translate multiline welcome */
    if (!inst)
    {
	inst = malloc(strlen(BM_WELCOME)+1);
	if (!inst)
	{
	    exit(2);
	}
    }
    strcpy(inst, BM_WELCOME);
    ptr = catgets(catfd, BI, BS_WELCOME, inst);
    parse(menu, 0, ptr, 2);

    /* If there is a TAPEDEVICE environment var set and
     * it is the name of a that can be read, the user
     * is installing from a tape that is not the original boot
     * device.  Tell the user how to install from the new tape.
     */
    errortext = getenv("TAPEDEVICE");
    if(errortext)
    {
        char tapedevice[11];
        FILE *fp;
        char *ptr;

        fp = fopen(errortext,"r");
        if(fp != NULL) 
        {
            fread(tapedevice,1,9,fp);
            fclose(fp);
            tapedevice[9] = '\0';
            MessageBox.MsgNum = 85;
            /* malloc space for message */
            ptr = malloc(200);
	    if (!ptr)
	    {
	        exit(2);
	    }
            sprintf(ptr, catgets(catfd,10,MessageBox.MsgNum,"Select 1 or 2 to install from tape device %s\n"),tapedevice);
            MessageBox.Text = ptr;
            menu->Message = &MessageBox;
        }     
    }
        
    /* If there is an ERRORTEXT environment var set, load the BOSmain
     * message text with the text and message number
     */
    errortext = getenv("ERRORTEXT");
    if (errortext)
    {    
	char *errorarg;				/* error argument */
	char *ptr;				/* a char ptr     */

	errorarg = getenv("ERRORARG");
	ptr = getenv("ERRORNUM");

	MessageBox.MsgNum = atoi(ptr);

	/* is there an argument to the error message? */
	if (errorarg)
	{
	    /* malloc space for message */
	    ptr = malloc(strlen(errortext) + strlen(errorarg) + 1);
	    sprintf(ptr, catgets(catfd, 10, MessageBox.MsgNum, errortext), 
		    errorarg);
	    
	    MessageBox.Text = ptr;
	}
	else
	    /* Message text passed in from bosmain: hard coded to set 10 */
	    MessageBox.Text= catgets(catfd, 10, MessageBox.MsgNum, errortext); 
	menu->Message = &MessageBox;
    }
	

    /* remove 'BIM_PREV  previous menu' */
    if (!DoLang)
	menu->Text[21] = 0;
    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: BOSmainDrvr
 *
 * FUNCTION: Bos main screen driver
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus
 *
 * NOTES: Determines whether to install AIX or select maintenance utilities
 *     
 * RETURNS: next menu - BOSmain
 */
struct Menu *BOSmainDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* reset error message */
    menu->Message = 0;

    switch (*input)
    {
    case 1:
	return &InstWarn;
	break;
    case 2:
	if (toupper(*ImageData.image_data.PRODUCT_TAPE ) == 'N')
	    return  &MksysbConfirm;
        else
	    return &Confirm;
	break;
    
    case 3:
	/* Utilities Menu */
	return &Maintenance;

    case BIM_HELP:
	/* help */
	DisplayHelp(BM_FIG_04, FIG_04);
	return menu;

    case BIM_PREV:
	if (DoLang)
	    return &BOSLang;
    default:
	menu->Message = &InvalidChoice;
	return menu;
    }
}

/*----------------------------------------------------------------------------
 * NAME: InstWarnPre
 *
 * FUNCTION: preformat welcome screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu 
 *
 * NOTES:
 *      Only formats the menus 1 time
 *      If DoLang is not set, the 'BIM_PREV  previous menu' is removed
 *     
 * DATA STRUCTURES: 
 *
 * RETURNS: 1
 */
int InstWarnPre( struct Menu *menu)
{
    static char *inst = 0;	/* instruction multi-line          */
    char *ptr;			/* text ptr                        */
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* translate multiline welcome */
    if (!inst)
    {
	inst = malloc(strlen(BM_WARN_INST)+1);
	if (!inst)
	{
	    exit(2);
	}
    }
    strcpy(inst, BM_WARN_INST);
    ptr = catgets(catfd, BI, BS_WARN_INST, inst);
    parse(menu, 3, ptr, 2);

    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: InstWarnDvr
 *
 * FUNCTION: Install warning popup driver
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus
 *
 * NOTES: Determines whether to install AIX or select maintenance utilities
 *     
 * RETURNS: next menu - BOSmain
 */
struct Menu *InstWarnDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* reset error message */
    menu->Message = 0;

    switch (*input)
    {
    case 1:
	/* write out the files */
	write_bosinstdata();
	write_imagedata();

	/* exit 0 status to bi_main */
	exit(0);
	break;

    case BIM_HELP:
	/* help */
	DisplayHelp(BM_FIG_041, FIG_041);
	return menu;

    case BIM_PREV:
	    return &BosMain;
    default:
	menu->Message = &InvalidChoice;
	return menu;
    }
}

/*----------------------------------------------------------------------------
 * NAME: ConfirmDrvr
 *
 * FUNCTION: driver for confirm screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      user input of 0 writes out selections and exits
 *      
 *     
 * DATA STRUCTURES: 
 *
 * RETURNS: next menu 
 */
struct Menu *ConfirmDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* reset error message */
    menu->Message = 0;

    switch (*input)
    {
    case 0:
	/* update settings file and exit */
	write_bosinstdata();
	write_imagedata();

	/* exit 0 status to bi_main */
	exit(0);
	break;

    case 1:
	/* If we can't preserve, the only possible choice is overwrite,
	 * so skip the method screen
	 */
	if (!preserveok)
	    return &MksysbChoice;

	/* Next menus is change method of install */
	return &ChgMethod;
    
    case 2:
	/* go to NLS screen */
	return &NLSMenu;
	break;

	/* Trusted Computing Base */
	
    case 3:
	if (*BosData.control_flow.TCB == 'y')
	    strcpy(BosData.control_flow.TCB, "no ");
	else
	    strcpy(BosData.control_flow.TCB, "yes");
	return menu;
	break;

    case BIM_HELP:
	/* display help screen */
	DisplayHelp(BM_FIG_06, FIG_06);
	return menu;
	break;

    case BIM_PREV:

	return &BosMain;
	break;

    default:
	/* redisplay menu */
	bibeep();
	menu->Message = &InvalidChoice;
	return menu;
	break;
    }
}
/*----------------------------------------------------------------------------
 * NAME: ConfirmPre
 *
 * FUNCTION: preformat the confirm screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *   	static storage for 5 lines of menus text are declared here:
 *	  install method, destination disk, cultural convention, message
 *	  translation, keyboard map, and keyboard type.
 *     
 * DATA STRUCTURES: 
 *	menu is updated
 *
 * RETURNS: 1
 */
int ConfirmPre( struct Menu *menu)
{
    /* static copy of menu text; address can be passed to th men system */
    static char method[80], 		/* mehtod of install        */
		destination[80],	/* new permanent strings    */
		TCB[80],		/*trusted computing base    */
		cultureConv[80],        /* NLS cultural conventions */
		msgTrans[80],		/* NLS message transation   */
		kbmap[80],		/* NLS keyboard map         */
		kbtype[80];		/* NLS keyboard type        */

    static char buf[190];		/* message tmp buffer       */
    static char *inst = 0;		/* instruction line         */
    struct Class *classp;		/* odm class ptr            */
    char criteria[30];			/* search criteria          */
    struct CC *ccp;			/* ODM CC structure         */
    struct MESSAGES *msgp;		/* ODM MESSAGES structure   */
    struct KEYBOARD *kbp;		/* ODM KEYBOARD structure   */
    struct listinfo ListInfo;		/* odm list info            */
    struct listinfo KListInfo;		/* odm list info            */

    char *ptr;			/* text ptr                        */
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }


    /* Parse the multi-line instruction field */
    if (!inst)
    {
	inst = malloc(strlen(BM_CONFIRM)+1);
	if (!inst) exit(2);

	/* this is the first pass.  If CC, MESSAGES, or KEYBOARD is not set,
	 * set it to C.
	 */
	if (!*BosData.locale.BOSINST_LANG)
	    strcpy(BosData.locale.BOSINST_LANG,"C");
	if (!*BosData.locale.CULTURAL_CONVENTION)
	    strcpy(BosData.locale.CULTURAL_CONVENTION,"C");
	if (!*BosData.locale.MESSAGES)
	    strcpy(BosData.locale.MESSAGES,"C");
	if (!*BosData.locale.KEYBOARD)
	    strcpy(BosData.locale.KEYBOARD,"C");
    }
    strcpy(inst, BM_CONFIRM);
    ptr = catgets(catfd, BI, BS_CONFIRM, inst);
    parse(menu, 2, ptr, 2);

    /* Need to change the menu to reflect the current settigns.
     * Copy the menu strings to the new storage area. Note that
     * although ALL selected disks are stored in TARGETDISKS file,
     * only the first is displayed on the confirmation
     */
    strcpy(method, menu->Text[6]);
    strcpy(destination, menu->Text[7]);

    /* copy the current method into the string */
    strcpy(&method[44], 
       catgets(catfd, BI, BS_IM_PRES+InstallMethod, strmethods[InstallMethod]));
    strcpy(&destination[44], BosData.targets->HDISKNAME);

    /* add eleipses if necessary */
    if (BosData.targets->next)
    {
	strcat(&destination[44], "...");
    }

    /* set the addresses of the menu text to point to the above static
     * storage
     */
    menu->Text[6] = method;
    menu->Text[7] = destination;

    /* Get the cultural conventions and other locale stuff */
    strcpy(cultureConv, menu->Text[10]);
    strcpy(msgTrans, menu->Text[11]);
    strcpy(kbmap, menu->Text[12]);
    strcpy(kbtype, menu->Text[13]);

    /* copy the current values into the text buffers */


    /* get the odm record for this CC and translate it */
    odm_initialize();
    classp = odm_mount_class("CC");
    sprintf(criteria, "locale=%s",BosData.locale.CULTURAL_CONVENTION); 
    odm_cc = odm_get_list(classp, criteria, &ListInfo, 1, 1);
    
    if (odm_cc)
	strcpy(&cultureConv[44], 
	    catgets(catfd, BL, odm_cc->text_string_id, odm_cc->text_string));
    else
	strcpy(&cultureConv[44], BosData.locale.CULTURAL_CONVENTION);

    /* free the cc from the odm get */
    odm_free_list(odm_cc, &ListInfo);

    /* get the odm record for this message tranlation and translate it */
    classp = odm_mount_class("MESSAGES");
    sprintf(criteria, "locale=%s",BosData.locale.MESSAGES); 
    odm_msg = odm_get_list(classp, criteria, &ListInfo, 1, 1);

    
    if (odm_msg)
	strcpy(&msgTrans[44],
	    catgets(catfd, BL, odm_msg->text_string_id, odm_msg->text_string));
    else
	strcpy(&msgTrans[44], BosData.locale.MESSAGES);
    /* free the cc from the odm get */
    odm_free_list(odm_msg, &ListInfo);


    /* get the odm record for this keyboard and translate it. */
    classp = odm_mount_class("KEYBOARD");
    sprintf(criteria, "keyboard_map=%s", BosData.locale.KEYBOARD);
    odm_kb = odm_get_list(classp, criteria, &ListInfo, 1, 1);

    if (odm_kb)
	strcpy(&kbmap[44], 
	    catgets(catfd, BL, odm_kb->text_string_id, odm_kb->text_string));
    else
	strcpy(&kbmap[44], BosData.locale.KEYBOARD);

    /* Determine whether or not to display the keyboard type line:
     *   get the CC that matches this keyboard's locale.
     *   if there is only one keyboard type, don't display the line.
     *   if the keyboard_map matches the fist keybaord in the list, display
     *   default, otherwise display alternate
     */

    classp = odm_mount_class("CC");
    sprintf(criteria, "locale=%s",odm_kb->locale);
    ccp = odm_get_list(classp, criteria, &KListInfo, 1, 1);
    if (ccp)
    {
	char *ptr;
	if ((ptr = strchr(ccp->keyboards, ' ')))
	{
	    if (!strcmp(ptr+1, odm_kb->keyboard_map))
		menu->Text[13] = catgets(catfd, BI, BS_ALTERNATE, BM_ALTERNATE);
	    else
		menu->Text[13] = catgets(catfd, BI, BS_DEFAULT, BM_DEFAULT);
	}
	else
	    menu->Text[13] = "";
	
	/* free the list */
	odm_free_list(ccp, &ListInfo);
	
    }
    else
	menu->Text[13] = "";

    /* free the list */
    odm_free_list(odm_kb, &ListInfo);

    /* force the menus to use the static buffers */
    menu->Text[10] = cultureConv;
    menu->Text[11] = msgTrans;
    menu->Text[12] = kbmap;

    /* update the TCB line */
    strcpy(TCB, menu->Text[15]);

    i = strlen(TCB);
    while ( i < 43 )	/* Pad message in case of short message */
	TCB[i++] = '.';
    TCB[43] = ' ';	/* Force a space in case of long message */

    if (*BosData.control_flow.TCB == 'y')
	strcpy(&TCB[44], catgets(catfd, BI, BS_YES, "yes"));
    else
	strcpy(&TCB[44], catgets(catfd, BI, BS_NO, "no "));

    menu->Text[15] = TCB;

    /* Display the warning message if its not already set */
    if (!menu->Message)
    {
	char *cp;
	if (InstallMethod == OVERWRITE)
	{
	    cp=catgets(catfd, BI, OverwriteWarn.MsgNum, BM_OVERWRITEWARN);
	    sprintf(buf,cp, BosData.targets->HDISKNAME);
	    OverwriteWarn.Text = buf;
	    menu->Message = &OverwriteWarn;
	}
	else
	{
	    sprintf(buf, 
	       catgets(catfd, BI, NonOverwriteWarn.MsgNum, BM_NONOVERWRITEWARN),
		BosData.targets->HDISKNAME);
	    NonOverwriteWarn.Text = buf;
	    menu->Message = &NonOverwriteWarn;
	}
    }

    return 1;

}

/*----------------------------------------------------------------------------
 * NAME: NLSMenuDrvr
 *
 * FUNCTION: driver for NLS selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      Page forward and page backward is computed based on an input value
 *      which is a multiple of a page size (10).
 *     
 * DATA STRUCTURES: 
 *	updates locale stanza of BosData
 *
 * RETURNS: next menu 
 */
struct NLS_screens
{
    char *screen_start;		/* generic ptr to CC, MSG, KYBD */
    int page_fwd;		/* page fwd id			*/
    int page_back;		/* page backwd			*/
} NLS_Screens[8];		/* can't go larger than 8 screens */
int NLS_Screens_index = 0;

struct Menu *NLSMenuDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int index;			/* loop index				*/
    static int bump = 0;	/* amount to skip to next page          */
    struct BosCC *ccp;		/* Bos Inst CC structure                */
    struct CC *cp;		/* ODM CC structure                     */

    /* reset error message */
    menu->Message = 0;

    /* Process the entered key */

    /* If the "input" is -1, it's a format error, language selection
     * is not available
     */
    if (*input == -1)
    {
	Confirm.Message = &NoILS;
	return &Confirm;
    }

    /* User selected customize */
    if (*input == Customize)
    {
	free(cc);
	cc = 0;
	return &NLSCulture;
    }

    /* page fwd */
    if (*input == NLS_Screens[NLS_Screens_index].page_fwd)
    {
	firstLang = (struct BosCC *) NLS_Screens[++NLS_Screens_index].screen_start;
	return menu;

    }
    /* page back */
    if (*input == NLS_Screens[NLS_Screens_index].page_back)
    {
	firstLang = (struct BosCC *) NLS_Screens[--NLS_Screens_index].screen_start;
	return menu;
    }
    /* process help */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_12, FIG_12);
	return menu;
    }

    /* process previous menu */
    if (*input == BIM_PREV)
    {
	/* wipe out the cc list */
	odm_free_list(odm_cc, &CCListInfo);
	cc = 0;
	return &Confirm;
    }

    /* find out what was selected */
    for(ccp=firstLang, index=0; 
       (index < 9) && (ccp <= cc+CCListInfo.num);
       index++, ccp++)
    {
	if (ccp->menu_index == *input)
	{
	    /* this is the one */
	    cc_default = ccp;
	    strcpy(BosData.locale.CULTURAL_CONVENTION, ccp->cc->locale);
	    strcpy(BosData.locale.MESSAGES, ccp->cc->messages);
	    strcpy(BosData.locale.KEYBOARD, ccp->cc->keyboards);

	    /* Determine if there is an alternate KB type. Find the CC which
	     * has this locale.  If there are 2 or more keyboards in the
	     * keyboards field, we need to call the KeyboardID menu
	     */
	    if (strchr(ccp->cc->keyboards,' '))
	    {
		return &NLSKeyboardID;
	    }
	    return &Confirm;
	}
    }

    /* Must be invalid, couldn't figure out what to do with it */
    bibeep();
    menu->Message = &InvalidChoice;
    return menu;


}

/*----------------------------------------------------------------------------
 * NAME: NLSMenuPre
 *
 * FUNCTION: preformat the predefined locale selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	All ILS data is retrieved from the ODM database.  The CC struct 
 *	contains links to MESSAGES and FONT structures, so that only one
 *	odm_get_list is required.
 *
 *	Since odm_get_list cannot process an "OR" in search criteria, all
 *	CCs are retrieved, then filterd.
 *     
 * DATA STRUCTURES: 
 *	updates locale stanze of BosData
 *	sets Customize
 *
 * RETURNS: 1
 */

int NLSMenuPre( struct Menu *menu)
{

    static int numCC = 0;		/* the number of CC used     */
    struct CC *ccp, *ccpc;		/* CC manipulation ptrs      */
    struct BosCC *bccp;			/* BosCC manipulation ptr    */
    struct Class *classp;		/* ODM class ptr             */
    char *classname = "CC";		/* classname to retireve     */
    char *cptr;				/* menu text ptr             */
    char *ptr;				/* a text ptr		     */
    char criteria[40];			/* search criteria           */
    static char buf[512];		/* storage for instructions  */
    static char combo[160];		/* storage for customize text */
    int i,j;				/* indexes                   */
  
    menu->DefaultLine = -1;
    /* if there is no cc list, open the odm database and retrieve it */
    if (!cc)
    {


	/* Criteria: use no criteria.  filter out later
	 */
	if (keyboard[0] != 'A')
	    sprintf(criteria, "menu like *%s* and bosinst_menu = y", keyboard);
	else
	    strcpy(criteria, "bosinst_menu = y");

	odm_initialize();
	classp = odm_mount_class(classname);

	odm_cc = odm_get_list(classp, criteria, &CCListInfo, 50, 4);

	
	/* Check validity of odm_cc */
	if ((int) odm_cc == -1) 
	    return -1;

	cc = malloc(CCListInfo.num * sizeof (struct BosCC ));
	if (!cc)
	    exit(2);

        firstLang = cc_default = cc;

	/* traverse the list, assign the menu select numbers */
	numCC = 0;
	ccp = odm_cc;
	for (bccp=cc, i=1, j=0; j < CCListInfo.num; ccp++, j++)
	{
	    bccp->cc = ccp;
	    bccp->menu_index = i++;
	    if ((i % 10) == 0) i +=2;
	    
	    
	    /* Should this be the default? */
	    if (!strcmp(ccp->locale, BosData.locale.CULTURAL_CONVENTION))
		cc_default = bccp;
	    
	    /* bump the real list ptr */
	    numCC++;
	    bccp++;
	}
	/* The last 'i' is the number of the choice for customize */
	Customize = i;

	/* if customize is the last selectable item on a page, subtract 2
	 * from it's menus index.
	 */
	if (((bccp-1)->menu_index+1) %10 == 0)
	    Customize -= 2;

        /* initialize the first stack frame */
	NLS_Screens_index = 0;
	NLS_Screens[0].page_back = 999;
	NLS_Screens[0].screen_start = (char *) cc;

    }
    /* Load up the instruction line */
    sprintf(buf, catgets(catfd, BI, BS_LOCALE_MSG, BM_LOCALE_MSG), Customize);
    parse(menu, 2, buf, 4);
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }


    for (i=0; i < 11; i++)
	menu->Text[8+i] = 0;

    i=0;
    if (firstLang != cc)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(ILSText[0] , 
	   catgets(catfd, BI, BS_PREV_CHOICE, "    %2d  ...PREVIOUS CHOICES"),
		   firstLang->menu_index-1);
	menu->Text[8] = ILSText[0];
	menu->DefaultLine = 8;
	i++;
	/* set the page backward index */
	NLS_Screens[NLS_Screens_index].page_back = firstLang->menu_index-1;
    }
    for(bccp = firstLang; (bccp < cc + numCC) && (i < 9); i++)
    {
	memset(ILSText[i], ' ', 80);
	sprintf(ILSText[i], "    %2d %-23.23s %-23.23s %-23.23s",
	    bccp->menu_index, 
	    catgets(catfd, BL, bccp->cc->text_string_id, bccp->cc->text_string),
	    catgets(catfd, BL, bccp->cc->messageLink->text_string_id, 
		    bccp->cc->messageLink->text_string),
            catgets(catfd, BL, bccp->cc->keyboardLink->text_string_id,
		    bccp->cc->keyboardLink->text_string));
	menu->Text[8+i] = ILSText[i];

	if (bccp == cc_default)
	    menu->DefaultLine = i+8;
	bccp++;
    }

    /* if there are more choices, display more choices */
    if (bccp <  cc + numCC)
    {
	sprintf(ILSText[i] , 
	       catgets(catfd, BI, BS_MORE_CHOICE, "    %2d  MORE CHOICES..."),
		      (bccp-1)->menu_index+1);
	menu->Text[8+i] = ILSText[i];
	if (menu->DefaultLine == -1)
	    menu->DefaultLine = i+8;

	/* set the page forward index */
	NLS_Screens[NLS_Screens_index+1].screen_start = (char*) bccp;
	NLS_Screens[NLS_Screens_index].page_fwd = (bccp-1)->menu_index+1;
    }
    else
    {
	/*  translate */
	cptr = catgets(catfd, BI, BS_LOCALE_COMBO, BM_LOCALE_COMBO);
	sprintf(combo , cptr, Customize);
	parse(menu, 8+i, combo, 2);
	/* set the page forward index */
	NLS_Screens[NLS_Screens_index].page_fwd = 999;
    }

    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: NLSCultureDrvr
 *
 * FUNCTION: Selection driver for CC screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      Page forward and page backward is computed based on an input value
 *      which is a multiple of a page size (13).
 *      
 *     
 * DATA STRUCTURES: 
 *	Bosdata locale stanza is updated
 *
 * RETURNS: next menu 
 */
struct Menu *NLSCultureDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    struct BosCC *ccp;		/* BosInst CC struct            */
    static int bump=0;		/* amount to skip to next page  */
    int index;			/* loop index                   */

    /* clear message box */
    menu->Message = 0;

    if (*input%13 == 0)
    {
	/* it's a page forward, see if it's valid */
	if (firstLang==cc)
	    bump =12;
	else
	    bump =11;

	firstLang += bump;
	if (firstLang  > (cc + CCListInfo.num-1))
	{
	    firstLang-=bump;
	    menu->Message = &InvalidChoice;
	}
	return menu;

    }
    if ((*input%13 == 1) && (*input != 1))
    {
	/* it's a page backward, see if it's valid */
	if (firstLang == cc)  
	{
	    menu->Message = &InvalidChoice;
	    return menu;
	}

	/* back up one page
	 */
	firstLang -=bump;
	if (((unsigned int)firstLang - (unsigned int)cc) < 12)
	    firstLang = cc;
	return menu;
    }
    /* process help */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_16, FIG_16);
	return menu;
    }

    /* process previous menu */
    if (*input == BIM_PREV)
    {
	/* wipe out the cc list */
	cc = 0;
	return &NLSMenu;
    }

    /* find out what was selected */
    for(ccp=firstLang, index=0; 
       (index < 13) && (ccp <= cc+CCListInfo.num);
       index++, ccp++)
    {
	if (ccp->menu_index == *input)
	{
	    /* this is the one */
	    cc_default = ccp;
	    strcpy(BosData.locale.CULTURAL_CONVENTION, ccp->cc->locale);
	    return &NLSMsgTrans;
	}
    }

    /* Must be invalid, couldn't figure out what to do with it */
    menu->Message = &InvalidChoice;
    return menu;

}


/*----------------------------------------------------------------------------
 * NAME: NLSCulturePre
 *
 * FUNCTION: preformat the CC selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	First pass through the the CCs are retrieved and storage allocated for
 *	the traversal list (cc).  Each element of cc contains a pointer to
 *	a CC structure from ODM and a menu index.  
 *     
 *	Subsequent passes load the menu with a page of choices from the cc
 *	list.
 *
 * DATA STRUCTURES: 
 * 	menu updated
 *
 * RETURNS: 1
 */
int NLSCulturePre( struct Menu *menu)
{
    struct BosCC *bccp;			/* Bos Inst CC struct ptr         */
    struct CC *ccp;			/* ODM CC struct                  */
    struct Class *classp;		/* ODM class ptr                  */
    char *classname = "CC";		/* ODM class name                 */
    char criteria[25];			/* search criteria                */
    int i;				/* loop index                     */
    static char *ptr = 0;		/* catgets ptr */
    static char *inst;
  
    menu->DefaultLine = -1;
    /* if there is no cc list, open the odm database and retrieve it */
    if (!cc)
    {
	strcpy(criteria, "bosinst_menu = y");

	odm_initialize();
	classp = odm_mount_class(classname);

	ccp = odm_get_list(classp, criteria, &CCListInfo, 50, 4);

	/* allocate storage for the pointers */
	cc = malloc(CCListInfo.num * sizeof(struct BosCC ));
	if (!cc)
	    exit(2);

	firstLang = cc_default = cc;
	
	/* traverse the list, assign the menu select numbers */
	for (bccp = cc, i=1; bccp < cc+ CCListInfo.num; bccp++)
	{
	    bccp->cc = ccp;
	    bccp->menu_index = i++;
	    if ((i % 13) == 0) i +=2;

	    /* Should this be the default? */
	    if (!strcmp(bccp->cc->locale, BosData.locale.CULTURAL_CONVENTION))
		cc_default = bccp;
	    
	    /* increment the "real" list ptr */
	    ccp++;
	}

    }
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* get the mulitline instruction lines */
    if (!inst)
    {
	inst = malloc(strlen(BM_CC_INST)+1);
	if (!inst) exit(2);
    }
    strcpy(inst, BM_CC_INST);
    ptr = catgets(catfd, BI, BS_CC_INST, inst);
    parse(menu, 2, ptr, 2);


    /* Clear the selection area */
    for (i=6; i < 19; i++)
    {
	menu->Text[i] = 0;
    }

    i=0;
    if (firstLang != cc)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(ILSText[0] , 
	    catgets(catfd, BI, BS_PREV_CHOICE, "    %2d ...PREVIOUS CHOICES"),
	    firstLang->menu_index -1);
	i++;
	menu->DefaultLine = 6;
	menu->Text[6] = ILSText[0];
    }
    for(bccp = firstLang; (bccp < (cc + CCListInfo.num)) && (i < 12); i++)
    {
	memset(ILSText[i], ' ', 80);
	sprintf(ILSText[i], "    %2d %s",
	    bccp->menu_index, 
	    catgets(catfd, BL, bccp->cc->text_string_id, bccp->cc->text_string));
	menu->Text[6+i] = ILSText[i];

	if (bccp == cc_default)
	    menu->DefaultLine = i+6;
	bccp++;
    }
    /* if there are more choices, display more choices */
    if (bccp < cc+CCListInfo.num)
    {
	sprintf(ILSText[i] , 
	    catgets(catfd, BI, BS_MORE_CHOICE, "    %2d MORE CHOICES..."),
	    (bccp-1)->menu_index+1);
	menu->Text[6+i] = ILSText[i];
	if (menu->DefaultLine == -1)
	    menu->DefaultLine = i+6;
    }
    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: NLSMsgTransDrvr
 *
 * FUNCTION: Process user input for message tranlation screen 
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *	Page forward and backward are computed based on number of entries
 *	on a page (13).
 *      
 *     
 * DATA STRUCTURES: 
 *	BosData locale stanza updated
 *
 * RETURNS: next menu 
 */
struct Menu *NLSMsgTransDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    struct BosMESSAGES *msgp;		/* BosInst MESSAGES struct     */
    int index;				/* loop index                  */
    int bump;				/* amount to skip to next page */

    /* clear message box */
    menu->Message = 0;

    if (*input%13 == 0)
    {
	/* it's a page forward, see if it's valid */
	if (firstmsg==msgtrans)
	    bump =12;
	else
	    bump =11;

	firstmsg+=bump;
	if (firstmsg  > (msgtrans + MSGListInfo.num-1))
	{
	    firstmsg-=bump;
	    menu->Message = &InvalidChoice;
	}
	return menu;

    }
    if ((*input%13 == 1) && (*input != 1))
    {
	/* it's a page backward, see if it's valid */
	if (firstmsg == msgtrans) 
	{
	    menu->Message = &InvalidChoice;
	    return menu;
	}

	firstmsg = msgtrans + (firstmsg->menu_index/13-1) * 12;
	return menu;
    }

    /* process help */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_18, FIG_18);
	return menu;
    }

    /* process previous menu */
    if (*input == BIM_PREV)
    {
	return  &NLSCulture;
    }
    /* find out what was selected */
    for(msgp=firstmsg, index=0; 
       (index < 13) && (msgp <= msgtrans+MSGListInfo.num);
       index++, msgp++)
    {
	if (msgp->menu_index == *input)
	{
	    /* this is the one */
	    msg_default = msgp;
	    strcpy(BosData.locale.MESSAGES, msgp->msg->locale);
	    return &NLSKeyboard;
	}
    }

    /* Must be invalid, couldn't figure out what to do with it */
    bibeep();
    menu->Message = &InvalidChoice;
    return menu;


}


/*----------------------------------------------------------------------------
 * NAME: NLSMsgTransPre
 *
 * FUNCTION: preformat the  message translation selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	First pass through the the MESSAGES are retrieved and storage 
 *	allocated for the traversal list (msg).  Each element of msg 
 *	contains a pointer to MESSAGES structure from ODM and a menu index.  
 *     
 *	Subsequent passes load the menu with a page of choices from the cc
 *	list.
 *
 *     
 * DATA STRUCTURES: 
 *	menu updated
 *
 * RETURNS: 1
 */
int NLSMsgTransPre( struct Menu *menu)
{
    struct MESSAGES *msgp;		/* ODM MESSAGESA struct             */
    struct BosMESSAGES *bmsgp;		/* BosInst MESSAGES struct          */
    struct Class *classp;		/* ODM class ptr                    */
    char *classname = "MESSAGES";	/* ODM class name                   */
    char criteria[256];			/* search criteria                  */
    int i;				/* loop index                       */
    char *ptr;				/* text ptr			    */
  
    menu->DefaultLine = -1;
    /* if there is no cc list, open the odm database and retrieve it */
    if (!msgtrans)
    {
	strcpy(criteria, "bosinst_menu = y");

	odm_initialize();
	classp = odm_mount_class(classname);

	msgp = odm_get_list(classp, criteria, &MSGListInfo, 50, 4);
	msgtrans = malloc(MSGListInfo.num * sizeof (struct BosMESSAGES));
	if (!msgtrans)
	    exit(2);

	firstmsg = msgtrans;
	
	/* traverse the list, assign the menu select numbers */
	for (bmsgp = msgtrans, i=1; bmsgp < msgtrans+MSGListInfo.num; msgp++)
	{
	    /* determin default */
	    if (!strcmp(msgp->locale, odm_msg->locale))
		msg_default = bmsgp;
	    bmsgp->msg = msgp;
	    bmsgp->menu_index = i++;
	    if ((i % 13) == 0) i +=2;

	    /* bump the ptr */
	    bmsgp++; 
	}
    }

    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* Clear the selection area */
    for (i=6; i < 19; i++)
    {
	menu->Text[i] = 0;
    }

    i=0;
    if (firstmsg != msgtrans)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(ILSText[0] , 
	    catgets(catfd, BI, BS_PREV_CHOICE, "    %2d ...PREVIOUS CHOICES"),
	    firstmsg->menu_index-1);
	menu->Text[6] = ILSText[0];
	menu->DefaultLine = 6;
	i++;
    }
    for(bmsgp = firstmsg; (bmsgp < msgtrans + MSGListInfo.num) && (i < 12); i++)
    {
	memset(ILSText[i], ' ', 80);

	/* translate the message strings */
	sprintf(ILSText[i], "    %2d  %s",
	    bmsgp->menu_index, 
	    catgets(catfd, BL, bmsgp->msg->text_string_id, 
		    bmsgp->msg->text_string));
	menu->Text[6+i] = ILSText[i];

	if (bmsgp == msg_default)
	    menu->DefaultLine = i+6;
	bmsgp++;
    }
    /* if there are more choices, display more choices */
    if (bmsgp < msgtrans+MSGListInfo.num)
    {
	sprintf(ILSText[i] , 
	    catgets(catfd, BI, BS_MORE_CHOICE, "    %2d MORE CHOICES..."),
	    (bmsgp-1)->menu_index +1);
	menu->Text[6+i] = ILSText[i];
	if (menu->DefaultLine == -1)
	    menu->DefaultLine = i+6;
    }
    return 1;
}


/*----------------------------------------------------------------------------
 * NAME: NLSKeyboardDrvr
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *	Page forward and backward are computed based on number of entries
 *	on a page (13).
 *
 * DATA STRUCTURES: 
 *	menu updated
 *
 * RETURNS: next menu 
 */
struct Menu *NLSKeyboardDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    struct BosKEYBOARD *kbp;		/* Bosinst KEYBOARD struct       */
    struct BosCC *bccp;			/* BosInst CC struct             */
    int index;				/* loop index			 */
    static int bump;			/* amount to skip to next page   */

    /* clear message box */
    menu->Message = 0;

    if (*input%13 == 0)
    {
	/* it's a page forward, see if it's valid */
	if (firstkb==kb)
	    bump =12;
	else
	    bump =11;
	firstkb +=bump;

	if (firstkb  > (kb + KEYListInfo.num-1))
	{
	    firstkb-=bump;
	    menu->Message = &InvalidChoice;
	}
	return menu;

    }
    if ((*input%13 == 1) && (*input != 1))
    {
	/* it's a page backward, see if it's valid */
	if (firstkb == kb)
	{
	    menu->Message = &InvalidChoice;
	    return menu;
	}

	firstkb -= bump;
	if (((unsigned int)firstkb - (unsigned int)kb) < 12)
	    firstkb = kb;
	return menu;
    }

    /* process help */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_19, FIG_19);
	return menu;
    }

    /* process previous menu */
    if (*input == BIM_PREV)
    {
	return &NLSMsgTrans;
    }

    /* find out what was selected */
    for(kbp=firstkb, index=0; 
       (index < 13) && (kbp <= kb+KEYListInfo.num);
       index++, kbp++)
    {
	if (kbp->menu_index == *input)
	{
	    /* this is the one */
	    kb_default = kbp;
	    strcpy(BosData.locale.KEYBOARD, kbp->kb->keyboard_map);

	    /* need to free the CC list here */
	    odm_free_list(cc->cc, &CCListInfo);
	    free(cc);
	    cc = 0;
	    return &Confirm; 
	}
    }

    /* Must be invalid, couldn't figure out what to do with it */
    menu->Message = &InvalidChoice;
    return menu;


}


/*----------------------------------------------------------------------------
 * NAME: NLSKeyboardPre
 *
 * FUNCTION: preformat the keyboard selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	First pass through, the KEYBOARDS are retrieved and storage 
 *	allocated for the traversal list (kb).  Each element of kb 
 *	contains a pointer to KEYBOARDS structure from ODM and a menu index.  
 *     
 *	Subsequent passes load the menu with a page of choices from the kb
 *	list.
 *     
 * DATA STRUCTURES: 
 *	kb list allocated
 *	menu update
 *
 * RETURNS: 1
 */
int NLSKeyboardPre( struct Menu *menu)
{
    struct KEYBOARD *kbp;		/* ODM KEYBOARD struct           */
    struct BosKEYBOARD *bkbp;		/* BosInst KEYBOARD struct       */
    struct Class *classp;		/* ODM class ptr                 */
    char *classname = "KEYBOARD";	/* ODM class name                */
    char criteria[256];			/* search criteria               */
    int i;				/* loop index                    */
    char *ptr;				/* text ptr			 */
  
    menu->DefaultLine = -1;
    /* if there is no kb list, open the odm database and retrieve it */
    if (!kb)
    {
	strcpy(criteria, "bosinst_menu = y");

	odm_initialize();
	classp = odm_mount_class(classname);

	kbp = odm_get_list(classp, criteria, &KEYListInfo, 50, 4);
	kb = malloc(KEYListInfo.num * sizeof(struct BosKEYBOARD));
	if (!kb)
	    exit(2);
	firstkb = kb_default = kb;
	
	/* traverse the list, assign the menu select numbers */
	for (bkbp = kb, i=1; bkbp < kb+KEYListInfo.num; bkbp++)
	{
	    /* determine default */
	    if (!strcmp(kbp->keyboard_map, odm_kb->keyboard_map))
	       kb_default = bkbp;

	    bkbp->menu_index = i++;
	    bkbp->kb = kbp++;
	    if ((i % 13) == 0) i +=2;

	}
    }

    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }


    /* Clear the selection area */
    for (i=6; i < 19; i++)
    {
	menu->Text[i] = 0;
    }

    i=0;
    if (firstkb != kb)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(ILSText[0] , 
	    catgets(catfd, BI, BS_PREV_CHOICE, "    %2d ...PREVIOUS CHOICES"),
	    firstkb->menu_index-1);
	menu->Text[6] = ILSText[0];
	menu->DefaultLine = 6;
	i++;
    }
    for(bkbp = firstkb; (bkbp < kb+ KEYListInfo.num) && (i < 12); i++)
    {
	memset(ILSText[i], ' ', 80);

	/* translate kb text */
	sprintf(ILSText[i], "    %2d  %s",
	    bkbp->menu_index, 
	    catgets(catfd, BL, bkbp->kb->key_text_id,bkbp->kb->key_text));
	menu->Text[6+i] = ILSText[i];

	if (bkbp == kb_default)
	    menu->DefaultLine = i+6;
	bkbp++;
    }
    /* if there are more choices, display more choices */
    if (bkbp < kb+KEYListInfo.num)
    {
	sprintf(ILSText[i] ,
	    catgets(catfd, BI, BS_MORE_CHOICE, "    %2d  MORE CHOICES..."),
	    (bkbp-1)->menu_index+1);
	menu->Text[6+i] = ILSText[i];
	if (menu->DefaultLine == -1)
	    menu->DefaultLine = i+6;
    }
    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: NLSKeyboardIDDrvr
 *
 * FUNCTION: Process user selection for keyboard type
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *	The keyboard ID screen only appears whne the user selects a
 *	predefined locale which suports more than 1 keyboard.
 *
 *	Page forward and backward are computed based on number of items
 *	per page (13).
 *     
 * DATA STRUCTURES: 
 *	BosData locale stanza is updated
 *
 * RETURNS: next menu 
 */
struct Menu *NLSKeyboardIDDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    struct KEYBOARD *kbp;		/* ODM KEYBAOARD struct    	*/
    int index;				/* loop index			*/
    int bump;				/* amount to skip to next page 	*/

    /* clear message box */
    menu->Message = 0;
 
    /* process help */
    if (*input == BIM_HELP)
    {
	DisplayHelp(BM_FIG_21, FIG_21);
	return menu;
    }

    /* process previous menu */
    if (*input == BIM_PREV)
    {
	return &NLSMenu;
    }

    *input -= 1;
    /* Copy the selected type to the BosInst.data struct */
    if ((*input >= 0) && (*input < 5) && kb_types[*input])
    {
	strcpy(BosData.locale.KEYBOARD, kb_types[*input]->keyboard_map);
        return &Confirm;
    }

    /* Must be invalid, couldn't figure out what to do with it */
    menu->Message = &InvalidChoice;
    return menu;


}


/*----------------------------------------------------------------------------
 * NAME: NLSKeyboardIDPre
 *
 * FUNCTION: preformat the NLS keyboard ID selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	As mentioned in the dirver prolog, the keyboard ID screen only
 *	appears when the user selects a predefined locale which has more
 *	than one keyboard.
 *     
 * DATA STRUCTURES:
 *	menu updated
 *
 * RETURNS: 1
 */
int NLSKeyboardIDPre( struct Menu *menu)
{
    static char *buf;			/* storage for menu text	*/
    static char title[80];		/* storage for menu title	*/
    char *cptr;				/* char ptr			*/
    char kbtoken[20];			/* a keyboard locale		*/
    struct listinfo ListInfo;		/* ODM get list info		*/
    struct KEYBOARD *kbp, *tkbp;	/* ODM KEYBOARD struct ptrs     */
    struct Class *classp;		/* ODM class ptr		*/
    char criteria[256];			/* ODM search criteria		*/
    int i,j;				/* loop indexes			*/
    char *ptr;				/* text ptr			 */
  
    /* if there is no kb list, open the odm database and retrieve it */
    sprintf(criteria, "locale=%s", cc_default->cc->locale);

    odm_initialize();
    classp = odm_mount_class("KEYBOARD");

    kbp = odm_get_list(classp, criteria, &ListInfo, 5 , 1);
    

    sprintf(title, 
	catgets(catfd, BI, BS_KBTYPE_TITLE, BM_KBTYPE_TITLE),
	cc_default->cc->text_string);
    menu->Text[0] = title;

    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* parase the multiline instructions */
    if (!buf)
    {
	buf = catgets(catfd, BI, BS_KBTYPE_INSTRUCT, BM_KBTYPE_INSTRUCT);
	parse(menu, 2, buf, 2);
    }

    

    for (i=5, j=0 ; (i < 13) && (j < ListInfo.num); j++)
    {

	sprintf(ILSText[j], "    %d  %s", j+1, 
	       catgets(catfd, BL, kbp->key_text_id, kbp->key_text));
	kb_types[j] = kbp;
	menu->Text[i] = ILSText[j];
	i += 2;
	kbp++;
    }
    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: ChgMethodDrvr
 *
 * FUNCTION: Change method of install driver
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 * 	Sets the next menu appropriately for disk selection      
 *     
 * DATA STRUCTURES: 
 *	updates Bosdata control flow stanza
 *	resets diskIndexValid
 *
 * RETURNS: next menu 
 */
struct Menu *ChgMethodDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* clear out disk information */
    DiskIndexValid = 0;
    defaulted = -1;
    bzero(&diskList, 255* sizeof(struct DiskInfo *));

    /* process the user's input */
    switch (*input)
    {

    case 1: 
	InstallMethod = OVERWRITE;
	menu->DefaultLine = 4;
	strcpy(BosData.control_flow.INSTALL_METHOD, "overwrite");
	return &MksysbChoice;
	break;

    case 2:
	if (!preserveok) 
	{
	    bibeep();
	    menu->Message = &InvalidChoice;
	    return (menu);
	    break;
	}
	InstallMethod = PRESERVATION;
	strcpy(BosData.control_flow.INSTALL_METHOD, "preserve");
	menu->DefaultLine = 9;
	return &ChgRootVG;
	break;

    case 3:
	if (!migrateok) 
	{
	    bibeep();
	    menu->Message = &InvalidChoice;
	    return (menu);
	    break;
	}
	strcpy(BosData.control_flow.INSTALL_METHOD, "migrate");
	InstallMethod = MIGRATION;
	menu->DefaultLine = 15;
	return &ChgRootVG;
	break;

    case BIM_HELP:
	/* help stuff */
	DisplayHelp(BM_FIG_07, FIG_07);
	return menu;
	break;
    
    case BIM_PREV:
	/* previous menu - go back to comfirm menu.
	 */
	return &Confirm;
	break;
    
    default:
	/* bogus input, return same menu */
	bibeep();
	menu->Message = &InvalidChoice;
	return (menu);
    }

}

/*----------------------------------------------------------------------------
 * NAME: ChgMethodPre
 *
 * FUNCTION: preformat the change method of install selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	Only updates  the menu struct on first pass
 *     
 * DATA STRUCTURES: 
 *
 * RETURNS: 1
 */
ChgMethodPre( struct Menu *menu)
{
    struct disk_info *dip;		/* disk info ptr          */
    static char *ptr = 0;		/* ptr to catgets text    */
    static char *ovr, *psrv, *mig;	/* msg text ptrs          */
    int j;				/* traversal index        */
    int i;				/* traversal index	  */

    /* Depending on what the vg status is, zap menu items as follows:
     *     3 - Mirgation: only available if current level is 3.2 or 4.1
     *     2 - Preservation: only available if there is a rootvg
     */

    for (dip = head; dip; dip = dip->next)
    {
        if ( (strcmp(dip->level, "3.2") >= 0) && (strcmp(dip->level, curLevel) < 0) )
	    migrateok = 1;
	
	if (strcmp(dip->level,"0.0"))
	    preserveok = 1;

    }

    /* determine the default */
    if (InstallMethod == OVERWRITE)
	menu->DefaultLine = 4;
    else if (InstallMethod == PRESERVATION)
	menu->DefaultLine = 9;
    else
	menu->DefaultLine = 15;

    /* translate messages.... */
    if (!ovr)
    {
	ovr = malloc(strlen(BM_INSTALL_OVERWRITE)+1);
	if (!ovr) exit(2);

	psrv = malloc(strlen(BM_INSTALL_PRESERVE)+1);
	if (!psrv) exit(2);

	mig = malloc(strlen(BM_INSTALL_MIGRATE)+1);
	if (!mig) exit(2);
    }
    
    strcpy(ovr, BM_INSTALL_OVERWRITE);
    strcpy(psrv, BM_INSTALL_PRESERVE);
    strcpy(mig, BM_INSTALL_MIGRATE);

    ptr = catgets(catfd, 1, BS_INSTALL_OVERWRITE, ovr);
    parse(menu, 5, ptr, 4);

    ptr = catgets(catfd, 1, BS_INSTALL_PRESERVE, psrv);
    parse(menu, 10, ptr, 4);

    ptr = catgets(catfd, 1, BS_INSTALL_MIGRATE, mig);
    parse(menu, 16, ptr, 4);

    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* Remove illegal values from the menu */
    if (!migrateok)
    {
	for (j=15; j < 18; j++)
	    menu->Text[j] = "";
	if (menu->DefaultLine == 15)
	   menu->DefaultLine = 9;
    }

    if (!preserveok)
    {
	for (j=9; j < 14; j++)
	    menu->Text[j] = "";
	
	if (menu->DefaultLine == 9)
	   menu->DefaultLine = 4;
    }

return 1;

}

/*----------------------------------------------------------------------------
 * NAME: CfgSupplementalPre
 *
 * FUNCTION: preformat the configure suplemental disk instruction screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	menu is only updatedon first pass
 *     
 * DATA STRUCTURES: 
 *	menu updated
 *
 * RETURNS: 1
 */
CfgSupplementalPre( struct Menu *menu)
{
    static char *inst1, *inst2 = 0;	/* menu text ptr		*/
    char *ptr;				/* text ptr			*/
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* translate the menus */
    if (!inst1)
    {
	inst1 = malloc(strlen(BM_SUPLEMENTAL_INST)+1);
	inst2 = malloc(strlen(BM_SUPLEMENTAL_PROMPT)+1);
	if (!inst1 || !inst2) exit(2);
    }
    strcpy(inst1, BM_SUPLEMENTAL_INST);
    strcpy(inst2, BM_SUPLEMENTAL_PROMPT);

    ptr = catgets(catfd, BI, BS_SUPLEMENTAL_INST, inst1);
    parse(menu, 2, ptr, 2);

    ptr  = catgets(catfd, BI,BS_SUPLEMENTAL_PROMPT, inst2);
    parse(menu, 6, ptr, 2);

    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: CfgSupplementalDrvr
 *
 * FUNCTION: Process user input from configure suplemental disk 
 *	instruction screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      bicfgsup is started to read to floppy and rebuild the targetvgs file.
 *      The animate program is run while the supplemental drive script is run.
 *
 * RETURNS: next menu 
 */
struct Menu *CfgSupplementalDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int shpid;			/* bicfgsup shell pid		*/
    int wpid;			/* wait pid returned			*/
    int stat;			/* wait status				*/
    static struct Menu *readdisk;   /* Reading diskette menu            */
    char *ptr;                  /* text ptr                             */
    int i;                      /* Menu index                           */

    /* switch on then input */
    switch (*input)
    {
	case -1:
	case 0:


	    /* Free data in the target vg structure, 
	     * reread targetvgs file 
	     */
	    if ((shpid = fork()) == 0)
	    {
		/* child, exec bicfgsup */
		execl("/usr/lpp/bosinst/bicfgsup", "bicfgsup", 0);
		perror("bicfgsup");
		exit(0);
	    }
	    else if (shpid != -1)
	    {
		readdisk = &CfgWait;
		/* translation the readding diskette menu/prompt  */
		for (i=0; i < 24; i++)
		{
		    if (readdisk->MsgNum[i])
		    {
			/* get the translated string */
			ptr = catgets(catfd,BI, readdisk->MsgNum[i], "");
			if (ptr && *ptr)
			{
			    /* Copy the translated string back       */
			    readdisk->Text[i] = ptr;
			}
		    }
		}
		/*configure the other disks 
		*/
		DisplayMenu(readdisk);
		wpid = wait(&stat);

		DiskIndexValid = 0;
		head = readTargetVGS();
	    }
	    /* configure the disks */
	    StopAnim();

	/* Do not put break here */

	case BIM_PREV:
	    if ((*ImageData.image_data.PRODUCT_TAPE == 'y') && !MaintenancePath)
		return &MksysbChoice;
	    
	    /* must be a mksysb install */
	    return &MksysbChoice;

	    break;


	case BIM_HELP:
	    /* display help screen */
	    DisplayHelp(BM_FIG_09, FIG_09);
	    return menu;

	default:
	    /* bogus input, return same menu */
	    bibeep();
	    menu->Message = &InvalidChoice;
	    return (menu);
    }
}


/*----------------------------------------------------------------------------
 * NAME: ChgRootVGPre
 *
 * FUNCTION: preformat the change rootvg selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *	This menu screen appears for preserve and migrate installs.
 *
 * NOTES:
 *	If disk indexes are invalid, the disk list is scanned and menu
 *	indexs are set.
 *	
 *	Starting at screen_first, a page of disks are displayed in the menu.
 *	only the first disk of a volume group has the level displayed.
 *     
 * DATA STRUCTURES: 
 *	menu updated
 *	defaulted set
 *	DiskIndexesValid set
 *
 * RETURNS: 1
 */
int rvg_index;
struct screen_info
{
    short page_fwd;		/* value of page forward		*/
    short page_back;		/* value of page backward		*/
    short screen_first;		/* index of first disk on screen	*/
} rvg_screens[10];		/* space for 20 pages of disks 		*/
ChgRootVGPre( struct Menu *menu)
{
    static int firsttime = 0;		/* first time called flag	*/
    int i,j,k;				/* loop indexes			*/
    char currentvg[17];			/* vg currently processed	*/
    struct disk_info *dip;		/* disk info ptr		*/
    struct target_disk_data *tddp = 0;	/* target disk ptr		*/
    char *ptr;			/* text ptr                        */
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }


    /* Are the disk indexes valid? */
    if (! DiskIndexValid)
    {
	/* clear previous choices */

	/* loop through the disk_info structures and assign indexes,
	 * 12 per page
	 */
	numberDisks = 0;
	for (i=0, dip=head; dip; dip = dip->next)
	{

	    /* skip non-root vgs */
	    if (dip->vgstat != 1)
		continue;

	    if (InstallMethod == MIGRATION)
		if (!strcmp(dip->level, "3.1") || !strcmp(dip->level, curLevel))
		    continue;

	    diskList[numberDisks++] = dip;
	}
	DiskIndexValid = 1;
	screen_first = 0;
	rvg_index = 0;
	rvg_screens[0].screen_first = 0;
	rvg_screens[0].page_back = 0;

	/* find the first disk in Target Disks and select it */
	for (dip=head; dip; dip = dip->next)
	{
	    /* loop through the target disks structurs */
	    for (tddp = BosData.targets; tddp; tddp = tddp->next)
	    {
		/* match location, size, and name */
		if (!strcmp(dip->name , tddp->HDISKNAME) &&
		    !strcmp (dip->location , tddp->LOCATION) &&
		    !strcmp (dip->size , tddp->SIZE_MB))
		{
		    /* A winner! */
		    dip->selected = 1;
		    break;
		}
	    }
	    if (dip->selected) break;
	}
    }

    /* Make the first disk_info node the first one on the screen
     * if there is more than one screen of disks, set the last to
     * point to more disks
     */
    for (i=0; i<14; i++)
    {
	menu->Text[6+i] = 0;
	memset(disk_text[i], ' ', 80);
    }
    if (!diskList[screen_first]) 
    {
	strcpy(disk_text[0], 
	    catgets(catfd, BI, BS_NODISKS, "     No Disks Available"));
        
	return 1;
    }
    /* Loop through the available disk, format the screen buffer
     * for displaying each disk entry
     */
    screen_first = rvg_screens[rvg_index].screen_first;
    i=0;
    if (screen_first != 0)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(disk_text[0] , 
	   catgets(catfd, BI, BS_PREV_CHOICE, "    %2d  ...PREVIOUS CHOICES"),
	    rvg_screens[rvg_index].page_back);

	menu->Text[6] = disk_text[0];
	if (defaulted < screen_first)
	    menu->DefaultLine = 6;
	i++;
    }
    for (k=screen_first, dip=diskList[screen_first], j=rvg_screens[rvg_index].page_back+1;
	 (i < 12) && dip;
	 i++)
    {
        char ip[3];

	/* Check if this is a different VG than the curent one.
	 * If so, we need to include the level
	 */
	if (strcmp(currentvg, dip->vgid))
	{
	    /* Don't need to translate, its not really English anyway.  
	     * Convert j (disk index) into asci and strip leading 0 if 
	     * necessary.  Format the line of text.
	     */
	    dip->text_num = j;
	    sprintf(ip, "%2.2d", j);
	    if (ip[0] == '0')
		ip[0] = ' ';

	    sprintf(disk_text[i], 
		 "    %2s %-6.6s   %-20.20s %15.15s   %6.6s", ip,
		 dip->level,dip->name, dip->location, dip->size);

	    /* check for selected entry (aka default) */
	    if (dip->selected)
	    {
		strncpy(disk_text[i], ">>>",3);
		menu->DefaultLine= i+6;
		defaulted = k;
	    }

	    strcpy(currentvg, dip->vgid);
	    /* point the visible_disk element to this disk_info structure
	     * so we can find this entry later
	     */
	    j++;
	}
	else
	{
	    sprintf(&disk_text[i][4], 
		    "            %-20.20s %15.15s   %6.6s",
		      dip->name, dip->location, dip->size);

	}
	/* set the menu text line to the formated line */
	menu->Text[i+6] = disk_text[i];

	dip = diskList[++k];
    }
    if (( dip) && dip->bootable)
    {
	rvg_screens[rvg_index].page_fwd = j;
	rvg_screens[rvg_index+1].page_back = j+1;
	rvg_screens[rvg_index+1].screen_first = k;
	sprintf(disk_text[i] , 
	   catgets(catfd, BI, BS_MORE_CHOICE, "    %2d  MORE CHOICES..."), 
	       rvg_screens[rvg_index].page_fwd);
	menu->Text[6+i] = disk_text[i];
	if (defaulted == -1 )
	{
	    menu->DefaultLine = 6+i;
	}
	    
    }

    return 1;

}

/*----------------------------------------------------------------------------
 * NAME: ChgRootVGDrvr
 *
 * FUNCTION: Process user input from change rootvg selection screen 
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      Page forward/backward processed as before
 * 
 *	When a disk is selected, all the disks in the volume group are
 *	copied to BosData target disks stanza (after passing size check).
 *	The next menu is set to confirm.
 *
 * DATA STRUCTURES: 
 *	BosData.target_disks
 *	defaulted
 *
 * RETURNS: next menu 
 */
struct Menu *ChgRootVGDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int rc, i;				/* return code, index		*/
    int bump;				/* amount to skip to next page  */
    struct lv_data *lvdp;		/* logical volume data ptr	*/
    int NeededSize = 0;			/* space needed to install	*/
    int AvailableSize = 0;		/* space available for install  */
    struct target_disk_data *tddp;	/* target disk data ptr		*/
    struct target_disk_data *tddfree;	/* target disk data ptr		*/
    char *pvname = 0;			/* physical volume name		*/

    struct disk_info *dip;
    int index;

    menu->Message = 0;

    if (*input == rvg_screens[rvg_index].page_fwd)
    {
	rvg_index++;
	return menu;

    }
    if (*input== rvg_screens[rvg_index].page_back)
    {
	/* it's a page backward, see if it's valid */
	rvg_index--;
	return menu;
    }
    if (*input == BIM_HELP)
    {
	/* display help screen, return this same menu */
	DisplayHelp(BM_FIG_11, FIG_11);
	return menu;
    }
    
    if (*input == BIM_PREV)
    {
	/* go to previous menu */
	menu->Message = 0;
	return &ChgMethod;
    }

    /* scan the disk list for a match */
    for (index =0; index < numberDisks; index++)
    {
	if (diskList[index]->text_num == *input)
	    break;
    }
    if (index == numberDisks)
    {
	menu->Message = &InvalidChoice;
	bibeep();
	return menu;
    }

    if (diskList[index])
    {
	/* check size requirements */
	NeededSize = getNeededPreserve();
	AvailableSize = getAvailPreserve(diskList[index]->name);
	if ((NeededSize> 0) && (AvailableSize > 0))
	{
	    if (NeededSize > AvailableSize)
	    {
		if(strcmp(turbo,"turbo"))
		{
		    NoSpace.Text = catgets(catfd, BI,  
				NoSpace.MsgNum, NoSpace.Text);
		    menu->Message = &NoSpace;
		    return menu;
		}
	    }
	}
	/* yeah, its valid */
        if (defaulted >= 0)
            diskList[defaulted]->selected = 0;
	diskList[index]->selected = 1;
	defaulted = index;

	/* Free the current target disks */
	for (tddp = BosData.targets; tddp; )
	{
	    tddfree = tddp;
	    tddp = tddp->next;
	    free(tddfree);
	}
	BosData.last = 0;

	/* Update Bosdata structure */
	dip = diskList[index];
	while (1)
	{

	    tddp = (struct target_disk_data *) malloc(sizeof(struct target_disk_data));
	    if (!tddp)
	    exit(2);

	    if (BosData.last)
		BosData.last->next = tddp;
	    else
		BosData.targets = tddp;
		
	    BosData.last = tddp;

	    tddp->next = 0;
	    strcpy(tddp->LOCATION, dip->location);
	    strcpy(tddp->HDISKNAME, dip->name);
	    strcpy(tddp->SIZE_MB, dip->size);


	    index++;

	    /* see if there are more disks in this group */
	    if (strcmp(visible_disks[index]->vgid, dip->vgid))
		break;
	}
    
	/* go to previous menu */
	return &Confirm;
    }
    else bibeep();

    menu->Message = &InvalidChoice;
    return menu;
}

/*----------------------------------------------------------------------------
 * NAME: MaintenanceDrvr
 *
 * FUNCTION: Process user input from maintenance utilies screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 * 
 * NOTES: NEEDS INTEGRATION
 *
 * RETURNS: next menu 
 */
struct Menu *MaintenanceDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int dumppid;	/* dump pid	*/
    int dumpstat;	/* dump stat	*/
    /* reset error message */
    menu->Message = 0;

    /* process user's input */
    switch (*input)
    {
	case 1:
	    /* mount root volume group */
	    return &RVGWarn;
	    break;

	case 2:
	    /* copy system dump */
	    if ((dumppid = fork()) == 0)
	    {
		execl("/usr/bin/copydumpmenu", "copydumpmenu", 0);
		perror("exec copydumpmenu");
		exit(0);
	    }
	    else if (dumppid != -1)
	    {
		static struct MessageBox cdm_msg;

		static char *dumpret[] = {0, BM_MAINT_DR1, BM_MAINT_DR2,
					  BM_MAINT_DR3, BM_MAINT_DR4};

		static int dumpret_nos[] = {0, BS_MAINT_DR1, BS_MAINT_DR2,
					  BS_MAINT_DR3, BS_MAINT_DR4};

		wait(&dumpstat);	/* wait for copydumpmenu to exit */

		dumpstat >>= 8;
		if ((dumpstat > 0) && (dumpstat < 5))
		{
		    cdm_msg.MsgNum = dumpret_nos[dumpstat];
		    cdm_msg.Text = catgets(catfd, BI, cdm_msg.MsgNum,
				 dumpret[dumpstat]);
		    menu->Message = &cdm_msg;
		}
	    }
	    return menu;
	    break;

	case 3:
	    /* standalone shell */
	    return &ShellInfo;
	    break;

	case 4:
	    /* mksysb install */
	    MaintenancePath = 1;
	    return &MksysbTape;
	    break;

	case BIM_HELP:
	    /* help */
	    DisplayHelp(BM_FIG_29, FIG_29);
	    return menu;
	    break;

	case BIM_PREV:
	    /* previous menu */
	    MaintenancePath = 0;
	    return &BosMain;
	    break;
    }
    /* invalid choice */
    bibeep();
    menu->Message = &InvalidChoice;
    return menu;
}
/*----------------------------------------------------------------------------
 * NAME: MaintenancePre
 *
 * FUNCTION: preformat the maintenance utilities selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	If this was a network boot, remove mksysb install from
 *	the maintenance menu.
 *     
 * RETURNS: 1
 */
int MaintenancePre( struct Menu *menu)
{
    char *ptr;			/* text ptr                        */
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    /* check boot device */
    if (*boottype == '5')
    {
	/* remove mksysb option from menu */
	menu->Text[7] = 0;
    }

    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: RVGWarnPre
 *
 * FUNCTION: preformat the Root Volume Group Warning Screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	Parse the multi-line strings
 *     
 * RETURNS: 1
 */
int RVGWarnPre(struct Menu *menu)
{
    static char *inst;		/* menu text ptr			*/
    char *ptr;			/* text ptr                        */
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    if (!inst)
    {
	inst = malloc(strlen(BM_ROOTVG_WARN)+1);
	if (!inst) exit(2);
    }
    strcpy(inst, BM_ROOTVG_WARN);

    ptr = catgets(catfd, BI, BS_ROOTVG_WARN, inst);

    parse(menu, 2, ptr, 2);

    return 1;
}
/*----------------------------------------------------------------------------
 * NAME: RVGWarnDrvr
 *
 * FUNCTION: Process user input from Root Volume Group Warning Screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	This is one of the exit points from BosMenus.  When the user
 *	wants to proceed to access a root volume group, exit BosMenus
 *	with a status code of 1.
 *     
 * RETURNS: next menu
 */
struct Menu *RVGWarnDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{

    /* reset error message */
    menu->Message = 0;

    /* process user input */
    switch (*input)
    {
    case 0:
	/* exit with status of 1 */
	exit(1);
	break;

    case BIM_HELP:
	/* display help screen */
	DisplayHelp(BM_FIG_30, FIG_30);
	return menu;
	break;

    case BIM_PREV:
	/* go back to maintenance choices */
	return  &Maintenance;
	break;

    default:
	/* redisplay menu */
	menu->Message = &InvalidChoice;
	return menu;
	break;
    }

}

/*----------------------------------------------------------------------------
 * NAME: MksysbTapeDrvr
 *
 * FUNCTION: Process user input from mksysb install tape drive selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *	Assumes a maximum of 9 tape drives.
 *	
 *	The selected tape drive is written the path specified by TAPEDEVICE
 *     
 * RETURNS: next menu 
 */
struct Menu *MksysbTapeDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int index = *input-1;	/* tapedevice index			*/
    char *tapefile;		/* filename where selected tape drive stored */
    FILE *fp;			/* tapedevice file ptr			*/

    /* reset error message */
    menu->Message = 0;

    /* process user input */
    switch (*input)
    {

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    
	if (TapeDevices[index])
	{
	    tapefile = getenv("TAPEDEVICE");
	    if (!tapefile)
		tapefile = "tapedevice";
	    
	    fp = fopen(tapefile, "w");
	    fwrite(TapeDevices[index]+37, 1, 9, fp);
	    fprintf(fp, "\n");
	    fclose(fp);

	    /* need to exit here so that bi_main can restore the
	     * correct bosinst.data and image.data files from tape
	     */
	    exit(3);
	}
	bibeep();
	menu->Message = &InvalidChoice;
	return menu;
	break;

    case BIM_HELP:
	/* display help screen */
	DisplayHelp(BM_FIG_39, FIG_39);
	return menu;
	break;

    case BIM_PREV:
	/* go back to maintenance choices */
	return  &Maintenance;
	break;

    default:
	/* redisplay menu */
	menu->Message = &InvalidChoice;
	return menu;
	break;
    }
}
/*----------------------------------------------------------------------------
 * NAME: MksysbTapePre
 *
 * FUNCTION: preformat the tape drive selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	popen is used for simplicity.
 *     
 * DATA STRUCTURES: 
 *	tapedevices
 *
 * RETURNS: 1
 */
int MksysbTapePre( struct Menu *menu)
{
    FILE *fp;			/* file ptr for popen - lsdev          */
    FILE *odmfp;		/* file ptr for popen - odmget         */
    char line[80];		/* a line from lsdev                   */
    char lastline[80];		/* a line from lsdev                   */
    char device[10];		/* a line from lsdev                   */
    char odmget[80];		/* odmget cmdline                      */
    char *ptr;			/* parsing ptr                         */
    int i = 0;			/* loop index                          */
    static int firsttime = 1;	/* flag to only create the list 1 time */
    static char *inst;		/* instruction text */
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }
    if (!inst)
    {
	inst = malloc(strlen(BM_MKSYSB_TAPE_INST)+1);
	if (!inst) exit(2);
    }
    strcpy(inst, BM_MKSYSB_TAPE_INST);

    /* parse the multiline instructions */
    ptr = catgets(catfd, BI, BS_MKSYSB_TAPE_INST, inst);
    parse(menu, 2, ptr, 2);


    /* create this list only once */
    if (firsttime)
    {
	firsttime = 0;
	i=0;


	/* open lsdev -Cctape to get the available drives */
	fp = popen("/usr/sbin/lsdev -Cctape -S Available -F name", "r");

	while (1)
	{
	    if (!fgets(device, 10, fp))
		break;

	    /* replace newline with null */
	    device[strlen(device)-1] = '\0';
	    sprintf(odmget, "/usr/bin/odmget -qname=%s CuDv", device); 
	    odmfp = popen(odmget, "r");

	    /* We're only interested int he last line */
	    while (1)
	    {
		if (!fgets(lastline, 80, odmfp)) break;
		strcpy(line, lastline);
	    }
	    pclose(odmfp);

	    ptr = strchr(line, '\n');
	    *ptr = ' ';
	    /* replace the quotes with blanks, then back up to '=' */
	    ptr = strchr(line, '"');
	    *ptr++ = ' ';
	    ptr = strchr(ptr, '"');
	    *ptr++ = ' ';
	    ptr = strchr(line, '=')+1;
	    while (isspace(*ptr))
	    {
		ptr++;
	    }

	    TapeDevices[i] = malloc(80);
	    if (!TapeDevices[i])
		exit(2);
	    sprintf(TapeDevices[i],"    %1.1d %-29.29s  /dev/%s",
		i+1, ptr, device);

	    menu->Text[i+7] = TapeDevices[i];
	    i++;
	}
	pclose(fp);

	if (TapeDevices[0])
	    menu->DefaultLine = 7;
	else
	    TapeDevices[0] = 
		catgets(catfd, BI, BS_NOTAPE,"No tape devices available");
    }

    return 1;
	    
}
/*----------------------------------------------------------------------------
 * NAME: MksysbConfirmDrvr
 *
 * FUNCTION: Process user input from mksysb confirm screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *	For case 0, the settings are written to bosinst.data and image.data
 *	files and BosMenus exits with a 0 status.
 *      
 *  	Return to previous menu depends on how you got here.
 *
 * DATA STRUCTURES: 
 *	ImageData
 *
 * RETURNS: next menu 
 */
struct Menu *MksysbConfirmDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* reset error message */
    menu->Message = 0;
  
    /* process user's input */
    switch (*input)
    {
    case 0:
	/* update settings file and exit with 0 exit code */
	strcpy(ImageData.image_data.PRODUCT_TAPE, "no");
	write_bosinstdata();
	write_imagedata();
	exit(0);
	break;

    case 1:
	/* Next menus is change destination disks */
	DiskIndexValid = 0;
	return &MksysbChoice;
    
    case 2:
	/* toggle file systems */
	if (tolower(*ImageData.lv_policy.SHRINK) == 'y')
	    strcpy(ImageData.lv_policy.SHRINK , "no");
	else
	    strcpy(ImageData.lv_policy.SHRINK , "yes");

	return menu;
	break;

    case BIM_HELP:
	/* display help screen */
	DisplayHelp(BM_FIG_27, FIG_27);
	return menu;
	break;

    case BIM_PREV:
	/* IS it a maintenance install? */
	if (MaintenancePath)
	    return &MksysbTape;


	return &BosMain;
	break;

    default:
	/* redisplay menu */
	menu->Message = &InvalidChoice;
	return menu;
	break;
    }
}
/*----------------------------------------------------------------------------
 * NAME: MksysbConfirmPre
 *
 * FUNCTION: preformat the mksysb install confirm screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	The instruction line is only loaded on the first path
 *     
 * DATA STRUCTURES: 
 *	menu updated
 *	destination disk, maps, and shrink menu lines have storage allocated
 *	here.
 *
 * RETURNS: 1
 */
int MksysbConfirmPre( struct Menu *menu)
{
    
    /* static copy of menu text; address can be passed to the menu system */
    static char buf[180];		/* message buffer */
    static char destination[80],	/* new permanent strings    */
		maps[80], 	        /* NLS cultural conventions */
		shrink[80];		/* NLS message transation   */
    static char *ptr = 0;		/* catgets string ptr       */
    int exact_fit = 0;			/* exact fit flag	    */
    struct disk_info *dip;		/* disk info ptr		*/
    static char *inst;			/* instruction line		*/
    static int firsttime = 1;
    int i;
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    if (!inst)
    {
	inst = malloc(strlen(BM_MKSYSB_CONFIRM_INST)+1);
	if (!inst) exit(2);
    }
    strcpy(inst, BM_MKSYSB_CONFIRM_INST);
    ptr = catgets(catfd, BI, BS_MKSYSB_CONFIRM_INST, inst);
    parse(menu, 2, ptr, 2);
    /* load and parse the instrution line */
    if (firsttime)
    {

	firsttime = 0;
	/* Determine if EXACT_FIT should be yes of no */
	if (*ImageData.lv_data->MAPFILE)
	{
	    for (dip = head; dip; dip = dip->next)
	    {
		/* Determine if any disk has maps. For each disk in the dip
		 * list, check for a match in lv_data->LV_SOURCE_DISK_LIST
		 * and lv_data->MAPFILE.
		 */
		if (strstr(ImageData.vg_data.VG_SOURCE_DISK_LIST, dip->name))
		    dip->maps=1;
		else
		{
		    /* clear the dip maps */
		    dip->maps = 0;
		}
	    }
	}
	
	/* update the EXACT_FIT field */
	/*
	strcpy(ImageData.lv_policy.EXACT_FIT , yesno[exact_fit]);
	*/
    }


    /* Need to change the menu to reflect the current settigns.
     * Copy the menu strings to the new storage area. Note that
     * although ALL selected disks are stored in TARGETDISKS file,
     * only the first is displayed on the confirmation
     */
    memset(&destination[46], ' ', 30);
    memset(&maps[46], ' ', 30);
    memset(&shrink[46], ' ', 30);
    strncpy(destination, menu->Text[7], strlen(menu->Text[7]));
    strncpy(maps, menu->Text[8], strlen(menu->Text[8]));
    strncpy(shrink, menu->Text[9], strlen(menu->Text[9]));

    /* copy the current selection into the string */
    strncpy(&destination[47], BosData.targets->HDISKNAME, strlen(BosData.targets->HDISKNAME));
    if (BosData.targets->next)
	strcpy(&destination[47+strlen(BosData.targets->HDISKNAME)], "...");

    /* if shrink is enabled, disallow EXACT_FIT */
    if (tolower(*ImageData.lv_policy.SHRINK) == 'y')
    strcpy(ImageData.lv_policy.EXACT_FIT , "no ");


    if (ImageData.lv_policy.EXACT_FIT[0] == 'y')
	strcpy(&maps[47], catgets(catfd, BI, BS_YES, "Yes"));
    else
	strcpy(&maps[47], catgets(catfd, BI, BS_NO , "No "));

    if (ImageData.lv_policy.SHRINK[0] == 'y')
	strcpy(&shrink[47], catgets(catfd, BI, BS_YES , "Yes"));
    else
	strcpy(&shrink[47], catgets(catfd, BI, BS_NO , "No "));

    /* set the addresses of the menu text to point to the above static
     * storage
     */
    menu->Text[7] = destination;
    menu->Text[8] = maps;
    menu->Text[9] = shrink;
    sprintf(buf, catgets(catfd, BI, BS_OVERWRITEWARN, BM_OVERWRITEWARN),
	BosData.targets->HDISKNAME);
    OverwriteWarn.Text = buf;
    menu->Message = &OverwriteWarn;

    return 1;

}
/*----------------------------------------------------------------------------
 * NAME: MksysbChoiceDrvr
 *
 * FUNCTION: Process user input from mksysb disk selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      User is allowed to pick as many disks as he wants (hence the while loop)
 *	When 0 is selected, all the selected disks are checked for size 
 *	requirements and copied to BosData target disks stanza
 *     
 *	Page forward/backward is process as before, but the items/page is
 *	(ack!) only 5.
 *
 *	The user is allowed to configure suplemental disks from this screen.
 *
 * DATA STRUCTURES: 
 *	BosData.target_disks
 *
 * RETURNS: next menu 
 */
struct Menu *MksysbChoiceDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int index;				/* loop index		*/
    int bump;				/* amount to skip to next page */
    int NeededSize = 0;			/* space needed to install	*/
    int AvailableSize = 0;		/* space available for install	*/
    struct lv_data *lvdp;		/* logical volume data ptr	*/
    struct target_disk_data *tddp;	/* target disk data ptr		*/
    struct target_disk_data *tddfree;	/* target disk data ptr		*/
    struct disk_info *dip;		/* disk info ptr		*/
    struct disk_info *bootdip = 0;	/* boot disk info ptr		*/
    struct src_disk_data *sddp = 0;	/* source disk data ptr		*/

    /* reset error message */
    menu->Message = 0;

    /* process user's input */
    while (*input != -1)
    {

	switch (*input)
	{
	    case 0:
		/* Return to previous menu: but first, verify the user
		 * selected a large enough disk.  Added all the LPs in all
		 * volumes, multiply by pv_data.PPSIZE to get the required size.
		 */
		if (ImageData.lv_policy.SHRINK[0] == 'y')
		    NeededSize = getNeededShrink();
		else
		    NeededSize = getNeededOverwrite();


		/* compute available size based on the selected disks */
		AvailableSize = 0;
		for (dip = head, index=0; dip; dip = dip->next)
		{
		    if (dip->selected)
		    {
			index++;
			AvailableSize  += atoi(dip->size); 
		    }
		}

		if (index == 0)
		{
		    /* no disk selected */
		    NoDiskSelected.Text = catgets(catfd, BI,  
			       NoDiskSelected.MsgNum, NoDiskSelected.Text);
		    menu->Message = &NoDiskSelected;
		    return menu;

		}
		if (AvailableSize < NeededSize)
		{
		    /* Sorry, Charlie, not enough space */
	       	    if(strcmp(turbo,"turbo"))
	            {
		    	NoSpace.Text = catgets(catfd, BI,  
			    	   NoSpace.MsgNum, NoSpace.Text);
		    	menu->Message = &NoSpace;
		    	return menu;
		    }
		}

		/* make sure at least one disk is bootbale before we free
		 * the current targets 
		 */
		for (dip=head; dip; dip = dip->next)
		{
		    if ((dip->selected) && (dip->bootable)) break;
		}
		if (!dip)
		{

		    /* Sorry, Charlie, no bootbale disk selected */
			NoDisk.Text = catgets(catfd, BI,  
				   NoDisk.MsgNum, NoDisk.Text);
		    menu->Message = &NoDisk;
		    return menu;
		}

		/* Free the current target disks */
		for (tddp = BosData.targets; tddp; tddp = tddp->next)
		{
		    tddfree = tddp;
		    tddp = tddp->next;
		    free(tddfree);
		}
		BosData.last = 0;

		/* Now we need to verify that at least one disk is bootable, and
		 * must be the first in the list.  Find the first bootable
		 * disk copy it to targets. Remember this disk and don't
		 * write out twice!
		 */
		for (dip=head; dip; dip = dip->next)
		{
		    if (!dip->selected) continue;
		    if (dip->bootable)
		    {
			tddp = (struct target_disk_data *) 
				malloc(sizeof(struct target_disk_data));
			if (!tddp)
			    exit(2);

			BosData.targets = tddp;
			BosData.last = tddp;

			tddp->next = 0;
			strcpy(tddp->LOCATION, dip->location);
			strcpy(tddp->HDISKNAME, dip->name);
			strcpy(tddp->SIZE_MB, dip->size);
			bootdip = dip;
			break;
		    }
		}


		/* Update Bosdata structure */
		for (dip = head; dip; dip = dip->next)
		{

		    if (!dip->selected)
			continue;

		    /* skip the one already written */
		    if (dip == bootdip) 
			continue;

		    tddp = (struct target_disk_data *) malloc(sizeof(struct target_disk_data));
		    if (!tddp)
			exit(2);

		    if (BosData.last)
			BosData.last->next = tddp;
		    else
			BosData.targets = tddp;
			
		    BosData.last = tddp;

		    tddp->next = 0;
		    strcpy(tddp->LOCATION, dip->location);
		    strcpy(tddp->HDISKNAME, dip->name);
		    strcpy(tddp->SIZE_MB, dip->size);
		}
	    
		
		/* Loop through the target disks structurs looking for maps
		 * If all selected disks have maps, then maps
		 * will be marked available
		 */
		maps = 1;
		for (dip = head; dip; dip = dip->next)
		{
		    if (!dip->selected) continue;
		    if (!dip->maps)
		    {
			maps = 0;
			break;
		    }
		}
		/* update the EXACT_FIT field */
		strcpy(ImageData.lv_policy.EXACT_FIT , yesno[maps]);

		if (*ImageData.image_data.PRODUCT_TAPE == 'n')
		    return (maps ? &MksysbMap : &MksysbConfirm);
		return &Confirm;
		break;


	    case 66:
		/* config suplemental disk */
		return &CfgSupplemental;

	    case BIM_HELP:
		/* display help screen, return this same menu */
		if (*ImageData.image_data.PRODUCT_TAPE == 'n')
		    DisplayHelp(BM_FIG_28, FIG_28);
		else
		    DisplayHelp(BM_FIG_08, FIG_08);

		return menu;
		break;
	    
	    case BIM_PREV:
		/* go to previous menu */
		if (*ImageData.image_data.PRODUCT_TAPE == 'n')
		    return &MksysbConfirm;
		return &Confirm;
		break;

            default:
		if (*input%6 == 0)
		{
		    /* it's a page forward, see if it's valid */
		    if (screen_first == 0)
			bump =5;
		    else
			bump =4;

		    if ((screen_first+ bump)  >  numberDisks)
		    {
			menu->Message = &InvalidChoice;
			return menu;
		    }
		    screen_first +=bump;
		    return menu;

		}
		if ((*input%6 == 1) && (*input != 1))
		{
		    /* it's a page backward, see if it's valid */
		    if (screen_first == 0)
		    {
			menu->Message = &InvalidChoice;
			return menu;
		    }

		    screen_first -= 4;
		    if (screen_first < 4)
			screen_first = 0;
		    return menu;
		}

		/* User input a (probably) valid number */
		for (index = 0; index < numberDisks; index++)
		{
		    /* clear previous selection */
		    if (diskList[index]->text_num == *input)
		    {
			diskList[index]->selected = 
			    !diskList[index]->selected;
			break;
		    }

		}
		/* See if it was valid */
		if (index >= numberDisks)
		{
		    bibeep();
		    menu->Message = &InvalidChoice;
		    return menu;
		}
		break;
	}
	input++;
    }
    return menu;
}
   
/*----------------------------------------------------------------------------
 * NAME: MksysbChoicePre
 *
 * FUNCTION: preformat the mksysb disk selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	If DiskIndexValid is not set, the disk list is scanned and menu
 *	indexes are rebuilt.
 *
 *	Starting at screen_first, a page of disks is displayed in the menu
 *     
 * DATA STRUCTURES: 
 *
 * RETURNS: 1
 */
MksysbChoicePre( struct Menu *menu)
{
    static int firsttime = 0;		/* 1st pass through flag	*/
    static int exact_fit = 0;		/* exact fit set flag		*/
    int i,j,k;				/* loop indexes			*/
    struct disk_info *dip;		/* disk info ptr		*/
    struct target_disk_data *tddp = 0;	/* target disk info ptr		*/
    static char *inst;			/* instruction text		*/
    char *ptr;				/* menu text ptr		*/
    char *format;			/* disk line format		*/

    maps = 0;		/* maps valid this go-around? */

    /* Are the disk indexes valid? */
    if (! DiskIndexValid)
    {
	/* loop through the disk_info structures and assign indexes,
	 * 5 per page
	 */
	numberDisks = 0;
	for (i=1, dip=head; dip; dip = dip->next)
	{

	    dip->text_num = i++;
	    if ((i % 6 ) == 0)
		i+=2;
	    diskList[numberDisks++] = dip;
	}
	DiskIndexValid = 1;
	screen_first = 0;
    
	/* find the first disk in Target Disks and select it */
	for (dip=head; dip; dip = dip->next)
	{
	    /* loop through the target disks structurs */
	    for (tddp = BosData.targets; tddp; tddp = tddp->next)
	    {
		/* match location, size, and name */
		if (!strcmp(dip->name , tddp->HDISKNAME) &&
		    !strcmp (dip->location , tddp->LOCATION) &&
		    !strcmp (dip->size , tddp->SIZE_MB))
		{
		    /* A winner! */
		    dip->selected = 1;
		}
	    }
	}
    }
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }

    if (!inst)
    {
	inst = malloc(strlen(BM_MKSYSB_INST)+1);
	if (!inst) exit(2);
    }
    strcpy(inst, BM_MKSYSB_INST);
    /* get and parse the instruction line */
    ptr = catgets(catfd, BI, BS_MKSYSB_DSK_INST, inst);
    parse(menu, 2, ptr, 4);


    /* Make the first disk_info node the first one on the screen
     * if there is more than one screen of disks, set the last to
     * point to more disks
     */
     menu->DefaultLine = 16;
    for (i=9; i<15; i++)
	menu->Text[i] = 0;
    for (i=0; i<12; i++)
    {
	visible_disks[i] = 0;
	memset(disk_text[i], ' ', 80);
    }
    if (!diskList[screen_first]) 
    {
	strcpy(disk_text[0], 
	    catgets(catfd, BI, BS_NODISKS,"     No Disks Available"));
	return 1;
    }
    i = 0;
    if (screen_first)
    {
	/* not at the begining of the list, so display "..PREVIOUS CHOICES" */
	sprintf(disk_text[0] , 
	   catgets(catfd, BI, BS_PREV_CHOICE, "    %2d  ...PREVIOUS CHOICES"),
	    diskList[screen_first]->text_num-1);

	menu->Text[9] = disk_text[0];
	menu->DefaultLine = 9;
	i++;
    }
	
    if (*ImageData.image_data.PRODUCT_TAPE == 'n')
       format="    %2d  %-8.8s %-18.18s %6.6s   %12.12s %6.6s %6.6s";
    else
    {
       menu->MsgNum[7] = BS_CHGDSK_OVR_HDG;
       menu->Text[7] = catgets(catfd,BI, menu->MsgNum[7],
       "        Name      Location Code   Size(MB)  VG Status   Bootable");
       format="    %2d  %-8.8s %-18.18s %6.6s   %12.12s %6.6s";
    }
    for (k=screen_first,dip=diskList[k], j=0;i < 5; i++)
    {
	/* format the text buffer */
       
	if (!dip) break;


	sprintf(disk_text[i], format,
	   dip->text_num, dip->name, dip->location, dip->size, 
	   catgets(catfd, BI, BS_VGS_NONE+dip->vgstat,vgstat[dip->vgstat]),
	   catgets(catfd, BI, BS_NO+dip->bootable, yesno[dip->bootable]),
	   catgets(catfd, BI, BS_NO+dip->maps, yesno[dip->maps]));

	/* check for selected entry (aka default) */
	if (dip->selected)
	{
	    strncpy(disk_text[i], ">>>",3);
	    defaulted = j;
	}

	/* point the visible_disk element to this disk_info structure
	 * so we can find this entry later
	 */
	visible_disks[j++] = dip;
	dip = diskList[++k];

	/* set the menu text line to the formated line */
	menu->Text[i+9] = disk_text[i];

    }
    if (( dip) && dip->bootable)
    {
	sprintf(disk_text[i] , 
	   catgets(catfd, BI, BS_MORE_CHOICE, "    %2d  MORE CHOICES..."), 
	       dip->text_num-2);
	menu->Text[9+i] = disk_text[i];
    }

    return 1;

}

/*----------------------------------------------------------------------------
 * NAME: MksysbMapDrvr
 *
 * FUNCTION: Process user input from mksysb use maps dialog screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *	This screen is displayed when doing a mksysb install and maps are 
 *	available.
 *
 * DATA STRUCTURES: 
 *	ImageData.lv_policy     
 *
 * RETURNS: next menu 
 */
struct Menu *MksysbMapDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    /* reset error message */
    menu->Message = 0;

    /* process user's input */
    switch (*input)
    {
    case 1:
	maps = 1;
	strcpy(ImageData.lv_policy.EXACT_FIT , "yes");
	return &MksysbConfirm;
	break;
    
    case 2:
	maps = 0;
	strcpy(ImageData.lv_policy.EXACT_FIT , "no");
	return &MksysbConfirm;
	break;

    case BIM_HELP:
	/* help */
	DisplayHelp(BM_FIG_285, FIG_285);
	return menu;
	break;

    case BIM_PREV:
	/* previous menu */
	return &MksysbChoice;
	break;

    default:
	bibeep();
	menu->Message = &InvalidChoice;
	return menu;
    }
}

/*----------------------------------------------------------------------------
 * NAME: MksysbMapPre
 *
 * FUNCTION: preformat the mksysb use maps selection screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	This routine currently doesn't do anything.
 *     
 * RETURNS: 1
 */
int MksysbMapPre( struct Menu *menu)
{
    return 1;
}

/*----------------------------------------------------------------------------
 * NAME: ShellInfoPre
 *
 * FUNCTION: preformat the shell info screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES:
 *	This routine only formats the menu on the first pass
 *     
 * RETURNS: 1
 */
int ShellInfoPre( struct Menu *menu)
{
    char *ptr;			/* text ptr                        */
    int i;
    static char *inst;			/* instruction text		*/
    
    /* do any translations necessay */
    for (i=0; i < 24; i++)
    {
        if (menu->MsgNum[i])
        {
            /* get the translated string */
            ptr = catgets(catfd,BI, menu->MsgNum[i], "");
            if (ptr && *ptr)
            {
                /* Copy the translated string back 
                 */
                menu->Text[i] = ptr;
            }
        }
    }


    if (!inst)
    {
	inst = malloc(strlen(BM_SHELL_INFO)+1);
	if (!inst) exit(2);
    }

    strcpy(inst, BM_SHELL_INFO);
    ptr = catgets(catfd,BI, BS_SHELL_INFO, inst);
    /* parse the multiline info */
    parse(menu, 3, ptr, 2);

    return 1;
}


/*----------------------------------------------------------------------------
 * NAME: ShellInfoDrvr
 *
 * FUNCTION: Process user input from the shell info screen
 *
 * EXECUTION ENVIRONMENT:
 *      local procedure to BosMenus, called by DisplayMenu
 *
 * NOTES: 
 *      When the user says goforit, start a ksh.
 *     
 * RETURNS: next menu 
 */
struct Menu *ShellInfoDrvr(
struct Menu *menu,	/* current menu   */
int *input)		/* keyboard input */
{
    int shpid;		/* pid of shell */

    /* reset error message */
    menu->Message = 0;

    /* process user's input */
    switch (*input)
    {
	case 0:
	    /* start up shell */
	    shpid = fork();
	    if (shpid == 0)
	    {
		struct termios term;
		tcgetattr(0, &term);
		term.c_lflag |= ECHOE;
		tcsetattr(0, TCSANOW, &term);
		putenv("PS1=Enter 'exit' to return $");
		execl("/usr/bin/ksh", "ksh", 0);
		perror("sh exec failed");
		exit(0);
	    }
	    else if (shpid != -1)
		wait(0);
	    return &Maintenance;
	    break;

	case BIM_HELP:
	    /* help */
	    DisplayHelp(BM_FIG_37, FIG_37);
	    return menu;
	    break;

	case BIM_PREV:
	    /* previous menu */
	    return &Maintenance;
	    break;
    }
    bibeep();
    menu->Message = &InvalidChoice;
    return menu;
}
