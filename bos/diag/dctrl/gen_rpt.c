static char sccsid[] = "@(#)14	1.7.1.2  src/bos/diag/dctrl/gen_rpt.c, dctrl, bos411, 9428A410j 9/24/93 15:16:44";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS: gen_rpt
 *            append_menugoal
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
#include <sys/stat.h>
#include <errno.h>
#include "diag/diag.h"
#include "diag/diago.h"

/*
 * NAME: gen_rpt
 *
 * FUNCTION: Generate an error report file from fru bucket data.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: The latest error report file is obtained by executing a li -Smr
 *      command. This assumes that the timestamping is working correctly.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      0 - No error
 *     -1 - Error occurred
 */

#define MAX_FILE_NUMBER 25
#define ERROR_FILE_NAME "diagrpt"
#define ERROR_FILE_EXT  "dat"

/* Local variables */
FILE    *fd, *fopen(const char *filename, const char *mode);

/* External variables */
extern int diskette_based; 		/* executing off diskette       */
extern int exenvflg;                    /* execution mode               */
extern int errno;
extern int srn_generated;               /* was gen_rpt called with a srn */

 int
gen_rpt( menu_number, menutype, menuinfo )
long            menu_number;
ASL_SCR_TYPE    menutype;
ASL_SCR_INFO    *menuinfo;
{

        int     i, j;
        int     length;
        int     rc;
        int     mode = 0;
        int     num_files = MAX_FILE_NUMBER;
        time_t  last_mod_time[25];
        char    filename[256];
        char    buffer[100];
        char    *file_ptr;
        char    *buff_ptr;
        char    *datadir;       /* points to default data path          */
        struct  stat    buf;    /* holds file status information        */

        /* do not save if running from standalone media */
        if ( diskette_based )
                return(0);

        /* get directory path where error report files reside */
        if((datadir = (char *)getenv("DIAGDATADIR")) == NULL )
                datadir = DIAGDATA;

        /* verify the directory is present */
        if((rc = stat(datadir, &buf)) != 0)
                if (errno == ENOENT)
                        if ( mkdir(datadir, mode) )
                                return(-1);

        /* find the next file to put the fru bucket data into           */
        /* The maximum number of data files is 25.                      */
        for ( i=1; i <= num_files; i++ ) {
                sprintf(filename, "%s/%s%d.%s", datadir,
                                ERROR_FILE_NAME,i,ERROR_FILE_EXT);
                if((rc = stat(filename, &buf)) != 0)  {
                        if (errno == ENOENT)
                                break;          /* if file does not exist */
                        return(-1);
                }
                last_mod_time[i-1] = buf.st_mtime;
        }

        /* if 25 files are present, find the earliest one to write over */
        if ( i > num_files ) {
                for ( i=0; i <= num_files; i++ ) {
                        if ( i == (num_files - 1) ) {
                                if ( last_mod_time[i] >= last_mod_time[0] ) {
                                        i = 1;
                                        break;
                                }
                        }
                        else if ( last_mod_time[i] > last_mod_time[i+1] ) {
                                i+=2;;
                                break;
                        }
                }
                sprintf(filename, "%s/%s%d.%s", datadir,
                                ERROR_FILE_NAME,i,ERROR_FILE_EXT);
        }

        /* open the file for writing */
        if ((fd = fopen(filename,"w")) == NULL)
                return(-1);

        /* put in all the text lines from menuinfo */
        for ( i=0; i <=  menutype.max_index; i++ ) {

                /* if last line, add a blank line before the text */
                if ( i == menutype.max_index )
                        fprintf(fd, "\n");

                /* if title line, check to see if menu number has to be added */
                if ( i == 0 ) {
                        if ( menu_number == (long)NULL )
                                fprintf(fd, "%s\n\n", menuinfo[i].text);
                        else  {
                                length = strcspn(menuinfo[0].text, "\n");
                                strncpy(buffer, menuinfo[0].text, length);
                                for(j=length; j<74; buffer[j++] = ' ');
                                buffer[j] = '\0';
                                fprintf(fd, "%s%X%s\n\n", buffer, menu_number,
                                                &menuinfo[0].text[length]);
                        }
                }
                /* else just copy the text over */
                else
                        fprintf(fd, "%s\n", menuinfo[i].text);
        }

        /* set global variable srn_generated so that the menugoal info will */
        /* be appended to the end of this file instead of going into a new  */
        /* file.                                                            */
        srn_generated = DIAG_TRUE;

        fclose(fd);
        return (rc);
}


/*
 * NAME: append_menugoal
 *
 * FUNCTION: Appends menugoal information to the end of a data file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: The latest error report file is obtained by executing a li -Smr
 *      command. This assumes that the timestamping is working correctly.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      0 - No error
 *     -1 - Error occurred
 */

 int
append_menugoal( menu_number, menutype, menuinfo )
long            menu_number;
ASL_SCR_TYPE    menutype;
ASL_SCR_INFO    *menuinfo;
{

        int     i, j;
        int     length;
        int     rc;
        int     mode = 0;
        int     num_files = MAX_FILE_NUMBER;
        time_t  last_mod_time[25];
        char    filename[256];
        char    buffer[100];
        char    *file_ptr;
        char    *buff_ptr;
        char    *datadir;       /* points to default data path          */
        struct  stat    buf;    /* holds file status information        */
        char    blank[26] = "                         ";
        char    pattern[31] = "- - - - - - - - - - - - - - - ";


        /* determine execution mode */
        exenvflg = ipl_mode( &diskette_based );
        if ( diskette_based )
                return(0);

        /* get directory path where error report files reside */
        if((datadir = (char *)getenv("DIAGDATADIR")) == NULL )
                datadir = DIAGDATA;

        /* verify the directory is present */
        if((rc = stat(datadir, &buf)) != 0)
                if (errno == ENOENT)
                        if ( mkdir(datadir, mode) )
                                return(-1);

        /* find the next file to put the fru bucket data into           */
        /* The maximum number of data files is 25.                      */
        for ( i=1; i <= num_files; i++ ) {
                sprintf(filename, "%s/%s%d.%s", datadir,
                                ERROR_FILE_NAME,i,ERROR_FILE_EXT);
                if((rc = stat(filename, &buf)) != 0)  {
                        if (errno == ENOENT)
                                break;          /* if file does not exist */
                        return(-1);
                }
                last_mod_time[i-1] = buf.st_mtime;
        }

        /* if 25 files are present, find the earliest one to write over */
        if ( i > num_files ) {
                for ( i=0; i <= num_files; i++ ) {
                        if ( i == (num_files - 1) ) {
                                if ( last_mod_time[i] >= last_mod_time[0] ) {
                                        i = 1;
                                        break;
                                }
                        }
                        else if ( last_mod_time[i] > last_mod_time[i+1] ) {
                                i+=2;;
                                break;
                        }
                }
                sprintf(filename, "%s/%s%d.%s", datadir,
                                ERROR_FILE_NAME,i,ERROR_FILE_EXT);
        }

        /* if a diagnostic error report file has been generated with the */
        /* related SRN info append the menugoal info to that file, else  */
        /* create a new file for the menugoal info.                      */
        if (srn_generated == DIAG_TRUE) {
                if (i == 1)
                        i = num_files;
                else
                        i--;

                sprintf(filename, "%s/%s%d.%s", datadir,
                                ERROR_FILE_NAME,i,ERROR_FILE_EXT);
                if ((fd = fopen(filename,"a")) == NULL)
                        return(-1);

                fprintf(fd, "\n%s%s\n\n", blank, pattern);
        }
        else {
                if ((fd = fopen(filename,"w")) == NULL)
                        return(-1);
        }


        /* put in all the text lines from menuinfo */
        for ( i=0; i <=  menutype.max_index; i++ ) {

                /* if last line, add a blank line before the text */
                if ( i == menutype.max_index )
                        fprintf(fd, "\n");

                /* if title line, check to see if menu number has to be added */
                if ( i == 0 ) {
                        if ( menu_number == (long)NULL )
                                fprintf(fd, "%s\n\n", menuinfo[i].text);
                        else  {
                                length = strcspn(menuinfo[0].text, "\n");
                                strncpy(buffer, menuinfo[0].text, length);
                                for(j=length; j<74; buffer[j++] = ' ');
                                buffer[j] = '\0';
                                fprintf(fd, "%s%X%s\n\n", buffer, menu_number,
                                                &menuinfo[0].text[length]);
                        }
                }
                /* else just copy the text over */
                else
                        fprintf(fd, "%s\n", menuinfo[i].text);
        }

        /* reset srn_generated so that any future menugoal info will go into */
        /* a new file unless gen_rpt is called with a SRN.                   */
        srn_generated = DIAG_FALSE;

        fclose(fd);
        return (rc);
}
