static char sccsid[] = "@(#)48  1.8.1.3  src/bos/usr/lib/pios/fmtrs/piof42x4.c, cmdpios, bos411, 9433B411a 8/17/94 12:51:59";
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

/*** piof42x4.c - print formatter for IBM 4224 and 4234 data streams ***/

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




/*----------------------------- Lookup Table(s) -------------------------------+
|                                                                              |
|    This table permits the entry of a color name on the command line.  That   |
|    color name is then translated to an integer via this table.  The first    |
|    nine entries in the table are blank.  This is where the national langu-   |
|    age color names will be loaded.  They will be found in an external        |
|    message table which will be translated for each supported language.       |
|                                                                              |
|    Note: The procedure that uses this lookup table is not case sensitive.    |
|                                                                              |
+-----------------------------------------------------------------------------*/


char defclr[] = "dummy";
struct lktable colors[] =
    {
    ""        ,0   ,           /* \                                   */
    ""        ,1   ,           /*  |                                  */
    ""        ,2   ,           /*  |                                  */
    ""        ,3   ,           /*   \ _  leave room for the national  */
    ""        ,4   ,           /*   /    language color words to be   */
    ""        ,5   ,           /*  |     loaded into the lookup table */
    ""        ,6   ,           /*  |                                  */
    ""        ,8   ,           /*  |                                  */
    ""        ,10  ,           /* /                                   */

    "0"       ,0   , "00"       ,0   , "black"    ,0   ,
    "1"       ,1   , "01"       ,1   , "blue"     ,1   ,
    "2"       ,2   , "02"       ,2   , "red"      ,2   ,
    "3"       ,3   , "03"       ,3   , "magenta"  ,3   , "pink"       ,3  ,
    "4"       ,4   , "04"       ,4   , "green"    ,4   ,
    "5"       ,5   , "05"       ,5   , "cyan"     ,5   , "turquoise"  ,5  ,
    "6"       ,6   , "06"       ,6   , "yellow"   ,6   ,
    "8"       ,8   , "08"       ,8   ,
    "10"      ,10  , "brown"    ,10  , ""         ,0          /* end of table */
    };

/*-------------------------------- Definitions --------------------------------+
|                                                                              |
|  The printer driver definition must appear after the lookup table(s)         |
|                                                                              |
+-----------------------------------------------------------------------------*/

#include "piof42x4.h"


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
                         Hincr += Hincr * (Cont_dblwide || Line_dblwide);  }


/*------------------------------ Work Variables ------------------------------*/

int c_indent;           /* Command line indentation in Hres units             */
int x_indent;           /* Indentation called for by Esc X cmd in Hres units  */
int charcount;          /* number of bytes processed for the line             */
int vpos;               /* vertical position on page, in Vres units           */
int vtab_base;          /* base vpos value for vertical tab stops             */
int hpos;               /* horizontal position on page, in Hres units         */
int substchar;          /* substitute character to use when character         */
                        /* in input data stream can't be printed              */
char Vincr_cmd_norm[3]; /* the Vincr_cmd can be changed in ESC J, but the     */
                        /* orig cmd must be recorded to be retrieved later    */
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
| (NOTES:)  make sure width is greater than some reasonable char width         |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
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

char clrbuf[200];
char *cp;
PIO_CFLEVEL_DECL;

strncpy(clrbuf,getmsg(MF_PIOBE,1,MSG_4224COLORS),sizeof(clrbuf)-1);

if (!strcmp(clrbuf,defclr)) errorexit (MSG_BAD4224COLOR,"k",NULL,NULL);

cp = clrbuf;
for(cnt = 0 ; cnt <= 8 ; cnt++)
    {
    while(*cp && *cp != ':') cp++;
    if (!(*cp)) break;
    *cp++ = '\0';
    if (cnt == 8) break;
    colors[cnt].str = cp;
    }

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

SET_Pglen(Pglen);             /* Initialize Shadow Variables Not In Data Base */
SET_Tmarg(Tmarg);
SET_Bmarg(Bmarg);



/*--------------------------- Validate  Parameters ---------------------------*/

if (!piogetstr(PIO_CFLEVEL_ATTR, NULL, 0, NULL) ||
    ((void)piogetvals(piocfltbl,0),
     Piocflevel < PIO_CFLEVEL_NEW))
{
if (Emphasized     < 0 || Emphasized > 1)
                                      errorexit(MSG_BADFORMFLAG1,"e",NULL,NULL);
if (Beginpg       <= 0)
                                      errorexit(MSG_BADFORMFLAG2,"g",NULL,NULL);
if (Init_printer   < 0 || Init_printer > 2)
                                      errorexit(MSG_BADFORMFLAG1,"j",NULL,NULL);
if (Quality        < 0 || Quality > 3)
                                      errorexit(MSG_BADFORMFLAG5,"q",NULL,NULL);
if (Lpi           != 6 && Lpi != 8)
                                      errorexit(MSG_BADFORMFLAG6,"v",NULL,NULL);
if (Auto_crlf      < 0 || Auto_crlf > 2)
                                      errorexit(MSG_BADFORMFLAG7,"x",NULL,NULL);
if (Doublestrike   < 0 || Doublestrike > 1)
                                      errorexit(MSG_BADFORMFLAG1,"y",NULL,NULL);
if (Restoreprinter < 0 || Restoreprinter > 1)
                                      errorexit(MSG_BADFORMFLAG1,"J",NULL,NULL);
if (Condensed      < 0 || Condensed > 1)
                                      errorexit(MSG_BADFORMFLAG1,"K",NULL,NULL);
if (Wrap           < 0 || Wrap > 1)
                                      errorexit(MSG_BADFORMFLAG1,"L",NULL,NULL);
if (Cont_dblwide   < 0 || Cont_dblwide > 1)
                                      errorexit(MSG_BADFORMFLAG1,"W",NULL,NULL);

if (Pgwidth < 1)
                                      errorexit(MSG_BADFORMFLAG2,"w",NULL,NULL);
if (Indent >= Pgwidth)
                                      errorexit(MSG_BADFORMFLAG8,"i",NULL,NULL);
if (Indent < 0)
                                      errorexit(MSG_BADFORMFLAG3,"i",NULL,NULL);

if (Color < 0)
                                      errorexit(MSG_BAD4224COLOR,"k",NULL,NULL);
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
sharevars._vincr_cmd = Vincr_cmd_norm;    /*|address of vert inc cmd         |*/
sharevars._vdecr     = &Vdecr;            /*|vertical decrement of Vdecr cmd |*/
sharevars._vdecr_cmd = (&Vdecr_cmd)->ptr; /*|name of vertical decrement cmd  |*/
sharevars._ff_cmd    = (&Ff_cmd)->ptr;    /*|name of form feed command       |*/
sharevars._ff_at_eof = &Do_formfeed;      /*|address of ff flag              |*/
sharevars._set_cmd   = SWITCHMODE_CMD;    /*|name of command to switch modes |*/
                                          /*+--------------------------------+*/

strcpy(Vincr_cmd_norm,(&Vincr_cmd)->ptr); /* record normal Vincr_cmd          */

x_indent = 0;               /* Indentation set by ESC X command in Hres units */
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
int value,tabval,cnt,w1,w2,w3,w4;  /*|work variables                         |*/
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
/*-----------------------------------------------------------------------------+
|                                                                              |
|         Update Variables Shared With the Formatter Driver That May           |
|         Have Different Values Depending on the Value of "piomode             |
|                                                                              |
+-----------------------------------------------------------------------------*/

sharevars._vincr = &Vincr;
sharevars._vdecr = &Vdecr;
sharevars._vdecr_cmd = (&Vdecr_cmd)->ptr;
strcpy(Vincr_cmd_norm,(&Vincr_cmd)->ptr);    /* retrieve normal Vincr_cmd */

if (Line_dblwide)           /* Line_dblwide mode is turned off at the end */
    {                       /* of every line.  Hincr, etc. must be adjusted. */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    }

/* Perform Indentation (before turning on overscore or underscore) */
if ( hpos = Hpos_from_prev ) Hpos_from_prev = 0;
else hpos = x_indent + c_indent;

if (Workint1 = hpos) CMDOUT(HORZ_SPACE_CMD);
if (Cont_overscore)  CMDOUT(OVERSCORE_CMD); /* If Overscore and/or Underscore */
if (Cont_undrscore)  CMDOUT(UNDERSCORE_CMD); /* should be on then do it */

if (Ovrstrike_mode)  CMDOUT(OVERSTRIKE_CMD);
if (Undrscore_mode)  CMDOUT(UNDRSCORE_CMD);

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
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
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

case 3:               /* heart   */
case 4:               /* diamond */
case 5:               /* club    */
case 6:               /* spade   */
case 21:
    pioputchar(c1);
    break;

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
            {                            /*+---------------------------------+*/
            if (Cont_overscore)          /*|A tab stop has been found.       |*/
                {                        /*|                                 |*/
                w1 = TRUE;               /*|The printer manuals say that     |*/
                Cont_overscore = FALSE;  /*|whitespace produced by a tab     |*/
                CMDOUT(OVERSCORE_CMD);   /*|won't be under/over scored if the|*/
                }                        /*|attribute is on.                 |*/
            else w1 = FALSE;             /*|                                 |*/
                                         /*|That means that just putting out |*/
            if (Cont_undrscore)          /*|spaces (which would be scored)   |*/
                {                        /*|would make the printer behave    |*/
                w2 = TRUE;               /*|incorrectly, therefore if either |*/
                Cont_undrscore = FALSE;  /*|attribute is on, record that     |*/
                CMDOUT(UNDERSCORE_CMD);  /*|fact, turn the attribute off     |*/
                }                        /*+---------------------------------+*/
            else w2 = FALSE;

            if (Ovrstrike_mode)
                {
                w3 = TRUE;
                Ovrstrike_mode = FALSE;
                CMDOUT(OVERSTRIKE_CMD);
                }
            else w3 = FALSE;

            if (Undrscore_mode)
                {
                w4 = TRUE;
                Undrscore_mode = FALSE;
                CMDOUT(UNDRSCORE_CMD);
                }
            else w4 = FALSE;

            while(hpos < tabval)         /* output spaces to get to tab stop */
                {
                pioputchar(' ');
                hpos += Hincr;
                }

            if (Cont_overscore = w1) CMDOUT(OVERSCORE_CMD);  /* then turn the */
            if (Cont_undrscore = w2) CMDOUT(UNDERSCORE_CMD); /* attr. back on.*/

            if (Ovrstrike_mode = w3) CMDOUT(OVERSTRIKE_CMD);
            if (Undrscore_mode = w4) CMDOUT(UNDRSCORE_CMD);
            }
        }
    break;

case 11:                                               /* '0B' - Vertical Tab */
    tabs = (&Vert_tabs)->ptr;          /* array of tab stops */
    value = vpos - vtab_base;          /* base vpos for tab stops */
    for (cnt = 0; tabs[cnt] != 0; cnt++)
        {
        tabval = (tabs[cnt] - 1) * Vincr;
        if (tabval > value) break;
        }
    if (tabs[cnt] != 0)                 /* if found a tab stop */
        vpos = tabval + vtab_base;      /* advance to the tab stop */
    else                                /* otherwise */
        vpos += Vincr;                  /* no tab stop, so do line feed */

    if (Auto_crlf != 2) Hpos_from_prev = hpos;
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    goto ENDLINE;

case 12:                                                  /* '0C' - Form Feed */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    vpos = pglen;
    goto ENDLINE;

case 13:                                            /* '0D' - Carriage Return */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    if (Auto_crlf == 1)
        {
        vpos += Vincr * Linespacing;
        goto ENDLINE;
        }
    CMDOUT(CR_CMD);
    hpos = c_indent + x_indent;
    if ( Workint1 = hpos ) CMDOUT(HORZ_SPACE_CMD);
    break;

case 14:                         /* '0E' - Start Double-Wide Printing by Line */
    Line_dblwide = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SO_CMD);
    break;

case 15:                                   /* '0F' - Start Condensed Printing */
    Condensed = TRUE;
    SET_Pitch(Pitch);
    CMDOUT(SET_COND_CMD);
    break;

case 18:                                /* '12' - DC2: 10 Characters Per Inch */
    Condensed = FALSE;
    SET_Pitch(10);
    CMDOUT(SET_PITCH_CMD);
    break;

case 20:                   /* '14' - DC4: Cancel Double-Wide Printing by Line */
    Line_dblwide = FALSE;
    SET_Pitch(Pitch);
    CMDOUT(DC4_CMD);
    break;

case 24:                  /* Cancel - clears page on laser printer - shouln't */
    break;                /*          be in the data stream - toss it         */

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

/* Don't Leave Overscore or Underscore On Across Lines */
if (Cont_overscore)
    {
    Cont_overscore = FALSE;
    CMDOUT(OVERSCORE_CMD);
    Cont_overscore = TRUE;
    }
if (Cont_undrscore)
    {
    Cont_undrscore = FALSE;
    CMDOUT(UNDERSCORE_CMD);
    Cont_undrscore = TRUE;
    }

if (Ovrstrike_mode)
    {
    Ovrstrike_mode = FALSE;
    CMDOUT(OVERSTRIKE_CMD);
    Ovrstrike_mode = TRUE;
    }
if (Undrscore_mode)
    {
    Undrscore_mode = FALSE;
    CMDOUT(UNDRSCORE_CMD);
    Undrscore_mode = TRUE;
    }

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
| NAME: restore                                                                |
|                                                                              |
| FUNCTION:                                                                    |
|                                                                              |
| EXECUTION ENVIRONMENT:                                                       |
|                                                                              |
| (NOTES:)   (Restoreprinter) will be the default (or command line) value      |
|            only when this routine is called for the last time for a print    |
|            job.  Otherwise, it will be zero and the printer will not be      |
|            restored.                                                         |
|                                                                              |
| (RECOVERY OPERATION:)                                                        |
|                                                                              |
| (DATA STRUCTURES:)                                                           |
|                                                                              |
| RETURNS:                                                                     |
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
| NAME: escseq                                                                |
|                                                                             |
| FUNCTION: handles escape sequences appearing in the data stream             |
|                                                                             |
| EXECUTION ENVIRONMENT:                                                      |
|                                                                             |
| (NOTES:)                                                                    |
|                                                                             |
| (RECOVERY OPERATION:)                                                       |
|                                                                             |
| (DATA STRUCTURES:)                                                          |
|                                                                             |
| RETURNS:                                                                    |
|                                                                             |
+----------------------------------------------------------------------------*/

escseq(fileptr)
FILE *fileptr; {

int c2;                              /* character after the escape character */
int cnt, value, n1, n2, n3;          /* work variables */
char *tabs;                          /* pointer to array of tab stops */

CHARIN(c2);
switch ((char)c2)
{
/*----------------------------------------------------------------------------*/

case '-':                                     /* Continuous underscore on/off */
    CHARIN(n1);
    Cont_undrscore = (n1 & 0x01);
    CMDOUT(UNDERSCORE_CMD);
    break;

case '0':                                 /* zero - Trigger 8 lines per inch */
    SET_Lpi(8);
    Vertspace_mode = FALSE;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '1':                                       /* Trigger 10 lines per inch */
    Vincr = Vdecr = (Vres * 7) / Tunits;
    Vertspace_mode = TRUE;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '2':                               /* Trigger line spacing set by ESC A */
    n1 = Tunits/Text_linesp;
    if ((n1 == 6) || (n1 == 8))
        {             /* braces must remain due to macro expansion */
        SET_Lpi(n1);
        }
    else Vincr = Vdecr = ((Vres * Text_linesp) / Tunits);
    Vertspace_mode = 2;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '3':                                       /* Set graphics line spacing */
    CHARIN(n1);
    Graphic_linesp = n1;                /* value to be sent to printer */
    Vincr = Vdecr = (Vres * n1) / Gunits;
    Vertspace_mode = 3;
    CMDOUT(SET_VERTSP_CMD);
    break;

case '4':                                                 /* Set Top of Form */
    break;

case '5':                                             /* Automatic line feed */
    CHARIN(n1);
    Auto_crlf = (n1 & 0x01);
    break;

case '6':                                               /* Select Char set 2 */
case '7':                                               /* Select Char set 1 */
    break;

case ':':                                                      /* Set 12 cpi */
    Condensed = FALSE;
    Prop_spacing = FALSE;
    SET_Pitch(12);
    CMDOUT(SET_COND_CMD);
    break;

case '=':                                         /* Download a chacater set */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(DOWNLOAD_CMD,cnt);
    break;

/*----------------------------------------------------------------------------*/

case 'A':                       /* Set text line spacing - triggered by ESC 2 */
    CHARIN(Text_linesp);
    CMDOUT(SET_LS_CMD);
    break;

case 'B':                                                /* Set Vertical Tabs */
    tabs = (&Vert_tabs)->ptr;
    if (Max_vtabs > 0)
        {
        for (cnt = 0; cnt <= Max_vtabs; cnt++)
            {
            CHARIN(value);
            if ((tabs[cnt] = (char)value) == 0) break;
            }
       if (cnt > Max_vtabs)
           {
           cnt = Max_vtabs;
           tabs[cnt] = 0;
           }
       }
    break;

case 'C':                                                 /* Set form length */
    CHARIN(n1);
    if (!(n1)) CHARIN(n1);    /* if n1=0 then read char (Form len (inches)) */
    Tmarg = Bmarg = 0;
    break;

case 'D':                                              /* Set Horizontal Tabs */
    tabs = (&Horz_tabs)->ptr;
    if (Max_htabs > 0)
       {
       for (cnt = 0; cnt <= Max_htabs; cnt++)
           {
           CHARIN(value);
           if ((tabs[cnt] = (char)value) == 0) break;
           }
       if (cnt > Max_htabs)
           {
           cnt = Max_htabs;
           tabs[cnt] = 0;
           }
        }
    break;

case 'E':                                                    /* Emphasized on */
    Emphasized = TRUE;
    CMDOUT(SET_EMPH_CMD);
    break;

case 'F':                                                   /* Emphasized off */
    Emphasized = FALSE;
    CMDOUT(SET_EMPH_CMD);
    break;

case 'G':                                                 /* Double strike on */
    Doublestrike = TRUE;
    CMDOUT(SET_DBLSTR_CMD);
    break;

case 'H':                                                /* Double strike off */
    Doublestrike = FALSE;
    CMDOUT(SET_DBLSTR_CMD);
    break;




    /*-------------------------------------------------------------------------+
    |                                                                          |
    |               This logic applicable to the LinePrinter 4224              |
    |                                                                          |
    +-------------------------------------------------------------------------*/

case 'I':                                                /* Select print mode */
    CHARIN(n1);

    if (Pr_model_num == 4224)
        {
        Quality = n1;
        Prop_spacing = (n1 > 3 ? 1 : 0);
        CMDOUT(SET_MODE_CMD);
        break;
        }

    if (Pr_model_num == 4234)
        {
        Quality = ((n1 & 0x03) ? ((n1 & 0x01) ? 0 : 2) : 1);
        Prop_spacing = ((n1 == 0x62) ? 1 : 0);
        Workint1 = ((n1 > 0x62) ? ((n1 == 0x63) ? 0x63 : 0x64) : 0);
        if (Workint1) Pitch = 10;
        CMDOUT(SET_MODE_CMD);
        SET_Pitch(Pitch);
        break;
        }


case 'J':                                   /* Graphics Variable line spacing */
    CHARIN(Workint1);                       /* value to be sent to printer    */
    Vincr = Vdecr = (Vres * Workint1) / Gunits;  /* inc Vincr by n1*Gunits    */
    strcpy(Vincr_cmd_norm,VAR_LS_CMD);      /* change endline cmd to ESC J    */
    return(EOL);                            /* let formatter issue new lf     */

case 'K':                                                  /* 60 DPI Graphics */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);                /* number of bytes of graphics data */
    CMDOUT_PT(GRA_60_CMD, cnt);           /* output the graphics command      */
    hpos += (cnt / 60) * Hres;
    break;

case 'L':                                     /* 120 DPI Graphics (low speed) */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_120LS_CMD, cnt);
    hpos += (cnt / 120) * Hres;
    break;

case 'N':                                       /* Set auto perforation skip */
    CHARIN(n1);
    if (!n1) break;              /* if n1=0 leave the current margins active */
    if (n1 > Pglen)                   /* if n1>Pglen then cancel the margins */
        {
        Tmarg = Bmarg = 0;
        break;
        }
    Tmarg = Perf_percent * n1 / 100;    /* split perf skip lines between top */
    Bmarg = n1 - Tmarg;                 /* & bottom margins according to the */
    break;                              /* spec'd perf_percent */

case 'O':                                           /* Kill perforation skip */
    Tmarg = Bmarg = 0;
    break;

case 'P':                                     /* Proportional spacing on/off */
    CHARIN(n1);
    Prop_spacing = (n1 & 0x01);
    CMDOUT(SET_PROPSP_CMD);
    break;

case 'Q':                                /* Deselect on positive query reply */
    CHARIN(n1);
    break;

case 'R':                                    /* Reset horiz & vert tab stops */
    tabs = (&Vert_tabs)->ptr;
    piomode = 0;
    strcpy(tabs,Vert_tabs);
    piomode = 1;

    tabs = (&Horz_tabs)->ptr;
    piomode = 0;
    strcpy(tabs,Horz_tabs);
    piomode = 1;
    break;

case 'S':                                       /* Superscript / subsript on */
    CHARIN(n1);
    if (n1 & 0x01)
        {
        Subscr_mode = TRUE;
        CMDOUT(SET_SUB_CMD);
        }
    else
        {
        Superscr_mode = TRUE;
        CMDOUT(SET_SUPER_CMD);
        }
    break;

case 'T':                                      /* Superscript / subsript off */
    Subscr_mode = Superscr_mode = FALSE;
    CMDOUT(SET_SUB_CMD);
    break;

case 'U':                                       /* Bi/unidirectional printing */
    CHARIN(n1);
    L_to_r_print = (n1 & 0x01);    /* n1=1 sets uni-directional printing */
    CMDOUT(LR_PRINT_CMD);
    break;

case 'W':                                      /* Continuous double wide mode */
    CHARIN(n1);
    Line_dblwide = FALSE;
    Cont_dblwide = (n1 & 0x01);
    SET_Pitch(Pitch);
    CMDOUT(CONT_DW_CMD);
    break;

case 'X':                                           /* Set horizontal margins */
    CHARIN(n1);
    CHARIN(n2);
    if (n2) SET_Pgwidth(n2);
    if (n1) x_indent = (n1 - 1) * Hincr;
    CMDOUT(SET_MAR_CMD);
    break;

case 'Y':                                 /* 120 DPI Graphics (normal speed) */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_120NS_CMD, cnt);
    hpos += (cnt / 120) * Hres;
    break;

case 'Z':               /* 240 DPI Graphics (high density bit-image graphics */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(GRA_240_CMD, cnt);
    hpos += (cnt / 240) * Hres;
    break;

/*---------------------------------------------------------------------------*/

case '\\':                         /* Print multiple chars from ALL CHAR set  */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);        /* no. of bytes to be sent                  */
    value = (Width - hpos)/Hincr; /* no. of chars that will fit on line       */

    if (value < cnt)          /* if # that will fit is less than # to be sent */
        {
        CMDOUT_PT(PRTALL_CMD,value);
        if (Wrap)             /* if wrap on then the command and the no.      */
            {                 /* of remaining chars must be stuffed back      */
            cnt -= value;     /* into the buffer to be sent next              */
            n2 = cnt / 256;   /* decr CNT by the no. of chars just sent       */
            n1 = cnt % 256;
            UNCHARIN(n2);     /* stuff char count remainder into buffer       */
            UNCHARIN(n1);
            UNCHARIN(92);     /* stuff ESC 'backslash' into buffer            */
            UNCHARIN(27);
            return(EOL);      /* tell pioformat we're at the end of a line    */
            }
        else for (;value<cnt;value++) CHARIN(c2)               /* wrap is off -
                                                           toss rest of chars */
        }

    else           /* all those to be sent will fit on the line we're on */
        {
        CMDOUT_PT(PRTALL_CMD,cnt);
        hpos += Hincr * cnt;
        }
    break;

case ']':                                               /* Reverse line feed */
    if ((vpos - Vdecr) >= vtab_base) vpos -= (Vincr + Vdecr) * Linespacing;
    Line_dblwide = FALSE;    /* Vincr * Linespacing is subtracted from */
    SET_Pitch(Pitch);        /* because it will be added right back in */
    return(EOL);             /* by the LINEFEED routine                */

case '^':                             /* Print single char from ALL CHAR set */
    if ((hpos + Hincr) > Width)
        {
        UNCHARIN('^');
        UNCHARIN(27);
        return(EOL);
        }
    CMDOUT_PT(PRTNEXT_CMD,1);
    hpos += Hincr;
    break;

case '_':                                      /* Continuous overscore on/off */
    CHARIN(n1);
    Cont_overscore = (n1 & 0x01);
    CMDOUT(OVERSCORE_CMD);
    break;

case 'd':                                   /* Relative move inline forward   */
    CHARIN(n1);                             /* get number of 120ths to move   */
    CHARIN(n2);
    cnt = ((n1 + (n2 * 256)) * Hres) / 120; /* turn it into Hres units        */
    value = Width - hpos;                   /* how far to the right margin    */
    Workint1 = (value < cnt ? value : cnt); /* use the lesser of the two      */
    hpos += Workint1;                       /* reduce hpos by the movement    */
    Workint1 = (Workint1 * 120) / Hres;     /* convert movement into 120ths   */
    CMDOUT(MOVE_RIGHT_CMD);
    break;

case 'e':                                   /* Relative move inline backward  */
    CHARIN(n1);                             /* get number of 120ths to move   */
    CHARIN(n2);
    cnt = ((n1 + (n2 * 256)) * Hres) / 120; /* turn it into Hres units        */
    value = hpos - c_indent;                /* how far to the left margin     */
    Workint1 = (value < cnt ? value : cnt); /* use the lesser of the two      */
    hpos -= Workint1;                       /* reduce hpos by the movement    */
    Workint1 = (Workint1 * 120) / Hres;     /* convert movement into 120ths   */
    CMDOUT(MOVE_LEFT_CMD);
    break;

case 'j':                                                   /* Stop printing */
    CMDOUT(STOP_PRT_CMD);
    break;

case 'n':                                             /* Select aspect ratio */
    CHARIN(n1);
    Workint1 = (n1==2 ? 2 : 1);
    CMDOUT(SET_ASPRAT_CMD);
    break;

case '*':                      /* Select graphics mode (AGM) - not supported */
    CHARIN(Workint1);
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);              /* byte count */
    CMDOUT_PT(AGMGRAPH_CMD,cnt);
    break;



/*----------------------------------------------------------------------------+
|                                                                             |
|                        Color commands for the 4224                          |
|                                                                             |
+----------------------------------------------------------------------------*/

case 'b': Color = 0; goto SELECT_COLOR;                             /* black */
case 'c': Color = 5; goto SELECT_COLOR;                              /* cyan */
case 'm': Color = 3; goto SELECT_COLOR;                           /* magenta */
case 'y': Color = 6;                                              /* yeollow */

SELECT_COLOR:
    CMDOUT(SEL_COLOR_CMD);
    break;

/*----------------------------------------------------------------------------+
|                                                                             |
|                        Escape [ subset for the 4224                         |
|                                                                             |
+----------------------------------------------------------------------------*/

case '[':
    CHARIN(n1);
    switch((char)n1)
    {
    case '@':                         /* Special presentation characteristics */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);                              /* get byte count */

        if (Pr_model_num == 4224)
            {
            CHARIN(n1);                  /* datastream value for ht & spacing */
            Italics = ((n1 & 0x03) == 0x01 ? 1 : 0);
            CMDOUT(SET_ITAL_CMD);
            break;
            }

        if (Pr_model_num == 4234)
            {
            if (cnt) CHARIN(n1);    /* if cnt at least 1 read nul */
            if (cnt-1) CHARIN(n2);  /* if cnt at least 2 read nul */

            if (cnt-2)              /* if cnt at least 3 read next byte */
                {
                CHARIN(n1);         /* datastream value for italics & spacing */
                if (n1)
                    {
                    if (n1 & 0x03) Italics = ((n1 & 0x02)?1:0);
                    if (n1 & 0x30) Linespacing = ((n1 & 0x20)?2:1);
                    CMDOUT(SET_ITAL_CMD);
                    }
                }

            if (cnt-3)              /* if cnt at least 4 read next byte */
                {
                CHARIN(n2);     /* set char width if changed */
                if (n2)
                    {
                    Line_dblwide = FALSE;
                    Cont_dblwide = ((n2 & 0x02)?1:0);
                    SET_Pitch(Pitch);
                    CMDOUT(CONT_DW_CMD);
                    }
                }
            break;
            }

    case 'A':                                                /* Overstrike on */
        CHARIN(n1);
        CHARIN(n2);              /* get byte count - must be 2                */
        CHARIN(O_strike_char);   /* chaacter to use as overstrike             */
        CHARIN(O_strike_byp);    /* bypass overstrike whitespace 0; no 1; yes */
        Ovrstrike_mode = TRUE;
        CMDOUT(OVERSTRIKE_CMD);
        break;

    case 'B':                                                /* Underscore on */
        CHARIN(n1);
        CHARIN(n2);              /* get byte count - must be 2                */
        CHARIN(U_score_byp);     /* bypass underscore whitespace 0; no 1; yes */
        Undrscore_mode = TRUE;
        CMDOUT(UNDRSCORE_CMD);
        break;

    case 'D':                                               /* Overstrike off */
        CHARIN(n1);
        CHARIN(n2);
        Ovrstrike_mode = FALSE;
        CMDOUT(OVERSTRIKE_CMD);
        break;

    case 'E':                                              /* Underscore  off */
        CHARIN(n1);
        CHARIN(n2);
        Undrscore_mode = FALSE;
        CMDOUT(UNDRSCORE_CMD);
        break;

    case 'J':                               /* Graphics Variable line spacing */
        CHARIN(n1);
        CHARIN(n2);
        Workint1 = n1 + n2 * 256;           /* value to be sent to printer    */
        Vincr = Vdecr = Workint1;
        break;     /* this command does not result in a printer line feed */

    case 'K': n3 = FALSE; goto BLIND_PASSTHRU;          /* Initialize printer */

    case 'M':                                                 /* Select color */
        CHARIN(n1);
        CHARIN(n2);             /* ignore byte count - must be 1 */
        CHARIN(Color);
        CMDOUT(SEL_COLOR_CMD);
        break;

    case 'P': n3 = FALSE; goto BLIND_PASSTHRU;               /* Set page size */

    case 'Q': n3 = TRUE;  goto BLIND_PASSTHRU;           /* Set page position */

    case 'Y': n3 = TRUE;  goto BLIND_PASSTHRU;       /* Initialize local font */

    case 'Z': n3 = TRUE;  goto BLIND_PASSTHRU;     /* Set local character set */

    case '[': n3 = FALSE; goto BLIND_PASSTHRU;           /* Delete local font */

    case '\\':                                          /* Set vertical units */
        CHARIN(n1);                   /* get count */
        CHARIN(n2);
        cnt = n1 + (n2 * 256);

        CHARIN(n1);                   /* get Tunits and make sure the */
        CHARIN(n2);                   /* number is legal              */
        value = n1 + (n2 * 256);
        if (value && ((value == 72) || (value == 144)))
            Tunits = value;

        if (cnt > 2)
            {
            CHARIN(n1);               /* get Gunits and make sure the */
            CHARIN(n2);               /* number is legal              */
            value = n1 + (n2 * 256);
            if (value && ((value == 72) || (value == 144) || (value == 216)))
                Gunits = value;
            }

        CMDOUT(SET_VUNITS_CMD);
        break;

    case '_': n3 = TRUE;  goto BLIND_PASSTHRU;       /* Graphics segment list */

    case '~': n3 = TRUE;  goto BLIND_PASSTHRU;                    /* Bar Code */

    case 'a': n3 = TRUE;  goto BLIND_PASSTHRU;             /* Segment control */

    case 'b': n3 = TRUE;  goto BLIND_PASSTHRU;             /* Overlay control */

    case 'p': n3 = TRUE;  goto BLIND_PASSTHRU;     /* Graphic mapping control */

    case 's':                                               /* Printer status */
    default:  n3 = FALSE;                                /* Unknown character */

BLIND_PASSTHRU:
        Workint1 = n1;
        CHARIN(n1);
        CHARIN(n2);             /* get byte count */
        cnt = n1 + (n2 * 256);
        if (n3) {CMDOUT_PT(PASS_THRU,cnt)}
        else {CMDOUT_PT(DONT_PASS_THRU,cnt)}
        break;

    }
    break;

/*----------------------------------------------------------------------------+
|                                                                             |
|                            end of Escape [ subset                           |
|                                                                             |
+----------------------------------------------------------------------------*/

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
