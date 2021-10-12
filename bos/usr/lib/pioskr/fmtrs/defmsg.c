/* @(#)70	1.1  src/bos/usr/lib/pioskr/fmtrs/defmsg.c, cmdpioskr, bos411, 9428A410j 5/25/92 15:14:49 */
/*
 * COMPONENT_NAME :     CMDPIOSKR
 *
 * FUNCTIONS :          defmsg.h
 *
 * ORIGINS :            27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_JFMTRS_MSG 
#define _H_JFMTRS_MSG 
#include <limits.h>
#include <nl_types.h>
#define MF_JFMTRS "kfmtrs.cat"

/* definitions for set 1 */
#define MSG_BADFORMFLAG 121
#define MSG_BADFORMFLAG1 122
#define MSG_BADFORMFLAG2 123
#define MSG_BADFORMFLAG3 124
#define MSG_BADFORMFLAG4 125
#define MSG_BADFORMFLAG5 126
#define MSG_BADFORMFLAG6 127
#define MSG_BADFORMFLAG7 128
#define MSG_BADFORMFLAG8 129
#define MSG_BADFORMFLAG9 130
#define MSG_NOPITCH 131
#define MSG_BAD4224COLOR 132
#define MSG_BADQUALITY 133
#define MSG_BADFONTSTYLE1 134
#define MSG_BADFONTSTYLE2 135
#define MSG_BADPITCH 136
#define MSG_BADLPI 137
#define MSG_BADPAPERFLAG 138
#define MSG_BADPAPERSRC 139
#define MSG_BADROTATION 140
#define MSG_BADUSERFONT1 141
#define MSG_BADUSERFONT2 142
#define MSG_BADUSERFONT3 143
#define MSG_BADUSERFONT4 144
#define MSG_BADUSERFONT5 145
#define MSG_BADUSERFONT6 146
#define MSG_BADCODEPAGE 147
#define MSG_MEMALLOCERR 148
#define MSG_FATAL 149
#define MSG_IBUFFULL 150
#define MSG_IBUFFATAL 151
#endif 

static struct _stdefmsg {
	int	mid;
	char	*defmsg;
} Stdefmsg[40] = {

{ 120 ,
	"Dummy Message." },

{ MSG_BADFORMFLAG ,
	"0782-500 The parameter value specified with the -%.1s flag is not valid.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG1 ,
"0782-501 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be + or !.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG2 ,
"0782-502 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1 or greater.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG3 ,
"0782-503 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0 or greater.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG4 ,
"0782-504 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 10 or 12.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG5 ,
"0782-505 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0, 1, 2, or 3.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG6 ,
"0782-506 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 6 or 8.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG7 ,
"0782-507 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 0, 1, or 2.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG8 ,
"0782-508 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be less than the page width.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFORMFLAG9 ,
"0782-509 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1, 2, or 3.\n\
\tCheck the parameter value.\n" },

{ MSG_NOPITCH ,
"0782-510 Cannot specify a pitch value directly.\n\
\tThe pitch value is implied by the font identifier.\n\
\tSpecify an appropriate font identifier with the -I flag.\n\
\tNote: To determine the correct font identifier, consult the manual\n\
\t      that came with your printer.\n" },

{ MSG_BAD4224COLOR ,
"0782-511 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    black   (or 0)\n\
\t    blue    (or 1)\n\
\t    red     (or 2)\n\
\t    magenta (or 3)\n\
\t    green   (or 4)\n\
\t    cyan    (or 5)\n\
\t    yellow  (or 6)\n\
\t    brown   (or 10)\n\
\t Check the parameter value.\n" },

{ MSG_BADQUALITY ,
"0782-700 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be 1 or 2.\n\
\tCheck the parameter value.\n" },

{ MSG_BADFONTSTYLE1 ,
"0782-701 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    m10\n\
\t    m12\n\
\t    gothic\n\
\t    courier\n\
\t    elite\n\
\t Check the parameter value.\n" },

{ MSG_BADFONTSTYLE2 ,
"0782-702 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    m12\n\
\t    gothic\n\
\t    courier\n\
\t    elite\n\
\t    orator\n\
\t    ocr\n\
\t Check the parameter value.\n " },

{ MSG_BADPITCH ,
"0782-703 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t    10\n\
\t    12\n\
\t    13.4\n\
\t    15\n\
\t Check the parameter value.\n " },

{ MSG_BADLPI ,
"0782-704 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     2\n\
\t     3\n\
\t     4\n\
\t     5\n\
\t     6\n\
\t     7.5\n\
\t     8\n\
\t Check the parameter value.\n" },

{ MSG_BADPAPERFLAG ,
"0782-705 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     1: Manual\n\
\t     2: Pinwheel\n\
\t     3: Sheetfeed\n\
\t Check the parameter value.\n " },

{ MSG_BADPAPERSRC ,
"0782-706 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     1: Upper\n\
\t     2: Lower\n\
\t Check the parameter value.\n " },

{ MSG_BADROTATION ,
"0782-707 The parameter value specified with the -%.1s flag is not valid.\n\
\t The parameter value must be one of the following:\n\
\t     0: Portrait\n\
\t     1: Landscape right\n\
\t     2: Portrait Upside-down\n\
\t     3: Landscape left\n\
\t Check the parameter value.\n " },

{ MSG_BADUSERFONT1 ,
"0782-708 Cannot access the specified user-defined font file %s.\n" },

{ MSG_BADUSERFONT2 ,
"0782-709 The file %s specified as a user-defined font file is not \n\
a regular file. \n" },

{ MSG_BADUSERFONT3 ,
"0782-710 Cannot allocate memory for the user-defined font file %s. \n" },

{ MSG_BADUSERFONT4 ,
"0782-711 Cannot open the specified user-defined font file %s. \n" },

{ MSG_BADUSERFONT5 ,
"0782-712 Cannot load the specified user-defined font file %s. \n" },

{ MSG_BADUSERFONT6 ,
"0782-713 The file %s specified as a user-defined font file is not a \n\
user-defined font file in snf format.\n" },

{ MSG_BADCODEPAGE ,
"0782-714 The specified code page is not supported or bad.\n" },

{ MSG_MEMALLOCERR ,
"0782-715 Memory allocation error has happened.\n" },

{ MSG_FATAL ,
"0782-716 Fatal error.\n" },

{ MSG_IBUFFULL ,
"0782-717 Internal buffer full.\n" },

{ MSG_IBUFFATAL ,
"0782-718 Internal buffer fatal error.\n" },

{ 0, NULL }

};



/* 
 * NAME: errorexit
 * 
 * FUNCTION: displays error messages 
 * 
 * EXECUTION ENVIRONMENT: Called by setup ()
 *
 * (NOTE:)
 * 
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	NONE 
 */
#define PIOMSGCAT "kfmtrs.cat"
#define PIOMSGSET 1

#if defined( PIOTRANS )
void errorexit2(msgnum, stringval)
#else /* PIOTRANS */
void errorexit (msgnum, stringval)
#endif /* PIOTRANS */
int msgnum;
char *stringval;
{
    char defmsg[500];
    char msgbuf[1000];
	static nl_catd	md = -1;
	struct _stdefmsg *p = Stdefmsg;

	if( md == -1 ){	/* message catalog file must not changed in a process */
		md = catopen( PIOMSGCAT, 0 );
	}

	for( ; p->mid != 0 && p->defmsg != NULL; p++ ){
		if( p->mid == msgnum ) break;
	}

	if( p->mid == 0 || p->defmsg == NULL )	p = Stdefmsg;

	

    (void) sprintf ( msgbuf, 
			catgets( md, PIOMSGSET, msgnum, p->defmsg ),
		     stringval
		   );
    (void) piomsgout (msgbuf);
    if (piodatasent && Do_formfeed)
	piocmdout (FF_CMD, NULL, 0, NULL);
    pioexit (PIOEXITBAD);
}

#if defined( PIOTRANS )
#define	PIOMSGCAT0	"piobe.cat"
#define	PIOMSGSET0	1

void errorexit(msgnum, stringval1, stringval2, stringval3)
int msgnum;
char *stringval1, *stringval2, *stringval3;
{
char msgbuf[1000];
extern char *getmsg( char *Catname, int Gid, int Mid );

(void) sprintf(
        msgbuf, getmsg(PIOMSGCAT0, PIOMSGSET0, msgnum),
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
char *getmsg(CatName, set, num)
char *CatName;  
int set;
int num;
{
static char *msgbuf = NULL;
static nl_catd catd;
static nl_catd def_catd;
static char save_name[100];

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
#endif
