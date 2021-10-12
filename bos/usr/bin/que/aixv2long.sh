#!/bin/ksh
# @(#)00	1.10  src/bos/usr/bin/que/aixv2long.sh, cmdque, bos411, 9428A410j 7/14/94 16:05:18
# COMPONENT_NAME: (CMDQUE) filter for remote print status output
#
# FUNCTIONS: aixv2long.sh
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
# ver 3.1 qstatus output (for long-form output)
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
	if ($1 == HOSTDOWN) {
		printf(" %-7.7s %-5.5s %-9.9s\n",QNAME,RNAME,HOSTDOWN);
	}
	else if ( $1 == DASHES ) {
		++STATE
		DASHES="-----"
		if ( STATE == 2) {
			STATUS = "'"$MSGSTQUE"'"
		}
	}
	else {
		if (STATE == 0 && $1 !~ /^dev$/ && NF > 0 ) {
			printf(" %s:%s\n", QNAME,$0 )
		}
		if ( STATE == 1 ) {
			#----Print the running queue line
			if ($2 == RNAME && $1 !~ /^queue$/  && NF > 0) {
				SNAME = $1
				if ($3 == "OFF" ) $3 = "DOWN"
				if ($3 == "RUNNING" ) $3 = STATUS
				#-- device and queue names
				printf(" %-7.7s", QNAME)
				printf(" %-5.5s", RNAME)
				#-- status
				printf(" %-9.9s", $3)
				if ( NF > 3 ) {
					#--- blank job number
					printf("%3.3s ", "")
					printf("%4s", "")
					#--- print filename 
					printf(" %-14.14s", $4)
					PP = $5
					PCT = $6
					LNUM = 1
				} else printf("\n")
			}
		} 
		if (STATE == 2 && $1 == SNAME) {
			#---- queued lines
			if (LNUM != 1 && NF > 0) {  #-- non-running lines
				#---skip queue and dev fields and all jobs are queued
				printf("%-14.14s %-9.9s", "",STATUS)
				#---now print variable stuff from line start with jobid
				#--- jobid blank field
				printf( "%3s ", "")
				#print blank space
				printf("%4s", "")
				#--- filename
				printf(" %-14.14s", $3)
				PP = "    "  # -- 4 blanks
				PCT = "  "   # -- 2 blanks
			}
			if (NF > 0) {
			#--- user/from
			printf(" %-14.14s", $2)
			#-- user/to
			printf(" %-14.14s\n", $NF)
			#print spaces to position second line
			printf("%33s", "")
			#-- time submitted
			printf("%-8.8s", $(NF-1))
			#-- print blank rank
			printf(" %4.4s", "")
			#-- priority of job
			printf(" %3.3s", $(NF-2))
			#-- print blank space for positioning
			printf("%6s", "")
			#-- no of blocks to print
			if (NF > 7)
				printf("%5.1d", $4)
			else printf("%5.5s","")
			#-- no. of copies
			printf("%3.1d",$(NF-3))
			#-- PP & PCT
			printf("%7s", "")
			printf(" %4.4s", PP)
			printf(" %2.2s", PCT)
			#-- go to third line
			printf("\n%31s%-48.48s\n", "", $3)
			LNUM = 0
			}
		}
	}
}
'
