static char sccsid[] = "@(#)65  1.2.1.5  src/bos/usr/bin/license/chlicense.c, cmdlicense, bos411, 9430C411a 7/21/94 17:00:43";
/*
 * COMPONENT_NAME: CMDLICENSE
 *
 * FUNCTIONS: main
 *
 * ORIGIN: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <strings.h>
#include <sys/license.h>
#include "license_msg.h"

#define FLOAT_ON        "on"
#define FLOAT_OFF       "off"

extern char *optarg;             /* for getopt() */
extern char *command_name;
extern nl_catd scmc_catd;       /* for messages */

extern  int     add_netls_daemon(void);
extern  int     remove_netls_daemon(void);
extern  int     create_netls_file(void);
extern  int     delete_netls_file(void);

/*
 * NAME: chlicense (main)
 *
 * FUNCTION: Changes the number of fixed licenses and the status of
 *           floating licensing.
 *
 * NOTES:
 *        This program can be invoked as:
 *
 *                chlicense -uLicenses -fStatus
 *
 *        where the 'Licenses' value is the maximum number of fixed licenses.
 *        This value should be positive. 
 *
 *        where the "Status" value is either "on" or "off" indicating the
 *        status of floating licensing on the system.
 *
 *
 * RETURNS: 0 if command completes successfully. A positive value
 *          is returned if an error is encountered.
 *
 */
main (int argc, char *argv[], char *envp)
{

	int flag;                   /* Flag from getopt                 */
	int number_of_users = 0;    /* Max. number of users             */
	char *error_location;       /* If error converting the string   */
	int uflg = 0;	            /* Flag for number of users	        */
	int fflg = 0;	            /* Flag for licensing mode	        */
	int float_on = 0;	    /* Set floating status to on        */
	int float_off = 0;	    /* Set floating status to off       */

#ifndef R5A
	setlocale(LC_ALL, "");
	scmc_catd = catopen(CATFILE, NL_CAT_LOCALE);
#endif

	command_name = argv[0];
   	while ((flag = getopt(argc,argv,"u:f:")) != EOF)
	{
		switch (flag)
         	{
           	    case 'u':
	                /*
			 * Get number of fixed licenses. Must be a 
			 * integer greater than 0.
			*/
			uflg++;
                	number_of_users = strtol(optarg,&error_location,0);
                        if (*error_location != '\0' || number_of_users < 1 || number_of_users > 32767)
                  	{
                      	     fprintf(stderr,catgets(scmc_catd,CHANGE_SET,
		             CHANGE_NUM, "%s: The number of fixed licenses must be an integer between 1 and 32767.\n"), command_name);
                      	     exit (1);
                  	}
             		break;

	   	    case 'f':
			fflg++;
                	if (strcmp(optarg, FLOAT_ON) &&                                                     strcmp(optarg, FLOAT_OFF))
        		{
                	     fprintf(stderr, catgets(scmc_catd, CHANGE_SET,                                  CHANGE_FLOAT, "%s: The floating licensing status must be either \"on\" or \"off\".\n"), command_name);
                	     exit (1);
        		}
			if (!strcmp(optarg, FLOAT_ON))
		    	     float_on++;
			else
		             float_off++;
	        	break;

                    default:
       			/*
         		 * Print out the correct command syntax
       			*/
       			fprintf(stderr,catgets(scmc_catd,CHANGE_SET,
                        CHANGE_USAGE, "Usage: chlicense -uUsers -fStatus\n\
\tChanges the number of fixed licenses and the floating licensing status.\n"));
       			exit(1);
         	} /* endswitch */
	} /* endwhile */

	/*
        * Process the -u option and set the number of fixed licenses.
        */
	if (uflg)
  	if (change_max_users(number_of_users) < 0)
       		exit(2);

	/*
        * Process the -f option and set the status of the floating licensing.
        */
 	if (fflg)
        if (float_off)
        {
                /*
                 * -off:  (Turn floating licensing off.)
                 * Delete /etc/security/.netls and remove the iFOR/LS
                 * daemon from /etc/inittab.
                 */
                if (delete_netls_file() || remove_netls_daemon())
                        exit (2);
        }
        else
        {
                /*
                 * -on:   (Turn floating licensing on.)
                 * Add the iFOR/LS daemon to /etc/inittab and create
                 * /etc/security/.netls.
                 */
                if (add_netls_daemon() || create_netls_file())
                        exit (2);

                /*
                 * Start the monitord daemon.
                 */
                system(MONITORD_PROGRAM);
        }
	/*
	* If number of fixed licenses changes, print message to reboot
	* The system must be rebooted in order for the new # fixed licenses
        * to take effect.
	*/
	if (uflg)
		fprintf(stderr,catgets(scmc_catd,CHANGE_SET,CHANGE_REBOOT,                          "%s: The system must be rebooted before the new number of\n\
\t   fixed licenses will take effect.\n"), command_name);
		
  	exit(0);
}
