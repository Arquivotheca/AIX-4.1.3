static char sccsid[] = "@(#)60  1.9  src/bos/usr/lib/pios/fmtrs/piofpt.c, cmdpios, bos411, 9428A410j 11/9/93 11:02:54";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: setup, initialize, lineout, passthru, restore, errorexit, escseq
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

/*** piofpt.c - print formatter for pass-through data stream ***/

#include <stdio.h>
#include "piostruct.h"
#include "piobe_msg.h"

/* include for getmsg routine */
#include <nl_types.h>
char *getmsg();

#define PIOMSGCAT "piobe.cat"
#define PIOMSGSET 1
extern int errno;
void errorexit();

/********************************* Definitions *********************************
*                                                                              *
*  The printer driver definition must appear after the lookup table(s)         *
*                                                                              *
*******************************************************************************/

#include "piofpt.h"

#define CMDOUT(attr)            {piocmdout(attr, NULL   , 0 , NULL);}


/******************************* Work Variables *******************************/

char wkbuf[1000];       /* work buffer */


/*******************************************************************************
*                                                                              *
* NAME: setup                                                                  *
*                                                                              *
* FUNCTION:                                                                    *
*                                                                              *
* EXECUTION ENVIRONMENT:                                                       *
*                                                                              *
* (NOTES:)  make sure width is greater than some reasonable char width         *
*                                                                              *
* (RECOVERY OPERATION:)                                                        *
*                                                                              *
* (DATA STRUCTURES:)                                                           *
*                                                                              *
* RETURNS:                                                                     *
*                                                                              *
*******************************************************************************/

struct shar_vars *
setup(argc, argv, passthru)          /* 0 = do formatting; 1 = pass through */
    unsigned argc;
    char *argv[];
    int passthru;
{
PIO_CFLEVEL_DECL;

piogetvals(attrtable, NULL); /* Tell Formatter About DBase Vbles*/
piogetopt(argc, argv, OPTSTRING, NULL);    /* Process Command Line Flags */


/*--------------------------- Validate  Parameters ---------------------------*/


if (!piogetstr(PIO_CFLEVEL_ATTR, NULL, 0, NULL) ||
    ((void)piogetvals(piocfltbl,0),
     Piocflevel < PIO_CFLEVEL_NEW))
{
if (Init_printer   < 0   || Init_printer > 2)
                                      errorexit(MSG_BADFORMFLAG1,"j",NULL,NULL);
if (Restoreprinter < 0   || Restoreprinter > 1)
                                      errorexit(MSG_BADFORMFLAG1,"J",NULL,NULL);
}



/* Passthru Mode, So That's All That Needs to be Done for Setup */
return(NULL);

}



/*******************************************************************************
*                                                                              *
* NAME: initialize                                                             *
*                                                                              *
* FUNCTION:                                                                    *
*                                                                              *
* EXECUTION ENVIRONMENT:                                                       *
*                                                                              *
* (NOTES:)                                                                     *
*                                                                              *
* (RECOVERY OPERATION:)                                                        *
*                                                                              *
* (DATA STRUCTURES:)                                                           *
*                                                                              *
* RETURNS:                                                                     *
*                                                                              *
*                                                                              *
*******************************************************************************/

initialize()
{
FILE *fontfileptr;
int val;
char wkstr[20];

/* Is the printer to be initialized */
if (!Init_printer) return(0);

/* Output the Command String to Initialize the Printer */
CMDOUT(INIT_CMD);


/* Download Font (if any) */
piogetstr(DOWNLOAD_FONT, wkbuf, sizeof(wkbuf), NULL); /* put name into wkbuf */
if (*wkbuf != NULL)
    {
    if ((fontfileptr = fopen(wkbuf, "r")) == NULL)
        {
        (void) sprintf(wkstr,"%d",errno);
        errorexit(MSG_OPEN_FONTS,wkbuf,wkstr,NULL);
        }
    while ((val = piogetc(fontfileptr)) != EOF) pioputchar(val);
    }


return(0);
}


/*******************************************************************************
*                                                                              *
* NAME: lineout (not used)                                                     *
*                                                                              *
* FUNCTION:                                                                    *
*                                                                              *
* EXECUTION ENVIRONMENT:                                                       *
*                                                                              *
* (NOTES:)                                                                     *
*                                                                              *
* (RECOVERY OPERATION:)                                                        *
*                                                                              *
* (DATA STRUCTURES:)                                                           *
*                                                                              *
* RETURNS:                                                                     *
*                                                                              *
*******************************************************************************/

lineout(fileptr)
FILE *fileptr; {

errorexit(MSG_LNOUT_PSTHRU,NULL,NULL,NULL);

return(0);

}



/*******************************************************************************
*                                                                              *
* NAME: passthru                                                               *
*                                                                              *
* FUNCTION:                                                                    *
*                                                                              *
* EXECUTION ENVIRONMENT:                                                       *
*                                                                              *
* (NOTES:)                                                                     *
*                                                                              *
* (RECOVERY OPERATION:)                                                        *
*                                                                              *
* (DATA STRUCTURES:)                                                           *
*                                                                              *
* RETURNS:                                                                     *
*                                                                              *
*******************************************************************************/

passthru()
{
int c1;

while ((c1 = piogetchar()) != EOF)
    pioputchar(c1);

if (piodatasent && Do_formfeed) piocmdout(FF_CMD, NULL, 0, NULL);

return(0);
}



/*******************************************************************************
*                                                                              *
* NAME: restore                                                                *
*                                                                              *
* FUNCTION:                                                                    *
*                                                                              *
* EXECUTION ENVIRONMENT:                                                       *
*                                                                              *
* (NOTES:)   (Restoreprinter) will be the default (or command line) value      *
*            only when this routine is called for the last time for a print    *
*            job.  Otherwise, it will be zero and the printer will not be      *
*            restored.                                                         *
*                                                                              *
* (RECOVERY OPERATION:)                                                        *
*                                                                              *
* (DATA STRUCTURES:)                                                           *
*                                                                              *
* RETURNS:                                                                     *
*                                                                              *
*******************************************************************************/

restore()
{
/* Output the Command String to Restore the Printer */
if (Restoreprinter) CMDOUT(REST_CMD);

return(0);
}




/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     errorexit                                                         |
|                                                                             |
| FUNCTION: produces an error message from the referenced message catalog     |
|           and exits                                                         |
|                                                                             |
| EXECUTION ENVIRONMENT:                                                      |
|                                                                             |
| (NOTES:)                                                                    |
|                                                                             |
| (RECOVERY OPERATION:)                                                       |
|                                                                             |
| (DATA STRUCTURES:)                                                          |
|                                                                             |
| RETURNS:  nada                                                              |
|                                                                             |
+----------------------------------------------------------------------------*/

void errorexit(msgnum, stringval1, stringval2, stringval3)
int msgnum;
char *stringval1, *stringval2, *stringval3;
{
char msgbuf[1000];

(void) sprintf(
        msgbuf, getmsg(PIOMSGCAT, PIOMSGSET, msgnum),
        stringval1, stringval2, stringval3);
(void) piomsgout(msgbuf);
if (piodatasent && Do_formfeed) piocmdout(FF_CMD, NULL, 0, NULL);
pioexit(PIOEXITBAD);
}


/*
*******************************************************************************
*******************************************************************************
** NAME:        getmsg()
**
** DESCRIPTION: Replaces the NLgetamsg routine used in 3.1 code  If the catalog
**              is not found in the NLSPATH, it will look for a default in
**              /usr/lib/lpd/pio/etc.
**
** ROUTINES
**   CALLED:    catopen() - gets catalog descriptor
**
**              catgets() - gets message
**              catclose  - closes catalog
**
** PARAMETERS:  catalog name, set number, message number
**
**
*******************************************************************************
*******************************************************************************
*/
static char *msgbuf = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

char *getmsg(CatName, set, num)
char *CatName;  
int set;
int num;
{
	char *ptr;
	char *nlspath;
	char *defpath = malloc(200);
	char default_msg[100];

	if (strcmp(CatName,save_name) != 0)  /* is it a different catalog */
	{
	catclose(catd);  /* close /usr/lpp message catalog */
	catclose(def_catd);  /* close default message catalog */
	catd = def_catd = (nl_catd)NULL;  /* set catalog descriptors to NULL */
	strcpy(save_name,CatName);
	}

	if (catd != -1)  
		if (catd == 0) /* if it hasn't been open before */
			catd = catopen(CatName,NL_CAT_LOCALE);
	
	if (catd != -1)
		{
		ptr = catgets(catd,set,num,"dummy");
		if (!msgbuf)
			msgbuf = malloc(4001);
		if (msgbuf)
			strncpy(msgbuf, ptr, 4000);
		if (strcmp(ptr,"dummy") != 0)  /* did catgets fail? */
			return(msgbuf);
		}
	
	if (def_catd == 0)
		{
		sprintf(defpath,"/usr/lib/lpd/pio/etc/%s",CatName);
		def_catd = catopen(defpath,NL_CAT_LOCALE);
		}
	
	sprintf(default_msg,"Cannot access message catalog %s.\n",CatName);
	ptr = catgets(def_catd,set,num, default_msg);
	if (!msgbuf)
		msgbuf = malloc(4001);
	if (msgbuf)
		strncpy(msgbuf, ptr, 4000);

	free(defpath);
	return(msgbuf);
}
