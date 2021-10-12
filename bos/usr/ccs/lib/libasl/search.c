static char sccsid[] = "@(#)67  1.2  src/bos/usr/ccs/lib/libasl/search.c, libasl, bos411, 9428A410j 4/21/94 15:22:22";

/*
 *   COMPONENT_NAME: CMDMSMIT
 *
 *   FUNCTIONS: ERROR
 *		GETC
 *		PEEKC
 *		RETURN
 *		UNGETC
 *		search_string
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#ifdef AIX3.1
/* macro definitions needed by the NLregexp.h functions                      */
#define INIT            register char *sp=instring;
#define GETC()          (*sp++)
#define PEEKC(c)        (*sp)
#define UNGETC(c)       (--sp)
#define RETURN(c)       return;
#define ERROR(c)        printf("Error %d\n",c);

#include <regexp.h>
#include <Xm/MainW.h>

#else

#include <regex.h>
#endif

/****************************************************************************/
/* Name     : search_string (str, pattern)                                  */
/*                                                                          */
/* Function : This function will call the regular expression functions:     */
/*              compile                                                     */
/*              step                                                        */
/*            which will locate the 'pattern' in the 'str'.                 */
/*                                                                          */
/* Input    : str = character string to be searched                         */
/*            pattern = the regular expression pattern to locate            */
/*                                                                          */
/* Returns  : NULL = if no pattern exists                                   */
/*            tmp_string = located pattern                                  */
/*                                                                          */
/****************************************************************************/
char*
search_string (str, pattern)
char *str;      /* String to search in */
char **pattern; /* Pattern to look for */
{
#ifdef AIX3.1
  char expression_buffer[1024];
  char *tmp_string = NULL;


/* AIX3.1 is used for the 3.1 level regular expression code. */

  compile (*pattern, expression_buffer, expression_buffer + 1023, '\0');

  if (step (str, expression_buffer))
  {
      *pattern = loc1;
      return (loc2);
  }

  return (NULL);
#else
  regex_t preg;   /* regular expression structure */
  int regstat;    /* return code from regular expression call */
  regmatch_t loc2;

  regstat = regcomp(&preg, *pattern, REG_ICASE) ;

  loc2.rm_so = 0;
  loc2.rm_eo = 0;
  regstat = regexec (&preg, str ,(size_t) 1, &loc2, 1);
  regfree(&preg);     /* added in defect 145366 */
  if (regstat == 0)
  {
        *pattern = &str[loc2.rm_so];
        return (&str[loc2.rm_eo]);
   }
   else   /* if didn't find anything */
        return (NULL);
#endif
} /* search () */
