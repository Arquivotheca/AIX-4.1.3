static char sccsid[] = "@(#)00	1.2  src/bos/usr/lib/nls/loc/jim/jkkc/OpenFile.c, libKJI, bos411, 9428A410j 8/16/93 21:39:00";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:		OpenFile
 *
 * DESCRIPTIVE NAME:	Open File. File descriptor is not 0, 1, and 2.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:		file descriptor (int)
 *
 ******************** END OF SPECIFICATIONS *****************************/

#include <stdio.h>
#include <fcntl.h>
/************************************************************************
 *	Open File
 ************************************************************************/
int	OpenFile
(
char	*file,				/* file name string		*/
int	mode				/* file open mode		*/
)
{
	int	i, j, fd[4];

	if(file == NULL || *file == '\0'){
		return(-1);
	}
	
	if((fd[0] = open(file, mode)) == -1){
		return(-1);
	}else if(fd[0] > 2){
		return(fd[0]);
	}else{
		for(i = 1; i < 4; i++){
			if((fd[i] = dup(fd[0])) == -1){
				return(-1);
			}else if(fd[i] > 2){
				for(j = 0; j < i; j++){
					close(fd[j]);
				}
				fcntl(fd[i], F_SETFD, 1);
				return(fd[i]);
			}
		}
		return(-1);
	}
}
