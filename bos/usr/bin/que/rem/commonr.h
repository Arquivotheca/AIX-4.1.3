/* @(#)77	1.12  src/bos/usr/bin/que/rem/commonr.h, cmdque, bos411, 9428A410j 1/30/93 09:42:28 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <stdio.h>
#include "common.h"

char  *sconcat(), *scopy(), *ralloc();

#define MAXLPDFILES 255

extern char	**saveArgv;

#ifdef PROTO

/*

BSD LPR PROTOCOL

[routine_name]	The BSD lpr/lpd routine names in brackets are who is writing 
		the data to the socket.
ack		Means a null byte was written to the socket. (write(skfd,"",1);

** note that only simple requests, (i.e. requests 1 and 2, which have no user
   names or job numbers associated with them,) require an ack from the server.

--------- --------- --------- --------- --------- --------- --------- ---------
client (lpr on request 1, lpd on all other requests)	server(lpd)
--------- --------- --------- --------- --------- --------- --------- ---------
*startjob
	[startdaemon]
	1printername\n	---->
						[printjob]
						<--- ack
--------- --------- --------- --------- --------- --------- --------- ---------
*sendjob (the #size requests handled by sendfile implement a sub-protocol)

	[openpr]
	2printername\n	---->
						[readjob]
						<--- ack
	[sendfile]
|------>3size dfA000host\n ---->
|						[readfile]
|						<--- ack
|	[sendfile]
|	data to prt ---->
|	[sendfile]
|	ack ---->
|						[readfile]
|						<--- ack
(go back to 3 if more data files in this request)
 	[sendfile]
	2size cfA000host\n ---->
 						[readfile]
						<--- ack
 	[sendfile]
	control file for job---->
 	[sendfile]
 	ack ---->
 						[readfile]
						<--- ack
--------- --------- --------- --------- --------- --------- --------- ---------
*qstatus short
*qstatus long

	[displayq]
	(will accept)
	3printername username ### ### username\n ---->
or
	(actually sent)
	4printername ### ### username username\n ---->
or
	(actually sent)
	3printername ### ### username username\n ---->
or
	(will accept)
	4printername ### username ### username\n ---->
							[displayq]
							<-----	qstatus data
								or errors
--------- --------- --------- --------- --------- --------- --------- ---------
*cancel a job

	[rmjob]
	(will accept)
	5printername ### username ### username\n ---->
or
	(actually sent)
	5printername username username ### ###\n ---->
or
	(actually sent)
	5printername -all\n ---->
							[rmjob]
							<-----	error string
								     or
							<-----	exit(pipe dies)
--------- --------- --------- --------- --------- --------- --------- ---------


The control file that is sent across might look like this.

Hmy_host		Host name job is from
Pme			User name job is from
Jmy_job			Job name
Cmy_host		Job classification
Lme			Log name 
I5			Indent
Mme			Mail message upon comletion of job to...
2font			Load font
W77			Width 
Tmy_title		Title                             \
pdfA013my_host		Use pr on 1st copy of file        |
pdfA013my_host		Use pr on 2nd copy of file        |---- for 1st file
UdfA013my_host		Delete file after printing        |
N/etc/motd		What this file really was called  /
Tmy_title		                                  \ 		 
pdfB013my_host		                                  |
pdfB013my_host		                                  |---- for 2nd file
UdfB013My_host		                                  |
N/etc/passwd						  /
-N2			AIX Number of copies              \
-Zme@my_host		AIX Job is from...                |
-tyou@this_host		AIX Who job is for                |
-Tmy_title		AIX Title of job                  |---- AIX options 
-Bga			AIX Burst Options                 |     3.1 ===> 3.1 only
-n			AIX Notify sender when done       |
-m"pink paper"		AIX Operator message              /
*/

#endif PROTO
