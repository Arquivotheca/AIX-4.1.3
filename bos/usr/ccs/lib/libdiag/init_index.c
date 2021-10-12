static char sccsid[] = "@(#)45	1.12.2.5  src/bos/usr/ccs/lib/libdiag/init_index.c, libdiag, bos411, 9428A410j 4/20/94 13:40:12";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS: 	file_present
 *		read_diskette
 *		chkdskt
 *		restore_files
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ustat.h>
#include <dirent.h>

/* FUNCTION PROTOTYPES */
int file_present(char *);
int read_diskette(char *);
char *chkdskt(char *);
int restore_files(char *);

/* NAME: file_present
 *
 * FUNCTION: Determines if a file is present.
 * 
 * NOTES: Issues a stat() system call to get the status of a file
 *	  Use device ID from stat() system call as input to ustat()
 *		system call to get file system statistics.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	 0 - file is NOT present
 *	 1 - file is present
 */


int file_present(char *fname)
{
	int	rc;
	struct	stat	buf;
	struct	ustat	ubuf;

	if ( (rc = stat(fname, &buf)) == -1 )
		return (0);
	return(1);
}

/*   */ 
/* NAME: read_diskette
 *
 * FUNCTION: Reads a diskette
 * 
 * NOTES: 
 *	  Invokes /etc/rmfiles to remove files
 *	  Invokes cpio to read diskette
 *	  Run diagstartX script 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	-1 - failure
 *	 0 - success
 */

int read_diskette(char *current_volume)
{
	char param[64];
	char buffer[128];
	char	volume[4];
	int	rc;
	FILE *fd;
	char *outb=NULL;
	char *ebuf=NULL;

	/* read newly inserted diskette header */
	sprintf(volume ,"%s", chkdskt("DIAG"));

	/* remove files from previous diskette */
	sprintf( param, "%s", current_volume );

	if (odm_run_method("/etc/rmfiles", param, &outb, &ebuf))
		return(-1);

	strcpy(buffer,"-iduC36 </dev/rfd0");

	setleds(0xa09);

	rc=odm_run_method("cpio", buffer, &outb, &ebuf);

	setleds(0xfff);

	if(rc < 0)
		return(-1);

	sprintf(buffer, "/etc/diagstart%s 1>/dev/null 2>&1", volume);
	system(buffer);

	return(0);
}

/*   */ 
/* NAME: chkdskt
 *
 * FUNCTION: check the first block of diagnostic diskette for the diskette
 *	     volume
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *       0      The diskette was read but the volume could not be identified.
 *       2      Volume 2 is in the drive.
 *       3A     Volume 3A is in the drive.
 *       3B     Volume 3B is in the drive.
 *       .      Volume . is in the drive.
 *       9      Volume 9 is in the drive.
 *       S      Diskette is a Supplemental Diagnostic Diskette.
 *       U      Diskette is an Update Diagnostic Diskette.
 *       B      Bad diskette.
 *       E      The diskette drive is empty.
 */
char *chkdskt(char *string)
{
#define BUFFSIZE 512
	extern int errno;
	char	buff[BUFFSIZE+1], *ptr;
	int 	i, fdes;
	static char diag_idx[5];

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
					if (strpbrk(ptr, "23456789SU")) {
						memset(diag_idx, 0, 5);
						strcpy(diag_idx,ptr);
						close(fdes);
						return(diag_idx);
					}
				}
				ptr = (char *)strtok( NULL, " \n" );
			}
			close(fdes);
			return("0");
		}
		close(fdes);
		return("B");
	}
	else {
		switch(errno){
		case ENOTREADY : 
			close(fdes);
			return("E");
		default :
			close(fdes);
			return("B");
		} 
	}
}

/*   */ 
/* NAME: restore_files
 *
 * FUNCTION: Run the script to restore files needed to execute a Diagnostics
 *	     application program or a service aid.
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *
 */

 int
 restore_files(char *volume)
 {
	char param[64];
	int  rc=0;
	char buffer[64];
	char *outb=NULL;
	char *ebuf=NULL;

	/* link new files */
	sprintf( param, "%s REMOVE", volume );
	rc=odm_run_method("/etc/restore_files", param, &outb, &ebuf);
	return(rc);
 }
