static char sccsid[] = "@(#) 99 1.6 src/bos/usr/lpp/bosinst/BosMenus/readBos.c, bosinst, bos411, 9435A411a 94/08/25 17:10:37";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: read_bosinstdata, write_bosinstdata
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
/* readBos.c
 *
 * description: reads the bosinst.data file into the global structure
 * 	BosData.
 */
#include <string.h>
#include <stdio.h>
#include "BosInst.h"
extern struct BosInst BosData;                 /* bosinst.data file    */

struct comment                          /* list of comments in the file */
{
    struct comment *next;
    char *text;                         /* text of comment              */
};
static struct comment *first_comment, *last_comment;


/*
 * NAME: read_bosinstdata
 *
 * FUNCTION: read the bosinst.data file
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure is called by BosMenus and CheckSize to read the
 *	bosinst.data file.
 *
 * NOTES:
 *	Environment variable BOSINSTDATA points to bosinst.data.  If not 
 *	set, the default is current directory
 *
 * DATA STRUCTURES:
 *	BosData structure filled in
 *
 * RETURNS: None.
 */

read_bosinstdata()
{
    char *BosFileName;			/* bosinst.data file name	*/
    FILE *fp;				/* fiel ptr to bosinst.data	*/
    char wholeline[80];			/* a line from bosinst.data	*/
    char *line;				/* ptr within line		*/
    char *ptr, *eptr;			/* another ptr within line	*/
    struct target_disk_data *tddp = 0;	/* target disk data ptr		*/
    struct comment *cmt;		/* save comment structure	*/

    BosFileName = getenv("BOSINSTDATA");
    if (!BosFileName)
	BosFileName = "bosinst.data";
    
    fp = fopen(BosFileName, "r");
    if (!fp) return;

    /* loop until the end of file */
    while (1)
    {
	/* read a line from the file */
	line = wholeline;
	if(!fgets(line, 80, fp)) break;

        /* save comment line */
        if (*line == '#')
        {
            cmt = malloc(sizeof (struct comment));
            cmt->text = malloc(strlen(line));
            if (first_comment)
            {
                last_comment->next = cmt;
            }
            else
                first_comment = cmt;

            last_comment = cmt;
            strncpy(cmt->text, line, 160);
            continue;
        }



	/* put a trailing blank on any null field */
	eptr = strchr(line, '\n');
	if (!eptr)   /* If line is full, the new char will be overwritten */
	    eptr = line + 79;
	ptr = strchr(line, '=');
	if (ptr)
	{
            if((unsigned)(eptr-ptr) < 2)
                /* this field has no value, so continue */
                continue;
            else
            {
                *eptr-- = '\0';
                /* remove all trailing blanks */
                while (*eptr == ' ')
                    *eptr-- = '\0';

		/* if this failed is empty, continue */
                if (eptr == ptr)
		    continue;
            }

	}

	if (!strncmp(line, "target_disk_data:", 17))
	{

	    /* malloc a new node for target disk stucture */

	    tddp= (struct target_disk_data *)malloc (sizeof(struct target_disk_data));
	    if (!tddp)
		exit(2);

	    if (BosData.last)
		BosData.last->next = tddp;
	    else
		BosData.targets = tddp;

	    BosData.last = tddp;

	    tddp->next = (struct target_disk_data *)0;
	    continue;
	}

        /* That takes care of all of the stanza lines.  For the
         * fields to be correctly identified, line needs to be
         * bumped past any whitespace
         */
        while (isspace(*line))
                line++;


	/* Point ptr to the data in a field */
	ptr = strchr(line, '=');
	if (!ptr++) continue;
	while (isspace(*ptr))
		ptr++;

	/* determine the correct field and copy the data to it */
	if (!strncmp("CONSOLE",line,7))
	{
	    strcpy(BosData.control_flow.CONSOLE, ptr);
	    continue;
	}

	if (!strncmp(line, "INSTALL_METHOD",10))
	{
	    strncpy(BosData.control_flow.INSTALL_METHOD, ptr, 14);
	    continue;
	}

	if (!strncmp(line, "PROMPT", 6))
	{
	    strcpy(BosData.control_flow.PROMPT, ptr);
	    continue;
	}

	if (!strncmp(line, "EXISTING_SYSTEM_OVERWRITE", 25)) 
	{
	    strcpy(BosData.control_flow.EXISTING_SYSTEM_OVERWRITE, ptr);
	    continue;
	}

	if (!strncmp(line, "INSTALL_X_IF_ADAPTER", 19))
	{
	    strcpy(BosData.control_flow.INSTALL_X_IF_ADAPTER, ptr);
	    continue;
	}

	if (!strncmp(line, "RUN_STARTUP",11))
	{
	    strcpy(BosData.control_flow.RUN_STARTUP, ptr);
	    continue;
	}

	if (!strncmp(line, "RM_INST_ROOTS",12))
	{
	    strcpy(BosData.control_flow.RM_INST_ROOTS, ptr);
	    continue;
	}

	if (!strncmp(line, "ERROR_EXIT",10))
	{
	    strcpy(BosData.control_flow.ERROR_EXIT, ptr);
	    continue;
	}

	if (!strncmp(line, "CUSTOMIZATION_FILE",18))
	{
	    strcpy(BosData.control_flow.CUSTOMIZATION, ptr);
	    continue;
	}

	if (!strncmp(line, "TCB",3))
	{
	    strcpy(BosData.control_flow.TCB, ptr);
	    continue;
	}

	if (!strncmp(line, "INSTALL_TYPE",12))
	{
	    strncpy(BosData.control_flow.INSTALL_TYPE, ptr, 8);
	    continue;
	}

	if (!strncmp(line, "BUNDLES",7))
	{
	    strncpy(BosData.control_flow.BUNDLES, ptr, 79);
	    continue;
	}

	if (!strncmp(line, "BOSINST_LANG",12))
	{
	    strcpy(BosData.locale.BOSINST_LANG, ptr);
	    continue;
	}

	if (!strncmp(line, "CULTURAL_CONVENTION", 19))
	{
	    strcpy(BosData.locale.CULTURAL_CONVENTION, ptr);
	    continue;
	}

	if (!strncmp(line, "MESSAGES", 8))
	{
	    strcpy(BosData.locale.MESSAGES, ptr);
	    continue;
	}

	if (!strncmp(line, "KEYBOARD",8))
	{
	    strcpy(BosData.locale.KEYBOARD, ptr);
	    continue;
	}

	if (!strncmp(line, "HDISKNAME", 9))
	{
	    if (BosData.last)
		strcpy(BosData.last->HDISKNAME, ptr);
	    continue;
	}

	if (!strncmp(line, "LOCATION", 8))
	{
	    if (BosData.last)
		strcpy(BosData.last->LOCATION, ptr);
	    continue;
	}

	if (!strncmp(line, "SIZE_MB",7))
	{
	    if (BosData.last)
		strcpy(BosData.last->SIZE_MB, ptr);
	    continue;
	}
    }
    fclose(fp);
}

/*-----------------------------------------------------------------------------
 * write_bosinst
 *   write out the bosinst.data file
 */
write_bosinstdata()
{

    char *BosFileName;
    FILE *fp;
    struct target_disk_data *tddp = 0;	/* target disk data		*/
    struct comment *cmt;		/* save comment structure	*/

    BosFileName = getenv("BOSINSTDATA");
    if (!BosFileName)
	BosFileName = "bosinst.data";
    
    fp = fopen(BosFileName, "w");
    if (!fp) return;

    /* write out the comment lines */
    for (cmt=first_comment; cmt; cmt = cmt->next)
    {
        fprintf(fp, "%s", cmt->text);
    }

    fprintf(fp, "control_flow:\n");
    fprintf(fp, "    CONSOLE = %s\n", BosData.control_flow.CONSOLE);
    fprintf(fp, "    INSTALL_METHOD = %s\n", BosData.control_flow.INSTALL_METHOD);
    fprintf(fp, "    PROMPT = %s\n", BosData.control_flow.PROMPT);
    fprintf(fp, "    EXISTING_SYSTEM_OVERWRITE = %s\n", 
		BosData.control_flow.EXISTING_SYSTEM_OVERWRITE);
    fprintf(fp, "    INSTALL_X_IF_ADAPTER = %s\n", 
		BosData.control_flow.INSTALL_X_IF_ADAPTER);
    fprintf(fp, "    RUN_STARTUP = %s\n", BosData.control_flow.RUN_STARTUP);
    fprintf(fp, "    RM_INST_ROOTS = %s\n", BosData.control_flow.RM_INST_ROOTS);
    fprintf(fp, "    ERROR_EXIT = %s\n", BosData.control_flow.ERROR_EXIT);
    fprintf(fp, "    CUSTOMIZATION_FILE = %s\n", BosData.control_flow.CUSTOMIZATION);
    fprintf(fp, "    TCB = %s\n", BosData.control_flow.TCB);
    fprintf(fp, "    INSTALL_TYPE = %s\n", BosData.control_flow.INSTALL_TYPE);
    fprintf(fp, "    BUNDLES = %s\n", BosData.control_flow.BUNDLES);
    fprintf(fp, "\n");

    for (tddp = BosData.targets; tddp; tddp = tddp->next)
    {
	fprintf(fp, "target_disk_data:\n");
	fprintf(fp, "    LOCATION = %s\n", tddp->LOCATION);
	fprintf(fp, "    SIZE_MB = %s\n", tddp->SIZE_MB);
	fprintf(fp, "    HDISKNAME = %s\n", tddp->HDISKNAME);
	fprintf(fp, "\n");
    }

    fprintf(fp, "locale:\n");
    fprintf(fp, "    BOSINST_LANG = %s\n", BosData.locale.BOSINST_LANG);
    fprintf(fp, "    CULTURAL_CONVENTION = %s\n", BosData.locale.CULTURAL_CONVENTION);
    fprintf(fp, "    MESSAGES = %s\n", BosData.locale.MESSAGES);
    fprintf(fp, "    KEYBOARD = %s\n", BosData.locale.KEYBOARD);
    fclose(fp);
}

