static char sccsid[] = "@(#)42  1.13  src/bos/usr/lib/pios/fmtrs/piof2000.c, cmdpios, bos411, 9433B411a 8/17/94 12:51:42";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** piof2000.c - print formatter for BP2000 data stream ***/

#include <stdio.h>
#include "piostruct.h"
#include "piobe_msg.h"

/* include for getmsg routine */
#include <nl_types.h>

#define PIOMSGCAT "piobe.cat"
#define PIOMSGSET 1
void errorexit();
char *getmsg();



/*-------------------------------- Definitions --------------------------------+
|                                                                              |
|  The printer driver definition must appear after the lookup table(s)         |
|                                                                              |
+-----------------------------------------------------------------------------*/

#include "piof2000.h"



#define MAXVARSTRLEN  200       /* memory to allocate for each variable string*/

#define EOL           -2        /* end-of-line detected */

#define CMDOUT_PT(attr, pt) { piocmdout(attr, fileptr, pt, NULL); \
                              charcount += pt; }

#define CMDOUT(attr)        { piocmdout(attr, NULL   , 0 , NULL); }

#define CHARIN(x)           { x = piocharin(fileptr,NULL); \
                              if (x == EOF) return(EOF); \
                              charcount++; }

#define CHARIN_TRANS(x)     { x = piocharin(fileptr,(&Codepagename)->ptr); \
                              charcount++; }

#define UNCHARIN(c1)        { piopushin(c1); \
                              if (charcount > 1) charcount--; }

#define CHARIN_STREAM(x)     do { if(((x) = getc(fileptr)) == EOF) \
				     return(EOF); \
                                  charcount++; } while(0)

#define UNCHARIN_STREAM(c1)  do { (void)ungetc((c1),fileptr); \
                                  if(charcount > 1) charcount--; } while(0)


int pglen;
int tmarg;
int bmarg;



/*------------------------------ Work Variables ------------------------------*/

int c_indent;           /* Command line indentation in Hres units             */
int charcount;          /* number of bytes processed for the line             */
int vpos;               /* vertical position on page, in Vres units           */
int vtab_base;          /* base vpos value for vertical tab stops             */
int hpos;               /* horizontal position on page, in Hres units         */
int substchar;          /* substitute character to use when character         */
                        /* in input data stream can't be printed              */
struct shar_vars sharevars;     /* vertical spacing variables shared by this  */
                        /* formatter & the formatter driver                   */
static struct stage2_info *sel_infop; /* ptr to stage 2  info. for code page
                           currently selected by printer */
char wkbuf[1000];       /* work buffer */


/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: setup                                                                  |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

struct shar_vars *
setup(argc, argv, passthru)          /* 0 = do formatting; 1 = pass through */
    unsigned argc;
    char *argv[];
    int passthru;
{
int cnt;
PIO_CFLEVEL_DECL;


/*-----------------------------------------------------------------------------+
|                                                                              |
|   This formatter uses values which can be modified by the user such as page  |
|   length, pitch, indentation, etc.  In order to find out about those values  |
|   the piogetvals() must be called to examine the database (or, more          |
|   correctly, an area of memory which contains the database values.           |
|                                                                              |
|   Piogetopts() is a subroutine which examines the flags which were passed    |
|   to the formatter via the command line.  Only those flags specified in the  |
|   'ia' line in the database are permitted.  Entering other flags on the      |
|   command line will cause the print job to terminate.  The command line flag |
|   values overlay the values set by examination of the database.              |
|                                                                              |
+-----------------------------------------------------------------------------*/

piogetvals(attrtable, MAXVARSTRLEN, NULL);



piogetopt(argc, argv, OPTSTRING, NULL);

pglen = Pglen * Vincr;        /* Initialize Shadow Variables Not In Data Base */
tmarg = Tmarg * Vincr;
bmarg = Bmarg * Vincr;



/*--------------------------- Validate  Parameters ---------------------------*/


if (!piogetstr(PIO_CFLEVEL_ATTR, NULL, 0, NULL) ||
    ((void)piogetvals(piocfltbl,0),
     Piocflevel < PIO_CFLEVEL_NEW))
{
if (Beginpg       <= 0)
                                      errorexit(MSG_BADFORMFLAG2,"g",NULL,NULL);
if (Init_printer   < 0 || Init_printer > 2)
                                      errorexit(MSG_BADFORMFLAG1,"j",NULL,NULL);
if (Lpi           != 6 && Lpi != 8)
                                      errorexit(MSG_BADFORMFLAG6,"v",NULL,NULL);
if (Auto_crlf      < 0 || Auto_crlf > 2)
                                      errorexit(MSG_BADFORMFLAG7,"x",NULL,NULL);
if (Restoreprinter < 0 || Restoreprinter > 1)
                                      errorexit(MSG_BADFORMFLAG1,"J",NULL,NULL);
if (Wrap           < 0 || Wrap > 1)
                                      errorexit(MSG_BADFORMFLAG1,"L",NULL,NULL);
if (Pgwidth < 1)
                                      errorexit(MSG_BADFORMFLAG2,"w",NULL,NULL);
if (Indent >= Pgwidth)
                                      errorexit(MSG_BADFORMFLAG8,"i",NULL,NULL);
if (Indent < 0)
                                      errorexit(MSG_BADFORMFLAG3,"i",NULL,NULL);
if (Pitch!=10 && Pitch!=12 && Pitch!=17)
                                      errorexit(MSG_BADFORMFLAG4,"p",NULL,NULL);
}


piopgskip = Beginpg - 1;          /* Arrange For Pages To Be Skipped (if any) */

/* Get Substitute Character to be Used if Character Can't Be Printed */
substchar=(piogetstr(SUBSTCHAR, wkbuf, sizeof(wkbuf), NULL)?(int) *wkbuf:-1);

cnt  = piotab2(BASE_TABLE, &sel_infop);
cnt += piotab2(TABLE_1, &sel_infop);
cnt += piotab2(TABLE_2, &sel_infop);
cnt += piotab2(TABLE_3, &sel_infop);
cnt += piotab2(TABLE_4, &sel_infop);
cnt += piotab2(TABLE_5, &sel_infop);
cnt += piotab2(TABLE_6, &sel_infop);
cnt += piotab2(TABLE_7, &sel_infop);
cnt += piotab2(TABLE_8, &sel_infop);
cnt += piotab2(TABLE_9, &sel_infop);

if (cnt == -10) errorexit(MSG_NEED_TRANS2,NULL,NULL,NULL);


/* If Passthru Mode, That's All That Needs to be Done for Setup */
if (Passthru = passthru) return(NULL);


/*-----------------------------------------------------------------------------+
|                                                                              |
|  Initialize Pointers to Vertical Spacing Variables Shared With Formatter     |
|  Driver (except for command names, all values pointed to are in Vres units)  |
|                                                                              |
+-----------------------------------------------------------------------------*/

                                          /*+--------------------------------+*/
sharevars._pl        = &pglen;            /*|page length                     |*/
sharevars._tmarg     = &tmarg;            /*|top margin                      |*/
sharevars._bmarg     = &bmarg;            /*|bottom margin                   |*/
sharevars._vpos      = &vpos;             /*|vertical position of next line  |*/
sharevars._vtab_base = &vtab_base;        /*|base for vertical tab stops     |*/
sharevars._vincr     = &Vincr;            /*|vertical increment of Vincr cmd |*/
sharevars._vincr_cmd = (&Vincr_cmd)->ptr; /*|name of vertical increment cmd  |*/
sharevars._ff_cmd    = (&Ff_cmd)->ptr;    /*|name of form feed command       |*/
sharevars._ff_at_eof = &Do_formfeed;      /*|address of ff flag              |*/
sharevars._set_cmd   = SWITCHMODE_CMD;    /*|name of command to switch modes |*/
                                          /*+--------------------------------+*/

c_indent = Indent * Hincr;  /* Command line indentation in Hres units         */



return(&sharevars);
}


/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: initialize                                                             |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)                                                                     |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

initialize()
{
/*
FILE *fileptr;
int c1;
int still_sending = TRUE;
*/

/* Output the Command String to Initialize the Printer */
if (Init_printer)
    {
    CMDOUT(INIT_CMD);
    CMDOUT(sel_infop->cmdp);     /* Activate the currently required code page */
    }
}


/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: lineout                                                                |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)                                                                     |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

lineout(fileptr)
FILE *fileptr; {
                                   /*+---------------------------------------+*/
int value, cnt, tabval;            /*|work variables                         |*/
int c1;                            /*|character from input data stream       |*/
char wkstr1[20], wkstr2[20];       /*|work strings                           |*/
int doingsubchar;                  /*|TRUE if processing a substitute char.  |*/
struct stage2_info *cur_infop;     /*|ptr to stage 2 info. for code          |*/
                                   /*|page currently being looked at         |*/
struct transtab *tabentp;          /*|ptr to stage 2 table entry             |*/
int c2;                            /*|code point for printer code page       |*/
int done;                          /*|are we done with the input char?       |*/
int first_of_multi;                /*|this is the first of multiple code page|*/
                                   /*|to be searched, starting at sel_infop  |*/
char *tabs;                        /*|pointer to array of tab stops          |*/
                                   /*+---------------------------------------+*/

/* Perform Indentation (before turning on underscore) */

if ( hpos = Hpos_from_prev ) Hpos_from_prev = 0;
else hpos = c_indent;

if (Workint1 = hpos) CMDOUT(HORZ_SPACE_CMD);

/*-----------------------------------------------------------------------------+
|                                                                              |
|    Loop on Input Characters Until End of Line (or End of File) Reached       |
|                                                                              |
+-----------------------------------------------------------------------------*/

for (charcount = 0;;)
    {
    CHARIN_TRANS(c1);
    if (c1 == EOF) goto ENDLINE;
    if (c1 > 31) goto GRAPHIC;  /* If a Graphic Character, Jump Past switch() */


/*-----------------------------------------------------------------------------+
|                                                                              |
|                              Control Character                               |
|                                                                              |
+-----------------------------------------------------------------------------*/

switch (c1)
{
case 9:                              /* '09' - Horizontal Tab */
    tabs = (&Horz_tabs)->ptr;       /* array of tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++) {
	    tabval = (tabs[cnt] - 1) * Hincr + c_indent;
	    if (tabval > hpos) break;
    }
    if (tabs[cnt] != 0) {          /* if found a tab stop */
	    if (tabval > Width) hpos = Width; /*tab to EOL*/
	    else {
		    while(hpos < tabval) {
			    pioputchar(' ');
			    hpos += Hincr;
		    }
	    }
    }
    break;

case 10:            /*** '0A' - Line Feed ***/
    vpos += Vincr * Linespacing;
    if (Auto_crlf != 2)
        {
        CHARIN_STREAM(value);
        UNCHARIN_STREAM(value);
        if (value != '\r') Hpos_from_prev = hpos;
        }
    goto ENDLINE;

case 12:                                                  /* '0C' - Form Feed */
    vpos = pglen;
    goto ENDLINE;

case 13:                                            /* '0D' - Carriage Return */
    if (Auto_crlf == 1)
        {
        vpos += Vincr * Linespacing;
        goto ENDLINE;
        }
    CMDOUT(CR_CMD);
    hpos = c_indent;
    if ( Workint1 = hpos ) CMDOUT(HORZ_SPACE_CMD);
    break;

default:                                              /* Ignore the Character */
    break;
}

continue;                                             /* on to next character */


/*-----------------------------------------------------------------------------+
|                                                                              |
|                               Graphic Character                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

GRAPHIC:

                                        /*+----------------------------------+*/
    if ((hpos += Hincr) > Width)        /*|If page width would be exceeded by|*/
        {                               /*|printing the next char then - if  |*/
        if (Wrap)                       /*|WRAP is on, stuff the char back   |*/
            {                           /*|into the buffer, increment VPOS,  |*/
            UNCHARIN(c1);               /*|and go to the next line; ELSE act |*/
            vpos += Vincr * Linespacing;/*|like the char was printed by      |*/
            goto ENDLINE;               /*|jumping down to FINISHED which    |*/
            }                           /*|closes the braces and exits.      |*/
        else goto FINISHED;             /*+----------------------------------+*/
        }


/*-----------------------------------------------------------------------------+
|                                                                              |
|  If the Codepage name is a null string then the user is saying that no       |
|  translation is to be done.  piotrans will return the character picked       |
|  off the datastream untranslated.  This routine will be skipped altogether.  |
|                                                                              |
+-----------------------------------------------------------------------------*/

if (!(&Codepagename)->len) {pioputchar(c1);}      /* No codepage translation  */
else {                                            /* called for - output 'c1' */

    doingsubchar = FALSE;    /* Get Ready to Run Circular Chain of Code Pages */

STARTAGAIN:

    done = FALSE;
    if (sel_infop->infop != sel_infop)       /* if muliple code pages chained */
        first_of_multi = TRUE;               /* remember: this is the first   */
    else
        first_of_multi = FALSE;


/*-----------------------------------------------------------------------------+
|                                                                              |
|    Start at the Code Page Currently Selected By the Printer and Run the      |
|    Circular Chain of Code Pages Looking For One That Has the Character       |
|                                                                              |
+-----------------------------------------------------------------------------*/


    for (cur_infop = sel_infop;
         done == FALSE;
         cur_infop = cur_infop->infop, first_of_multi = FALSE)
        {
        if (c1 < cur_infop->num_tabentries)
            {
            tabentp = cur_infop->tabp + c1;
            c2 = tabentp->outchar;
            }
        else
            {
            tabentp = NULL;
            c2 = SC;          /* can't print the character, so use substitute */
            }

        if (c2 < 0)
            {
            switch (c2)               /* special code instead of a code point */
                {
                case CP:                               /* Copy the Code Point */
                    c2 = c1;
                    break;

                case SC:                              /* Substitute Character */

/*-----------------------------------------------------------------------------+
|                                                                              |
|          Character not in this printer code page, so see if ready            |
|                 to print the substitute character instead                    |
|                                                                              |
+-----------------------------------------------------------------------------*/

                    if (cur_infop == sel_infop && !first_of_multi)
                        {

/*-----------------------------------------------------------------------------+
|                                                                              |
|              Have Been Through All the Printer Code Pages and                |
|              Couldn't Find the Character in Any of Them, So                  |
|              Print Substitute Character Instead                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

                        if (!doingsubchar)
                            {
                            if (substchar < 0) break;
                            c1 = substchar;
                            doingsubchar = TRUE;
                            goto STARTAGAIN;
                            }
                        else errorexit(MSG_SUBST_CHAR,NULL,NULL,NULL);
                        }        /* Can't Print Subst Char Either, So Give Up */
                    else continue;
                    break;

                default:
                    (void) sprintf(wkstr1, "%d", c1);
                    (void) sprintf(wkstr2, "%d", c2);
                    errorexit(MSG_CODEPOINT2,cur_infop->filename,
                                                            wkstr1,wkstr2,NULL);
                    break;
                }
            }

    /* If Needed, Tell the Printer to Select the Code Page */
            if (cur_infop != sel_infop)
                {
                CMDOUT(cur_infop->cmdp);
                sel_infop = cur_infop;
                }

    /* Output the Character and the Optional Prefix and/or Suffix Commands*/
            if (tabentp && (int)tabentp->index1 > 0)
                CMDOUT(cur_infop->cmdp+tabentp->index1);
            pioputchar(c2);

            if (tabentp && (int)tabentp->index2 > 0)
                CMDOUT(cur_infop->cmdp+tabentp->index2);

            done = TRUE;
        }  /* end of 'for' loop */
FINISHED:;
    }
}

ENDLINE:

return(charcount);

}



/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME: passthru                                                               |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)                                                                     |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
|                                                                              |
+-----------------------------------------------------------------------------*/

passthru()
{
int c1;


while ((c1 = piogetchar()) != EOF)
    pioputchar(c1);

if (piodatasent && Do_formfeed) piocmdout(FF_CMD, NULL, 0, NULL);


return(0);
}



/*-----------------------------------------------------------------------------+
|                                                                              |
| NAME:      restore                                                           |
|                                                                              |
| FUNCTION:  Restores the printer to the condition specified in the database   |
|            as being the default condition.                                   |
|                                                                              |
| (NOTES:)   (Restoreprinter) will be the default (or command line) value      |
|            only when this routine is called for the last time for a print    |
|            job.  Otherwise, it will be zero and the printer will not be      |
|            restored.                                                         |
|                                                                              |
| RETURNS:   nada                                                              |
|                                                                              |
+-----------------------------------------------------------------------------*/

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
