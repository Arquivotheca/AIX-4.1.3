#!/bin/ksh
# @(#)48	1.10  src/bos/usr/bin/que/aixlong.sh, cmdque, bos411, 9428A410j 7/14/94 15:55:55
# COMPONENT_NAME: (CMDQUE) filter for remote print status output
#
# FUNCTIONS: aixlong.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Shell script for translating AIX3.1 enq -qL printer queue output to ver 3.1
#	qstatus output. (for long-form output)
#

if MSGQUE=`/usr/bin/dspmsg qstat.cat -s 2 1 "Queue"`
then :
else MSGQUE="Queue"; fi
if MSGSUBM=`/usr/bin/dspmsg qstat.cat -s 2 2 "Submitted"`
then :
else MSGSUBM="Submitted"; fi
if MSGDWN2=`/usr/bin/dspmsg rem.cat 40 "HOST_DOWN"`
then :
else MSGDWN2="HOST_DOWN"; fi
AQUE=$1
RQUE=$2
/usr/bin/awk '
BEGIN	{
		#----Set the state machine to read in messages and header lines
		STATE = 0
		QNAME = "'"$AQUE"'"
		RNAME = "'"$RQUE"'"
		DASHES = "-------"
		LNUM = 0
		QUEUE = "'"$MSGQUE"'"
		SUBMIT = "'"$MSGSUBM"'"
		HOSTDOWN = "'"$MSGDWN2"'"
	}

END     {
                #----If an error occurred, display error message
                if(NR == 1 || NR == 2) {
                        #----Unable to communicate with host
                        if (SAVEONE == HOSTDOWN) {
                                printf(" %-7.7s %-5.5s %-9.9s\n",QNAME,RNAME,HOSTDOWN);
                        }
                        #----Some sort of error or message
                        else if(NR == 1) {
				#----Print one-line error message
                                printf(" %s: %s\n", QNAME, SAVEMSG)
                        }
			else {
				#----Print two-line error message
				printf(" %s: %s\n\t%s\n", QNAME, SAVEMSG, SAVEMSG2)
			}
                }
        }

	{
		#----If we are still in header section of remote status output...
		if(STATE == 0) {
			#----If we are at field descriptor section, dont print,
			#    and go on to status section, where the stuff we want is
			if(NR == 1) {
				#----Save entire line and field one of line
				SAVEMSG = $0
				SAVEONE = $1
			}
			else if(NR == 2) {
				#----Save second line of output
				SAVEMSG2 = $0
			}
			else if($1 == DASHES) {
				++STATE
			}
			#----Some sort of error or message
			else {
				printf(" %s: %s\n\t%s\n", QNAME, SAVEMSG, SAVEMSG2)
			}
		}
		#----Otherwise, we are in the status info lines for the remote queue
		else {
                        #----NOTE:  The socket attaches a '\0' at the end of the qstatus
                        #           output from the remote server.  AWK interprets this as
                        #           another line.  Thus, we check the number of fields to
                        #           insure a blank line is not printed. 
			if (LNUM == 0) {
				# -- now print formatted line
				# -- queue name
				printf(" %-7.7s ", QNAME)
				#-- dev name 
				printf("%-5.5s ", RNAME)
				#-- status
				printf("%-9.9s ", $3)
				if (NF < 4) printf("\n")
				else {
					#-- job
					printf("%3.3s ", $4)
					printf("%4.4s", "")
					#-- filename
					printf("%-14.14s ", $5)
					#-- from user
					printf("%-14.14s ", $6)
					#-- to user
					printf("%-14.14s\n", $7)
				}
			} 
			if( LNUM > 0 && NF > 0) {
				printf("%s\n",$0)
				}
			++LNUM
		}
	}
'
