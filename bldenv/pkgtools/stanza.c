static char sccsid[] = "@(#)10  1.1  src/bldenv/pkgtools/stanza.c, pkgtools, bos412, GOLDA411a 2/12/93 12:22:06";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: readStanza
 *              getOpt
 *              getEntry
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "stanza.h"

/*-----------------------------------------------------------------------------
| readStanza reads each stanza from the input file and saves the stanza in    |
| the buffer.                                                                 |
| The format of stanza is:						      |	
|	object name:							      |
|		<attribute> =						      |	
|		<attribute> =						      |
|		...							      |
-----------------------------------------------------------------------------*/
int readStanza(FILE *fp, char *buffer, int stanzaLen)
{
    int newlines = 0;
    int index = 0;
    int rc = 0;
    char c;
 
    /* Each stanza is separated by two new line characters.   */
    /* TERMINATOR contains the new line character number      */
    /* and is defined in the stanza.h file.		      */
    while ( (newlines != TERMINATOR) && (index < stanzaLen) )
    {
        /* Get one character from the input file. */
        c = getc(fp);
       
        if (feof(fp))
        {
           rc = EOF;
           newlines = 2;
        }
        else
        {
           /* Store character got from the file  */
           /* to the buffer.                     */
           buffer[index++] = c;
           if (c == '\n')
              newlines += 1;
           else
              newlines = 0;
        }
     }

     if ( index < stanzaLen )
	buffer[index] = '\0';  
     else
     {
	buffer[--index] = '\0';
	rc = OVERFLOW;
     }
     return (rc);
}


/*-----------------------------------------------------------------------------
| getOpt gets the option name from the "class =" attribute in the stanza      |
-----------------------------------------------------------------------------*/ 
char *getOpt(char *buffer)
{
        char    value[LENSIZE];          
        char    *token;                 
        int     rc;

        value[0] = NULL;

        /* The format of the "class =" attribute is:         */
        /*    class = apply,inventory,<option>               */
        /*                                                   */
        /* Get the value of "class" atrribute(For this       */
        /* example is "apply,inventory,<option>" and parse   */
        /* option from this value.                           */ 
        if ( !getEntry("class", buffer, value)) 
        {
            token = strtok(value, ",");
            while (token != NULL) 
            {
                if (strcmp("apply", token) &&
                strcmp("update", token) &&
                strcmp("inventory", token))
                    return(token);
                token = strtok(NULL, ",");
            }
        }
        
        /* Error: cannot get the options from the "class"     */   
        /* attribute or the "class" attribute is not in the   */
        /* stanza.                                            */
        fprintf (stderr, "ERROR: For the stanza:\n%s", buffer);
        fprintf (stderr, "It does not contain option value in the \"class\"\n");
        fprintf (stderr, "attribute or the \"class\" attribute is not in\n");
        fprintf (stderr, "the this stanza.\n");
        return(NULL);
}      


/*-----------------------------------------------------------------------------
| getEntry gets the value of the specified attribute from the stanza.         |
-----------------------------------------------------------------------------*/
int getEntry(char *keyword, char *buffer, char *value)
{                     
    int rc = 0;      
    int len;              
    int match = 1; 	/* Not match; cannot find the keyword. */ 	 
    int i;             
    char *ptr;

    len = strlen (keyword);  
    ptr = buffer;               

    /* Skip the first line in the stanza.              */
    /* The first line contain the object name.         */
    ptr = strchr (ptr,'\n');
    /* ptr points to the first character of the second */
    /* line in the stanza.                             */
    ptr++;                 

    while ((*ptr != '\0') && (match == 1))
    {
        /* Skip the special characters or the blank */
        while ((*ptr <= ' ') && (*ptr != '\n') && (*ptr != '\0')) ptr++;

        if ((*ptr != '*') && (*ptr != '\n') && (*ptr != '\0'))
        {
           if (strncmp(ptr, keyword, len) == 0)
           {
              /* Find the keyword in the stanza.       */
              i = 0;
              /* Skip the keyword.                    */
              while ((i < len) && (*ptr != '\0')) 
              { 
                  ptr++; 
                  i++;
              }

              if ((*ptr == '=') || (*ptr == ' '))
              {
                 match = 0; 	/* Find the value */ 
                 /* Skip the characters " = "           */ 
                 while ((*ptr == ' ') || (*ptr == '=')) ptr ++;

                 i = 0;
                 while ((*ptr != '\n') && (i < LENSIZE))
                 {
                     /* Save the value */
                     value[i++] = *ptr;
                     ptr++;
                 }
                 /* Check if the value length excedess the */
                 /* maximum length capacity.               */
                 if (i < LENSIZE)
                    value[i] = '\0';     
                 else /* i = LENSIZE */
		 {
                    value[--i] = '\0';  
		    return (OVERFLOW);
		 }
              }  
           }
        }
        if (*ptr != '\0')
        {
           /* Go to the next line of the stanza.         */
           ptr = strchr(ptr, '\n');
           ptr++;
        }
    }

    return (match);
}
