static char sccsid[] = "@(#)79	1.1  src/bos/diag/boot/chkdskt.c, diagboot, bos411, 9428A410j 2/22/94 10:11:25";
/*
 * COMPONENT_NAME: DIAGBOOT
 *
 * FUNCTIONS: 	Read the diskette and returns the Label.
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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* GLOBAL VARIABLES */
#define BUFFSIZE 512

/* EXTERNALLY DEFINED VARIABLES	*/
extern int errno;
extern int optind;
extern char *optarg;

/* EXTERNAL ROUTINES */

/*
 * NAME: main
 *                                                                    
 * FUNCTION: Identifies the diskette volume by looking in the first
 *           block for the specified string ( -s string ).  The volume
 *           is identified by the next character(s).
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This program executes as an application of the kernel.  It
 *      executes under a process, and can page fault.
 *                                                                   
 * NOTES:
 *
 *      chkdskt [-s string] 
 *
 *      -s string	String to search for with the first block  
 *			of data from /dev/rfd0
 *
 * RETURNS:
 *       0      The diskette was read but the volume could not be identified.
 *       2      Volume 2 is in the drive.
 *       3      Volume 3 is in the drive.
 *       .      Volume . is in the drive.
 *       9      Volume 9 is in the drive.
 *	 S	Supplemental diagnostic diskette.
 *	 U	Update diagnostic diskette.
 *       B      Bad diskette.
 *       E      The diskette drive is empty.
 *       N      No option specified.
 */

main (argc, argv)
int argc;
char *argv[];
{
	char	*string=NULL, buff[BUFFSIZE+1], *ptr;
	int 	c, i, fdes;

	while((c = getopt(argc,argv,"s:")) != EOF) {
		switch(c) {
		case 's':
			string = optarg;
			break;
		}
	}

	if (string == NULL) {
		printf("N");
		exit(1);
	}

	if ((fdes = open("/dev/rfd0", 0)) != -1){
		if (read(fdes, buff, BUFFSIZE) == BUFFSIZE){
			for (i=0; i<BUFFSIZE; i++){
				if (buff[i] == (char) NULL)
					buff[i] = ' ';
			}
			buff[BUFFSIZE] = (char) NULL;
			ptr = (char *)strtok( buff, " \n" );
			while( ptr != NULL ){
				if ( !strcmp(ptr, string) ){
					ptr = (char *)strtok( NULL , " \n" );
					if (strpbrk(ptr, "23456789SU"))
						printf("%s", ptr);
						exit(0);
				}
				ptr = (char *)strtok( NULL, " \n" );
			}
			printf("0");
			exit(1);
		}
		printf("B");
		exit(1);
	}
	else {
		switch(errno){
		case ENOTREADY : 
			printf("E");
			exit(1);
			break;
		default :
			printf("B");
			exit(1);
			break;
		} 
	}
}
