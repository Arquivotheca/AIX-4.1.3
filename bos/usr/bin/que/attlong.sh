#!/bin/ksh
# @(#)45	1.7  src/bos/usr/bin/que/attlong.sh, cmdque, bos411, 9428A410j 7/14/94 16:06:06
# COMPONENT_NAME: (CMDQUE) filter for remote print status output
#
# FUNCTIONS: attlong.sh
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
# Shell script for translating ATT5.2 lpstat printer queue output to ver 3.1 
#	qstatus output. (for verbose output)
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
		#----Read in the arguments sent from qstatus, set up variables
		QNAME = "'"$AQUE"'"	# Input argument queue name
		RNAME = "'"$RQUE"'"	# Input argument device name
		UNKNOWN = "'"$MSGUNK"'"	# For Unknown fields
		STATUS = "'"$MSGRUN"'"	# Status used for active job on queue
		HOSTDOWN = "'"$MSGDWN2"'" # Unable to communicate with host for some reason
	}
	{
		#----Unable to communicate with host.
		if ($1 == HOSTDOWN) {
			printf(" %-7.7s %-5.5s %-9.9s\n",QNAME,RNAME,HOSTDOWN);
		}
                #----NOTE:  The socket attaches a '\0' at the end of the qstatus
                #           output from the remote server.  AWK interprets this as
                #           another line.  Thus, we check the number of fields to
                #           insure a blank line is not printed.
		else if (NF > 0) {
			#----Do some massaging to necessary fields
			BLOCKS = ($3 / 512) + 1
			#----If in the first line of the status section, we want
			#    to print the queue and device names in the proper fields
			if(STATE == 0) {
				printf(" %-7.7s",QNAME)		# Queue name
				printf(" %-5.5s",RNAME)		# Device name
			#----Otherwise, just leave space in the queue and device fields
			} else {
				printf("%-14.14s","")		# Empty que and dev fields
			} 
			#----Print the rest of the status fields
			printf(" %-9.9s",STATUS)		# Dev status
			printf("%4d",0)				# Job number
			printf("%4s","")			# blank space
			printf(" %-14.14s",UNKNOWN)		# Job name
			printf(" %-14.14s",$2)			# User requesting job
			printf(" %-14.14s",UNKNOWN)		# Destination user
			printf("\n")
			#----Second line begins here
			printf("%33s","")			# blank space
			printf("%-8.8s",$6)			# Time submitted
			printf("%4.1d",NR)			# Ranking of job
			printf(" %3.1d",15)			# Priority of job
			printf("%6s","")			# blank space
			printf("%6.1d",BLOCKS)			# No. of blocks to print
			printf("%3.1d",1)			# No. of copies
			printf("%7s","")			# blank space
			printf("%5.1d",0)			# Pages to print
			printf(" %2.1d",0)			# Percentage printed
			printf("\n")
			#----Third line starts here
			printf("%31s","")			# blank space
			printf("%-48.48s",UNKNOWN)		# Files to print
			printf("\n")
			#----Insure state machine says that we are now past first 
			#    status line, and adjust output fields accordingly
			++STATE
			STATUS = "'"$MSGQUE"'"
		}
	}
'
