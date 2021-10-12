static char sccsid[] = "@(#)65        1.2  src/bos/usr/bin/errlg/liberrlg/validate.c, cmderrlg, bos411, 9428A410j 2/16/94 13:30:18";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: valid_class, valid_type
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errlg.h>


/*
 * NAME:     valid_class
 * FUNCTION: Validate the provided string as an error class.
 *			 Valid classes are: H,S,O,U.
 * RETURNS:  0 failure
 *           1 success
 */

int
valid_class(char * class)
{
	int	rc=TRUE;
	char *token;
	char *str;

	if (class == NULL)
		rc = FALSE;
	else if ((str=malloc(strlen(class)+1)) == NULL)
		rc = FALSE;
	else if (strcpy(str,class) == NULL)
		rc = FALSE;
	else if((token=strtok(str,", ")) != NULL)
	{
		do
		{
			if (strcmp(token,"H") == 0)
				;
			else if (strcmp(token,"S") == 0)
				;
			else if (strcmp(token,"O") == 0)
				;
			else if (strcmp(token,"U") == 0)
				;
			else
			{
				rc = FALSE;
				break;
			}
		}
		while((token=strtok(NULL,", ")) != NULL);
	}
	else
		rc = FALSE;

	if (rc == FALSE)
	{
		cat_error(CAT_BAD_ERR_CLASS,
"Bad value supplied for error class: %s\n\
Rerun the command with a valid error class.\n",class);
	}
	if ( str != NULL)
		free(str);

	return (rc);
}

/*
 * NAME:     valid_type
 * FUNCTION: Validate the provided string as an error type.
 *			 Valid types are: PERM,TEMP,PERF,PEND,UNKN,INFO
 * RETURNS:  0 failure
 *           1 success
 */

int
valid_type(char * type)
{
	int	rc=TRUE;
	char *token;
	char *str;

	if (type == NULL)
		rc = FALSE;
	else if ((str=malloc(strlen(type)+1)) == NULL)
		rc = FALSE;
	else if (strcpy(str,type) == NULL)
		rc = FALSE;
	else if((token=strtok(str,", ")) != NULL)
	{
		do
		{
			if (strcmp(token,"PERM") == 0)
				;
			else if (strcmp(token,"TEMP") == 0)
				;
			else if (strcmp(token,"PERF") == 0)
				;
			else if (strcmp(token,"PEND") == 0)
				;
			else if (strcmp(token,"UNKN") == 0)
				;
			else if (strcmp(token,"INFO") == 0)
				;
			else
				rc = FALSE;
		}
		while((token=strtok(NULL,", ")) != NULL);
	}

	if (rc == FALSE)
	{
		cat_error(CAT_BAD_ERR_TYPE,
"Bad value supplied for error type: %s\n\
Rerun the command with a valid error type.\n",type);
	}
	if ( str != NULL)
		free(str);

	return (rc);
}

/*
 * NAME:     valid_keyword
 * FUNCTION: Validate the provided string as an error template keyword. 
 *           Valid error template keywords are: report, log, alert 
 *           Valid error template keyword values are:  0, 1
 * RETURNS:  0 failure
 *           1 success
 */

int
valid_keyword(char * input)
{
        int     rc=TRUE;
        char *token;
	char *keyword;
	char *value;
        char *str;
	int  bad_keyword = 0;
	int  bad_value = 0;

        if (input == NULL)
                rc = FALSE;
        else if ((str=malloc(strlen(input)+1)) == NULL)
                rc = FALSE;
        else if (strcpy(str,input) == NULL)
                rc = FALSE;
        else if((token=strtok(str,", ")) != NULL)
        {
                do
                {
		if ((value=strchr(token,'=')) != NULL)
			{
			value=value+1;
                        if (strcmp(value,"1") == 0)
                                ;
                        else if (strcmp(value,"0") == 0)
				;
			else
				{
				bad_value = 1;
				rc = FALSE;
				break;
				}
			}
		value = value - 1;
		*value = '\0';
                if (strcasecmp(token,"alert") == 0)
                        ;
                else if (strcasecmp(token,"log") == 0)
                       ;
                else if (strcasecmp(token,"report") == 0)
                       ;
                else
			{
			bad_keyword = 1;
			rc = FALSE;
			break;
			}	
                }
                while((token=strtok(NULL,", ")) != NULL);
        }

        if ((rc == FALSE) && bad_keyword)
        {
                cat_error(CAT_BAD_ERR_KEYWORD,
"Bad value supplied for error template keyword:  %s\n\
Valid keyword values are:  Alert, Report, Log\n\
Rerun the command with a valid error template keyword.\n",input);
        }

        if ((rc == FALSE) && bad_value)
        {
                cat_error(CAT_BAD_ERR_KEYWORD_VALUE,
"Bad value supplied for error template keyword value:  %s\n\
Valid error template values for keywords Alert, Log, and\n\
Report are '0' and '1'.  Rerun the command with a valid error\n\
template keyword value.\n",input);
        }

        if ( str != NULL)
                free(str);

        return (rc);
}

