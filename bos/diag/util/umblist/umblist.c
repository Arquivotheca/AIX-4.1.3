static char sccsid[] = "@(#)92  1.9.1.9  src/bos/diag/util/umblist/umblist.c, dsaumblist, bos41J, 9510A_all 2/27/95 03:41:30";
/*
 * COMPONENT_NAME: (DUTIL) Diagnostic Utility - UMBLIST
 *
 * FUNCTIONS:   main
 *              get_devices
 *              get_new_msgno
 *              action
 *              display_IPList
 *              append_str
 *              count_IPList
 *              device_name
 *              str_format
 *              erase_IPList
 *              alter_IPList
 *              dev_check
 *              do_error
 *              int_handler
 *              genexit
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
#include <signal.h>
#include <sys/cfgdb.h>
#include <sys/mdio.h>
#include <locale.h>
#include <diag/class_def.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_define.h>
#include "umblist.h"
#include "umblist_msg.h"


/* GLOBAL VARIABLES     */
int             mode;                           /* indicates 1 of 3 IPLists */
nl_catd         fdes;                           /* catalog file descriptor  */
MACH_DD_IO      nv;                             /* ioctl struct             */

uchar           iplist[IPLIST_LENGTH];          /* IPList data              */
uchar           unavailable;                    /* Bad Device on IPList     */

uchar generic_used[MAX_GENERIC];                /* TRUE=device in IPList */

typedef struct  {
        char    dev_name[NAMESIZE+1];
        char    descript[MAX_CHAR+1];
        uchar   used;
} *cudv_ptr;

cudv_ptr dev_cudv;
int      num_cudv;


/* LOCAL FUNCTION PROTOTYPES */
int     get_devices(void);
int     get_new_msgno(struct CuDv *);
void    action(void);
void    display_IPList(void);
void    append_str(char *, int);
int     count_IPList(void);
char*   device_name(int, int *);
void    str_format(char *, char *);
void    erase_IPList(void);
void    alter_IPList(void);
char*   dev_check(char *);
void    do_error(int, char *);
void    int_handler(int);
void    genexit(int);

/* EXTERNAL FUNCTION DECLARATIONS */
extern char*    diag_cat_gets();
extern char*    diag_device_gets();
extern char*    strcpy();
extern char*    strncpy(char *, const char *, size_t);
extern nl_catd  diag_catopen(char *, int);
extern nl_catd  diag_device_catopen(char *, int);

/* EXTERNAL VARIABLES   */
extern ASL_SCR_TYPE dm_menutype;

/****************************************************************
* NAME: main
*
* FUNCTION: Determines which bootlist is to be displayed or
*           altered.  Displays first menu of this service aid.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       get_devices()
*       action()
*       genexit()
*
* RETURNS:
*       Exits with 0    (=no problems)
****************************************************************/
main()
{
        int status = -1;
        struct sigaction act;
        static struct msglist menulist[] =
        {
                {MODE_SET, MODE_TITLE},
                {MODE_SET, MODE_OPT1},
                {MODE_SET, MODE_OPT2},
                {MODE_SET, MODE_OPT3},
                {MODE_SET, MODE_LAST},
                {(int )NULL, (int )NULL}
        };

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* initialize ASL       */
        diag_asl_init("DEFAULT");

        /* open the catalog file containing the menus */
        fdes = diag_catopen(MF_UMBLIST, 0);

        /* initialize the ODM   */
        odm_initialize();

        /* Get all AVAILABLE and non-MISSING CuDv's */
        num_cudv = get_devices();

        /****************************************************************
        * If the user presses CANCEL or EXIT, then stop.  Else, select  *
        * desired IPList.                                               *
        ****************************************************************/
        while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT)
        {
                status = diag_display(UMBLIST_MENU1, fdes, menulist,
                                DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                NULL, NULL);
                if (status == DIAG_ASL_COMMIT)
                {
                        switch (DIAG_ITEM_SELECTED(dm_menutype))
                        {
                        case 1 : mode = NORMAL_MODE;    break;
                        case 2 : mode = SERVICE_MODE;   break;
                        case 3 : mode = PREVBOOT_MODE;  break;
                        }
                        action();
                }
        }
        genexit(0);
}

/****************************************************************
* NAME: get_devices
*
* FUNCTION: Retrieves all CuDv's that are AVAILABLE and are
*           not MISSING.  Determines which of these devices
*           can be used as IPL devices.  For each of these,
*           save the device name and descriptive text in
*           "dev_cudv".
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       get_new_msgno()
*       genexit()
*
* RETURNS:
*       # of potential IPL devices.
****************************************************************/
int get_devices(void)
{
        int     ii, jj, k1;
        int     len, setno, msgno;
        char*   text;

        char    criteria[80];
        nl_catd cudv_cat;
        struct  CuDv *cudv;                             /* List of CuDv         */
        struct  listinfo cudv_info;                     /* CuDv info            */

        /****************************************************************
        * Get all CuDv that are not MISSING and are AVAILABLE.  Check   *
        * each device to see if it is useable as an IPL device.  If so, *
        * copy the device name and retrieve a text description of the   *
        * device.                                                       *
        ****************************************************************/
        sprintf(criteria, "chgstatus != %d AND status = %d", MISSING, AVAILABLE);
        cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 50, 2);
        if (cudv == (struct CuDv *) -1) do_error(ODM_ACCESS_ERR, "CuDv");
        if (cudv_info.num > 0)
                dev_cudv = (cudv_ptr ) calloc(cudv_info.num, sizeof(dev_cudv[0]));

        k1 = 0;
        for (ii=0; ii < cudv_info.num; ii++)
        {
                for (jj=0; jj < NUM_SPECIFIC; jj++)
                {
                        if (!strcmp(specific_str[jj], cudv[ii].PdDvLn->prefix))
                                break;
                }

                /**************************************************************
                 * If the name of the CuDv has matched one of the prefixes      *
                 * in specific_str[], then it is a potential IPL device.        *
                 **************************************************************/
                if (jj < NUM_SPECIFIC)
                {
                        strcpy(dev_cudv[k1].dev_name, cudv[ii].name);
                        if (strlen(cudv[ii].name) >= NAMESIZE)
                                dev_cudv[k1].dev_name[NAMESIZE] = '\0';

                        cudv_cat = diag_device_catopen(cudv[ii].PdDvLn->catalog,0);
                        setno = cudv[ii].PdDvLn->setno;
                        msgno = cudv[ii].PdDvLn->msgno;
                        if (msgno == 0)
                                msgno = get_new_msgno(cudv + ii);

                        text = diag_device_gets(cudv_cat, setno, msgno, "n/a");
                        strcpy(dev_cudv[k1].descript, text);
                        if (strlen(text) > MAX_CHAR)
                                dev_cudv[k1].descript[MAX_CHAR+1] = '\0';

                        catclose(cudv_cat);
                        k1++;
                }
        }

        odm_free_list(cudv, &cudv_info);
        return k1;
}
/****************************************************************
* NAME: get_new_msgno
*
* FUNCTION: Looks for a valid message number in the CuAt
*           class or the PdAt class.  This routine is modelled
*           after routine get_device_text() in libdiag.a
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*         do_error()
*
* RETURNS:
*       None
****************************************************************/
int get_new_msgno(struct CuDv *cudv)
{
        int msgno = 0;
        struct CuAt *cuat = (struct CuAt *) NULL;
        struct PdAt *pdat = (struct PdAt *) NULL;
        struct listinfo c_info;
        char crit[100];

        /* check for type 'T' attribute in customized attributes */

        sprintf(crit, "name = %s AND type = T", cudv->name);
        cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit, &c_info, 1, 1);
        if (cuat == (struct CuAt *) -1)
                do_error(ODM_ACCESS_ERR, "CuAt");

        /* if no customized attribute, then get default from PdAt */

        if (c_info.num == 0) {
                sprintf(crit, "uniquetype = %s AND type = T",
                                cudv->PdDvLn_Lvalue);
                pdat = (struct PdAt *)get_PdAt_list(PdAt_CLASS, crit,
                                &c_info, 1, 1);

                if (pdat == (struct PdAt *) -1)
                        do_error(ODM_ACCESS_ERR, "PdAt");
                else if (c_info.num == 1)
                        msgno = atoi(pdat->deflt);
        }
        else
                msgno = atoi(cuat->value);

        if (cuat != (struct CuAt *)NULL)
                odm_free_list(cuat, &c_info);

        return(msgno);
}

/*  */
/****************************************************************
* NAME: action
*
* FUNCTION: Determine what action the user wants to perform
*           on the selected bootlist: display, alter, or erase.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       display_IPList()
*       alter_IPList()
*       erase_IPList()
*
* RETURNS:
*       None
****************************************************************/
void action(void)
{
        int status = -1;
        static struct msglist menulist1[] =
        {
                {ACTION_SET, ACTION_TITLE_N},
                {ACTION_SET, ACTION_OPT1},
                {ACTION_SET, ACTION_OPT2},
                {ACTION_SET, ACTION_OPT3},
                {ACTION_SET, ACTION_LAST},
                {(int )NULL, (int )NULL},
        };


        static struct msglist menulist2[] =
        {
                {ACTION_SET, ACTION_TITLE_P},
                {ACTION_SET, ACTION_OPT4},
                {ACTION_SET, ACTION_OPT5},
                {ACTION_SET, ACTION_LAST},
                {(int )NULL, (int )NULL},
        };


        /****************************************************************
        * Customize the menu to reflect the IPList selected.            *
        ****************************************************************/
        if (mode == NORMAL_MODE)  menulist1[0].msgid = ACTION_TITLE_N;
        if (mode == SERVICE_MODE) menulist1[0].msgid = ACTION_TITLE_S;

        /****************************************************************
        * If user presses CANCEL or EXIT then return.  Else, perform    *
        * desired action on selected IPList.                            *
        ****************************************************************/
        while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT)
        {
                if (mode == PREVBOOT_MODE)
                {
                        status = diag_display(UMBLIST_MENU2, fdes, menulist2,
                                        DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        NULL, NULL);

                        if (status == DIAG_ASL_COMMIT)
                        {
                                switch (DIAG_ITEM_SELECTED(dm_menutype))
                                {
                                case 1 : display_IPList();      break;
                                case 2 : erase_IPList();        break;
                                }
                        }
                }
                else    /* NORMAL or SERVICE IPList */
                {
                        status = diag_display(UMBLIST_MENU2, fdes, menulist1,
                                        DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        NULL, NULL);
                        if (status == DIAG_ASL_COMMIT)
                        {
                                switch (DIAG_ITEM_SELECTED(dm_menutype))
                                {
                                case 1 : display_IPList();      break;
                                case 2 : alter_IPList();        break;
                                case 3 : erase_IPList();        break;
                                }
                        }
                }
        }

        return;
}
/****************************************************************
* NAME: display_IPList
*
* FUNCTION: Displays the devices that are contained in the
*           selected bootlist.  This can be specific devices or
*           generic devices.  There may be devices on the
*           bootlist which are in the MISSING state or which
*           may no longer be in the CuDv object class.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       count_IPList()
*       device_name()
*       append_str()
*
* RETURNS:
*       None
****************************************************************/
void display_IPList(void)
{
        int rc, status;
        int num_dev;
        int line;
        int disp_title;
        int empty;
        int generic_flag;

        char *str1, *str2;

        ASL_SCR_INFO *disp_info;
        static ASL_SCR_TYPE disp_type = DM_TYPE_DEFAULTS;

        /****************************************************************
        * Customize menu to reflect selected IPList.                    *
        ****************************************************************/
        switch (mode)
        {
        case (NORMAL_MODE):
                disp_title = DISP_TITLE_N;
                empty      = DISP_EMPTY_N;
                break;
        case (SERVICE_MODE):
                disp_title = DISP_TITLE_S;
                empty      = DISP_EMPTY_S;
                break;
        case (PREVBOOT_MODE):
                disp_title = DISP_TITLE_P;
                empty      = DISP_EMPTY_P;
                break;
        }

        /****************************************************************
        * Count the # of devices in the specified IPList.  For each     *
        * device, create a line on the menu with the device name and    *
        * a description of the device.  If the device is missing but    *
        * still in the CuDv object class, then the description is       *
        * "Unavailable device".  If the device is not in the CuDv       *
        * class, then the device name is listed as "*****".             *
        *                                                               *
        * If the IPList is empty, display a message indicating such.    *
        ****************************************************************/
        num_dev = count_IPList();
        if (num_dev == 0)
                rc = diag_hmsg(fdes, DISP_SET, empty, NULL);
        else
        {
                disp_info = (ASL_SCR_INFO *) calloc((num_dev + 2),
                                sizeof(ASL_SCR_INFO));
                str1 = str2 = (char *)malloc((num_dev + 2)*(MAX_CHAR+1));
                disp_info[0].text = diag_cat_gets(fdes, DISP_SET, disp_title);
                for (line=1; line < num_dev+1; line++)
                {
                        strcpy(str1, device_name(line - 1, &generic_flag));
                        append_str(str1, generic_flag);
                        disp_info[line].text = str1;
                        str1 = str1 + strlen(str1) + 1;
                }

                disp_info[line].text = diag_cat_gets(fdes, DISP_SET, DISP_OPT1);
                disp_type.max_index = line;
                rc = diag_display(UMBLIST_MENU3, fdes, NULL, DIAG_IO,
                                ASL_DIAG_ENTER_SC, &disp_type, disp_info);
                free(disp_info);
                free(str2);
        }

        return;
}
/****************************************************************
* NAME: append_str
*
* FUNCTION: This function appends the appropriate text
*           description to the device name.  If the device is
*           generic, then find the description in the GEN_SET
*           messages in the umblist catalog.  If the device is
*           a specific one, then get the description from the
*           "dev_cudv" structure.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*               str_format()
*
* RETURNS:
*       None
****************************************************************/
struct {
        char    *name;
        char    msg_num;
} msgtbl[] = {
        {FD,            GEN_FD},
        {SCDISK,        GEN_SCDISK},
        {BADISK,        GEN_BADISK},
        {CDROM,         GEN_CDROM},

        {RMT,           GEN_RMT},
        {TOK,           GEN_TOK},
        {ENT,           GEN_ENT},
        {FDDI,          GEN_FDDI},
};
#define SZ_MSGTBL       (sizeof(msgtbl)/sizeof(msgtbl[0]))

#define msg_no(idx) ( (idx>=0 && idx<SZ_MSGTBL)? msgtbl[idx].msg_num : -1)

void append_str(char *str, int generic_flag)
{
        int  jj;

        /****************************************************************
        * If the device name is generic, then append a message from set *
        * GEN_SET of catalog umblist.cat to string "str".
        ****************************************************************/
        if (generic_flag)
        {
                int     jj      = msgtbl_idx(str);

                if( jj >= 0 )
                        generic_used[jj] = TRUE;
                str_format(str, diag_cat_gets(fdes, GEN_SET, msg_no(jj) ));
        }

        /****************************************************************
        * For a specific device, try to resolve the device name (as     *
        * pointed to by the IPList and retrieved from the CuDv object   *
        * class) with the list of available IPL devices.  If the name   *
        * is recognized, append the device description to the name.     *
        * Otherwise, indicate that the device is not available as an    *
        * IPL device (even though it is in the IPList).                 *
        ****************************************************************/
        else
        {
                for (jj=0; jj < num_cudv; jj++)
                {
                        if (!strcmp(str, dev_cudv[jj].dev_name))
                        {
                                dev_cudv[jj].used = TRUE;
                                str_format(str, dev_cudv[jj].descript);
                                break;
                        }
                }

                if (jj == num_cudv)
                {
                        unavailable = TRUE;
                        str_format(str, diag_cat_gets(fdes, ERROR_SET, NO_DEVICE));
                }
        }

        return;
}
/****************************************************************
* NAME: msgtbl_idx
*
* FUNCTION: This function determines the position in the message
*           table of the string passed in as the argument.  This
*           position, or index, will be used to find the message
*           number GEN_XXX of the description of the generic device
*           named by the argument string.
*
* EXECUTION ENVIRONMENT:
*
* RETURNS:
*       None
****************************************************************/
int msgtbl_idx(char *str)
{
        int jj;
        for (jj=0; jj < SZ_MSGTBL; jj++)
        {
                if (!strcmp(str, msgtbl[jj].name))
                        return(jj);
        }
        return(-1);
}

/****************************************************************
* NAME: count_IPList
*
* FUNCTION: Count the number of devices on the selected
*           bootlist.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       do_error()
*
* RETURNS:
*       # of devices on bootlist
****************************************************************/
int count_IPList(void)
{
        int address;                    /* address to read from */
        int length;                     /* number of bytes to read */
        int fd;                 /* fd for NVRAM */
        int ii, jj;                     /* temp. integers */
        int code_chk1, code_chk2;

        /****************************************************************
        * Initialize variables to access the correct IPList.            *
        ****************************************************************/
        switch (mode)
        {
        case (NORMAL_MODE):
                length  = IPLIST_LENGTH;
                address = IPLIST_NORMAL;
                code_chk1 = NORMAL_VALIDITY_CODE_1;
                code_chk2 = NORMAL_VALIDITY_CODE_2;
                break;
        case (SERVICE_MODE):
                length  = IPLIST_LENGTH;
                address = IPLIST_SERVICE;
                code_chk1 = SERVICE_VALIDITY_CODE_1;
                code_chk2 = SERVICE_VALIDITY_CODE_2;
                break;
        case (PREVBOOT_MODE):
                length  = PREVBOOT_LENGTH;
                address = IPLIST_PREVBOOT;
                break;
        }

        /* open NVRAM */
        if ((fd = open(NVRAM, 2)) < 0)
                do_error(OPEN_ERR, NVRAM);

        /* initialize the machine device driver record */
        nv.md_incr = MV_BYTE;
        nv.md_data = iplist;
        nv.md_size = length;
        nv.md_addr = address;

        /* get the data from NVRAM */
        if (ioctl(fd, MIONVGET, &nv) < 0)
                do_error(IOCTL_ERR, NVRAM);

        close(fd);

        /****************************************************************
        * The NORMAL and SERVICE IPList each start with a 2-byte        *
        * initialization code.  Each device listed starts with a byte   *
        * indicating the # of bytes describing the device.  Use this    *
        * format to count through the devices in the IPList.  If the    *
        * PREVBOOT IPList starts with a non-zero byte then it contains  *
        * 1 device.                                                     *
        ****************************************************************/
        if (mode == PREVBOOT_MODE)      /* count prevboot list */
        {
                jj = 0;
                if (0 < (int )iplist[0]) jj = 1;
        }
        else    /* count NORMAL or SERVICE list */
        {
                if ((iplist[0] != code_chk1) ||
                    (iplist[1] != code_chk2))
                {
                        if ((iplist[0] == 0) && (iplist[1] == 0))
                                jj = 0;
                        else
                                do_error(IPLIST_ERR, NULL);
                }
                else
                {
                        jj = 0;
                        ii = 2;
                        while ((0 < (int )iplist[ii]) && (IPLIST_LENGTH > jj))
                        {
                                jj++;
                                ii += iplist[ii] + 1;
                        }
                }
        }
        return jj;
}
/****************************************************************
* NAME: device_name
*
* FUNCTION: Determine the name of each device listed in
*           the selected bootlist.  The name can represent
*           either a specific device (e.g. hdisk0) or a generic
*           device (e.g. badisk).  The algorithms used here
*           to decode the information in the bootlist follow
*           the code in command "bootlist".
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*               do_error()
*
* RETURNS:
*       NULL   (= device name could not be determined)
*       string (= name of the cnt'th device)
****************************************************************/
char *device_name(int cnt, int *generic_flag)
{
        int  idx1;
        int  idx2;
        int  jj;              /* temp. integer for small loops */
        int  disp_num;
        int  found=0;

        char str[132];
        char sn_str[SSA_SN_SIZE + 1];
        char criteria[132];
        char *locstr;
        char cmpstr[3];

        struct CuAt *cuat;              /* CuAt ptr */
        struct CuDv *cudv;              /* CuDv ptr */
        struct listinfo cuat_info;      /* odm info */
        struct listinfo cudv_info;      /* odm info */

        /****************************************************************
        * "cnt" specifies which device to decipher from the bootlist    *
        * If the bootlist is for the NORMAL or SERVICE mode, then       *
        * remember to skip the first 2 bytes in iplist[].               *
        ****************************************************************/
        idx1 = 2;
        if (mode == PREVBOOT_MODE) idx1 = 0;

        for (jj=0; jj < cnt; jj++)
                idx1 += iplist[idx1] + 1;
        idx2 = idx1 + iplist[idx1];
        idx1++;

        str[0] = '\0';
        *generic_flag = FALSE;

        /****************************************************************
        *  iplist[idx1] indicates a generic device                      *
        ****************************************************************/
        switch (iplist[idx1])
        {
        case GENERIC_CODE:
                *generic_flag = TRUE;
                idx1++;
                for (jj=0; jj < MAX_GENERIC; jj++)
                {
                        if (iplist[idx1] == generic[jj].code2)
                                sprintf(str, "%s", generic[jj].name);
                }
                break;

        /****************************************************************
        * iplist[idx1] indicates a floppy disk device.  The next value  *
        * in iplist[] then indicates the connwhere value for the CuDv   *
        * class.  Since there may be multiple devices with this         *
        * connwhere value, choose the CuDv whose PdDvLn points to a     *
        * PdDv whose prefix value = FD ("fd").                          *
        ****************************************************************/
        case FD_CODE:
                {
                        int     ja;

                        sprintf(criteria, "connwhere=%1x", iplist[idx2]);
                        cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 2, 2);
                        if (cudv == (struct CuDv *) -1) do_error(ODM_ACCESS_ERR, "CuDv");
                        if (cudv_info.num == 0) strcpy(str, "*****");
                        for (ja=0; ja < cudv_info.num; ja++)
                                if (!(strcmp(FD, cudv[ja].PdDvLn->prefix)))
                                {
                                        strcpy(str, cudv[ja].name);
                                        ja = cudv_info.num;
                                }
                }
                break;
       /****************************************************************
        * iplist[idx1] indicates a NVRAM section.                       *
        ****************************************************************/
        case NVLOAD_CODE:
                idx1++;
                if (iplist[idx1] == 1)
                        strcpy(str, NVLOAD1);
                else if (iplist[idx1] == 2)
                        strcpy(str, NVLOAD2);
                break;

        /****************************************************************
        * iplist[idx1] indicates a PVID value.  The CuAt object whose   *
        * "value" field equals this PVID value then indicates the       *
        * specific device.                                              *
        ****************************************************************/
        case PVID_CODE:
                {
                        int     ka;
                        char    hexstr[2*PVID_LENGTH];

                        for (ka=0; ka < PVID_LENGTH-1; ka++)
                        {
                                sprintf(hexstr+2*ka, "%02x", iplist[1+ka+idx1]);
                        }
                        sprintf(criteria, "attribute=pvid AND value=%s", hexstr);
                        cuat = get_CuAt_list(CuAt_CLASS, criteria, &cuat_info, 1, 1);

                        if (cuat == (struct CuAt *) -1)
                                do_error(ODM_ACCESS_ERR, "CuAt");

                        if      (cuat_info.num == 0)    strcpy(str, "*****");
                        else if (cuat_info.num == 1)    strcpy(str, cuat[0].name);
                        else if (cuat_info.num >  1)    do_error(CRITERIA_ERR, "> 1");
                }
                break;

        /****************************************************************
        * iplist[idx1] indicates a bus-attached disk.  The next position*
        * in iplist[] contains a modified connection address (connwhere)*
        * for the device.  This device can only go in bus slots 7 or    *
        * 8.  The value in iplist[] is either 1 or 2, so 6 is added to  *
        * yield the correct connwhere value.                            *
        ****************************************************************/
        case BADISK_CODE:
                idx1++;
                sprintf(criteria, "connwhere=%1x", iplist[idx1] + 6);
                cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);

                if (cudv == (struct CuDv *) -1)
                        do_error(ODM_ACCESS_ERR, "CuDv");

                if      (cudv_info.num == 0) strcpy(str, "*****");
                else if (cudv_info.num == 1) strcpy(str, cudv[0].name);
                else if (cudv_info.num >  1) do_error(CRITERIA_ERR, "> 1");

                break;

        /****************************************************************
        * iplist[idx1] indicates no a network device, so look for the   *
        * parent bus name, then for a device connected to that bus at   *
        * the slot given by iplist[1+idx1]                              *
        ****************************************************************/
        case ENT_CODE:
        case TOK_CODE:
        case FDDI_CODE:
                idx1++;
                sprintf(criteria, "connwhere=%1x AND PdDvLn = bus/sys/mca", iplist[idx1] & 0x01);
                cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);
                if (cudv == (struct CuDv *) -1)
                        do_error(ODM_ACCESS_ERR, "CuDv");

                if (cudv_info.num !=  1)
                        do_error(CRITERIA_ERR, "!= 1");

                sprintf(criteria, "connwhere=%1x AND parent = %s", 1+iplist[idx1+1], cudv->name);
                cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);

                if (cudv == (struct CuDv *) -1)
                        do_error(ODM_ACCESS_ERR, "CuDv");


                /* If ent0 is not found with bus0 as the parent, check to see if */
                /* there is a built-in communication adapter connection to the   */
                /* sio planar.                                                   */

                if (cudv_info.num == 0) {
                        sprintf(criteria, "connwhere=0 AND PdDvLn LIKE adapter/mca/sio*");
                        cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);
                        if (cudv == (struct CuDv *) -1)
                                do_error(ODM_ACCESS_ERR, "CuDv");

                        if (cudv_info.num !=  1)
                                do_error(CRITERIA_ERR, "!= 1");

                        sprintf(criteria, "connwhere=15 AND parent = %s", cudv->name);
                        cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);

                        if (cudv == (struct CuDv *) -1)
                                do_error(ODM_ACCESS_ERR, "CuDv");

                        if      (cudv_info.num == 0) strcpy(str, "*****");
                        else if (cudv_info.num == 1) strcpy(str, cudv[0].name);
                        else if (cudv_info.num >  1) do_error(CRITERIA_ERR, "> 1");
                }
                else if (cudv_info.num == 1) strcpy(str, cudv[0].name);
                else if (cudv_info.num >  1) do_error(CRITERIA_ERR, "> 1");

                break;

        /****************************************************************
        * iplist[idx1] indicates no PVID value, so look for the first   *
        * non-zero value.  This should indicate a SCSI device.  The     *
        * last three iplist[] values for this device indicate the       *
        * device type and the connection address.  Use this info. to    *
        * retrieve the device name from CuDv.                           *
        ****************************************************************/
        case 0:
                while (iplist[++idx1] == 0) ;
                if (iplist[idx1] == SCSI_CODE)
                {
                        char* jdevtype = NULL;

                        switch (iplist[idx2-2])
                        {
                        case SCSI_DISK:         jdevtype = HDISK;       break;
                        case SCSI_DISKETTE:     jdevtype = FD;          break;
                        case SCSI_CDROM:        jdevtype = CDROM;       break;
                        case SCSI_RMT:          jdevtype = RMT;         break;
                        default:                jdevtype = NULL;        break;
                        }

                        if( jdevtype != NULL )
                                sprintf(criteria, "connwhere=%1x,%1x AND name like %s*",
                                        iplist[idx2-1], iplist[idx2], jdevtype);


                        cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);

                        if (cudv == (struct CuDv *) -1)
                                do_error(ODM_ACCESS_ERR, "CuDv");

                        if      (cudv_info.num == 0) strcpy(str, "*****");
                        else if (cudv_info.num == 1) strcpy(str, cudv[0].name);
                        else if (cudv_info.num >  1) {
                           sprintf(cmpstr, "%d%d", iplist[idx2-6]-0x20, iplist[idx2-4]+0x1);
                           for(jj=0; ((jj<cudv_info.num) && (found==0)) ; jj++) {
                              if(cudv[jj].chgstatus != MISSING) {
                                 locstr=strtok(cudv[jj].location, "-");
                                 locstr=strtok(NULL, "-");  /* Get second set of loc'n numbers */
                                 if(!strncmp(locstr, cmpstr, 2)) {
                                    strcpy(str, cudv[jj].name);
                                    found=1;  /* A match was found */
                                 }
                              }
                           }
                           if(found == 0) do_error(NOT_FOUND_ERR, cmpstr);
                        }
                }
                break;
        /****************************************************************
        * iplist[idx1] indicates an SSA value.  Bytes 2 to 16 of the    *
        * device list entry represent the serial number of the SSA      *
        * device. This is held in the connwhere field of the CuDv and   *
        * is used to get the CuDv entry and thus the device name.       *
        ****************************************************************/
        case SSA_CODE:
                strncpy(sn_str, &iplist[idx1+1], SSA_SN_SIZE);
                sn_str[SSA_SN_SIZE] = '\0';
                sprintf(criteria,
                        "connwhere=%s AND name like %s*",
                         sn_str, HDISK);

                cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 1);

                if (cudv == (struct CuDv *) -1)
                        do_error(ODM_ACCESS_ERR, "CuDv");

                if      (cudv_info.num == 0) strcpy(str, "*****");
                else if (cudv_info.num == 1) strcpy(str, cudv[0].name);
                else if (cudv_info.num >  1) do_error(CRITERIA_ERR, "> 1");

                break;

        } /* endof  switch(iplist[idx1]) */

        return str;
}

/****************************************************************
* NAME: str_format
*
* FUNCTION: Concatenate str2 onto str1 with the restrictions
*           that str1 start off being no longer than
*           NAMESIZE characters long and finishes no longer
*           than MAX_CHAR characters long.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       None  (str1 is modified)
****************************************************************/
void str_format(char *str1, char *str2)
{
        char    tmpstr1[512];

        strcpy(tmpstr1, str1);

        /* will result, if NAMESIZE  == 4 and MAX_CHAR == 11             */
        /* for example, in a string of total length 11, of first field 4 */

        sprintf(str1, "%-*.*s%-*.*s",
                NAMESIZE, NAMESIZE, tmpstr1, MAX_CHAR-NAMESIZE, MAX_CHAR-NAMESIZE, str2);
}
/****************************************************************
* NAME: erase_IPList
*
* FUNCTION: Re-initialize the selected bootlist by writing
*           zeros to the appropriate portion of NVRAM.  Actually,
*           command "bootlist" is used to do the erasing.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       None
****************************************************************/
void erase_IPList(void)
{
        int rc, status;
        int erase_msg;
        int erase_set;
        int erase_opt;

        char *new_out;
        char *new_err;
        char options[80];
        char temp[80];
        char* modestr;

        struct msglist *menulist;
        static struct msglist menulist0[] =
        {
                {ERASE_SET_N, ERASE_TITLE_N},
                {ERASE_SET_N, ERASE_NO_N},
                {ERASE_SET_N, ERASE_YES_N},
                {ERASE_SET_N, ERASE_ACTION_N},
                {(int )NULL, (int )NULL}
        };


        static struct msglist menulist1[] =
        {
                {ERASE_SET_S, ERASE_TITLE_S},
                {ERASE_SET_S, ERASE_NO_S},
                {ERASE_SET_S, ERASE_YES_S},
                {ERASE_SET_S, ERASE_ACTION_S},
                {(int )NULL, (int )NULL}
        };


        static struct msglist menulist2[] =
        {
                {ERASE_SET_P, ERASE_TITLE_P},
                {ERASE_SET_P, ERASE_NO_P},
                {ERASE_SET_P, ERASE_YES_P},
                {ERASE_SET_P, ERASE_ACTION_P},
                {(int )NULL, (int )NULL}
        };


        /****************************************************************
        * Customize the confirmation menu and the notification popup    *
        * menu to reflect the chosen IPList.  Also, customize the       *
        * command to be executed.  Erasing of the IPList is performed   *
        * with the "bootlist" routine.                                  *
        ****************************************************************/
        switch (mode)
        {
        case (NORMAL_MODE):
                modestr   = "-mnormal";
                menulist  = menulist0;
                erase_set = ERASE_SET_N;
                erase_opt = ERASE_OPT_N;
                break;
        case (SERVICE_MODE):
                modestr   = "-mservice";
                menulist  = menulist1;
                erase_set = ERASE_SET_S;
                erase_opt = ERASE_OPT_S;
                break;
        case (PREVBOOT_MODE):
                modestr   = "-mprevboot";
                menulist  = menulist2;
                erase_set = ERASE_SET_P;
                erase_opt = ERASE_OPT_P;
                break;
        }

        sprintf(options, "%s -i", modestr);
        new_out = NULL;

        /****************************************************************
        * Display confirmation menu.  If second menu item is selected   *
        * then the user wants to erase the IPList.                      *
        ****************************************************************/
        status = diag_display(UMBLIST_MENU5, fdes, menulist, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, NULL, NULL);
        if (status == DIAG_ASL_COMMIT)
        {
                if (2 == DIAG_ITEM_SELECTED(dm_menutype))
                {
                        status = odm_run_method(BOOTLIST, options,
                                        &new_out, &new_err);

                        if (status == 0)
                                rc = diag_hmsg(fdes, erase_set, erase_opt, NULL);
                        else
                        {
                                strcpy(temp, diag_cat_gets(fdes, ERROR_SET, BOOTLIST_ERR));
                                diag_asl_msg(temp, new_err);
                                free(new_err);
                        }
                }
        }

        return;
}
/****************************************************************
* NAME: alter_IPList
*
* FUNCTION: Allows the user to alter the selected bootlist
*           by specifying which devices should be used.  This
*           list of devices is then written to a file.  The
*           file is used in conjunction with command "bootlist"
*           to actually alter NVRAM.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       count_IPList()
*       device_name()
*       append_str()
*       str_format()
*       dev_check()
*
* RETURNS:
*       None
****************************************************************/
void alter_IPList(void)
{
        int ii, jj, k1, len;
        int rc, status;
        int num_dev, inum;
        int line;
        int mod_title;
        int empty;
        int generic_flag;

        char *str1, *str2, *str3;
        char criteria[80];
        char options[80];
        char *new_out;
        char *new_err;

        struct PdDv     *pddv;          /* PdDv ptr */
        struct listinfo pddv_info;      /* odm info */

        FILE            *fd;
        ASL_SCR_INFO    *mod_info;

        static ASL_SCR_TYPE     mod_type = DM_TYPE_DEFAULTS;
        char*   modestr;

        /****************************************************************
        * Initialize variables.  Customize menu and the "bootlist"      *
        * command to reflect the chosen IPList.  Allocate storage for   *
        * information to be put on the menu.                            *
        ****************************************************************/
        unavailable = FALSE;
        for (ii=0; ii < num_cudv; ii++)
                dev_cudv[ii].used = FALSE;

        for (ii=0; ii < MAX_GENERIC; ii++)
                generic_used[ii] = FALSE;

        switch(mode)
        {
        case NORMAL_MODE:
                modestr = "-mnormal";
                mod_title = ALT_TITLE_N;
                break;
        case SERVICE_MODE:
                modestr = "-mservice";
                mod_title = ALT_TITLE_S;
                break;
        }
        sprintf(options, "%s -f %s", modestr, DEVICE_FILENAME);
        new_out = NULL;

        /****************************************************************
        * Allocate memory for the menu.  The number of display lines    *
        * to allocate, inum, is overkill since many (but perhaps not    *
        * all) of the devices represented by num_cudv are also re-      *
        * presented by num_dev.                                         *
        ****************************************************************/
        num_dev = count_IPList();
        inum    = num_dev + num_cudv + MAX_GENERIC + 2;
        mod_info = (ASL_SCR_INFO *) calloc(inum, sizeof(ASL_SCR_INFO));

        memset(mod_info, 0, inum*sizeof(ASL_SCR_INFO));
        str1 = str2 = (char *)malloc(inum*(MAX_CHAR+1));
        mod_info[0].text = diag_cat_gets(fdes, ALT_SET, mod_title);

        /****************************************************************
        * Fill in the menu with the names of devices already on the     *
        * IPList.  Indicate the access order of each device.            *
        ****************************************************************/
        for (line=1; line < num_dev+1; line++)
        {
                strcpy(str1, device_name(line - 1, &generic_flag));
                append_str(str1, generic_flag);

                mod_info[line].text = str1;
                mod_info[line].entry_size = 2;
                mod_info[line].entry_type = ASL_NUM_ENTRY;
                mod_info[line].data_value = (char *)calloc(1, 3);

                sprintf(mod_info[line].data_value, "%2d", line);
                mod_info[line].disp_values = mod_info[line].data_value;
                str1 = str1 + strlen(str1) + 1;
        }

        /****************************************************************
        * Add to the menu those CuDv's which can be used for IPL and    *
        * which are not on the current IPList.                          *
        ****************************************************************/
        for (ii=0; ii < num_cudv; ii++)
        {
                if (!dev_cudv[ii].used)
                {
                        strcpy(str1, dev_cudv[ii].dev_name);
                        str_format(str1, dev_cudv[ii].descript);

                        mod_info[line].text = str1;
                        mod_info[line].entry_size = 2;
                        mod_info[line].entry_type = ASL_NUM_ENTRY;
                        mod_info[line].data_value = (char *)calloc(1, 3);

                        sprintf(mod_info[line].data_value, "%s", "  ");
                        mod_info[line].disp_values = mod_info[line].data_value;
                        str1 = str1 + strlen(str1) + 1;
                        line++;
                }
        }

        /****************************************************************
        * Add the names of generic devices which can be used for IPL.   *
        * Pick only those device names which are not already on the     *
        * IPList.                                                       *
        ****************************************************************/
        for (jj=0; jj < MAX_GENERIC; jj++)
        {
                int     k = msgtbl_idx(generic[jj].name);
                strcpy(str1, generic[jj].name);
                if (!generic_used[k])
                {

                        str_format(str1, diag_cat_gets(fdes, GEN_SET, msg_no(k)));

                        mod_info[line].text = str1;
                        mod_info[line].entry_size = 2;
                        mod_info[line].entry_type = ASL_NUM_ENTRY;
                        mod_info[line].data_value = (char *)calloc(1, 3);

                        sprintf(mod_info[line].data_value, "%s", "  ");
                        mod_info[line].disp_values = mod_info[line].data_value;
                        str1 = str1 + strlen(str1) + 1;
                        line++;
                }
        }

        /****************************************************************
        * Add the instruction line to the menu and present it to the    *
        * user.  If the user responds with an DIAG_ASL_COMMIT, then     *
        * read the values specified for the access order.  For each     *
        * device specified, write the device name to a file.  This      *
        * file is then used as input to the "bootlist" routine          *
        * which will perform the actual modification of the IPList.     *
        ****************************************************************/
        mod_info[line].text = diag_cat_gets(fdes, ALT_SET, ALT_OPT1);
        mod_type.max_index = line;

        status = -1;
        while (status != 0)
        {
                if (unavailable)
                {
                        rc = diag_hmsg(fdes, ERROR_SET, UNAVAILABLE, NULL);
                        unavailable = FALSE;
                }

                rc = diag_display(UMBLIST_MENU4, fdes, NULL, DIAG_IO,
                                ASL_DIAG_DIALOGUE_SC, &mod_type, mod_info);

                if (rc == DIAG_ASL_COMMIT)
                {
                        fd = fopen(DEVICE_FILENAME, "w");
                        ii = 1;
                        while (ii < line)
                        {
                                for (jj=1; jj < line; jj++)
                                        if ((ii == atoi(mod_info[jj].data_value)) &&
                                            ((str3 = dev_check(mod_info[jj].text)) != NULL))
                                        {
                                                fprintf(fd, "%s\n", str3);
                                                ii++;
                                                break;
                                        }

                                if (jj == line)
                                        break;
                        }

                        fclose(fd);

                        if (!unavailable)
                        {
                                status = odm_run_method(BOOTLIST, options,
                                                &new_out, &new_err);
                                if (status != 0)
                                {
                                        str3 = diag_cat_gets(fdes, ERROR_SET, BOOTLIST_ERR);
                                        diag_asl_msg(str3, new_err);
                                        free(new_err);
                                }
                        }
                        unlink(DEVICE_FILENAME);
                }
                else if (rc == DIAG_ASL_HELP)
                        continue;

                else /* User entered CANCEL or EXIT*/
                        status = 0;

        }  /* end of while (status != 0) */

        free(mod_info);
        free(str2);

        return;
}
/****************************************************************
* NAME: dev_check
*
* FUNCTION: Checks to see if the user has selected a device
*           which is no longer available for IPL.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       NULL    (= device NOT available for IPL)
*       string  (= device name to be used in "bootlist" command)
****************************************************************/
char *dev_check(char *text)
{
        static char str1[MAX_CHAR+1];
        char *str2, *str3;

        str1[0] = '\0';
        str2 = text + NAMESIZE;
        str3 = diag_cat_gets(fdes, ERROR_SET, NO_DEVICE);

        if (!strncmp(str2, str3, MIN_STR(str2, str3)))
                unavailable = TRUE;
        else
        {
                sprintf(str1, "%-*.*s", NAMESIZE, NAMESIZE, text);
        }

        return str1;
}
/****************************************************************
* NAME: do_error
*
* FUNCTION: Generate a menu containing the error message.
*           Call the generic exit routine.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       genexit()
*
* RETURNS:
*       None
****************************************************************/
void do_error(int emsg_num, char *msg_str)
{
        int  rc;
        char temp[80];

        /*      Perror(fdes, ERROR_SET, emsg_num, msg_str);             */
        strcpy(temp, diag_cat_gets(fdes, ERROR_SET, emsg_num));
        diag_asl_msg(temp, msg_str);
        genexit(-1);
}
/****************************************************************
* NAME: int_handler
*
* FUNCTION: In case of an interrupt, this routine is called.
*           (NOTE:  It was unclear whether this routine would
*           be called with 1 argument, as indicated by the
*           sigaction structure, or 3 arguments, as indicated
*           in the documentation of system call sigaction().)
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       genexit()
*
* RETURNS:
*       None
****************************************************************/
/* void int_handler(int sig, int code, struct sigcontext *scp)  */
void int_handler(int sig)
{
        diag_asl_clear_screen();
        genexit(1);
}

/****************************************************************
* NAME: genexit
*
* FUNCTION: Exit the ASL menu mode.  Relinquish all acquired
*           ODM data.  Close the catalog "umblist.cat".
*           Exit to process that invoked umblist.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       None
****************************************************************/
void genexit(int exitcode)
{
        diag_asl_quit();
        odm_terminate();
        catclose(fdes);
        exit(exitcode);
}
