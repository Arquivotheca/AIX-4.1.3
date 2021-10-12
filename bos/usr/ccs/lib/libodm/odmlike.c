static char sccsid[] = "@(#)88  1.5  src/bos/usr/ccs/lib/libodm/odmlike.c, libodm, bos411, 9428A410j 10/26/93 09:21:26";

/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: cmpkmch
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*********************************************************************/
/*                                                                   */
/* MODULE NAME    = cmpkmch                                          */
/*                                                                   */
/*                                                                   */
/* DESCRIPTIVE NAME = String matching                                */
/*                                                                   */
/*                                                                   */
/* FUNCTION = String matching                                        */
/*    This function matches a wildcard pattern against a string.     */
/*    The wild card symbols are:                                     */
/*    *     matches any string, including the null string.           */
/*    ?     matches any single character.                            */
/*    [...] matches any one of the enclosed characters.  A pair of   */
/*          characters separated by - matches any character between  */
/*          and including the pair.  If the first character in the   */
/*          brackets is a !, the match is on any characters not in   */
/*          the bracketed range.  See sh(1) for more information.    */
/*                                                                   */
/*                                                                   */
/* INPUT = pattern - The pointer to the partial key                  */
/*         string  - The pointer to the key string to be matched     */
/*                                                                   */
/* OUTPUT = none                                                     */
/*                                                                   */
/* NORMAL RETURN =  False = 0 - no match                             */
/*                  True  = 1 - match                                */
/*                                                                   */
/*********************************************************************/
#include <sys/types.h>
#include <odmi.h>
#include <stdlib.h>
#include "odmlib.h"
#include "odmhkids.h"
#include "odmtrace.h"

#define PERFTRC(x,y)
#ifdef HOOKR5A
#include <sys/dynaprobe.h>
#include <dmstrace.h>
extern int dynaflag;
#endif
#ifdef HOOKV3
#include <sys/trchkid.h>
#include <dmstrace.h>
#endif

int  cmpkmch ( pattern, string )

register char   *pattern;            /* WILDCARD PATTERN TO BE USED */
register char   *string;             /* STRING TO BE MATCHED        */

{

    register int  strcur;                /* CURRENT STRING CHARACTER      */
    register int  patcur;                /* CURRENT WILDCARD PATTERN CHAR */
    register int  retcode = FALSE;
    register int  notflag;
    int  lowchar;                        /* LOW CHAR IN [.-.] SEARCH      */
    int  highchar;                       /* HIGH CHAR IN [.-.] SEARCH     */
    int  looping;


    PERFTRC(USERID,DYNCMPKMCH)
    START_ROUTINE(ODMHKWD_CMPKMCH);
    TRC("cmpkmch","Looking in %s",string,"for %s",pattern);
    if (pattern == NULL || string == NULL)
      {
        TRC("cmpkmch","NULL value(s)!","","","");
        odmerrno = ODMI_BAD_CRIT;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    /* DETERMINE WHAT TYPE OF CHARACTER WE ARE MATCHING AGAINST */
    switch (*pattern)   {

        /* CHECK FOR AN OPENING BRACKET */
    case '[':

        /* GET THE CURRENT STRING CHARACTER TO MATCH. MASK TO 8 BITS */
        strcur = *string;
        strcur &= 0377;

        /* GET OUT IF TRYING TO MATCH AGAINST THE NULL-TERMINATOR */
        if (strcur == 0)
            break;

        pattern++;
        string++;

        /* THE LOWEST CHARACTER IS INITIALLY SET LOWER THAN ANY POSSIBLE CHAR */
        lowchar = -1;

        /* CHECK FOR ! */
        if (*pattern == '!')   {
            notflag = TRUE;
            pattern++;
          }
        else
            notflag = FALSE;

        looping = TRUE;

        /* LOOP UNTIL A ] IS FOUND */
        while (looping)   {
            patcur = *pattern++;
            patcur &= 0377;

            switch (patcur)   {
            case ']':

                /* IF THE ! WAS USED, SWAP THE RESULT OF THE MATCH. */
                if (notflag)   {
                    if (retcode)
                        retcode = FALSE;
                    else
                        retcode = TRUE;
                  }

                /* TRY TO MATCH THE REST OF THE STRING AFTER THE [...]       */
                /* IF WE HAVE SUCCESSFULLY MATCHED THE PART IN THE BRACKETS. */
                if (retcode)
                    retcode = cmpkmch (pattern, string);
                looping = FALSE;
                break;

            case '-':
                highchar = *pattern++;
                highchar &= 0377;

                /* CHECK TO SEE IF THE CHARACTER IS WITHIN THE RANGE */
                if ((strcur >= lowchar) && (strcur <= highchar))
                    retcode = TRUE;
                break;

            case 0:

                /* IF THE END OF THE PATTERN IS ENCOUNTERED BEFORE ], THEN ERROR */
                retcode = FALSE;
                looping = FALSE;
                break;

            default:

                /* GET THE LOW CHARACTER FOR FUTURE USE */
                lowchar = patcur;
                if (strcur == patcur)
                    retcode = TRUE;
                break;
              }
          }
        break;

    default:

        /* PATTERN AND CURRENT STRING CHARACTER MUST MATCH EXACTLY */
        if (*pattern != *string)
            break;

        /* IF THE CURRENT PATTERN CHARACTER MATCHES THE CURRENT STRING */
        /* CHARACTER, FALL THROUGH AS IF A MATCH ON ANY                */
        /* SINGLE CHARACTER HAD OCCURRED.                              */
	if (MB_CUR_MAX != 1 && *string != '\0')
	{
		int len, len2;
		len=mblen(pattern,MB_CUR_MAX);
		len2=mblen(string,MB_CUR_MAX);
		if(len != len2 || strncmp(string,pattern,len)!=0)
			break;
            	pattern+=len;
                string+=len;
                retcode = cmpkmch (pattern, string);
		break;
	}

    case '?':
        if (*string != '\0')   {
            pattern++;
            string++;
            retcode = cmpkmch (pattern, string);
          }
        break;

    case '*':

        /* AN * MATCHES ZERO OR MORE CHARACTERS.                  */
        /* IF THERE ARE MULTIPLE ASTERISKS, REMOVE THE DUPLICATES */
        while (*pattern == '*')
            pattern++;

        /* IF THERE ARE NO MORE CHARACTERS IN THE PATTERN, THEN SUCCESSFUL. */
        if (*pattern == '\0')
            retcode = TRUE;
        else   {
            while (*string != '\0')   {
                if (retcode = cmpkmch (pattern, string))
                    break;
                string++;
              }
          }
        break;

    case '\0':

        /* THE PATTERN HAS TERMINATED. */
        if (*string == '\0')
            retcode = TRUE;
        break;
      }

    TRC("cmpkmch","Returning %d",retcode,"","");
    STOP_ROUTINE;
    PERFTRC(USERID,-DYNCMPKMCH)

        return(retcode);

}
