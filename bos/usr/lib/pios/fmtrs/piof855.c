static char sccsid[] = "@(#)54  1.9.1.3  src/bos/usr/lib/pios/fmtrs/piof855.c, cmdpios, bos411, 9433B411a 8/17/94 12:52:30";
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

/*** piof855.c - print formatter for TI 855 data stream (DP mode only) ***/

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



/*-------------------------------- Definitions --------------------------------+
|                                                                              |
|  The printer driver definition must appear after the lookup table(s)         |
|                                                                              |
+-----------------------------------------------------------------------------*/

#include "piof855.h"



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




/*----------------------------- Shadow Variables ------------------------------+
|                                                                              |
|  Shadow variables are "shadows" of data base variables.  Their meaning is    |
|  the same as the corresponding data base variables, but their values are     |
|  in different units (e.g., Vres units instead of lines).  Whenever a shadow  |
|  variable is updated, the corresponding data base variable (and perhaps other|
|  variables) must be updated also.  Macros are defined below for updating the |
|  variables.                                                                  |
|                                                                              |
|  Shadow variables such as "pglen", "tmarg", and "bmarg" that are the same for|
|  all values of "piomode" (see "piostruct.h") are defined below.  Shadow      |
|  variables such as "Hincr", "Vincr", and "Width"  that may have different    |
|  values depending on the current value of "piomode" are defined as work      |
|  variables in the data base.                                                 |
|                                                                              |
+-----------------------------------------------------------------------------*/

int pglen;
#define SET_Pglen(lines)        {Pglen = lines;  \
                                 pglen = lines * Vincr;}
int tmarg;
#define SET_Tmarg(lines)        {Tmarg = lines;  \
                                 tmarg = lines * Vincr;}
int bmarg;
#define SET_Bmarg(lines)        {Bmarg = lines;  \
                                 bmarg = lines * Vincr;}

#define SET_Lpi(lines)          {Lpi = lines;  \
                                 Vincr = Vdecr = Vres / lines;}

#define SET_Pgwidth(chars)      {Width = (chars) * Hres / Pitch + c_indent; \
                                 Pgwidth = Width / Hincr + 1; }

#define SET_Pitch(cpi)  {Pitch = cpi;   \
                         Hincr = (10 * Hres) /  \
                        (Condensed ? 150 : (10 * Pitch));  \
                         Hincr += Hincr * Line_dblwide; }



/*------------------------------ Work Variables ------------------------------*/

int c_indent;           /* Command line indentation in Hres units             */
int x_indent;           /* Indentation established by data stream - ESC 9     */
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
if (Paper_source < 1 || Paper_source > 3)
                                      errorexit(MSG_BADFORMFLAG9,"u",NULL,NULL);
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

x_indent = 0;
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
FILE *fileptr;
int c1;
char wkstr[20];

if (!Init_printer) return(0);             /* Is the printer to be initialized */

CMDOUT(INIT_CMD);      /* Output the Command String to Initialize the Printer */

/* Download Font (if any)      put name into wkbuf */
piogetstr(DOWNLOAD_FONT, wkbuf, sizeof(wkbuf), NULL);
if (*wkbuf != NULL)
    {
    if ((fileptr = fopen(wkbuf, "r")) == NULL)
        {
        (void) sprintf(wkstr,"%d",errno);
        errorexit(MSG_OPEN_FONTS,wkbuf,wkstr,NULL);
        }
    while ((c1 = piogetc(fileptr)) != EOF)
    {
       pioputchar(c1);
       charcount++;
    }
    }

CMDOUT(sel_infop->cmdp);         /* Activate the currently required code page */

return(0);
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
int value,tabval,cnt;              /*|work variables                         |*/
int c1;                            /*|character from input data stream       |*/
char *tabs;                        /*|pointer to array of tab stops          |*/
char wkstr1[20], wkstr2[20];       /*|work strings                           |*/
int doingsubchar;                  /*|TRUE if processing a substitute char.  |*/
struct stage2_info *cur_infop;     /*|ptr to stage 2 info. for code          |*/
                                   /*|page currently being looked at         |*/
struct transtab *tabentp;          /*|ptr to stage 2 table entry             |*/
int c2;                            /*|code point for printer code page       |*/
int done;                          /*|are we done with the input char?       |*/
int first_of_multi;                /*|this is the first of multiple code page|*/
                                   /*|to be searched, starting at sel_infop  |*/
                                   /*+---------------------------------------+*/

/* Perform Indentation (before turning on underscore) */

if ( hpos = Hpos_from_prev ) Hpos_from_prev = 0;
else hpos = c_indent + x_indent;

if (Line_dblwide)
    {
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    }

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
case 10:            /*** '0A' - Line Feed ***      most used, so put it first */
LINEFEED:
    vpos += Vincr * Linespacing;
    if (Auto_crlf != 2)
        {
        CHARIN_STREAM(value);
        UNCHARIN_STREAM(value);
        if (value != '\r') Hpos_from_prev = hpos;
        }
    goto ENDLINE;

case 0:                                                         /* '00'  Null */
    CMDOUT(NULL_CMD);
    break;

case 1:
case 2:
case 3:
case 5:
case 6:
case 16:
case 17:
case 19:
case 21:
case 22:
case 23:
case 24:
case 25:
case 26:
case 28:
case 30:
case 31:
    pioputchar(c1);
    break;

case 4: goto ENDLINE;      /* special character acts as end of job terminator */

case 7:                                              /* '07' - Sound the Bell */
    CMDOUT(BEL_CMD);
    break;

case 8:                                                   /* '08' - Backspace */
    if (hpos <= c_indent) break;
    hpos -= Hincr;
    CMDOUT(BS_CMD);
    break;

case 9:                                              /* '09' - Horizontal Tab */
    tabs = (&Horz_tabs)->ptr;             /* array of tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++)
        {
        tabval = (tabs[cnt] - 1) * Hincr + x_indent + c_indent;
        if (tabval > hpos) break;
        }
    if (tabs[cnt] != 0)                   /* if found a tab stop */
        {
        if (tabval > Width) hpos = Width; /* tab to EOL */
        else
            {
            while(hpos < tabval)
                {
                pioputchar(' ');
                hpos += Hincr;
                }
            }
        }
    break;

case 11:                                               /* '0B' - Vertical Tab */
    tabs = (&Vert_tabs)->ptr;             /* array of tab stops */
    value = vpos - vtab_base;             /* base vpos for tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++)
        {
        tabval = (tabs[cnt] - 1) * Vincr;
        if (tabval > value) break;
        }
    if (tabs[cnt] != 0) vpos = tabval + vtab_base;     /* if found a tab stop */
    if (Auto_crlf != 2) Hpos_from_prev = hpos;
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
    hpos = c_indent + x_indent;
    if ( Workint1 = hpos ) CMDOUT(HORZ_SPACE_CMD);
    break;

case 14:                                    /* '0E' - Start Expanded Printing */
    Line_dblwide = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SO_CMD);
    break;

case 15:                               /* '0F' - SI: Start Condensed Printing */
    Condensed = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SET_COND_CMD);
    break;

case 18:                               /* '12' - DC2: Stop Condensed Printing */
    Condensed = FALSE;
    SET_Pitch(Pitch);
    CMDOUT(SET_PITCH_CMD);
    break;

case 20:                   /* '14' - DC4: Cancel Double-Wide Printing by Line */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    CMDOUT(DC4_CMD);
    break;

case 27:                                                     /* '1B' - Escape */
    switch (escseq(fileptr))
        {
        case 0:       break;                    /* normal return */
        case EOL:     goto LINEFEED;            /* end of line */
        case EOF:     goto ENDLINE;             /* end of input file */
        }
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




/*----------------------------------------------------------------------------+
|                                                                             |
| NAME:     escseq                                                            |
|                                                                             |
| FUNCTION: handles escape sequences appearing in the data stream             |
|                                                                             |
| RETURNS:  nada                                                              |
|                                                                             |
+----------------------------------------------------------------------------*/

escseq(fileptr)
FILE *fileptr; {

int cnt, n1, n2;                                            /* work variables */

CHARIN(Workint1);
switch ((char)Workint1)
{
/*----------------------------------------------------------------------------*/

case 'y':                                                    /* Select 10 cpi */
case 'z':                                                 /* Select 12/15 cpi */
case 'P':                                                 /* Start compressed */
case 'Q':                                                  /* Stop compressed */
case 'S':                                                   /* Start expanded */
case 'T':                                                    /* Stop expanded */

case '2':                                                     /* Select 6 lpi */
case '0':                                                     /* Select 8 lpi */
case '1':                                                  /* Select 10.7 lpi */
case 'v':                                                   /* Half line feed */
case '^':                                           /* Reverse half line feed */

case 'G':                                                       /* Start bold */
case 'H':                                                        /* Stop bold */
case 'E':                                                     /* Start shadow */
case 'F':                                                      /* Stop shadow */
case '>':                                                       /* Set MSB on */
case '=':                                                      /* Set MSB off */

case '@':                                                   /* Software reset */

    CMDOUT(TWO_BYTE_CMD);
    break;

case 'A':                                                          /* Set VMI */
case '3':                                                          /* Set VMI */
case 'C':                                  /* Set top of form to current line */
    CHARIN(Workint2);
    CMDOUT(THREE_BYTE_CMD);
    break;

case 'D':                                                  /* Horizontal tabs */
case 'B':                                                    /* Vertical tabs */
    do {CHARIN(Workint2);}                /* Read buffer until 'NUL' received */
    while (Workint2);                     /* to waste tab info                */
    break;

case 'K':                                   /* Raster Graphics -  60 x 72 dpi */
case 'N':                                   /* Raster Graphics -  72 x 72 dpi */
case 'J':                                   /* Raster Graphics - 120 x 72 dpi */
case 'L':                                   /* Raster Graphics - 120 x 72 dpi */
case 'O':                                   /* Raster Graphics - 144 x 72 dpi */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + n2 * 256;
    CMDOUT_PT(GRAPHICS,cnt);
    break;

default:
    break;
}
return(0);
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
