#ifndef lint
static char sccsid[] = "@(#)76 1.1 src/bos/usr/lib/pios/fmtrs/piofbull.c, cmdpios, bos411, 9428A410j 6/23/94 10:47:42";
#endif
/*
 * COMPONENT_NAME: (CMDPIOS) Print formatter for IBM 4201/4202 and 4207/4208
 *                           Proprinter data streams and bull epson printer
 *
 * FUNCTIONS: setup, initialize, lineout, passthru, restore, errorexit, escseq
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

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

#include "piofbull.h"

#define MAXVARSTRLEN  200       /* memory to allocate for each variable string*/

#define EOL           -2        /* end-of-line detected */

#define CMDOUT_PT(attr, pt) { piocmdout(attr, fileptr, pt, NULL); \
                              charcount += pt; }

#define CMDOUT(attr)        { piocmdout(attr, NULL   , 0 , NULL); }

#define CHARIN(x)           { x = piocharin(fileptr,NULL); \
                              if (x == EOF) return(EOF); \
                              charcount++; }
/*******PIERA********/
#define DC4  "\024\024"     
/*******PIERA*END*****/

#define CHARIN_TRANS(x)     { x = piocharin(fileptr,(&Codepagename)->ptr); \
                              charcount++; }

#define UNCHARIN(c1)        { piopushin(c1); \
                              if (charcount > 1) charcount--; }



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


#define SET_Pitch(cpi)          {Pitch = cpi;   \
                                 Hincr = (10 * Hres) /  \
     (Condensed ? ((Pitch==10)?171:((Cond_type==1)?120:200)) : (10 * Pitch));  \
     Hincr += Hincr * (Cont_dblwide || Line_dblwide); }


/*------------------------------ Work Variables ------------------------------*/

int c_indent;           /* Command line indentation in Hres units             */
int x_indent;           /* Indentation called for by Esc X cmd in Hres units  */
int charcount;          /* number of bytes processed for the line             */
int vpos;               /* vertical position on page, in Vres units           */
int vtab_base;          /* base vpos value for vertical tab stops             */
int hpos;               /* horizontal position on page, in Hres units         */
int substchar;          /* substitute character to use when character         */
                        /* in input data stream can't be printed              */
int fonts_bad;          /* fonts need to be refreshed */
char Vincr_cmd_norm[3]; /* the Vincr_cmd can be changed in ESC J, but the     */
                        /* orig cmd must be recorded to be retrieved later    */
/*** PIERA  start ***/
char wdc4[2];		/* proprietary initial commands		*/	
int piera;
/*** PIERA end ***/
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

/* ANGELA INIZIO */
if (Highspeed           < 0 || Highspeed > 1)
                                      errorexit(MSG_BADFORMFLAG1,"S",NULL,NULL);
/* ANGELA FINE */

if (Doublehigh     < 0 || Doublehigh > 1)
                                      errorexit(MSG_BADFORMFLAG1,"E",NULL,NULL);
/*if (Pitch        != 10 && Pitch != 12)*/
/***********PIERA *********/
if (Pitch != 8 && Pitch != 10 && Pitch != 12 && Pitch != 13 && Pitch != 15 && Pitch != 17 && Pitch != 20)
                                      errorexit(MSG_BADFORMFLAG4,"p",NULL,NULL);
/***********PIERA *END*****/

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
int still_sending = TRUE;
char wkstr[20];

if (!Init_printer) return(0);             /* Is the printer to be initialized */

CMDOUT(INIT_CMD);      /* Output the Command String to Initialize the Printer */

fonts_bad = 0;

/* Download Font (if any)      put name into wkbuf */
piogetstr(DOWNLOAD_FONT, wkbuf, sizeof(wkbuf), NULL);
if (*wkbuf != NULL)
    {
    if ((fileptr = fopen(wkbuf, "r")) == NULL)
        {
        (void) sprintf(wkstr,"%d",errno);
        errorexit(MSG_OPEN_FONTS,wkbuf,wkstr,NULL);
        }
    while(still_sending)
        {
        c1 = piocharin(fileptr,NULL);
        charcount++;
        if (c1 != EOF) {pioputchar(c1);}
        else still_sending = FALSE;
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
int value,tabval,cnt,w1,w2;        /*|work variables                         |*/
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
int c3;
FILE *fp;

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
        CHARIN_TRANS(value);
        UNCHARIN(value);
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
        tabval = (tabs[cnt] - 1) * Hincr + x_indent;
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

            while(hpos < tabval)         /* output spaces to get to tab stop */
                {
                pioputchar(' ');
                hpos += Hincr;
                }

            if (Cont_overscore = w1) CMDOUT(OVERSCORE_CMD);  /* then turn the */
            if (Cont_undrscore = w2) CMDOUT(UNDERSCORE_CMD); /* attr. back on.*/

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
    /**** PIERA start ****/
#ifdef DEBUGPIERA
f1=open("/tmp/pieratmp",012);
write(f1,"DC4 Case 20\n",12);
close(f1);
#endif
    CHARIN(c2);

    if (c2 == c1)
    {
	/* '14' - DC4 again */
    	CHARIN(c3);
	if(c3 == 27)
	{
	    /* '1B' Escape */
#ifdef DEBUGPIERA
f1=open("/tmp/pieratmp",012);
write(f1,"DC4 DC4 1B Case 20\n",19);
close(f1);
#endif
	    my_escseq(fileptr);
	}
	else
		UNCHARIN(c3);
    }
    else {
	    UNCHARIN(c2);

	    Line_dblwide = FALSE;
	    SET_Pitch(Pitch);
	    CMDOUT(DC4_CMD);
     }
	    break;

/*  in Epson : DC3 */
case 23:                                /* Deselect on positive query reply */
    CMDOUT(DC3_CMD);
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

    /* Are we at the Top of page with a dbl high char to print */
    if ((vpos == vtab_base) & (Doublehigh))
        {
        UNCHARIN(c1);
        Hpos_from_prev = hpos - Hincr;
        vpos += Vincr;
        goto ENDLINE;
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
int c1 = 0;
int c2 = 0;

while ((c1 = piogetchar()) != EOF)
    {
    pioputchar(c1);
    if (c2 == 27 && c1 == '=')
	fonts_bad = 1;
    c2 = c1;
    }

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
char filename[100];

if (!fonts_bad) {
	piogetstr(FLAGS_DIR,filename,sizeof(filename),NULL);
	piogetstr(PR_DEVICE,wkbuf,sizeof(wkbuf),NULL);
	strcat(filename,"/");
	strcat(filename,wkbuf);
	unlink(filename);
}
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
int cnt, value, n1, n2;              /* work variables */
char *tabs;                          /* pointer to array of tab stops */
FILE *fr;
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

/****************** PIERA***/

case 'M':                                                      /* Set 12 cpi */
    Condensed = FALSE;
    Prop_spacing = FALSE;
    SET_Pitch(12);
    CMDOUT(SET_COND_CMD);
    break;

case 'g':                                                      /* Set 15 cpi */
    Condensed = FALSE;
    Prop_spacing = FALSE;
    SET_Pitch(15);
    CMDOUT(SET_PITCH_CMD);
    break;
/***************** END PIERA      *********************/
case '=':                                         /* Download a chacater set */
    CHARIN(n1);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(DOWNLOAD_CMD,cnt);
    fonts_bad = 1;
    break;

/****************** PIERA */
case '&':                                         /* Download a chacater set */
    CMDOUT(SET_DWL_CMD);
    CHARIN(n1);
    if (!n1)
	break;
    CMDOUT(SET_NULL_CMD);
    CHARIN(n2);
    cnt = n1 + (n2 * 256);
    CMDOUT_PT(DOWNLOAD_CMD,cnt);
    fonts_bad = 1;
    break;
/***************** END PIERA      *********************/
/*----------------------------------------------------------------------------*/

case 'A':                       /* Set text line spacing - triggered by ESC 2 */
    CHARIN(Text_linesp);
    CMDOUT(SET_LS_CMD);
    break;

case 'B':                                                /* Set Vertical Tabs */
    CHARIN(n1);
 /* confronta con NULL  PIERA  */
    if (n1 == 0)
        {
        CMDOUT(SET_RTAB_CMD);
        }
    else
        {
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



case 'I':                                                /* Select print mode */
    CHARIN(n1);

    switch(I_type)
        {
    /*-------------------------------------------------------------------------+
    |                                                                          |
    |             This logic applicable to the original ProPrinter             |
    |                                                                          |
    +-------------------------------------------------------------------------*/
        case 0:
            Quality = ((n1 & 0x02)?2:1);     /* prt qual=1 (drft) or 2 (NLQ) */
            CMDOUT(SET_QUAL_CMD);
            break;

    /*-------------------------------------------------------------------------+
    |                                                                          |
    |             This logic applicable to the ProPrinter X24E  (4207)         |
    |                                                                          |
    +-------------------------------------------------------------------------*/
        case 1:
            if((n1&0x01)&&((n1!=3)&&(n1!=7))) break;  /* only legal odds  */
                                                      /* are 3 and 7      */
            Prop_spacing = (n1 & 0x01);               /* Set to ON or OFF */
            CMDOUT(SET_PROPSP_CMD);

            if (!(n1 & 0x01))              /* If off then consider the pitch */
                {
                Pitch = ((n1 & 0x08) ? 12 : 10);
                Condensed = ( (Pitch==12) ? 0 : ((n1&0x10) ? 1 : 0) );
                SET_Pitch(Pitch);
                CMDOUT(SET_COND_CMD);

                Quality = ((n1 & 0x02)?2:1); /* prt qual=1 (drft) or 2 (NLQ) */
                CMDOUT(SET_QUAL_CMD);
                }

            break;

    /*-------------------------------------------------------------------------+
    |                                                                          |
    |             This logic applicable to the ProPrinter III  (4201)          |
    |                                                                          |
    +-------------------------------------------------------------------------*/
        case 2:
            if ((n1 & 0x0b) == 1) SET_Pitch(12);
                                                                /* n1 Quality */
            value = (n1 & 0x03);                                /* 0    1     */
            Quality = (!value ? 1 : (value == 1 ? 0 : value));  /* 1    0     */
            CMDOUT(SET_QUAL_CMD);                               /* 2    2     */
                                                                /* 3    3     */
            Italics = ((n1 & 0x08) ? 1 : 0);
            CMDOUT(SEL_ITALIC_CMD);

            break;

        default:
            break;
    }
    break;

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

/*****PIERA***********/
case 'p':                                     /* Proportional spacing on/off */
    CHARIN(n1);
    Prop_spacing = (n1 & 0x01);
    CMDOUT(SET_PROPSP_CMD);
    break;
/*****PIERA*END*******/
/* Deselect on positive query reply */
/*case 'Q':                              
    CHARIN(n1);
    break;
*/

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

/******PIERA ************/
case 'l':                                           /* Set Right margin */

#ifdef DEBUGPIERA
f1=open("/tmp/pieratmp",012);
write(f1,"SET RIGHT MARGIN\n",16);
close(f1);
#endif

    CHARIN(n1);
    SET_Pgwidth(n1);
    CMDOUT(SET_LMAR_CMD);
    break;

case 'Q':                                           /* Set Left margin */

#ifdef DEBUGPIERA
f1=open("/tmp/pieratmp",012);
write(f1,"SET LEFT MARGIN\n",16);
close(f1);
#endif

    CHARIN(n1);
    x_indent = (n1 - 1) * Hincr;
    CMDOUT(SET_RMAR_CMD);
    break;

/******PIERA *END********/
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
|                        Escape [ subset for the 4200                         |
|                                                                             |
+----------------------------------------------------------------------------*/

case '[':
    CHARIN(n1);
    switch((char)n1)
    {
    case '@':                             /* Double high & wide printing */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);  /* get byte count - should be 4 */

        if (cnt) CHARIN(n1);    /* if cnt at least 1 read nul */
        if (cnt-1) CHARIN(n2);  /* if cnt at least 2 read nul */

        if (cnt-2)              /* if cnt at least 3 read next byte */
            {
            CHARIN(n1);     /* datastream value for ht & spacing */
            if (n1)
                {
                if (n1 & 0x03) Doublehigh = ((n1 & 0x02)?1:0);
                if (n1 & 0x30) Linespacing = ((n1 & 0x20)?2:1);
                CMDOUT(SET_DWDH_CMD);
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

    case 'K':                    /* Set initial condition from the datastream */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + n2*256;
        CMDOUT_PT(DS_INIT_CMD,cnt);
        break;

    case 'T':                                                /* Set code page */
        CHARIN(n1);
        CHARIN(n2);
        CMDOUT(SET_CODEPG_CMD);
        break;

    case '\\':                                          /* Set vertical units */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);
        CHARIN(n1);                         /* Tunits - not used */
        CHARIN(n2);
        if (cnt > 2)
            {
            CHARIN(n1);                     /* Gunits */
            CHARIN(n2);
            value = n1 + (n2 * 256);
            }
        if (Gunit_type == 1)
              Gunits = ((value==216)?216:((value==180)?180:Gunits));
        if (Gunit_type == 2) Gunits = 216;
        CMDOUT(SET_VUNITS_CMD);
        break;

    case 'g':                                            /* High res graphics */
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256) - 1;

        CHARIN(Workint1);
        CMDOUT_PT(HIRES_GRA_CMD, cnt);
        if (Workint1 == 0) n2 = 60;
        if (Workint1 == 1) n2 = 120;
        if (Workint1 == 2) n2 = 120;
        if (Workint1 == 3) n2 = 240;
        if (Workint1 == 8) n2 = 60 / 3;           /*  spacing divided by 3 to */
        if (Workint1 == 9) n2 = 120 / 3;            /* account for the 24 pin */
        if (Workint1 == 11) n2 = 180 / 3;       /* printers requiring 3 bytes */
        if (Workint1 == 12) n2 = 360 / 3;               /* per character cell */
        hpos += (cnt / n2) * Hres;
        break;

    default:                                             /* Unknown character */
        Workint1 = n1;
        CHARIN(n1);
        CHARIN(n2);
        cnt = n1 + (n2 * 256);
        CMDOUT_PT(PASS_THRU, cnt);
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
			catd = catopen(CatName,0);
	
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
		def_catd = catopen(defpath,0);
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
/***** PIERA START ********/
/*----------------------------------------------------------------------------+
|                                                                             |
| NAME: my_escseq                                                             |
|                                                                             |
| FUNCTION: handles escape sequences appearing in the data stream             |
|	    for the proprietary Bull commands (DC4DC4ESC....)	              |	
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

my_escseq(fileptr)
FILE *fileptr;
{
int c2;                              /* character after the escape character */
int value, n1;		              /* work variables */
CHARIN(c2);

	switch ((char)c2)
	{
	case 'p':                                      /* Select print mode */
	    CHARIN(n1);

	    if(n1 == 0x02)	 /* prt qual=1 (drft) or 2 (NLQ) **/
	    {
		    Quality = 2;
		    CMDOUT(SET_QUAL_CMD);
	    }
	    else if (n1 == 0x05)
	    {
		    Quality = 1;
		    CMDOUT(SET_QUAL_CMD);
	    }
            break;

        default:
            break;
	}
}
/***** PIERA END ********/
