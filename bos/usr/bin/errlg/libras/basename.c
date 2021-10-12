static char sccsid[] = "@(#)82 1.2  src/bos/usr/bin/errlg/libras/basename.c, cmderrlg, bos411, 9428A410j 10/7/93 14:39:05";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: basename, dirname
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

/*
 * NAME:      basename
 * FUNCTION:  return a pointer to the "basename" of the string.
 *            The basename is the last part of the filename (minus directories)
 * INPUT:     'str' character pointer to filename.
 * RETURNS:   pointer within 'str' to the basename.
 */

extern char *strrchr();

char *basename(str)
char *str;
{
	char *cp;
 
	if(cp = strrchr(str,'/'))
		cp++;
	else
		cp = str;
	return(cp);
}

char *dirname(str)
char *str;
{
	int len;
	char *cp;
	static char buf[256];
 
	if(cp = strrchr(str,'/'))
		{
		len = cp - str;
		if (len == 0)
               		{
                	buf[0] = '/';   
                	buf[1] = '\0';
                	return(buf);
                	}
		}
	else
		len = strlen(str);
	memcpy(buf,str,len);
	buf[len] = '\0';
	return(buf);
}

