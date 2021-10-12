static char sccsid[] = "@(#)00  1.9  src/bos/diag/util/udtl/udtl.c, dsaudtl, bos411, 9428A410j 3/23/93 09:37:03";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS:   main
 *              display
 *              add
 *              delete
 *              disp_dtl_menu
 *              resource_desc
 *              copy_text
 *              search_db
 *              get_next_device
 *              int_handler
 *              genexit
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

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <locale.h>
#include <sys/cfgdb.h>
#include "udtl_msg.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"

#define DISPLAY_TYPE    1
#define DELETE_TYPE     2
#define ADD_TYPE        3

/* GLOBAL VARIABLES     */
int consoleflg = CONSOLE_TRUE;
int num_All;                    /* number of devices in All              */
int num_Top;                    /* number of devices in Top              */
nl_catd fdes;                   /* catalog file descriptor               */
diag_dev_info_t     *Top;       /* pointer to all device data structures */
diag_dev_info_ptr_t *All;       /* array containing devices in All       */
diag_dev_info_ptr_t *DTL_disp;  /* array containing devices listed       */

/* LOCAL FUNCTION PROTOTYPES */
void display(void);
void add(void);
void delete(void);
int disp_dtl_menu(int, int, ASL_SCREEN_CODE, struct msglist *);
int resource_desc(char *, diag_dev_info_t *);
int copy_text(int, char *, char *);
void search_db(int, int, int);
void get_next_device(int, int, int, char *, char *);
void int_handler(int);
void genexit(int);

/* EXTERNAL VARIABLES   */
extern ASL_SCR_TYPE dm_menutype;

/* CALLED FUNCTIONS     */
diag_dev_info_t     *init_diag_db();
diag_dev_info_ptr_t *generate_All();
extern char *diag_cat_gets();
extern nl_catd diag_catopen(char *, int);

/*  */
/*
 * NAME: main
 *
 * FUNCTION: This unit displays the Diagnostic Test List Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

main(argc,argv)
int argc;
char *argv[];
{

        int     status = -1;
        struct sigaction act;
        static struct msglist menulist[] = {
                {MSET, MTITLE},
                {MSET, MOPT1},
                {MSET, MOPT2},
                {MSET, MOPT3},
                {MSET, MLASTLINE},
                {(int )NULL, (int )NULL}
        };

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* initialize ASL       */
        diag_asl_init("DEFAULT");

        /* open the catalog file containing the menus */
        fdes = diag_catopen(MF_UDTL,0);

        /* initialize the ODM   */
        init_dgodm();

        /* initialize the data structures for the test devices  */
        if ((Top = init_diag_db( &num_Top )) == (diag_dev_info_t *) -1 )
                genexit(-1);

        /* get list of all devices to be included in diag test selection */
        All = generate_All( num_Top, Top , &num_All);

        DTL_disp = (diag_dev_info_ptr_t *)
                   calloc(num_All+5, sizeof(DTL_disp[0]));

        while( status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT )  {

                status = diag_display(0x802000, fdes, menulist,
                                        DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        NULL, NULL);
                if( status == DIAG_ASL_COMMIT)
                        switch ( DIAG_ITEM_SELECTED(dm_menutype) )  {
                                case 1 :
                                        display();
                                        break;
                                case 2 :
                                        add();
                                        break;
                                case 3 :
                                        delete();
                                        break;
                        }
        }
        genexit(0);
}
/*  */
/*
 * NAME: display
 *
 * FUNCTION: Display Test List Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */

void display(void)
{
        int     rc;
        static struct msglist menulist[] = {
                {MSET1, M1TITLE},
                {MSET1, M1LASTLINE},
                {(int )NULL, (int )NULL}
        };

        rc = disp_dtl_menu(DISPLAY_TYPE, 0x802001,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC, menulist);

}

/*  */
/*
 * NAME: add
 *
 * FUNCTION: Displays Add Resource Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */

void add(void)
{
        int     rc;
        int     idx;
        static struct msglist menulist[] = {
                { MSET3, M3TITLE},
                { MSET3, M3LASTLINE_A},
                {(int )NULL, (int )NULL}
        };

        rc = disp_dtl_menu(ADD_TYPE, 0x802002,
                           ASL_DIAG_LIST_COMMIT_SC, menulist);

        if ( rc == DIAG_ASL_COMMIT )
                for(idx=0; idx < num_All; idx++)
                        if (All[idx]->Asterisk == '*') {
                                All[idx]->Asterisk = ' ';
                                if ( All[idx]->T_CDiagDev != NULL )
                                        All[idx]->T_CDiagDev->RtMenu =
                                                                RTMENU_DEF;
                        }
}
/*  */
/*
 * NAME: delete
 *
 * FUNCTION: Displays Delete Resource Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */

void delete(void)
{
        int     rc;
        int     idx;
        static struct msglist menulist[] = {
                {MSET4, M4TITLE},
                {MSET4, M4LASTLINE_A},
                {(int )NULL, (int )NULL}
        };

        rc = disp_dtl_menu(DELETE_TYPE, 0x802003,
                           ASL_DIAG_LIST_COMMIT_SC, menulist);

        if ( rc == DIAG_ASL_COMMIT )
                for(idx=0; idx < num_All; idx++)
                        if (All[idx]->Asterisk == '*') {
                                All[idx]->Asterisk = ' ';
                                if ( All[idx]->T_CDiagDev != NULL )
                                        All[idx]->T_CDiagDev->RtMenu =
                                                                RTMENU_DDTL;
                        }
}

/*  */
/* NAME: disp_dtl_menu
 *
 * FUNCTION: Display diagnostic test list menu.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS:
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *      DIAG_ASL_COMMIT
 */

int disp_dtl_menu(int type,                     /* DISPLAY, DELETE, or ADD  */
                  int menu_number,              /* menu number              */
                  ASL_SCREEN_CODE screen_type,  /* asl screen type          */
                  struct msglist *menulist)     /* menu title and lastlines */
{

        int             i;
        int             idx;
        int             num_disp_devices;
        int             rc;
        int             line = 0;
        int             selection;
        int             cnt;
        char            *string;
        char            *free_string;
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,(num_All+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = (char *)malloc(num_All*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array   */
        resinfo[line++].text = diag_cat_gets(fdes,
                        menulist[0].setid, menulist[0].msgid);

        /* read the entries from All, get the DName, location, and
           description and put them in the array        */
        for (idx=0, num_disp_devices=0; idx < num_All; idx++)  {
                if ((All[idx]->T_PDiagDev == NULL) ||
                    (All[idx]->T_CuDv->chgstatus == MISSING))
                        continue;
                if ((type == ADD_TYPE)  &&
                    (All[idx]->T_CDiagDev->RtMenu == RTMENU_DDTL))
                        resource_desc(string,All[idx]);
                else if ((type != ADD_TYPE)  &&
                         (All[idx]->T_CDiagDev->RtMenu == RTMENU_DEF))
                        resource_desc(string,All[idx]);
                else
                        continue;

                resinfo[line].text = string;
                resinfo[line].non_select = ASL_NO;
                string = string + strlen(string) + 1;
                line++;
                DTL_disp[num_disp_devices] = All[idx];
                DTL_disp[num_disp_devices]->Asterisk = ' ';
                num_disp_devices++;
        }

        /* finally add the last line */
        resinfo[line].text = diag_cat_gets(fdes,
                                menulist[1].setid, menulist[1].msgid);
        restype.max_index = line;

        cnt = num_disp_devices;
        if (type == ADD_TYPE)
          {
          if (cnt == 0)
            rc = diag_hmsg(fdes, ESET, NO_ADD, NULL);
          }
        else if (type == DELETE_TYPE)
          {
          for (cnt=0, idx=0; idx < num_disp_devices; idx++)
            if ((DTL_disp[idx]->T_PDiagDev->Menu & DIAG_NOTDLT) == 0) cnt++;
          if (cnt == 0)
            rc = diag_hmsg(fdes, ESET, NO_DEL, NULL);
          }

        /* now display menu */
        while (cnt) {
                /* flag the selected devices */
                for (idx=0, line=1; idx < num_disp_devices;
                     idx++, line++)
                        if (DTL_disp[idx]->T_PDiagDev->Menu & DIAG_NOTDLT)
                                resinfo[line].item_flag = '-';
                        else if (DTL_disp[idx]->Asterisk == '*')
                                resinfo[line].item_flag = '*';
                        else
                                resinfo[line].item_flag = ' ';
                rc = diag_display(menu_number, fdes, NULL, DIAG_IO,
                                        screen_type,
                                        &restype, resinfo);
                if ((type == DISPLAY_TYPE) || (rc == DIAG_ASL_EXIT)
                                           || (rc == DIAG_ASL_CANCEL)
                                           || (rc == DIAG_ASL_COMMIT)
                                           || (num_disp_devices == 0))
                        break;

                selection = DIAG_ITEM_SELECTED(restype);
                search_db(type, --selection, rc);
        }
        free ( free_string );
        free ( resinfo );

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
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * NOTES:
 *      This function takes as input a line type which indicates the type
 *      of display line to build.
 *
 * RETURNS: 0
 */

int resource_desc(char *string,
                  diag_dev_info_t *dev_ptr)
{

        char    *tmp;

        sprintf(string, "%-16s %-16.16s ",
                        dev_ptr->T_CuDv->name,
                        dev_ptr->T_CuDv->location);
        tmp = string + strlen(string);
        copy_text( strlen(string), tmp, dev_ptr->Text);
        return(0);
}

/*   */
/* NAME: copy_text
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: 0
 */

int copy_text(int string_length,        /* current length of text already in buffer     */
              char *buffer,             /* buffer to copy text into     */
              char *text)               /* text to be copied            */
{

        int     i;
        int     space_count;
        int     char_positions;

        /* determine if length of text string will fit on one line */
        char_positions = LINE_LENGTH - string_length;
        if ( char_positions < strlen(text))  {

                /* dont break the line in the middle of a word */
                if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
                        while ( --char_positions )
                                if( text[char_positions] == ' ')
                                        break;
                if ( char_positions == 0 )
                        char_positions = LINE_LENGTH - string_length;

                for ( i = 0; i < char_positions; i++, buffer++, text++ )
                        *buffer = ( *text == '\n' ) ? ' ' : *text;
                *buffer++ = '\n';
                while ( *text == ' ' )   /* remove any leading blanks */
                        text++;
                space_count = string_length;
                while ( space_count-- )
                        *buffer++ = ' ';
                copy_text( string_length, buffer, text);
        }
        else
                sprintf(buffer, "%s", text);

        return(0);
}

/*  */
/*
 * NAME: search_db
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void search_db(int type,
               int dev_idx,
               int action)
{

        int deselect;

        /* dev_idx is index into DTL_disp device structure      */

        /* if this device should not be deleted, then return    */
        if (DTL_disp[dev_idx]->T_PDiagDev->Menu & DIAG_NOTDLT )
          {
          diag_hmsg(fdes, ESET, ERROR2, DTL_disp[dev_idx]->T_CuDv->name);
          return;
          }

        if (DTL_disp[dev_idx]->Asterisk == '*') {
                DTL_disp[dev_idx]->Asterisk = ' ';
                deselect = 1;
        }
        else {
                DTL_disp[dev_idx]->Asterisk = '*';
                deselect = 0;
        }

        get_next_device(action, type, deselect,
                                DTL_disp[dev_idx]->T_CuDv->parent,
                                DTL_disp[dev_idx]->T_CuDv->name);
}

/*  */
/*
 * NAME: get_next_device
 *
 * FUNCTION: Search database for indicated device
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void get_next_device(int action,
                     int type,
                     int deselect,
                     char *parent,
                     char *name)
{
        int             count;

        if ( ( (type == ADD_TYPE) && !deselect) ||
             ( (type == DELETE_TYPE) && deselect) ) {
                for ( count=0; count < num_All; count++ )
                        if(!strcmp(parent, All[count]->T_CuDv->name) ) {
                                All[count]->Asterisk = (deselect) ? ' ' : '*';
                                get_next_device(action, type, deselect,
                                        All[count]->T_CuDv->parent,
                                        All[count]->T_CuDv->name);
                        }
        }
        else
                for ( count=0; count < num_All; count++ )
                        if(!strcmp(name, All[count]->T_CuDv->parent) ) {
                                All[count]->Asterisk = (deselect) ? ' ' : '*';
                                get_next_device(action, type, deselect,
                                        All[count]->T_CuDv->parent,
                                        All[count]->T_CuDv->name);
                        }
}

/*  */
/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
        diag_asl_clear_screen();
        genexit(1);
}

/*  */
/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void genexit(int exitcode)
{
        /* save all changes to the Customized Diag Object Class */
        if (Top != (diag_dev_info_t *) -1 )  {
                if ( save_cdiag ( Top, num_Top ) )
                        Perror( fdes, ESET, ERROR1, "CDiagDev");
        }

        diag_asl_quit();
        catclose(fdes);
        exit(exitcode);
}
