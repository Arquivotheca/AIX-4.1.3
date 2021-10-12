#!/bin/ksh
# @(#)01	1.10  src/bos/usr/bin/que/aixv2short.sh, cmdque, bos411, 9428A410j 7/14/94 16:05:32
# COMPONENT_NAME: (CMDQUE) filter for remote print status output
#
# FUNCTIONS: aixv2short.sh
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
# Shell script for translating AIX2.2.1 print -q printer queue output to
# ver 3.1 qstatus output (for short-form output)
#
if MSGUNK=`/usr/bin/dspmsg qstat.cat -s 2 3 "Unknown"`
then :
else MSGUNK="Unknown"; fi
if MSGSTRUN=`/usr/bin/dspmsg qstat.cat -s 2 4 "RUNNING"`
then :
else MSGSTRUN="RUNNING"; fi
if MSGSTQUE=`/usr/bin/dspmsg qstat.cat -s 2 5 "QUEUED"`
then :
else MSGSTQUE="QUEUED"; fi
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
	STATUS = "'"$MSGSTRUN"'"
	DASHES = "---"
	UNKNOWN = "'"$MSGUNK"'"
	HOSTDOWN = "'"$MSGDWN2"'"
	}
	{
	if ( $1 == HOSTDOWN ) {
		printf("%-7.7s %-5.5s %-9.9s\n",QNAME,RNAME,HOSTDOWN);
	}
	else if ( $1 == DASHES ) {
		++STATE
		DASHES="-----"
		if ( STATE == 2) {
			STATUS = "'"$MSGSTQUE"'"
		}
	}
	else {
		if (STATE == 0 && $1 !~ /^dev$/ ) { # probable err msg
			if (NF > 0) printf("%s: %s\n",  QNAME,$0)
		}
		if ( STATE == 1 && $2 == RNAME ) {
			#----Print the running queue line
			SNAME = $1
			if ($1 !~ /^queue$/  && NF > 0) {
				if ($3 == "OFF" ) $3 = "DOWN"
				if ($3 == "RUNNING" ) $3 = STATUS
				if ($3 == "UNKNOWN") $3 = UNKNOWN
				#--print device and queue names
				printf("%-7.7s ", QNAME)
				printf("%-5.5s ", RNAME)
				#--print status
				printf("%-9.9s ", $3)
				if ( NF > 3 ) {
					#---print blank job number
					printf("%3.3s ", "")
					#--- filename 
					printf("%-19.19s", $4)
					# save pp & %done to do after getting
					# user from top line of queued stuff
					PP = $5 + 0
					PCT = $6 + 0
					LNUM = 1
				} else printf("\n")
			}
		} 
		if (STATE == 2 && $1 == SNAME) {
			if (LNUM == 1) {
			#finish printing queue status line
				printf(" %-10.10s", $2)
				printf("%5.1d", PP)
				printf(" %2.1d", PCT)
				#print blks, no of copies, and blank rank
				#blks may be blank if unfriendly backend
				if (NF < 8) printf(" %5.5s %2.1d\n","",  $4)
				else printf(" %5.1d %2.1d\n", $4, $5)
				++LNUM
			} else if (NF > 0) {
			#----print queued lines
			#---skip queue and dev fields and all jobs are queued
			printf("%7.7s %5.5s %-9.9s ","", "", STATUS)
			#---now print variable stuff from line start with jobid
			#---print jobid blank field
			printf("%3.3s ", "")
			#---print filename
			printf("%-19.19s ", $3)
			#---print user
			printf("%-10.10s ", $2)
			#---print blank PP and % done fields
			printf("%4.4s %2.2s ", "", "")
			#---print blks, number of copies and blank rank
			#blks may be blank if unfriendly backend
			if (NF < 8) printf("%5.5s %2.1d\n", "", $4 + 0)
			else printf("%5s %2s\n", $4, $5)
			}
		}
	}
}
'
