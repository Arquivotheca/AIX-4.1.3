static char sccsid[] = "@(#)51	1.2  src/bos/usr/lib/piosCN/fmtrs/defmsg.c, ils-zh_CN, bos41J, 9512A_all 3/14/95 10:11:52";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: errorexit
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <nl_types.h>
#include <piostruct.h>

#if defined(EUC)
#include "cfmtrs_msg.h"
#define PIOMSGCAT "cfmtrs.cat"
#elif defined(UTF8)
#include "ufmtrs_msg.h"
#define PIOMSGCAT "ufmtrs.cat"
#endif

#define PIOMSGSET 1

extern int piodatasent;
extern int piomode;
extern int *_Do_formfeed;
#define Do_formfeed (*(_Do_formfeed + piomode))  /* variable name */

#if !defined(FF_CMD)
#define FF_CMD "af"
#endif

static struct _stdefmsg {
	int	mid;
	char	*defmsg;
} Stdefmsg[8] = {

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

{ MSG_BADFORMFLAG8 ,
"0782-508 The parameter value specified with the -%.1s flag is not valid.\n\
\tThe parameter value must be less than the page width.\n\
\tCheck the parameter value.\n" },

{ MSG_BADUSERFONT1 ,
"0782-708 Cannot access the specified user-defined font file %s.\n" },

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

void errorexit (msgnum, stringval)
int msgnum;
char *stringval;
{
    char defmsg[500];
    char msgbuf[1000];
	static nl_catd	md = (nl_catd)-1;
	struct _stdefmsg *p = Stdefmsg;

	if( md == (nl_catd)-1 ){	/* message catalog file must not changed in a process */
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
